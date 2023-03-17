#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Args {
    int childProcesses;
} Args;

Args* parseArgs(int argc, char** argv) {
    Args* args = malloc(sizeof(Args));
    char* ptr;
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return NULL;
    }
    args->childProcesses = (int) strtol(argv[1], &ptr, 10);
    if (strcmp(ptr, "") != 0) {
        fprintf(stderr, "first argument not a number: %s\n", argv[1]);
        return NULL;
    }
    return args;
}

void childPrint(int childId) {
    printf("child %d \tPPID: %d\tPID: %d\n", childId, (int) getppid(), (int) getpid());
}

int main(int argc, char** argv) {
    Args* args = parseArgs(argc, argv);
    pid_t child_pid;
    if (!args) return 1;
    for (int i = 0; i < args->childProcesses; i++) {
        child_pid = fork();
        if (child_pid == 0) {
            childPrint(i);
            return 0;
        }
    }
    waitpid(child_pid, NULL, 0);
    printf("spawned child processes: %d\n", args->childProcesses);
    return 0;
}
