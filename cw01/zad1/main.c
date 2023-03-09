#include <stdio.h>
#include "wc_block_lib.h"

int main() {
    WcBlocks* wcBlocks = create_wc_block(10);
    add_wc_output(wcBlocks, "/bin/grep");
    add_wc_output(wcBlocks, "/bin/ls");
    add_wc_output(wcBlocks, "/bin/find");
    print_wc_blocks(wcBlocks);
    remove_block(wcBlocks, 1);
    print_wc_blocks(wcBlocks);
  return 0;
}
