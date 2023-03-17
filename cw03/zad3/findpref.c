#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

typedef struct Args {
    DIR* currDir;
    char* filePrefix;
} Args;

Args* parseArgs(int argc, char** argv) {
    Args* args = malloc(sizeof(Args));
    if (argc != 3) {
        fprintf(stderr, "invalid number of arguments: %d\n", argc - 1);
        return NULL;
    }
    args->currDir = opendir(argv[1]);
    if (args->currDir == NULL) {
        fprintf(stderr, "first argument not a valid directory: %s\n", argv[1]);
        return NULL;
    }
    args->filePrefix = argv[2];
    if (strlen(args->filePrefix) > 255) {
        fprintf(stderr, "second argument too big: %s\n", argv[2]);
        return NULL;
    }
    return args;
}

void checkPrefix(char* filePath, char* prefix) {
//    printf("checking prefix of: %s\n", filePath);
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        perror("error reading file");
        return;
    }
    char* buf = malloc(strlen(prefix) + 1);
    fread(buf, 1, strlen(prefix), file);

    if (strncmp(prefix, buf, strlen(prefix)) == 0) {
        printf("%s\t%d\n", filePath, (int) getpid());
    }
    fclose(file);
    free(buf);
}

void searchDir(DIR* currDir, char* prefix) {
    if (currDir == NULL) {
        perror("error opening directory");
        return;
    }
    struct dirent* currDirent = readdir(currDir);
    struct stat fileStat;
    char absolutePath[PATH_MAX + 1];
    pid_t child_pid;
    while (currDirent) {
        stat(currDirent->d_name, &fileStat);
        if (strcmp(currDirent->d_name, ".") == 0 || strcmp(currDirent->d_name, "..") == 0) {
            currDirent = readdir(currDir);
            continue;
        }
        if (S_ISDIR(fileStat.st_mode)) {
            child_pid = fork();
            if (child_pid == 0) {
                chdir(currDirent->d_name);
                searchDir(opendir("./"), prefix);
                exit(0);
            }
        } else {
            checkPrefix(realpath(currDirent->d_name, absolutePath), prefix);
        }
        currDirent = readdir(currDir);
    }
    closedir(currDir);
}

int main(int argc, char** argv) {
    Args* args = parseArgs(argc, argv);
    if (!args) return 1;

    chdir(argv[1]);
    searchDir(args->currDir, args->filePrefix);

    return 0;
}
