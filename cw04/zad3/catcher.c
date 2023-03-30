#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

enum Command {
    NUMBERS, TIME, REQUESTS, CLOCK, STOP
};

int numberOfRequests = 0;
int clockPid = -1;

int executeCommand(int command) {
    printf("PID: %d\tExecuting command %d\n", getpid(), command);
    time_t t;
    numberOfRequests++;
    if (clockPid != -1) {
        kill(clockPid, SIGKILL);
        clockPid = -1;
    }

    switch (command - 1) {
        case NUMBERS:
            for (int i = 1; i <= 100; i++) printf("%d ", i);
            printf("\n");
            return 0;
        case TIME:
            time(&t);
            printf("Current time: %s\n", ctime(&t));
            return 0;
        case REQUESTS:
            printf("Number of requests: %d\n", numberOfRequests);
            return 0;
        case CLOCK:
            clockPid = fork();
            if (clockPid == 0) {
                while (1) {
                    time(&t);
                    printf("Current time: %s\n", ctime(&t));
                    sleep(1);
                }
            }
            return 0;
        case STOP:
            printf("Stopping\n");
            return 1;
    }
    return 0;
}

void receiveCommand(int sig, siginfo_t* siginfo, void* context) {
    int command = siginfo->si_value.sival_int;
    int senderPid = siginfo->si_pid;
    printf("PID: %d\tReceived command %d from %d\n", getpid(), command, senderPid);
    int stop = executeCommand(command);
    kill(senderPid, SIGUSR1);
    printf("PID: %d\tSending confirmation to %d\n", getpid(), senderPid);
    if (stop) exit(0);
}


int main() {
    printf("Catcher PID: %d\n", getpid());

    struct sigaction act;
    act.sa_sigaction = receiveCommand;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);

    while(1) sleep(100);
}