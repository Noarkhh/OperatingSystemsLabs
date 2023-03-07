#include <stdlib.h>

typedef struct WcBlock {
    char** blocks;
    int max_blocks;
    int current_block;
} WcBlock;

WcBlock create_wc_block(int max_blocks) {
    WcBlock wcBlock = {malloc(max_blocks), max_blocks, 0};
    return wcBlock;
}