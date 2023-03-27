#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

enum command {
    IGNORE, HANDLER, MASK, PENDING
};

int main(int argc, char** argv) {
    sigset_t pendingSigs;
    int cmd = argv[1][0];

    switch (cmd) {
        case IGNORE:
            printf("PID: %d\texec sending signal to ignore\n", getpid());
            raise(SIGUSR1);
            break;
        case HANDLER:
            printf("PID: %d\texec process sending signal to handle\n", getpid());
            raise(SIGUSR1);
            break;
        case MASK:
            printf("PID: %d\texec process sending signal to block\n", getpid());
            raise(SIGUSR1);
            break;
        case PENDING:
            sigpending(&pendingSigs);
            printf("PID: %d\tis SIGUSR1 signal pending: %d\n", getpid(), sigismember(&pendingSigs, SIGUSR1));
            break;
    }
    return 0;
}