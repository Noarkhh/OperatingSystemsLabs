#include <dlfcn.h>
#include "wc_repl_lib.h"


int execute_command(WcBlocks** wcBlocks, char* cmd, char* args) {
    char* ptr;

    if (!cmd) {
        return 2;
    }
    if (strcmp(cmd, "init") == 0) {
        int size = (int) strtol(args, &ptr, 10);
        if (strcmp(ptr, "") != 0 || size < 0) {
            printf("Invalid Arguments!\n");
            return -1;
        }
        *wcBlocks = create_wc_block((unsigned int) size);

    } else if (strcmp(cmd, "count") == 0) {
        if (!*wcBlocks) {
            printf("Not Yet Initialized!\n");
            return -1;
        }
        if (add_wc_output(*wcBlocks, args) < 0) {
            printf("Invalid Arguments!\n");
            return -1;
        }

    } else if (strcmp(cmd, "show") == 0) {
        if (!*wcBlocks) {
            printf("Not Yet Initialized!\n");
            return -1;
        }
        int index = (int) strtol(args, &ptr, 10);
        if (strcmp(ptr, "") != 0 || index < 0) {
            printf("Invalid Arguments!\n");
            return -1;
        }
        printf("%d: %s\n", index, get_block(*wcBlocks, index));

    } else if (strcmp(cmd, "delete") == 0) {
        if (!*wcBlocks) {
            printf("Not Yet Initialized!\n");
            return -1;
        }
        int index = (int) strtol(args, &ptr, 10);
        if (strcmp(ptr, "") != 0 || index < 0) {
            printf("Invalid Arguments!\n");
            return -1;
        }
        remove_block(*wcBlocks, index);

    } else if (strcmp(cmd, "destroy") == 0) {
        if (!*wcBlocks) {
            printf("Not Yet Initialized!\n");
            return -1;
        }
        if (args != NULL) {
            printf("Invalid Arguments!\n");
            return -1;
        }
        free_wc_blocks(*wcBlocks);
    } else if (strcmp(cmd, "showall") == 0) {
        if (!*wcBlocks) {
            printf("Not Yet Initialized!\n");
            return -1;
        }
        print_wc_blocks(*wcBlocks);

    } else if (strcmp(cmd, "quit") == 0) {
        return 1;
    } else {
        printf("Invalid Command!\n");
        return -1;
    }
    return 0;
}

void start_repl_loop() {
    char* buffer = malloc(50);
    char* cmd;
    char* args;

    WcBlocks* wcBlocks = NULL;

    struct tms start_cpu, end_cpu;
    struct timespec start_real, end_real;

    printf("]>> ");
    while (fgets(buffer, 50, stdin)) {
        cmd = strtok(buffer, " \n");
        if (cmd != NULL) args = strtok(NULL, " \n");
        double clock_ticks = (double) sysconf(_SC_CLK_TCK);

        times(&start_cpu);
        clock_gettime(CLOCK_REALTIME, &start_real);
        int result = execute_command(&wcBlocks, cmd, args);
        clock_gettime(CLOCK_REALTIME, &end_real);
        times(&end_cpu);

        if (result == 0) {
            printf("Time Elapsed | Real: %.3f [ms] | ", (double) (end_real.tv_nsec - start_real.tv_nsec) / 1000);
            printf("User: %.3f [ms] | ", (double) (end_cpu.tms_utime - start_cpu.tms_utime) * 1000 / clock_ticks);
            printf("System: %.3f [ms]\n", (double) (end_cpu.tms_stime - end_cpu.tms_stime) * 1000 / clock_ticks);
        } else if (result == 1) break;

        printf("]>> ");
    }
}

