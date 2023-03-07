#ifndef CW01_WC_BLOCK_LIB_H
#define CW01_WC_BLOCK_LIB_H

typedef struct WcBlock {
    char** blocks;
    int max_blocks;
    int current_block;
} WcBlock;

WcBlock create_wc_block(int max_blocks);
int add_file_wc_to_block(WcBlock wcBlock, char* file_name);

#endif //CW01_WC_BLOCK_LIB_H
