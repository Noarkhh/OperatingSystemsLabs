#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Args {
    char* lsDir;
} Args;

Args* parseArgs(int argc, char** argv) {
    Args* args = malloc(sizeof(Args));
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return NULL;
    }
    args->lsDir = argv[1];
//    if (strcmp(ptr, "") != 0) {
//        fprintf(stderr, "first argument not a number: %s\n", argv[1]);
//        return NULL;
//    }
    return args;
}

int main(int argc, char** argv) {
    Args* args = parseArgs(argc, argv);
    printf("%s", argv[0]);
    fflush(stdout);
    execl("/bin/ls", "ls", args->lsDir, NULL);
    return 0;
}
