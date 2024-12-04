#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

//#include "queue.h"
//#include "rgba.h"

#define MAGIC "qoif"
#define END_MARKER "\x00\x00\x00\x00\x00\x00\x00\x01"

#ifndef MAX_BLOCK_SIZE
#define MAX_BLOCK_SIZE (3 * 1024 * 1024)
#endif

// Full byte check
#define QOI_OP_RGB   (0xfe)
#define QOI_OP_RGBA  (0xff)

// Check two leading bits of byte
#define QOI_OP_INDEX (0x00)
#define QOI_OP_DIFF  (0x40)
#define QOI_OP_LUMA  (0x80)
#define QOI_OP_RUN   (0xc0)

#define ERR_QUEUE_OUT_OF_MEMORY (-1 * (1 << 0))
#define ERR_QUEUE_INVALID_SIZE (-1 * (1 << 1))
#define ERR_QUEUE_EMPTY (-1 * (1 << 2))
#define ERR_QUEUE_FILE_OPEN (-1 * (1 << 8))
#define ERR_QUEUE_FILE_IO (-1 * (1 << 9))

struct queue {
    char *chars;
    int pos;
    int base;
    int cap;
};

struct rgba {
    int8_t r;
    int8_t g;
    int8_t b;
    int8_t a;
};

/* 
 * Dequeue size bytes from store->chars to data 
 * Uses base to know what the bottom should be
 * */
int dequeue(struct queue *store, char *data, int size){
    if (size <= 0){
        return ERR_QUEUE_INVALID_SIZE;
    }
    if(size > store->pos - store->base){
        size = store->pos - store->base;
    }

    memcpy(data, store->chars + store->base, size);
    store->base = store->base + size;
    return size;
}

// Enqueue size bytes to store->chars
int enqueue(struct queue *store, char *data, int size){
    if (size <= 0){
        return ERR_QUEUE_INVALID_SIZE;
    }
    if (store->pos + size > store->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;
    }

    memcpy(store->chars + store->pos, data, size);
    store->pos = store->pos + size;
    return size;
}

/*
 * Move size bytes from readQueue to writeQueue
 */
int exchange(struct queue *readQueue, struct queue *writeQueue, int size){
    if (size <= 0){
        return ERR_QUEUE_INVALID_SIZE;
    }
    if(size > readQueue->pos - readQueue->base){
        size = readQueue->pos - readQueue->base;
    }
    if (writeQueue->pos + size > writeQueue->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;
    }
    memcpy(writeQueue->chars + writeQueue->pos, readQueue->chars + readQueue->base, size);
    readQueue->base = readQueue->base + size;
    writeQueue->pos = writeQueue->pos + size;
    return size;
}

// Dequeue a single byte from store->chars
int dequeuec(struct queue *store, char *cptr){
    if (store->base == store->pos){
        return ERR_QUEUE_OUT_OF_MEMORY;;
    }

    *cptr = store->chars[store->base];
    store->base = store->base + 1;
    return 0;
}

// Enqueue a single byte to store->chars
int enqueuec(struct queue *store, char c){
    if (store->pos + 1 > store->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;;
    }

    *(store->chars + store->pos) = c;
    store->pos = store->pos + 1;
    return 0;
}

// Copy from store->chars over [base, pos) to start of store->chars
int foldDown(struct queue *store){
    int diff = store->pos - store->base;
    if(diff > store->base){
        return ERR_QUEUE_OUT_OF_MEMORY;
    }

    memcpy(store->chars, store->chars + store->base, diff);
    store->pos = diff;
    store->base = 0;
    return diff;
}

// Enqueue MAX_BLOCK_SIZE bytes from a file into store->chars
int fenqueue(FILE *readFile, struct queue *store, int size){
    int read = 0;
    if (store->pos + size > store->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;;
    }
    if(ferror(readFile)){
        return ERR_QUEUE_FILE_IO;
    }

    read = fread(store->chars + store->pos, 1, size, readFile);
    store->pos = store->pos + read;
    return read;
}

