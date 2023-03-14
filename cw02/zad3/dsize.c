#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

int main() {
    DIR* curr_dir = opendir("./");
    struct dirent* dirent = readdir(curr_dir);
    struct stat file_stat;
    long long total_size = 0;
    while (dirent) {
        stat(dirent->d_name, &file_stat);
        if (!S_ISDIR(file_stat.st_mode)) {
            printf("%ld\t%s\n", file_stat.st_size, dirent->d_name);
            total_size += file_stat.st_size;
        }
        dirent = readdir(curr_dir);
    }
    printf("%lld\ttotal size\n", total_size);
    return 0;
}
