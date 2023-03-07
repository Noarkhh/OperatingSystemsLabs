#include <stdio.h>
#include "wc_block_lib.h"

int main() {
    WcBlock wcBlock = create_wc_block(10);
    printf("%d\n", wcBlock.max_blocks);
    printf("%d\n", add_file_wc_to_block(wcBlock, "/tmp/beee"));
    printf("%s\n", wcBlock.blocks[0]);
    printf("%d\n", wcBlock.current_block);
  return 0;
}
