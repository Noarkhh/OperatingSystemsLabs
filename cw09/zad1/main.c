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

int is_santa_helping = 0;
int is_santa_delivering = 0;
int is_santa_sleeping = 1;

int completed_deliveries = 0;

int waiting_elves[3] = {-1, -1, -1};
int waiting_elves_length = 0;

int waiting_reindeers[9] = {[0 ... 8] = -1};
int waiting_reindeers_length = 0;

pthread_mutex_t waiting_elves_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waiting_reindeers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_helping_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_delivering_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t santa_sleeping_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t waiting_elves_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t waiting_reindeers_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_helping_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_delivering_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_sleeping_cond = PTHREAD_COND_INITIALIZER;


int lock(pthread_mutex_t* mutex) {
    return pthread_mutex_lock(mutex);
}

int unlock(pthread_mutex_t* mutex) {
    return pthread_mutex_unlock(mutex);
}

int cwait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    return pthread_cond_wait(cond, mutex);
}

int broadcast(pthread_cond_t* cond) {
    return pthread_cond_broadcast(cond);
}

void stop() {
    printf("All deliveries finished, stopping...\n");
    exit(0);
}

void* reindeer_thread_routine(void* id_ptr) {
    int id = *(int*) id_ptr;
    while (1) {
        int vacation_time = rand() % 6 + 5;
        printf("Reinder %d: going on vacation for %d [s]...\n", id, vacation_time);
        sleep(vacation_time);

        lock(&waiting_reindeers_mutex);
        waiting_reindeers[waiting_reindeers_length] = id;
        waiting_reindeers_length++;
        printf("Reindeer %d: %d reindeers now waiting\n", id, waiting_reindeers_length);

        while (waiting_reindeers_length < 9) {
            cwait(&waiting_reindeers_cond, &waiting_reindeers_mutex);
        }

        if (waiting_reindeers[8] == id) {
            printf("Reindeer %d: waking up Santa\n", id);
            lock(&santa_delivering_mutex);
            is_santa_delivering = 1;
            unlock(&santa_delivering_mutex);
            lock(&santa_sleeping_mutex);
            is_santa_sleeping = 0;
            broadcast(&santa_sleeping_cond);
            unlock(&santa_sleeping_mutex);
        }

        unlock(&waiting_reindeers_mutex);

        printf("Reindeer %d: embarking for delivery...\n", id);
        lock(&santa_delivering_mutex);
        is_santa_delivering = 1;

        while (is_santa_delivering) {
            cwait(&santa_delivering_cond, &santa_delivering_mutex);
        }
        unlock(&santa_delivering_mutex);

        printf("Reindeer %d: finished delivering\n", id);
    }

}

void* elf_thread_routine(void* id_ptr) {
    int id = *(int*) id_ptr;
    while (1) {
        int work_time = rand() % 4 + 2;
        printf("Elf %d: working for %d [s]...\n", id, work_time);
        sleep(work_time);

        lock(&waiting_elves_mutex);
        if (waiting_elves_length < 3) {
            waiting_elves[waiting_elves_length] = id;
            waiting_elves_length++;
            printf("Elf %d: %d elves now waiting\n", id, waiting_elves_length);
        } else {
            printf("Elf %d: solving problem on my own\n", id);
            unlock(&waiting_elves_mutex);
            continue;
        }

        while (waiting_elves_length < 3) {
            cwait(&waiting_elves_cond, &waiting_elves_mutex);
        }

        if (waiting_elves[2] == id) {
            printf("Elf %d: waking up Santa\n", id);
            lock(&santa_helping_mutex);
            is_santa_helping = 1;
            unlock(&santa_helping_mutex);

            lock(&santa_sleeping_mutex);
            is_santa_sleeping = 0;
            broadcast(&santa_sleeping_cond);
            unlock(&santa_sleeping_mutex);
        }

        unlock(&waiting_elves_mutex);
        printf("Elf %d: Getting help from Santa...\n", id);
        lock(&santa_helping_mutex);
//        is_santa_helping = 1;

        while (is_santa_helping) {
            cwait(&santa_helping_cond, &santa_helping_mutex);
        }
        unlock(&santa_helping_mutex);

        printf("Elf %d: Got help from Santa\n", id);
    }

}

void santa_help() {
    lock(&waiting_elves_mutex);
    int help_time = rand() % 2 + 1;
    printf("Santa: helping elves %d %d %d for %d [s]...\n", waiting_elves[0], waiting_elves[1], waiting_elves[2], help_time);
    unlock(&waiting_elves_mutex);

    pthread_cond_broadcast(&waiting_elves_cond);

    sleep(help_time);
    lock(&waiting_elves_mutex);
    lock(&santa_helping_mutex);
    printf("Santa: all elves helped\n");

    waiting_elves_length = 0;
    for (int i = 0; i < 3; i++) waiting_elves[i] = -1;
    is_santa_helping = 0;

    broadcast(&santa_helping_cond);

    unlock(&waiting_elves_mutex);
    unlock(&santa_helping_mutex);
}

void santa_deliver() {
    lock(&waiting_reindeers_mutex);
    int delivery_time = rand() % 3 + 2;
    printf("Santa: embarking for delivery for %d [s]...\n", delivery_time);
    unlock(&waiting_reindeers_mutex);

    broadcast(&waiting_reindeers_cond);

    sleep(delivery_time);

    lock(&waiting_reindeers_mutex);
    lock(&santa_delivering_mutex);
    completed_deliveries++;
    printf("Santa: presents delivery %d complete\n", completed_deliveries);

    waiting_reindeers_length = 0;
    for (int i = 0; i < REINDEERS; i++) waiting_reindeers[i] = -1;
    is_santa_delivering = 0;

    broadcast(&santa_delivering_cond);

    unlock(&waiting_reindeers_mutex);
    unlock(&santa_delivering_mutex);
}

void* santa_thread_routine() {
    while (1) {
        lock(&santa_sleeping_mutex);
        while (is_santa_sleeping) {
            cwait(&santa_sleeping_cond, &santa_sleeping_mutex);
        }
        printf("Santa: woke up\n");
        unlock(&santa_sleeping_mutex);

        lock(&santa_delivering_mutex);
        lock(&santa_helping_mutex);
        if (is_santa_helping) {
            unlock(&santa_delivering_mutex);
            unlock(&santa_helping_mutex);
            santa_help();
        } else if (is_santa_delivering) {
            unlock(&santa_delivering_mutex);
            unlock(&santa_helping_mutex);
            santa_deliver();
        }

        lock(&santa_sleeping_mutex);
        lock(&santa_delivering_mutex);
        lock(&santa_helping_mutex);
        if (!is_santa_delivering && !is_santa_helping) is_santa_sleeping = 1;
        if (completed_deliveries == 3) {
            stop();
        }
        unlock(&santa_sleeping_mutex);
        unlock(&santa_delivering_mutex);
        unlock(&santa_helping_mutex);
    }
}


int main() {
    srand(time(NULL));
    for (int i = 0; i < 10; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&elf_threads[i], NULL, elf_thread_routine, (void*) id);
    }
    for (int i = 0; i < 9; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&reindeer_threads[i], NULL, reindeer_thread_routine, (void*) id);
    }
    pthread_t santa_id;
    pthread_create(&santa_id, NULL, santa_thread_routine, NULL);
    while(wait(NULL));
    return 0;
}

