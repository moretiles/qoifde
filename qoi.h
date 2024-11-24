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
int isDiff(struct rgba diff);

// Write small difference chunk
void writeDiff(struct queue *store, struct rgba diff);

// Check if a large (luma) difference exists
int isLuma(struct rgba diff);

// Write large (luma) difference chunk
void writeLuma(struct queue *store, struct rgba diff);

// Write RGB chunk
void writeRGB(struct queue *store, struct rgba c);

// Write RGBA chunk
void writeRGBA(struct queue *store, struct rgba c);

/*
 * Read rgba pixels from infile and output an encoded qoi image as outfile
 * The encoded image will have headers of width, height, channels, colorspace
 */
int encodeQOI(char *infile, char *outfile, int width, int height, char channels, char colorspace);

// Store color into seen using specification hashing formula
void cacheColor(struct rgba *seen, struct rgba color);

// Place the 4 bytes that make up color into store
void enqueueRgba(struct queue *store, struct rgba color, char channels);

/*
 * Read rgba pixels from infile and output an encoded qoi image as outfile
 * The encoded image will have headers of width, height, channels, colorspace
 */
int decodeQOI(char *infile, char *outfile);
