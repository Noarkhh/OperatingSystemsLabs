#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#define ELVES 10
#define REINDEERS 9

pthread_t elf_threads[ELVES];
pthread_t reindeer_threads[REINDEERS];
pthread_t* santa_thread;

int waiting_elves_number = 0;
int waiting_elves[3] = {-1, -1, -1};

pthread_mutex_t waiting_elves_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t getting_help_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t waiting_elves_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t waiting_reindeer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_sleeping_cond = PTHREAD_COND_INITIALIZER;


void* reindeer_thread_routine(void* id_ptr) {
    int id = *(int*) id_ptr;


}

void* elf_thread_routine(void* id_ptr) {
    int id = *(int*) id_ptr;
    while (1) {
        printf("Elf %d: working...\n", id);
        sleep(rand() % 4 + 2);

        pthread_mutex_lock(&waiting_elves_mutex);
        if (waiting_elves_number < 3) {
            waiting_elves[waiting_elves_number] = id;
            waiting_elves_number++;
            printf("Elf %d: %d elves now waiting\n", id, waiting_elves_number);
        } else {
            printf("Elf %d: solving problem on my own\n", id);
            pthread_mutex_unlock(&waiting_elves_mutex);
            continue;
        }

        while (waiting_elves_number < 3) {
            pthread_cond_wait(&waiting_elves_cond, &waiting_elves_mutex);
        }

        if (waiting_elves[2] == id) {
            printf("Elf %d: waking up Santa\n", id);
            pthread_cond_broadcast(&santa_sleeping_cond);
            pthread_cond_broadcast(&waiting_elves_cond);
        }

        pthread_mutex_unlock(&waiting_elves_mutex);
        printf("Elf %d: Getting help from Santa...\n", id);
        sleep(rand() % 2 + 1);
    }

}

void* santa_thread_routine() {
    while (1) {
        pthread_mutex_lock(&waiting_elves_mutex);
        while (waiting_elves_number < 3) {
            pthread_cond_wait(&waiting_elves_cond, &waiting_elves_mutex);
        }
        printf("Santa: helping elves %d %d %d...\n", waiting_elves[0], waiting_elves[1], waiting_elves[2]);
        pthread_mutex_unlock(&waiting_elves_mutex);
        sleep(rand() % 2 + 1);
        pthread_mutex_lock(&waiting_elves_mutex);
        printf("Santa: all elves helped\n");
        waiting_elves_number = 0;
        for (int i = 0; i < 3; i++) waiting_elves[i] = -1;
        pthread_mutex_unlock(&waiting_elves_mutex);

    }
}

int main() {
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        int* id = malloc(sizeof(int));
        id[0] = i;
        pthread_create(&elf_threads[i], NULL, elf_thread_routine, (void*) id);
    }
    pthread_t santa_id;
    pthread_create(&santa_id, NULL, santa_thread_routine, NULL);
    while(wait(NULL));
    return 0;
}

