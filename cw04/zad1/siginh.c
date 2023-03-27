#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>

enum command {
    IGNORE, HANDLER, MASK, PENDING
};

typedef struct Args {
    enum command command;
} Args;

Args* parseArgs(int argc, char** argv) {
    Args* args = malloc(sizeof(Args));
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return NULL;
    }
    char* cmd = argv[1];
    int wasCommandSet = 0;
    char* legalCommands[4] = {"ignore", "handler", "mask", "pending"};
    for (int i = 0; i < 4; i++) {
        if (strcmp(cmd, legalCommands[i]) == 0) {
            switch (i) {
                case 0:
                    args->command = IGNORE;
                    break;
                case 1:
                    args->command = HANDLER;
                    break;
                case 2:
                    args->command = MASK;
                    break;
                case 3:
                    args->command = PENDING;
                    break;
                default:
                    break;
            }
            wasCommandSet = 1;
        }
    }
    if (!wasCommandSet) {
        fprintf(stderr, "invalid command: %s\n", argv[1]);
        return NULL;
    }

    return args;
}

void handleSig(int sig) {
    printf("PID: %d\tReceived signal %d\n", getpid(), sig);
}


int main(int argc, char** argv) {
#ifndef MODE
#define MODE 1
#endif
    Args* args = parseArgs(argc, argv);
    if (args == NULL) return 1;

    struct sigaction act;
    sigset_t procMask;

    switch (args->command) {
        case IGNORE:
            signal(SIGUSR1, SIG_IGN);
            break;
        case HANDLER:
            act.sa_handler = handleSig;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            sigaction(SIGUSR1, &act, NULL);
            break;
        case MASK:
        case PENDING:
            sigaddset(&procMask, SIGUSR1);
            sigprocmask(SIG_BLOCK, &procMask, NULL);
            break;
    }

    printf("PID: %d\tparent process sending signal\n", getpid());
    raise(SIGUSR1);
    if (MODE == 1) {
        if (fork() == 0) {
            sigset_t pendingSigs;

            switch (args->command) {
                case IGNORE:
                    printf("PID: %d\tchild process sending signal to ignore\n", getpid());
                    raise(SIGUSR1);
                    break;
                case HANDLER:
                    printf("PID: %d\tchild process sending signal to handle\n", getpid());
                    raise(SIGUSR1);
                    break;
                case MASK:
                    printf("PID: %d\tchild process sending signal to block\n", getpid());
                    raise(SIGUSR1);
                    break;
                case PENDING:
                    sigpending(&pendingSigs);
                    printf("PID: %d\tis SIGUSR1 signal pending: %d\n", getpid(), sigismember(&pendingSigs, SIGUSR1));
                    break;
            }
            exit(0);
        }
    } else {
        char buf[2];
        buf[0] = args->command;
        buf[1] = 0;
        execl("./execinheritor", "execinheritor", buf, NULL);
    }
    while (wait(NULL) > 0);
    return 0;
}
