#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "wc_block_lib.h"

WcBlocks* create_wc_block(unsigned int maxBlocks) {
    WcBlocks* wcBlocks = malloc(sizeof(WcBlocks));
    wcBlocks->blocks = calloc(maxBlocks, sizeof(char*));
    wcBlocks->maxBlocks = maxBlocks;
    wcBlocks->currentBlock = 0;
    return wcBlocks;
}

int add_wc_output(WcBlocks* wcBlocks, const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (file == NULL) return -1;

    char* cmd = malloc(strlen(fileName) + 20);
    if (sprintf(cmd, "wc %s > /tmp/wc_tmp", fileName) < 0) return -2;

    system(cmd);
    free(cmd);

    FILE* wcFile = fopen("/tmp/wc_tmp", "rb");
    if (wcFile == NULL) return -3;

    fseek(wcFile, 0L, SEEK_END);
    size_t outputLength = ftell(wcFile);
    rewind(wcFile);

    char* buffer = malloc(outputLength + 1);
    fread(buffer, sizeof(char), outputLength, wcFile);
    buffer[outputLength] = '\0';
    wcBlocks->blocks[wcBlocks->currentBlock] = buffer;
    wcBlocks->currentBlock = (wcBlocks->currentBlock + 1) % wcBlocks->maxBlocks;
    remove("tmp/wc_tmp");
    fclose(wcFile);
    return 0;
}

char* get_block(const WcBlocks* wcBlocks, unsigned int index) {
    return wcBlocks->blocks[index];
}

int remove_block(WcBlocks* wcBlocks, unsigned int index) {
    if (wcBlocks->blocks[index] == NULL) return -1;
    free(wcBlocks->blocks[index]);
    wcBlocks->blocks[index] = NULL;
    return 0;
}

void free_wc_blocks(WcBlocks* wcBlocks) {
    for (int i = 0; i < wcBlocks->maxBlocks; i++) {
        remove_block(wcBlocks, i);
    }
    wcBlocks->currentBlock = 0;
    free(wcBlocks->blocks);
    wcBlocks->blocks = NULL;
}

void print_wc_blocks(const WcBlocks* wcBlocks) {
    printf("WcBlocks(maxBlocks = %d, currentBlocks = %d)\n", wcBlocks->maxBlocks, wcBlocks->currentBlock);
    if (wcBlocks->blocks == NULL) return;
    for (int i = 0; i < wcBlocks->maxBlocks; i++) {
        printf("%d: %s\n", i, wcBlocks->blocks[i] == NULL ? "(null)" : wcBlocks->blocks[i]);
    }
}