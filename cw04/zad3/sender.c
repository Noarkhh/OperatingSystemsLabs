#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

typedef struct Args {
    int catcherPid;
    int commandsLen;
    int* commands;
} Args;

Args* parseArgs(int argc, char** argv) {
    Args* args = malloc(sizeof(Args));
    char* ptr;
    if (argc < 3) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        free(args);
        return NULL;
    }

    args->catcherPid = (int) strtol(argv[1], &ptr, 10);
    if (strcmp(ptr, "") != 0) {
        fprintf(stderr, "first argument not a number: %s\n", argv[1]);
        free(args);
        return NULL;
    }
    args->commandsLen = argc - 2;
    args->commands = malloc(args->commandsLen * sizeof(int));
    for (int i = 2; i < argc; i++) {
        args->commands[i - 2] = (int) strtol(argv[i], &ptr, 10);
        if (strcmp(ptr, "") != 0) {
            fprintf(stderr, "command %d not a number: %s\n", i - 2, argv[1]);
            free(args->commands);
            free(args);
            return NULL;
        }
        if (args->commands[i - 2] < 1 || args->commands[i - 2] > 5) {
            fprintf(stderr, "command %d not a number between 1-5: %s\n", i - 2, argv[1]);
            free(args->commands);
            free(args);
            return NULL;
        }
    }

    return args;
}

void printArgs(Args* args) {
    printf("args: catcherPid %d; commands [", args->catcherPid);
    for (int i = 0; i < args->commandsLen; i++) {
        printf("%d, ", args->commands[i]);
    }
    printf("]\n");
}

void receiveConfirmation(int sig) {
    printf("PID: %d\tReceived confirmation\n", getpid());
}

int main(int argc, char** argv) {
    Args* args = parseArgs(argc, argv);
    if (!args) return 1;

    sigset_t sigusrMask;
    sigemptyset(&sigusrMask);
    sigaddset(&sigusrMask, SIGUSR1);

    struct sigaction act;
    act.sa_handler = receiveConfirmation;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    sigset_t emptyMask;
    sigemptyset(&emptyMask);

    sigprocmask(SIG_BLOCK, &sigusrMask, NULL);

//    printf("Proc mask: %d\n", sigismember(&procMask, SIGUSR1));

    sigaction(SIGUSR1, &act, NULL);

    for (int i = 0; i < args->commandsLen; i++) {
        union sigval command;
        command.sival_int = args->commands[i];
        printf("PID: %d\tSending command %d to %d\n", getpid(), command.sival_int, args->catcherPid);
        sigqueue(args->catcherPid, SIGUSR1, command);
        sigsuspend(&emptyMask);
    }

    return 0;
}
