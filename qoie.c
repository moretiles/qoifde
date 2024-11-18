#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "stack.h"

#define MAGIC "qoif"
#define END_MARKER "\x00\x00\x00\x00\x00\x00\x00\x01"

#define HEADER_SIZE_BYTES (4 + 4 + 4 + 1 + 1)
#define MAX_CHUNKS_SIZE_BYTES (IMAGE_HEIGHT * IMAGE_WIDTH * (CHANNELS + 1))
#define FOOTER_SIZE_BYTES 8
#define MAX_QOI_SIZE (HEADER_SIZE_BYTES + MAX_CHUNKS_SIZE_BYTES + FOOTER_SIZE_BYTES)

// full byte check
#define QOI_OP_RGB   (0xfe)
#define QOI_OP_RGBA  (0xff)

// check two leading bits of byte
#define QOI_OP_INDEX (0x00)
#define QOI_OP_DIFF  (0x40)
#define QOI_OP_LUMA  (0x80)
#define QOI_OP_RUN   (0xc0)

int IMAGE_WIDTH = 366;
int IMAGE_HEIGHT = 206;
char CHANNELS = 3;
char COLORSPACE = 1;

struct rgba {
    int8_t r;
    int8_t g;
    int8_t b;
    int8_t a;
};

/*
 * Checks if a struct rgba is { 0, 0, 0, 0 }
 * Only called on the difference
 */
int colorsEqual (struct rgba diff){
    return diff.r == 0 && diff.g == 0 && diff.b == 0 && diff.a == 0;
}

// Hash function that indexes into the cached colors
int seenHash(struct rgba c){
    return (c.r * 3 + c.g * 5 + c.b * 7 + c.a * 11) % 64;
}

// Calculate difference between two rgba structs
struct rgba colorsDiff (struct rgba c1, struct rgba c2) {
    struct rgba diff = { 0, 0, 0, 0 };
    diff.r = c1.r - c2.r;
    diff.g = c1.g - c2.g;
    diff.b = c1.b - c2.b;
    diff.a = c1.a - c2.a;
    return diff;
}

// Copy a color into colors at pos
void addSeen(struct rgba color, struct rgba *colors, int pos){
    colors[pos].r = color.r;
    colors[pos].b = color.b;
    colors[pos].g = color.g;
    colors[pos].a = color.a;
}

// Check if color has been cached in colors
int colorIndex(struct rgba color, struct rgba *colors){
    int pos = (64 + seenHash(color)) % 64;
    struct rgba testcolor = colorsDiff(color, colors[pos]);
    if (colorsEqual(testcolor)){
        return pos;
    } else {
        addSeen(color, colors, pos);
        return -1;
    }
}

// Check if a small difference exists
int isDiff(struct rgba diff){
    char rtest, gtest, btest, atest;
    rtest = diff.r >= -2 && diff.r <= 1;
    gtest = diff.g >= -2 && diff.g <= 1;
    btest = diff.b >= -2 && diff.b <= 1;
    atest = diff.a == 0;
    return rtest && gtest && btest && atest;
}

// Write small difference chunk
void writeDiff(struct stack *store, struct rgba diff){
    char result = QOI_OP_DIFF;
    result = result | ((2 + diff.r) << 4);
    result = result | ((2 + diff.g) << 2);
    result = result | ((2 + diff.b) << 0);
    push(store, &result, 1);
}

// Check if a large (luma) difference exists
int isLuma(struct rgba diff){
    char gtest, rgdiff, rtest, bgdiff, btest, atest;
    gtest = diff.g >= -32 && diff.g <= 31;
    rgdiff = diff.r - diff.g;
    rtest = rgdiff >= -8 && rgdiff <= 7;
    bgdiff = diff.b - diff.g;
    btest = bgdiff >= -8 && bgdiff <= 7;
    atest = diff.a == 0;
    return rtest && gtest && btest && atest;
}

