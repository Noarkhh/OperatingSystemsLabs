#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

void handleSig(int sig) {
    printf("PID: %d\tReceived signal %d, handling... \n", getpid(), sig);
    sleep(1);
    printf("PID: %d\tFinished handling signal %d\n", getpid(), sig);
}

void handleSigInfo(int sig, siginfo_t* siginfo, void* context) {
    printf("PID: %d\tReceived signal %d, handling... \n", getpid(), sig);
    printf("PID: %d\tSIGINFO: Received signal from PID %d \n", getpid(), siginfo->si_pid);
    sleep(1);
    printf("PID: %d\tFinished handling signal %d\n", getpid(), sig);
}

int main() {

    int handlingPid = getpid();
    struct sigaction act;
    act.sa_handler = handleSig;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    printf("Started testing the SA_SIGINFO flag\n");

    if (fork() == 0) {
        printf("PID: %d\tSending signal %d to PPID %d\n", getpid(), SIGUSR1, handlingPid);
        kill(handlingPid, SIGUSR1);
        exit(0);
    }
    sleep(1);

    printf("Set SA_SIGINFO flag\n");
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handleSigInfo;
    sigaction(SIGUSR1, &act, NULL);

    if (fork() == 0) {
        printf("PID: %d\tSending signal %d to PPID %d\n", getpid(), SIGUSR1, handlingPid);
        kill(handlingPid, SIGUSR1);
        exit(0);
    }
    sleep(1);

    printf("Finished testing the SA_SIGINFO flag\n");
    printf("------------------------------------\n");
    printf("Started testing the SA_NODEFER flag\n");
    printf("Handling two signals without the flag\n");
    act.sa_flags = 0;
    act.sa_sigaction = NULL;
    act.sa_handler = handleSig;
    sigaction(SIGUSR1, &act, NULL);


    if (fork() == 0) {
        kill(handlingPid, SIGUSR1);
        usleep(100);
        kill(handlingPid, SIGUSR1);
        exit(0);
    }
    sleep(1);

    act.sa_flags = SA_NODEFER;
    sigaction(SIGUSR1, &act, NULL);
    printf("Set SA_NODEFER flag\n");

    sleep(1);

    if (fork() == 0) {
        kill(handlingPid, SIGUSR1);
        usleep(100);
        kill(handlingPid, SIGUSR1);
        exit(0);
    }

    sleep(1);
    printf("Finished testing the SA_NODEFER flag\n");
    printf("------------------------------------\n");
    printf("Started testing the SA_RESETHAND flag\n");
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);


    if (fork() == 0) {
        kill(handlingPid, SIGUSR1);
        usleep(100);
        kill(handlingPid, SIGUSR1);
        exit(0);
    }
    sleep(1);

    act.sa_flags = SA_RESETHAND;
    sigaction(SIGUSR1, &act, NULL);
    printf("Set SA_RESETHAND flag\n");

    if (fork() == 0) {
        kill(handlingPid, SIGUSR1);
        usleep(100);
        kill(handlingPid, SIGUSR1);
        exit(0);
    }
    sleep(1);

    return 0;
}