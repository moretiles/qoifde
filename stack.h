#include <stdio.h>
#include <sys/stat.h>

#define ERR_STACK_OUT_OF_MEMORY (1 << 0)
#define ERR_STACK_INVALID_SIZE (1 << 1)
#define ERR_STACK_EMPTY (1 << 2)
#define ERR_STACK_FILE_OPEN (1 << 9)
#define ERR_STACK_FILE_IO (1 << 10)

// 4 MB block size for writes
#define MAX_BLOCK_SIZE (4 * 1024 * 1024)

struct stack {
    char *chars;
    int pos;
    int cap;
};

int pop(struct stack *store, char *data, int size){
    if (size <= 0){
        return ERR_STACK_INVALID_SIZE;
    }
    if (store->pos - size < 0){
        return ERR_STACK_OUT_OF_MEMORY;
    }

    int i = 0;
    for(i = 0; i < size; i++){
        data[i] = store->chars[store->pos - i];
    }
    store->pos = store->pos - size;
    return 0;
}

/* bad */
int popPseudoQueue(struct stack *store, char *data, int size){
    if (size <= 0){
        return ERR_STACK_INVALID_SIZE;
    }
    if (store->pos - size < 0){
        return ERR_STACK_OUT_OF_MEMORY;
    }

    int i = 0;
    for(i = 0; i < size; i++){
        data[i] = store->chars[(store->cap - store->pos) + i];
    }
    store->pos = store->pos - size;
    return 0;
}

// Push size bytes to store->chars
int push(struct stack *store, char *data, int size){
    if (size <= 0){
        return ERR_STACK_INVALID_SIZE;
    }
    if (store->pos + size > store->cap){
        return ERR_STACK_OUT_OF_MEMORY;
    }

    int i = 0;
    for(i = 0; i < size; i++){
        store->chars[store->pos + i] = data[i];
    }
    store->pos = store->pos + size;
    return 0;
}

// Push size bytes to store->chars reversing byte order
int pushl(struct stack *store, char *data, int size){
    if (size <= 0){
        return ERR_STACK_INVALID_SIZE;
    }
    if (store->pos + size > store->cap){
        return ERR_STACK_OUT_OF_MEMORY;;
    }

    int i = 0;
    for(i = 0; i < size; i++){
        store->chars[store->pos + i] = data[size - 1 - i];
    }
    store->pos = store->pos + size;
    return 0;
}

// Push a single byte to store->chars
int pushc(struct stack *store, char c){
    if (store->pos + 1 > store->cap){
        return ERR_STACK_OUT_OF_MEMORY;;
    }

    store->chars[store->pos] = c;
    store->pos = store->pos + 1;
    return 0;
}

// Push all the data in a file into store->chars
int pushFromFile(char *readFilename, struct stack *store){
    FILE *readFile = NULL;
    int size = 0;
    struct stat st;
    stat(readFilename, &st);
    size = st.st_size;
    if (size <= 0) {
        return ERR_STACK_INVALID_SIZE;
    }
    if (store->pos + size > store->cap){
        return ERR_STACK_OUT_OF_MEMORY;;
    }
    readFile = fopen(readFilename, "rb");
    if(readFile == NULL){
        return ERR_STACK_FILE_OPEN;
    }

    int blockSize = 0;
    char *tmp = malloc(MAX_BLOCK_SIZE);
    do{
        blockSize = (size > MAX_BLOCK_SIZE) ? MAX_BLOCK_SIZE : size;
        fread(tmp, 1, blockSize, readFile);
        push(store, tmp, blockSize);
        size = size - MAX_BLOCK_SIZE;
    } while (size > 0);
    //printf("store->cap = %i, store->pos = %i\n", store->cap, store->pos);
    free(tmp);
    fclose(readFile);
    return 0;
}

// Pop all the data in store->chars into a file
int popToFile(char *writeFilename, struct stack *store){
    int blockSize = 0;
    FILE *writeFile = NULL;
    if (store->pos == 0){
        return ERR_STACK_EMPTY;
    }
    writeFile = fopen(writeFilename, "wb");
    if(writeFile == NULL){
        return ERR_STACK_FILE_IO;
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

// Push MAX_BLOCK_SIZE bytes from a file into store->chars
int pushChunkFromFile(FILE *readFile, struct stack *store){
    int read = 0;
    if (store->pos + MAX_BLOCK_SIZE > store->cap){
        return ERR_STACK_OUT_OF_MEMORY;;
    }
    if(ferror(readFile)){
        return ERR_STACK_FILE_IO;
    }
    
    read = fread(store->chars, 1, MAX_BLOCK_SIZE, readFile);
    store->pos = store->pos + read;
    return read;
}

// Pop MAX_BLOCK_SIZE bytes in store->chars to a file
int popChunkToFile(FILE *writeFile, struct stack *store){
    int write = 0;
    if (store->pos == 0){
        return ERR_STACK_EMPTY;
    }
    if(ferror(writeFile)){
        return ERR_STACK_FILE_IO;
    }

    write = fwrite(store->chars, 1, MAX_BLOCK_SIZE, writeFile);
    store->pos = store->pos - write;
    return write;
}
