#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

//#define MAGIC "qoif"
#define MAGIC 0x716f6966
//#define END_MARKER [0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1]
#define END_MARKER_1 0
#define END_MARKER_2 1

#define IMAGE_WIDTH 366
#define IMAGE_HEIGHT 206
#define CHANNELS 4

#define HEADER_SIZE_BYTES (4 + 4 + 4 + 1 + 1)
#define MAX_CHUNKS_SIZE_BYTES (IMAGE_HEIGHT * IMAGE_WIDTH * (CHANNELS + 1))
#define FOOTER_SIZE_BYTES 8
#define MAX_QOI_SIZE (HEADER_SIZE_BYTES + MAX_CHUNKS_SIZE_BYTES + FOOTER_SIZE_BYTES)

// full byte check
#define QOI_OP_RGB   0xfe
#define QOI_OP_RGBA  0xff

// check two leading bits of byte
#define QOI_OP_INDEX 0x00
#define QOI_OP_DIFF  0x40
#define QOI_OP_LUMA  0x80
#define QOI_OP_RUN   0xc0

void write32(char *buf, uint32_t val){
    char str[4];
    str[0] = (val >> 24) & 0xff;
    str[1] = (val >> 16) & 0xff;
    str[2] = (val >> 8) & 0xff;
    str[3] = (val >> 0) & 0xff;
    memcpy(buf, str, 4);
}

struct rgba {
    int8_t r;
    int8_t g;
    int8_t b;
    int8_t a;
};

int colorsEqual (struct rgba diff){
    return diff.r == 0 && diff.g == 0 && diff.b == 0 && diff.a == 0;
}

int seenHash(struct rgba c){
    return (c.r * 3 + c.g * 5 + c.b * 7 + c.a * 11) % 64;
}

struct rgba colorsDiff (struct rgba c1, struct rgba c2) {
    struct rgba diff = { 0, 0, 0, 0 };
    diff.r = c1.r - c2.r;
    diff.g = c1.g - c2.g;
    diff.b = c1.b - c2.b;
    diff.a = c1.a - c2.a;
    printf("c1s: r = %i, g = %i, b = %i, a = %i\n", c1.r, c1.g, c1.b, c1.a);
    printf("c2s: r = %i, g = %i, b = %i, a = %i\n", c2.r, c2.g, c2.b, c2.a);
    return diff;
}

void addSeen(struct rgba color, struct rgba *colors, int pos){
    colors[pos].r = color.r;
    colors[pos].b = color.b;
    colors[pos].g = color.g;
    colors[pos].a = color.a;
}

int colorIndex(struct rgba color, struct rgba *colors){
    int pos = (64 + seenHash(color)) % 64;
    //printf("in colorIndex pos is %i\n", pos);
    struct rgba testcolor = colorsDiff(color, colors[pos]);
    if (colorsEqual(testcolor)){
        return pos;
    } else {
        addSeen(color, colors, pos);
        return -1;
    }
}

int isDiff(struct rgba diff){
    //printf("in isDiff: diff.r = %i, diff.g = %i, diff.b = %i, diff.a = %i\n", diff.r, diff.g, diff.b, diff.a);
    char rtest, gtest, btest, atest;
    rtest = diff.r >= -2 && diff.r <= 1;
    gtest = diff.g >= -2 && diff.g <= 1;
    btest = diff.b >= -2 && diff.b <= 1;
    atest = diff.a == 0;
    //printf("isDiff result is %i\n", rtest && gtest && btest && atest);
    return rtest && gtest && btest && atest;
}

void writeDiff(char *buf, struct rgba diff, long int index){
    int8_t result = QOI_OP_DIFF;
    //printf("diff.r = %i, diff.g = %i, diff.b = %i\n", diff.r, diff.g, diff.b);
    result = result | ((2 + diff.r) << 4);
    result = result | ((2 + diff.g) << 2);
    result = result | ((2 + diff.b) << 0);
    //printf("writeDiff result is %i\n", result);
    buf[index] = result;
    //*index = *index + 1;
}

int isLuma(struct rgba diff){
    char gtest, rgdiff, rtest, bgdiff, btest, atest;
    gtest = diff.g >= -32 && diff.g <= 31;
    rgdiff = diff.r - diff.g;
    rtest = rgdiff >= -8 && rgdiff <= 7;
    bgdiff = diff.b - diff.g;
    btest = bgdiff >= -8 && bgdiff <= 7;
    atest = diff.a == 0;
    //printf("Isluma check is %i\n", rtest && gtest && btest && atest);
    return rtest && gtest && btest && atest;
}

void writeLuma(char *buf, struct rgba diff, long int index){
    int8_t result = QOI_OP_LUMA;
    result = result | (diff.g + 32);
    buf[index] = result;
    //*index = *index + 1;
    result = (diff.r - diff.g + 8) << 4;
    result = result | (diff.b - diff.g + 8);
    buf[index + 1] = result;
    //*index = *index + 1;
}

void writeRGB(char *buf, struct rgba c, long int index){
    buf[index] = QOI_OP_RGB;
    buf[index + 1] = c.r;
    buf[index + 2] = c.g;
    buf[index + 3] = c.b;
    //*index = *index + 4;
}

