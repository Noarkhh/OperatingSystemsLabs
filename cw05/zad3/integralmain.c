#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

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

int main(int argc, char** argv) {
    Args* args = parseArgs(argc, argv);
    if (!args) return 1;

    mkfifo("pipe", 0666);
    int read_pipe;

    double section_length = (double) 1 / args->sections;
    int rectangles_per_process;
    if (args->sections % args->processes == 0) rectangles_per_process = args->sections / args->processes;
    else rectangles_per_process = args->sections / args->processes + 1;
    double sum = 0;
    double start;
    int rectangles_amount;
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);

    for (int i = 0, p = 0; i < args->sections; i += rectangles_per_process, p++) {
        start = i * section_length;
        rectangles_amount = args->sections - i < rectangles_per_process ? args->sections - i : rectangles_per_process;
        if (fork() == 0) {
            char start_str[23];
            char section_length_str[23];
            char rectangles_amount_str[10];
            sprintf(start_str, "%.16f", start);
            sprintf(section_length_str, "%.16f", section_length);
            sprintf(rectangles_amount_str, "%d", rectangles_amount);

            execl("integralsub", "integralsub", start_str, section_length_str, rectangles_amount_str, NULL);
            exit(0);
        }
        if (i == 0) read_pipe = open("pipe", O_RDONLY);
    }
    while(wait(NULL) > 0);
    double* partial_results = malloc(sizeof(double) * args->processes);
    read(read_pipe, partial_results, sizeof(double) * args->processes);

    for (int i = 0; i < args->processes; i++) {
        sum += partial_results[i];
    }
    clock_gettime(CLOCK_REALTIME, &end);
    long time_diff_ns = (end.tv_sec - begin.tv_sec) * 10e9 + (end.tv_nsec - begin.tv_nsec);
    printf("result: %.15f\nelapsed time: %f [ms]\n", sum, ((double) time_diff_ns) / 10e6);
    remove("pipe");
    return 0;
}