// Write large (luma) difference chunk
void writeLuma(struct stack *store, struct rgba diff){
    char result[2] = {0, 0};
    result[0] = QOI_OP_LUMA;
    result[0] = result[0] | (diff.g + 32);
    result[1] = (diff.r - diff.g + 8) << 4;
    result[1] = result[1] | (diff.b - diff.g + 8);
    push(store, result, 2);
}

// Write RGB chunk
void writeRGB(struct stack *store, struct rgba c){
    char result[4];
    result[0] = QOI_OP_RGB;
    result[1] = c.r;
    result[2] = c.g;
    result[3] = c.b;
    push(store, result, 4);
}

// Write RGBA chunk
void writeRGBA(struct stack *store, struct rgba c){
    char result[5];
    result[0] = QOI_OP_RGBA;
    result[1] = c.r;
    result[2] = c.g;
    result[3] = c.b;
    result[4] = c.a;
    push(store, result, 5);
}

int main(){
    struct rgba seen[64];
    struct rgba prev = { 0, 0, 0, 255 };
    struct rgba current = { 0, 0, 0, 255 };
    struct rgba diff = { 0, 0, 0, 0 };
    char colors[4] = { 0 };
    char matchIndex = 0;
    char runs = 0;

    char *rgbabuffer = NULL;
    char *qoibuffer = NULL;

    struct stack *readStack = &(struct stack) { NULL, 0, IMAGE_WIDTH * IMAGE_HEIGHT * CHANNELS };
    struct stack *writeStack = &(struct stack) { NULL, 0, MAX_QOI_SIZE };

    long readIndex = 0;
    long lastPixel = -1;

    memset(seen, 0, 64 * sizeof(struct rgba));

    qoibuffer = malloc(MAX_QOI_SIZE);
    writeStack->chars = qoibuffer;
    rgbabuffer = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * CHANNELS);
    readStack->chars = rgbabuffer;

    push(writeStack, MAGIC, 4);
    pushl(writeStack, (char *) &IMAGE_WIDTH, 4);
    pushl(writeStack, (char *) &IMAGE_HEIGHT, 4);
    pushc(writeStack, CHANNELS);
    pushc(writeStack, COLORSPACE);

    pushFromFile("assets/test.rgb", readStack);

    lastPixel = IMAGE_HEIGHT * IMAGE_WIDTH;
    for(readIndex = 0; readIndex < lastPixel; readIndex++){
        popPseudoQueue(readStack, colors, CHANNELS);
        current.r = colors[0];
        current.g = colors[1];
        current.b = colors[2];
        if (CHANNELS == 4){
            current.a = colors[3];
        }

        /*
         * Checks for runs
         * All the logic involving runs needs to be kept in main
         */
        diff = colorsDiff(current, prev);
        if (colorsEqual(diff)) {
            runs++;
            if (runs == 62 || (readIndex + 1) == lastPixel){
                pushc(writeStack, QOI_OP_RUN | (runs - 1));
                runs = 0;
            }
            goto endloop;
        } else if (runs > 0) {
            pushc(writeStack, QOI_OP_RUN | (runs - 1));
            runs = 0;
            // write runs and then process current color
        }

        // Check for indexes
        matchIndex = colorIndex(current, seen);
        if (matchIndex != -1) {
            pushc(writeStack, QOI_OP_INDEX | matchIndex);
            goto endloop;
        }

        // Check for small difference
        if (isDiff(diff)) {
            writeDiff(writeStack, diff);
            goto endloop;
        }

        // Check for luma
        if (isLuma(diff)) {
            writeLuma(writeStack, diff);
            goto endloop;
        }

        // no compression matches
        if (CHANNELS == 3 || current.a == prev.a){
            writeRGB(writeStack, current);
        } else {
            writeRGBA(writeStack, current);
        }

        // setup everything for next iteration
endloop:
        prev.r = current.r;
        prev.g = current.g;
        prev.b = current.b;
        prev.a = current.a;
    }

    push(writeStack, END_MARKER, 8);

    popToFile("out/mine.qoi", writeStack);

    free(qoibuffer);
    qoibuffer = NULL;
    free(rgbabuffer);
    rgbabuffer = NULL;

    return 0;
}
