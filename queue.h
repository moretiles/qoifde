#include <stdio.h>
#include <string.h>
//#include <sys/stat.h>

#define ERR_QUEUE_OUT_OF_MEMORY (-1 * (1 << 0))
#define ERR_QUEUE_INVALID_SIZE (-1 * (1 << 1))
#define ERR_QUEUE_EMPTY (-1 * (1 << 2))
#define ERR_QUEUE_FILE_OPEN (-1 * (1 << 8))
#define ERR_QUEUE_FILE_IO (-1 * (1 << 9))

#ifndef MAX_BLOCK_SIZE
#define MAX_BLOCK_SIZE (3 * 1024 * 1024)
#endif

struct queue {
    char *chars;
    int pos;
    int base;
    int cap;
};

/*
// Not sure what to do here exactly
int dequeueStack(struct queue *store, char *data, int size){
if (size <= 0){
return ERR_QUEUE_INVALID_SIZE;
}
if (store->pos - size < 0){
return ERR_QUEUE_OUT_OF_MEMORY;
}

int i = 0;
for(i = 0; i < size; i++){
data[i] = store->chars[store->pos - i];
}
store->pos = store->pos - size;
return 0;
}
*/


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

    /*
       int i = 0;
       for(i = 0; i < size; i++){
       data[i] = store->chars[store->base + i];
       }
       */
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
    memcpy(writeQueue->chars, readQueue->chars, size);
    readQueue->base = readQueue->base + size;
    writeQueue->pos = writeQueue->pos + size;
    return size;
}

/*
// Enqueue size bytes to store->chars reversing byte order
int enqueuel(struct queue *store, char *data, int size){
    if (size <= 0){
        return ERR_QUEUE_INVALID_SIZE;
    }
    if (store->pos + size > store->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;;
    }

    int i = 0;
    for(i = 0; i < size; i++){
        store->chars[store->pos + i] = data[size - 1 - i];
    }
    store->pos = store->pos + size;
    return 0;
}
*/

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

    store->chars[store->pos] = c;
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

/*
// Enqueue all the data in a file into store->chars
int enqueueFromFile(char *readFilename, struct queue *store){
        FILE *readFile = NULL;
    int size = 0;
    struct stat st;
    stat(readFilename, &st);
    size = st.st_size;
    if (size <= 0) {
        return ERR_QUEUE_INVALID_SIZE;
    }
    if (store->pos + size > store->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;;
    }
    readFile = fopen(readFilename, "rb");
    if(readFile == NULL){
        return ERR_QUEUE_FILE_OPEN;
    }

    int blockSize = 0;
    char *tmp = malloc(MAX_BLOCK_SIZE);
    do{
        blockSize = (size > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : size;
        fread(tmp, 1, blockSize, readFile);
        enqueue(store, tmp, blockSize);
        size = size - MAX_BLOCK_SIZE;
    } while (size > 0);
    //printf("store->cap = %i, store->pos = %i\n", store->cap, store->pos);
    free(tmp);
    fclose(readFile);
    return 0;
}
*/

/*
// Dequeue all the data in store->chars into a file
int dequeueToFile(char *writeFilename, struct queue *store){
        int blockSize = 0;
    FILE *writeFile = NULL;
    if (store->pos == 0){
        return ERR_QUEUE_EMPTY;
    }
    writeFile = fopen(writeFilename, "wb");
    if(writeFile == NULL){
        return ERR_QUEUE_FILE_IO;
    }

    do{
        blockSize = (store->pos > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : store->pos;
        fwrite(store->chars, 1, blockSize, writeFile);
        store->pos = store->pos - MAX_BLOCK_SIZE;
    } while (store->pos > 0);
    store->pos = 0;
    fclose(writeFile);
    return 0;
}
*/

// Enqueue MAX_BLOCK_SIZE bytes from a file into store->chars
int fenqueue(FILE *readFile, struct queue *store, int size){
        int read = 0;
    if (store->pos + size > store->cap){
        return ERR_QUEUE_OUT_OF_MEMORY;;
    }
    if(ferror(readFile)){
        return ERR_QUEUE_FILE_IO;
    }

    read = fread(store->chars, 1, size, readFile);
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
