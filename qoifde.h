#include <stdlib.h>

#include "queue.h"
#include "rgba.h"

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
        err = 2;
        goto encodeQOI_error;
    }

    readFile = fopen(infile, "rb");
    writeFile = fopen(outfile, "wb");
    if(readFile == NULL || writeFile == NULL){
        err = 1;
        goto encodeQOI_error;
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
        } else {
            if (runs > 0) {
                enqueuec(writeQueue, QOI_OP_RUN | (runs - 1));
                runs = 0;
                // write runs and then process current color
            }

            // Check for indexes
            matchIndex = colorIndex(current, seen);
            if (matchIndex != -1) {
                enqueuec(writeQueue, QOI_OP_INDEX | matchIndex);
            }

            // Check for small difference
            else if (isDiff(diff)) {
                writeDiff(writeQueue, diff);
            }

            // Check for luma
            else if (isLuma(diff)) {
                writeLuma(writeQueue, diff);
            }

            // no compression matches
            else if (channels == 3 || current.a == prev.a){
                writeRGB(writeQueue, current);
            } else {
                writeRGBA(writeQueue, current);
            }
        }

        // setup everything for next iteration
        prev.r = current.r;
        prev.g = current.g;
        prev.b = current.b;
        prev.a = current.a;

        if(readQueue->base == readQueue->pos){
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

encodeQOI_error:
    free(qoiBuffer);
    qoiBuffer = NULL;
    free(rgbaBuffer);
    rgbaBuffer = NULL;

    if(readFile != NULL){
        fclose(readFile);
        readFile = NULL;
    }

    if(writeFile != NULL){
        fclose(writeFile);
        writeFile = NULL;
    }

    return err;
}