// Dequeue MAX_BLOCK_SIZE bytes in store->chars to a file
int fdequeue(FILE *writeFile, struct queue *store, int size){
    int difference = 0;
    int write = 0;
    if (store->pos == 0){
        return ERR_QUEUE_EMPTY;
    }
    if(ferror(writeFile)){
        return ERR_QUEUE_FILE_IO;
    }

    difference = store->pos - store->base;
    size = (size > difference) ? difference : size;
    write = fwrite(store->chars, 1, size, writeFile);
    store->pos = store->pos - difference;
    if(store->pos < 0){
        store->pos = 0;
    }
    return write;
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

// Write QOI small difference chunk
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

// Write QOI large (luma) difference chunk
void writeLuma(struct queue *store, struct rgba diff){
    char result[2] = {0, 0};
    result[0] = QOI_OP_LUMA;
    result[0] = result[0] | (diff.g + 32);
    result[1] = (diff.r - diff.g + 8) << 4;
    result[1] = result[1] | (diff.b - diff.g + 8);
    enqueue(store, result, 2);
}

// Write QOI RGB chunk
void writeRGB(struct queue *store, struct rgba c){
    char result[4];
    result[0] = QOI_OP_RGB;
    result[1] = c.r;
    result[2] = c.g;
    result[3] = c.b;
    enqueue(store, result, 4);
}

// Write QOI RGBA chunk
void writeRGBA(struct queue *store, struct rgba c){
    char result[5];
    result[0] = QOI_OP_RGBA;
    result[1] = c.r;
    result[2] = c.g;
    result[3] = c.b;
    result[4] = c.a;
    enqueue(store, result, 5);
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
struct rgba colorsDiff (struct rgba c1, struct rgba c2){
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

    // Allocate bytes for buffer used to store rgb/rgba pixels and qoi pixels
    rgbaBuffer = malloc(MAX_BLOCK_SIZE);
    readQueue->chars = rgbaBuffer;
    qoiBuffer = malloc(5 * MAX_BLOCK_SIZE / 4);
    writeQueue->chars = qoiBuffer;
    if(rgbaBuffer == NULL || qoiBuffer == NULL){
        err = 2;
        goto encodeQOI_error;
    }

    // Try to open files for reading and writing
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

    // Check for error and also setup loop
    if(fdequeue(writeFile, writeQueue, 14) <= 0){
        cont = 0;
        err = 1;
    }
    if(fenqueue(readFile, readQueue, MAX_BLOCK_SIZE) <= 0 || readQueue->pos % channels != 0){
        cont = 0;
        err = 1;
    }

    while(cont){
        // Get rgb(a) pixel from readQueue
        dequeue(readQueue, colors, channels);
        current.r = colors[0];
        current.g = colors[1];
        current.b = colors[2];
        if (channels == 4){
            current.a = colors[3];
        }

        /*
         * Checks for runs
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
                // Write runs and then process current color
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

            // No compression matches
            else if (channels == 3 || current.a == prev.a){
                writeRGB(writeQueue, current);
            } else {
                writeRGBA(writeQueue, current);
            }
        }

        // Setup everything for next iteration
        prev.r = current.r;
        prev.g = current.g;
        prev.b = current.b;
        prev.a = current.a;

        // Check if we have read everything from readQueue
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

    // Write runs and endmarker
    if(err == 0){
        if(runs != 0){
            enqueuec(writeQueue, QOI_OP_RUN | (runs - 1));
            runs = 0;
            fdequeue(writeFile, writeQueue, 1);
        }
        enqueue(writeQueue, END_MARKER, 8);
        fdequeue(writeFile, writeQueue, 8);
    }

    // If any error occurs jump here to neatly return
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

/*
 * Read rgba pixels from infile and output an encoded qoi image as outfile
 * The encoded image will have headers of width, height, channels, colorspace
 */
int decodeQOI(char *infile, char *outfile, char channelsInput){
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

    // Allocate bytes for buffer used to store rgb/rgba pixels and qoi pixels
    qoiBuffer = malloc(MAX_BLOCK_SIZE);
    readQueue->chars = qoiBuffer;
    rgbaBuffer = malloc(MAX_BLOCK_SIZE);
    writeQueue->chars = rgbaBuffer;
    if(rgbaBuffer == NULL || qoiBuffer == NULL){
        err = 2;
        goto decodeQOI_error;
    }

    // Try to open files for reading and writing
    readFile = fopen(infile, "rb");
    writeFile = fopen(outfile, "wb");
    if(readFile == NULL || writeFile == NULL){
        err = 1;
        goto decodeQOI_error;
    }

    // Read QOI header
    if(fenqueue(readFile, readQueue, 14) != 14){
        cont = 0;
        err = 1;
    } else {
        // Bit shifts should work on both big/little endian machines
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
                channels < 3 || channels > 4 || channelsInput < 3 || channelsInput > 4 || \
                colorspace < 0 || colorspace > 1){
            cont = 0;
            err = 1;
        }
        // Set channels to channelsInput once we have checked the file is valid
        channels = channelsInput;
        readQueue->pos = 0;
        readQueue->base = 0;
    }

    // Check for error and also setup loop
    read = fenqueue(readFile, readQueue, MAX_BLOCK_SIZE);
    if(read <= 0){
        cont = 0;
    }
    if(read < 0){
        err = 1;
    }

    while(cont){
        // Read the first byte of a QOI chunk into currentOperation
        dequeuec(readQueue, (char *) &currentOperation);

        // Switch based on two most significant bytes of char
        switch (currentOperation & 0xc0){
            /*
             * Enqueue chunk at index
             * Also check for end of file
             */
            case QOI_OP_INDEX:
                if(currentOperation == 0 && currentOperation == lastOperation){
                    writeQueue->pos = writeQueue->pos - channels;
                    cont = 0;
                }else{
                    enqueueRgba(writeQueue, seen[0xff & currentOperation], channels);
                }
                needCaching = 0;
                break;

            // Enqueue difference
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

            // Enqueue luma difference
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

            // May be a run, rgb chunk, or rgba chunk
            case QOI_OP_RUN:
                // Move rgb pixel from readQueue to writeQueue
                if(currentOperation == QOI_OP_RGB){
                    exchange(readQueue, writeQueue, 3);
                    if(channels == 4){
                        enqueuec(writeQueue, 0xff);
                    } 
                    needCaching = 1;
                    break;

                // Move rgba pixel from readQueue to writeQueue
                } else if (currentOperation == QOI_OP_RGBA){
                    exchange(readQueue, writeQueue, 4);
                    needCaching = 1;
                    break;

                // Process runs
                } else{
                    while(((uint8_t) currentOperation) >= 0xc0){
                        enqueueRgba(writeQueue, prev, channels);
                        currentOperation -= 1;
                    }
                    needCaching = 0;
                    break;
                }
        }

        // Setup everything for next iteration
        lastOperation = currentOperation;

        // Important to have this check
        if(writeQueue->pos >= channels){
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
        }

        if (needCaching == 1){
            cacheColor(seen, prev);
            needCaching = 0;
        }

        /*
         * Check if we have read almost everything from this fread
         * Move remaining data from the back to the front of the queue
         */
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

        /* 
         * Check if writeQueue is almost full
         * Stopping 64 bytes before is for safety
         */
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

    // Flush remaining data to writeFile
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

    // If any error occurs jump here to neatly return
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
