#define ERR_QUEUE_OUT_OF_MEMORY (-1 * (1 << 0))
#define ERR_QUEUE_INVALID_SIZE (-1 * (1 << 1))
#define ERR_QUEUE_EMPTY (-1 * (1 << 2))
#define ERR_QUEUE_FILE_OPEN (-1 * (1 << 8))
#define ERR_QUEUE_FILE_IO (-1 * (1 << 9))

#ifndef MAX_BLOCK_SIZE
#define MAX_BLOCK_SIZE (3 * 1024 * 1024)
#endif

#include <stdio.h>
#include <string.h>

struct queue {
    char *chars;
    int pos;
    int base;
    int cap;
};

// Not sure what to do here exactly
//int dequeueStack(struct queue *store, char *data, int size);

/* 
 * Dequeue size bytes from store->chars to data 
 * Uses base to know what the bottom should be
 * */
int dequeue(struct queue *store, char *data, int size);

// Enqueue size bytes to store->chars
int enqueue(struct queue *store, char *data, int size);

/*
 * Move size bytes from readQueue to writeQueue
 */
int exchange(struct queue *readQueue, struct queue *writeQueue, int size);

// Enqueue size bytes to store->chars reversing byte order
//int enqueuel(struct queue *store, char *data, int size);

// Dequeue a single byte from store->chars
int dequeuec(struct queue *store, char *cptr);

// Enqueue a single byte to store->chars
int enqueuec(struct queue *store, char c);

// Copy from store->chars over [base, pos) to start of store->chars
int foldDown(struct queue *store);

// Enqueue all the data in a file into store->chars
//int enqueueFromFile(char *readFilename, struct queue *store);

// Dequeue all the data in store->chars into a file
//int dequeueToFile(char *writeFilename, struct queue *store);

// Enqueue MAX_BLOCK_SIZE bytes from a file into store->chars
int fenqueue(FILE *readFile, struct queue *store, int size);

// Dequeue MAX_BLOCK_SIZE bytes in store->chars to a file
int fdequeue(FILE *writeFile, struct queue *store, int size);
