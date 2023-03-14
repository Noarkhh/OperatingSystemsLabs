#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>

long long total_size = 0;

int process_file(const char* name, const struct stat* file_stat, int flag) {
    if (flag == FTW_F) {
        printf("%ld\t%s\n", file_stat->st_size, name);
        total_size += file_stat->st_size;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return 1;
    }
    struct stat selected_dir_stat;
    if (stat(argv[1], &selected_dir_stat) == -1) {
        fprintf(stderr, "invalid path: %s\n", argv[1]);
        return 1;
    }

    ftw(argv[1], &(process_file), 10);
    printf("%lld\ttotal size\n", total_size);
    return 0;
}