void writeRGBA(char *buf, struct rgba c, long int index){
    buf[index] = QOI_OP_RGBA;
    buf[index + 1] = c.r;
    buf[index + 2] = c.g;
    buf[index + 3] = c.b;
    buf[index + 4] = c.a;
    //*index = *index + 5;
}

int main(){
    struct rgba prev = { 0, 0, 0, 255 };
    struct rgba current = { 0, 0, 0, 255 };
    struct rgba diff = { 0, 0, 0, 0 };
    struct rgba seen[64];
    char matchIndex = 0;
    uint32_t colors = 0;
    char runs = 0;

    char *qoibuffer = NULL;

    FILE *readFile  = NULL;
    FILE *writeFile = NULL;

    long readIndex = 0;
    long writeIndex = 0;
    long lastPixel = -1;

    memset(seen, 0, 64 * sizeof(struct rgba));

    qoibuffer = malloc(MAX_QOI_SIZE);
    write32(qoibuffer, MAGIC);
    write32(&(qoibuffer[4]), IMAGE_WIDTH);
    write32(&(qoibuffer[8]), IMAGE_HEIGHT);
    qoibuffer[12] = (char) CHANNELS;
    qoibuffer[13] = (char) 1;
    writeIndex = 14;

    readFile = fopen("assets/test.rgba", "rb");
    if(!readFile){
        fprintf(stderr, "failed to open file to read from\n");
        return 1;
    }

    lastPixel = IMAGE_HEIGHT * IMAGE_WIDTH;
    for(readIndex = 0; readIndex < lastPixel; readIndex++){
        printf("readindex: %li, writeindex: %li, want to write byte: %li\n", readIndex, writeIndex, writeIndex + 1);

        // read colors from rgba file
        if(fread(&colors, 1, CHANNELS, readFile) != CHANNELS){
            fprintf(stderr, "File is too short, expected a RGB/RGBA byte but read less than %i bytes\n", CHANNELS);
        }
        if (CHANNELS == 4){
            current.r = colors & 0xff;
            current.g = (colors >> (8 * (CHANNELS - 3))) & 0xff;
            current.b = (colors >> (8 * (CHANNELS - 2))) & 0xff;
            current.a = (colors >> (8 * (CHANNELS - 1))) & 0xff;
        } else {
            current.r = colors & 0xff;
            current.g = (colors >> (8 * (CHANNELS - 2))) & 0xff;
            current.b = (colors >> (8 * (CHANNELS - 1))) & 0xff;
        }

        // Checks for runs
        diff = colorsDiff(current, prev);
        if (colorsEqual(diff)) {
            runs++;
            //printf("runs is %i\n", runs);
            if (runs == 62 || (readIndex + 1) == lastPixel){
                qoibuffer[writeIndex] = QOI_OP_RUN | (runs - 1);
                writeIndex = writeIndex + 1;
                runs = 0;
            }
            goto endloop;
        } else if (runs > 0) {
            printf("run is %i, runbreaker is {%i, %i, %i, %i}, run was {%i, %i, %i, %i}\n", 
                    runs, current.r, current.g, current.b, current.a, prev.r, prev.g, prev.b, prev.a);
            qoibuffer[writeIndex] = QOI_OP_RUN | (runs - 1);
            writeIndex = writeIndex + 1;
            runs = 0;
        }

        // Check for indexes
        matchIndex = colorIndex(current, seen);
        //printf("I am at write position %li and matchIndex is %i\n", writeIndex, matchIndex);
        if (matchIndex != -1) {
            qoibuffer[writeIndex] = QOI_OP_INDEX | matchIndex;
            writeIndex = writeIndex + 1;
            goto endloop;
        }

        // Check for small difference
        if (isDiff(diff)) {
            writeDiff(qoibuffer, diff, writeIndex);
            writeIndex = writeIndex + 1;
            goto endloop;
        }

        // Check for luma
        if (isLuma(diff)) {
            writeLuma(qoibuffer, diff, writeIndex);    
            writeIndex = writeIndex + 2;
            goto endloop;
        }

        // no compression matches
        if (CHANNELS == 3 || current.a == prev.a){
            writeRGB(qoibuffer, current, writeIndex);
            writeIndex = writeIndex + 4;
        } else {
            writeRGBA(qoibuffer, current, writeIndex);
            writeIndex = writeIndex + 5;
        }

        // setup everything for next iteration
endloop:
        prev.r = current.r;
        prev.g = current.g;
        prev.b = current.b;
        prev.a = current.a;
    }

    fclose(readFile);

    write32(&(qoibuffer[writeIndex]), END_MARKER_1);
    writeIndex = writeIndex + 4;
    write32(&(qoibuffer[writeIndex]), END_MARKER_2);
    writeIndex = writeIndex + 4;

    writeFile = fopen("out/mine.qoi", "wb");
    fwrite(qoibuffer, writeIndex, 1, writeFile);
    fclose(writeFile);

    free(qoibuffer);

    return 0;
}
