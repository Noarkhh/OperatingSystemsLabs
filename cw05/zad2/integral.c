#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

typedef struct Args {
    int sections;
    int processes;
} Args;

Args* parseArgs(int argc, char** argv) {
    Args *args = malloc(sizeof(Args));
    char* ptr;

    if (argc != 3) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        free(args);
        return NULL;
    }
    args->sections = (int) strtol(argv[1], &ptr, 10);
    if (strcmp(ptr, "") != 0) {
        fprintf(stderr, "first argument not a number: %s\n", argv[1]);
        free(args);
        return NULL;
    }
    args->processes = (int) strtol(argv[2], &ptr, 10);
    if (strcmp(ptr, "") != 0) {
        fprintf(stderr, "second argument not a number: %s\n", argv[2]);
        free(args);
        return NULL;
    }
    return args;
}

double integrandFunction(double x) {
    return 4 / (x * x + 1);
}

double calculateRectangle(double a, double b) {
    return integrandFunction((a + b) / 2) * (b - a);
}

double calculateIntervalArea(double start, double section_length, int sections) {
    double sum = 0.0;
    double a, b;
    for (int i = 0; i < sections; i++) {
        a = start + i * section_length;
        b = a + section_length;
        sum += calculateRectangle(a, b);
    }
    return sum;
}


int main(int argc, char** argv) {
    Args* args = parseArgs(argc, argv);
    if (!args) return 1;

    int** pipes = malloc(args->processes * sizeof(int*));
    for (int i = 0; i < args->processes; i++) {
        pipes[i] = malloc(2 * sizeof(int));
        pipe(pipes[i]);
    }

    double section_length = (double) 1 / args->sections;
    int rectangles_per_process;
    if (args->sections % args->processes == 0) rectangles_per_process = args->sections / args->processes;
    else rectangles_per_process = args->sections / args->processes + 1;
    double sum = 0;
    double start;
    int rectangles_amount;
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);
    errno = 0;

    for (int i = 0, p = 0; i < args->sections; i += rectangles_per_process, p++) {
        start = i * section_length;
        rectangles_amount = args->sections - i < rectangles_per_process ? args->sections - i : rectangles_per_process;
        if (fork() == 0) {
            close(pipes[p][0]);
            double result = calculateIntervalArea(start, section_length, rectangles_amount);
            write(pipes[p][1], &result, sizeof(double));
            exit(0);
        } else close(pipes[p][1]);
    }
    while(wait(NULL) > 0);
    for (int i = 0; i < args->processes; i++) {
        double result;
        read(pipes[i][0], &result, sizeof(double));
        sum += result;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    long time_diff_ns = (end.tv_sec - begin.tv_sec) * 10e9 + (end.tv_nsec - begin.tv_nsec);
    printf("result: %.15f\nelapsed time: %f [ms]\n", sum, ((double) time_diff_ns) / 10e6);
    return 0;
}
