#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

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
    int write_pipe = open("pipe", O_WRONLY);
    double start = strtod(argv[1], NULL);
    double section_length = strtod(argv[2], NULL);
    int sections = (int) strtol(argv[3], NULL, 10);
    double result = calculateIntervalArea(start, section_length, sections);
    write(write_pipe, &result, sizeof(double));
    return 0;
}
