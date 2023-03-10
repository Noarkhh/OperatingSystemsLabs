#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Args {
    FILE* inputFile;
    FILE* outputFile;
} Args;

Args* parseArgs(int argc, char **argv) {
    Args* args = malloc(sizeof(Args));
    if (argc != 3) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return NULL;
    }

    char* input_file_name = argv[1];
    FILE* input_file = fopen(input_file_name, "r");
    if (!input_file) {
        fprintf(stderr, "invalid input file: %s\n", input_file_name);
        return NULL;
    }
    args->inputFile = input_file;

    char* output_file_name = argv[2];
    FILE* output_file = fopen(output_file_name, "w");
    if (!output_file) {
        fprintf(stderr, "invalid output file: %s\n", output_file_name);
        return NULL;
    }
    args->outputFile = output_file;
    return args;
}

int main(int argc, char **argv) {
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 1
#endif
    struct timespec start_real, end_real;
    clock_gettime(CLOCK_REALTIME, &start_real);

    Args* args = parseArgs(argc, argv);

    if (!args) return 1;

    fseek(args->inputFile, 0L, SEEK_END);
    long input_size = ftell(args->inputFile);
    rewind(args->inputFile);

    int blocks = input_size % BLOCK_SIZE == 0 ? input_size / BLOCK_SIZE : input_size / BLOCK_SIZE + 1;
    char* in_buf = malloc(blocks * BLOCK_SIZE);
    char* out_buf = malloc(input_size);

    fread(in_buf, BLOCK_SIZE, blocks, args->inputFile);

    for (int i = 0; i < input_size; i++) {
        out_buf[input_size - 1 - i] = in_buf[i];
    }
    fwrite(out_buf, 1, input_size, args->outputFile);

    free(in_buf);
    free(out_buf);
    fclose(args->inputFile);
    fclose(args->outputFile);

    clock_gettime(CLOCK_REALTIME, &end_real);
#ifdef TESTING
    printf("block size - %d: %.3f [ms]\n", BLOCK_SIZE, (double) (end_real.tv_nsec - start_real.tv_nsec) / 1000);
#endif
    return 0;
}

