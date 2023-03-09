#ifndef CW01_WC_BLOCK_LIB_H
#define CW01_WC_BLOCK_LIB_H

typedef struct WcBlocks {
    char** blocks;
    unsigned int maxBlocks;
    unsigned int currentBlock;
} WcBlocks;

WcBlocks* create_wc_block(unsigned int maxBlocks);
int add_wc_output(WcBlocks *wcBlocks, const char* fileName);
char* get_block(const WcBlocks* wcBlocks, unsigned int index);
int remove_block(WcBlocks* wcBlocks, unsigned int index);
void free_wc_blocks(WcBlocks* wcBlocks);
void print_wc_blocks(const WcBlocks* wcBlocks);

#endif //CW01_WC_BLOCK_LIB_H
