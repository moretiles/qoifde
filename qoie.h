#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"

#define MAGIC "qoif"
#define END_MARKER "\x00\x00\x00\x00\x00\x00\x00\x01"

#ifndef MAX_BLOCK_SIZE
#define MAX_BLOCK_SIZE (3 * 1024 * 1024)
#endif

// full byte check
#define QOI_OP_RGB   (0xfe)
#define QOI_OP_RGBA  (0xff)

// check two leading bits of byte
#define QOI_OP_INDEX (0x00)
#define QOI_OP_DIFF  (0x40)
#define QOI_OP_LUMA  (0x80)
#define QOI_OP_RUN   (0xc0)

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
void writeDiff(struct queue *store, struct rgba diff){
    char result = QOI_OP_DIFF;
    result = result | ((2 + diff.r) << 4);
    result = result | ((2 + diff.g) << 2);
    result = result | ((2 + diff.b) << 0);
    enqueue(store, &result, 1);
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
void writeLuma(struct queue *store, struct rgba diff){
    char result[2] = {0, 0};
    result[0] = QOI_OP_LUMA;
    result[0] = result[0] | (diff.g + 32);
    result[1] = (diff.r - diff.g + 8) << 4;
    result[1] = result[1] | (diff.b - diff.g + 8);
    enqueue(store, result, 2);
}

// Write RGB chunk
void writeRGB(struct queue *store, struct rgba c){
    char result[4];
    result[0] = QOI_OP_RGB;
    result[1] = c.r;
    result[2] = c.g;
    result[3] = c.b;
    enqueue(store, result, 4);
}

// Write RGBA chunk
void writeRGBA(struct queue *store, struct rgba c){
    char result[5];
    result[0] = QOI_OP_RGBA;
    result[1] = c.r;
    result[2] = c.g;
    result[3] = c.b;
    result[4] = c.a;
    enqueue(store, result, 5);
}

/*
 * Read rgba pixels from infile and output an encoded qoi image as outfile
 * The encoded image will have headers of width, height, channels, colorspace
 */
int encodeQOI(char *infile, char *outfile, int width, int height, char channels, char colorspace){
    int err = 0;
    int cont = 1;
    int read = 0;
    int write = 0;

    struct rgba seen[64];
    struct rgba prev = { 0, 0, 0, 255 };
    struct rgba current = { 0, 0, 0, 255 };
    struct rgba diff = { 0, 0, 0, 0 };
    char colors[4] = { 0 };
    char matchIndex = 0;
    char runs = 0;

    FILE *readFile = NULL;
    FILE *writeFile = NULL;

    char *rgbaBuffer = NULL;
    char *qoiBuffer = NULL;

    struct queue *readQueue = &(struct queue) { NULL, 0, 0, MAX_BLOCK_SIZE };
    struct queue *writeQueue = &(struct queue) { NULL, 0, 0, MAX_BLOCK_SIZE };

    memset(seen, 0, 64 * sizeof(struct rgba));

    rgbaBuffer = malloc(MAX_BLOCK_SIZE);
    readQueue->chars = rgbaBuffer;
    qoiBuffer = malloc(5 * MAX_BLOCK_SIZE / 4);
    writeQueue->chars = qoiBuffer;
    if(rgbaBuffer == NULL || qoiBuffer == NULL){
        return 2;
    }

    readFile = fopen(infile, "rb");
    writeFile = fopen(outfile, "wb");
    if(readFile == NULL || writeFile == NULL){
        return 1;
    }

    // Magic bytes
    enqueue(writeQueue, MAGIC, 4);

    // Need to store a big or little endian int as a big endian int
    enqueuec(writeQueue, width >> 24);
    enqueuec(writeQueue, width >> 16);
    enqueuec(writeQueue, width >> 8);
    enqueuec(writeQueue, width >> 0);
    enqueuec(writeQueue, height >> 24);
    enqueuec(writeQueue, height >> 16);
    enqueuec(writeQueue, height >> 8);
    enqueuec(writeQueue, height >> 0);

    // Store channels and colorspace as single bytes
    enqueuec(writeQueue, channels);
    enqueuec(writeQueue, colorspace);

    if(fdequeue(writeFile, writeQueue, 14) <= 0){
        cont = 0;
        err = 1;
    }
    if(fenqueue(readFile, readQueue, MAX_BLOCK_SIZE) <= 0 || readQueue->pos % channels != 0){
        cont = 0;
        err = 1;
    }

    while(cont){
        dequeue(readQueue, colors, channels);
        current.r = colors[0];
        current.g = colors[1];
        current.b = colors[2];
        if (channels == 4){
            current.a = colors[3];
        }

        /*
         * Checks for runs
         * All the logic involving runs needs to be kept in main
         */
        diff = colorsDiff(current, prev);
        if (colorsEqual(diff)) {
            runs = runs + 1;
            if (runs == 62){
                enqueuec(writeQueue, QOI_OP_RUN | (runs - 1));
                runs = 0;
            }
            goto endloop;
        } else if (runs > 0) {
            enqueuec(writeQueue, QOI_OP_RUN | (runs - 1));
            runs = 0;
            // write runs and then process current color
        }

        // Check for indexes
        matchIndex = colorIndex(current, seen);
        if (matchIndex != -1) {
            enqueuec(writeQueue, QOI_OP_INDEX | matchIndex);
            goto endloop;
        }

        // Check for small difference
        if (isDiff(diff)) {
            writeDiff(writeQueue, diff);
            goto endloop;
        }

        // Check for luma
        if (isLuma(diff)) {
            writeLuma(writeQueue, diff);
            goto endloop;
        }

        // no compression matches
        if (channels == 3 || current.a == prev.a){
            writeRGB(writeQueue, current);
        } else {
            writeRGBA(writeQueue, current);
        }

        // setup everything for next iteration
endloop:
        prev.r = current.r;
        prev.g = current.g;
        prev.b = current.b;
        prev.a = current.a;

        if(readQueue->pos == readQueue->base){
            readQueue->pos = 0;
            readQueue->base = 0;
            write = fdequeue(writeFile, writeQueue, writeQueue->pos);
            if(write <= 0){
                cont = 0;
            }
            if(write < 0){
                err = 1;
            }
            read = fenqueue(readFile, readQueue, MAX_BLOCK_SIZE);
            if(read <= 0 || read % channels != 0){
                cont = 0;
            }
            if(read < 0){
                err = 1;
            }
        }
    }
    if(err == 0){
        if(runs != 0){
            enqueuec(writeQueue, QOI_OP_RUN | (runs - 1));
            runs = 0;
            fdequeue(writeFile, writeQueue, 1);
        }
        enqueue(writeQueue, END_MARKER, 8);
        fdequeue(writeFile, writeQueue, 8);
    }

    free(qoiBuffer);
    qoiBuffer = NULL;
    free(rgbaBuffer);
    rgbaBuffer = NULL;

    return err;
}
