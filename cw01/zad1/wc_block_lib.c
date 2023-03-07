#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct WcBlock {
    char** blocks;
    int max_blocks;
    int current_block;
} WcBlock;

WcBlock create_wc_block(int max_blocks) {
    WcBlock wcBlock = {calloc(max_blocks, sizeof(char*)), max_blocks, 0};
    return wcBlock;
}

int add_file_wc_to_block(WcBlock wcBlock, char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (file == NULL) return -1;

    char* cmd = malloc(strlen(file_name) + 20);
    if (sprintf(cmd, "wc %s > /tmp/wc_tmp", file_name) < 0) return -2;

    system(cmd);
    free(cmd);

    FILE* wc_file = fopen("/tmp/wc_tmp", "r");
    if (wc_file == NULL) return -3;

    fseek(wc_file, 0L, SEEK_END);
    size_t file_size = ftell(wc_file);
    rewind(wc_file);

    char* wc_output = malloc(file_size);
    fread(wc_output, 1, file_size, wc_file);
    wcBlock.blocks[wcBlock.current_block] = wc_output;
    wcBlock.current_block = (wcBlock.current_block + 1) % wcBlock.max_blocks;

    fclose(wc_file);
    return 0;
}