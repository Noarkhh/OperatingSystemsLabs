#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;
    init_grid(foreground);

    signal(SIGUSR1, unpause);
    pthread_t* threads = malloc(sizeof(pthread_t) * GRIDHEIGHT * GRIDWIDTH);
//    UpdateCellArgs args = {0, 0, foreground, background};
    for (int i = 0; i < GRIDHEIGHT; ++i) {
        for (int j = 0; j < GRIDWIDTH; ++j) {
            UpdateCellArgs* args = malloc(sizeof(UpdateCellArgs));
//            UpdateCellArgs args = {0, 0, foreground, background};
            args->i = i;
            args->j = j;
            args->src = foreground;
            args->dst = background;
            pthread_create(&threads[i * GRIDWIDTH + j], NULL, update_cell, (void*) args);
        }
    }


	while (true)
	{
		draw_grid(foreground);
		usleep(500 * 1000);

		// Step simulation
		update_grid(foreground, background, threads);
		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}
