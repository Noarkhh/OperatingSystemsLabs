#include <stdio.h>
#include "wc_block_lib.h"

int main() {
  WcBlock wcBlock = create_wc_block(10);
  printf("%d\n", wcBlock.max_blocks);
  return 0;
}
