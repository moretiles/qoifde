#include <stdio.h>

// 4 MB block size for writes
#define MAX_BLOCK_SIZE (4 * 1024 * 1024)

struct stack {
    char *chars;
    int pos;
};

// Push size bytes to store->chars
void push(struct stack *store, char *data, int size){
    int i = 0;
    for(i = 0; i < size; i++){
        store->chars[store->pos + i] = data[i];
    }
    store->pos = store->pos + size;
}

// Push size bytes to store->chars reversing byte order
void pushl(struct stack *store, char *data, int size){
    int i = 0;
    for(i = 0; i < size; i++){
        store->chars[store->pos + i] = data[size - 1 - i];
    }
    store->pos = store->pos + size;
}

// Push a single byte to store->chars
void pushc(struct stack *store, char c){
    store->chars[store->pos] = c;
    store->pos = store->pos + 1;
}

// Push all the data in a file into store->chars
void pushFromFile(FILE *readFile, struct stack *store, int size){
    int blockSize = 0;
    char *tmp = malloc(MAX_BLOCK_SIZE);
    if (size == 0) {
        return;
    }
    do{
        blockSize = (size > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : size;
        fread(tmp, 1, blockSize, readFile);
        push(store, tmp, blockSize);
        store->pos = store->pos + blockSize;
        size = size - MAX_BLOCK_SIZE;
    } while (size > 0);
    free(tmp);
}

// Pop all the data in store->chars into a file
void popToFile(FILE *writeFile, struct stack *store, int size){
    int blockSize = 0;
    if (size == 0) {
        return;
    }
    do{
        blockSize = (size > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : size;
        fwrite(store->chars, 1, blockSize, writeFile);
        store->pos = store->pos - blockSize;
        size = size - MAX_BLOCK_SIZE;
    } while (size > 0);
}

