#ifndef CW01_WC_REPL_LIB_H
#define CW01_WC_REPL_LIB_H

#include "../zad1/wc_block_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/times.h>

int execute_command(WcBlocks** wcBlock, char* cmd, char* args);
void start_repl_loop();

#endif //CW01_WC_REPL_LIB_H
