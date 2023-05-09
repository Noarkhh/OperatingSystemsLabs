#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define GRIDWIDTH 30
#define GRIDHEIGTH 30

typedef struct UpdateCellArgs {
    int i;
    int j;
    char *src;
    char *dst;
} UpdateCellArgs;

char *create_grid();
void destroy_grid(char *grid);
void draw_grid(char *grid);
void init_grid(char *grid);
bool is_alive(int row, int col, char *grid);
void* update_cell(void* args_void);
void update_grid(char *src, char *dst, pthread_t* threads);
void unpause(int sig);
