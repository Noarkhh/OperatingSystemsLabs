#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct Args {
    char replacedChar;
    char replacingChar;
    FILE* inputFile;
    FILE* outputFile;
} Args;

Args* parseArgs(int argc, char **argv) {
    Args* args = malloc(sizeof(Args));
    if (argc != 5) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return NULL;
    }

    char* replaced_char = argv[1];
    if (strlen(replaced_char) != 1) {
        fprintf(stderr, "first argument not a character: %s\n", replaced_char);
        return NULL;
    }
    args->replacedChar = replaced_char[0];

    char* replacing_char = argv[2];
    if (strlen(replacing_char) != 1) {
        fprintf(stderr, "second argument not a character: %s\n", replacing_char);
        return NULL;
    }
    args->replacingChar = replacing_char[0];

    char* input_file_name = argv[3];
    FILE* input_file = fopen(input_file_name, "r");
    if (!input_file) {
        fprintf(stderr, "invalid input file: %s\n", input_file_name);
        return NULL;
    }
    args->inputFile = input_file;

    char* output_file_name = argv[4];
    FILE* output_file = fopen(output_file_name, "w");
    if (!output_file) {
        fprintf(stderr, "invalid output file: %s\n", output_file_name);
        return NULL;
    }
    args->outputFile = output_file;
    return args;
}

int main(int argc, char **argv) {
    struct timespec start_real, end_real;
    clock_gettime(CLOCK_REALTIME, &start_real);

    Args* args = parseArgs(argc, argv);
    if (!args) return 1;

    fseek(args->inputFile, 0L, SEEK_END);
    long input_size = ftell(args->inputFile);
    rewind(args->inputFile);

    char* buf = malloc(input_size);

    fread(buf, 1, input_size, args->inputFile);
    for (int i = 0; i < input_size; i++) if (buf[i] == args->replacedChar) buf[i] = args->replacingChar;
    fwrite(buf, 1, input_size, args->outputFile);

    free(buf);
    fclose(args->inputFile);
    fclose(args->outputFile);

    clock_gettime(CLOCK_REALTIME, &end_real);

#ifdef TESTING
    printf("std: %.3f [ms]\n", (double) (end_real.tv_nsec - start_real.tv_nsec) / 1000);
#endif
    return 0;
}