// Store color into seen using specification hashing formula
void cacheColor(struct rgba *seen, struct rgba color){
    int pos = (color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64;
    pos = (pos + 64) % 64;
    if(color.r != seen[pos].r || color.g != seen[pos].g || color.b != seen[pos].b || color.a != seen[pos].a){
        seen[pos].r = color.r;
        seen[pos].g = color.g;
        seen[pos].b = color.b;
        seen[pos].a = color.a;
    }
}

// Place the 4 bytes that make up color into store
void enqueueRgba(struct queue *store, struct rgba color, char channels){
    enqueuec(store, color.r);
    enqueuec(store, color.g);
    enqueuec(store, color.b);
    if(channels == 4){
        enqueuec(store, color.a);
    }
}

/*
 * Read rgba pixels from infile and output an encoded qoi image as outfile
 * The encoded image will have headers of width, height, channels, colorspace
 */
int decodeQOI(char *infile, char *outfile){
    int err = 0;
    int cont = 1;
    int read = 0;
    int write = 0;

    uint32_t width = 0;
    uint32_t height = 0;
    char channels = 0;
    char colorspace = 0;

    struct rgba seen[64];
    struct rgba prev = { 0, 0, 0, 255 };
    struct rgba diff = { 0, 0, 0, 0 };

    uint8_t currentOperation = 0;
    uint8_t lastOperation = 0xff;
    char lumaSecond = 0;
    char needCaching = 1;

    FILE *readFile = NULL;
    FILE *writeFile = NULL;

    char *qoiBuffer = NULL;
    char *rgbaBuffer = NULL;

    struct queue *readQueue = &(struct queue) { NULL, 0, 0, MAX_BLOCK_SIZE };
    struct queue *writeQueue = &(struct queue) { NULL, 0, 0, MAX_BLOCK_SIZE };

    memset(seen, 0, 64 * sizeof(struct rgba));

    qoiBuffer = malloc(MAX_BLOCK_SIZE);
    readQueue->chars = qoiBuffer;
    rgbaBuffer = malloc(MAX_BLOCK_SIZE);
    writeQueue->chars = rgbaBuffer;
    if(rgbaBuffer == NULL || qoiBuffer == NULL){
        err = 2;
        goto decodeQOI_error;
    }

    readFile = fopen(infile, "rb");
    writeFile = fopen(outfile, "wb");
    if(readFile == NULL || writeFile == NULL){
        err = 1;
        goto decodeQOI_error;
    }

    if(fenqueue(readFile, readQueue, 14) != 14){
        cont = 0;
        err = 1;
    } else {
        width   = (uint8_t) readQueue->chars[4 + 0] << 24;
        width  += (uint8_t) readQueue->chars[4 + 1] << 16;
        width  += (uint8_t) readQueue->chars[4 + 2] << 8;
        width  += (uint8_t) readQueue->chars[4 + 3];
        height  = (uint8_t) readQueue->chars[8 + 0] << 24;
        height += (uint8_t) readQueue->chars[8 + 1] << 16;
        height += (uint8_t) readQueue->chars[8 + 2] << 8;
        height += (uint8_t) readQueue->chars[8 + 3];
        channels = readQueue->chars[12];
        colorspace = readQueue->chars[13];
        if(strncmp(readQueue->chars, MAGIC, 4) != 0 || ((int32_t) width) < 0 || ((int32_t) height) < 0 || \
                channels < 3 || channels > 4 || colorspace < 0 || colorspace > 1){
            cont = 0;
            err = 1;
        }
        readQueue->pos = 0;
        readQueue->base = 0;
    }

    read = fenqueue(readFile, readQueue, MAX_BLOCK_SIZE);
    if(read <= 0){
        cont = 0;
    }
    if(read < 0){
        err = 1;
    }

    while(cont){
        dequeuec(readQueue, (char *) &currentOperation);

        /*
           switch ( currentOperation){
           case QOI_OP_RGBA:
           exchange(readQueue, writeQueue, 4);
           needCaching = 1;
           goto endconv;
           break;
           case QOI_OP_RGB:
           exchange(readQueue, writeQueue, 3);
           if(channels == 4){
           enqueuec(writeQueue, 0xff);
           }
           needCaching = 1;
           goto endconv;
           break;
           }
           */
        switch (currentOperation & 0xc0){
            case QOI_OP_INDEX:
                if(currentOperation == 0 && currentOperation == lastOperation){
                    writeQueue->pos = writeQueue->pos - channels;
                    cont = 0;
                }else{
                    enqueueRgba(writeQueue, seen[0xff & currentOperation], channels);
                }
                needCaching = 0;
                break;
            case QOI_OP_DIFF:
                diff.r  = prev.r - 2;
                diff.r += (currentOperation & 0x30) >> 4;
                diff.g  = prev.g - 2;
                diff.g += (currentOperation & 0xc) >> 2;
                diff.b  = prev.b - 2;
                diff.b += (currentOperation & 0x3) >> 0;
                diff.a = prev.a;
                enqueueRgba(writeQueue, diff, channels);
                needCaching = 1;
                break;
            case QOI_OP_LUMA:
                dequeuec(readQueue, &lumaSecond);
                diff.g  = (currentOperation & 0x3f) - 32;
                diff.r  = ((lumaSecond & 0xf0) >> 4) - 8 + diff.g;
                diff.b  = (lumaSecond & 0x0f) - 8 + diff.g;
                diff.g += prev.g;
                diff.b += prev.b;
                diff.r += prev.r;
                diff.a = prev.a;
                enqueueRgba(writeQueue, diff, channels);
                needCaching = 1;
                break;
            case QOI_OP_RUN:
                if(currentOperation == QOI_OP_RGB){
                    exchange(readQueue, writeQueue, 3);
                    if(channels == 4){
                        enqueuec(writeQueue, 0xff);
                    } 
                    needCaching = 1;
                    break;
                } else if (currentOperation == QOI_OP_RGBA){
                    exchange(readQueue, writeQueue, 4);
                    needCaching = 1;
                    break;
                } else{
                    while(((uint8_t) currentOperation) >= 0xc0){
                        enqueueRgba(writeQueue, prev, channels);
                        currentOperation -= 1;
                    }
                    needCaching = 0;
                    break;
                }
        }

        lastOperation = currentOperation;

        if (channels == 3){
            prev.r = writeQueue->chars[writeQueue->pos - 3];
            prev.g = writeQueue->chars[writeQueue->pos - 2];
            prev.b = writeQueue->chars[writeQueue->pos - 1];
        } else {
            prev.r = writeQueue->chars[writeQueue->pos - 4];
            prev.g = writeQueue->chars[writeQueue->pos - 3];
            prev.b = writeQueue->chars[writeQueue->pos - 2];
            prev.a = writeQueue->chars[writeQueue->pos - 1];
        }

        if (needCaching == 1){
            cacheColor(seen, prev);
            needCaching = 0;
        }

        if(lastOperation != 0 && readQueue->base + 8 >= readQueue->pos){
            if(readQueue->base != readQueue->pos){
                foldDown(readQueue);
            } else {
                readQueue->pos = 0;
                readQueue->base = 0;
            }
            read = fenqueue(readFile, readQueue, MAX_BLOCK_SIZE - readQueue->pos);
            if(read <= 0){
                cont = 0;
            }
            if(read < 0){
                err = 1;
            }
        }

        if(writeQueue->pos >= MAX_BLOCK_SIZE - 64){
            write = fdequeue(writeFile, writeQueue, writeQueue->pos);
            if(write <= 0){
                cont = 0;
            }
            if(write < 0){
                err = 1;
            }
        }
    }

    if(err == 0 && writeQueue->pos > 0){
        //write = fdequeue(writeFile, writeQueue, writeQueue->pos - (writeQueue->pos % channels));
        write = fdequeue(writeFile, writeQueue, writeQueue->pos);
        if(write <= 0){
            cont = 0;
        }
        if(write < 0){
            err = 1;
        }
    }

decodeQOI_error:
    free(qoiBuffer);
    qoiBuffer = NULL;
    free(rgbaBuffer);
    rgbaBuffer = NULL;

    if(readFile != NULL){
        fclose(readFile);
        readFile = NULL;
    }

    if(writeFile != NULL){
        fclose(writeFile);
        writeFile = NULL;
    }

    return err;
}
