#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define BARBERS 5
#define SEATS 3
#define WAITING 5
#define HAIRSTYLES 3


typedef struct Barber {
    int id;
    int seat;
} Barber;

typedef struct Client {
    int id;
    int hairstyle;
    int seat;
} Client;

typedef union Person {
    Barber barber;
    Client client;
} Person;

typedef struct Seat {
    Barber barber;
    Client client;
    int is_empty;
} Seat;

typedef struct Seats {
    int total;
    int free;
    sem_t* sem;
    Seat elems[SEATS];
} Seats;

typedef struct Queue {
    int cap;
    int head;
    int tail;
    int size;
    sem_t* sem;
    Person elems[BARBERS + WAITING];
} Queue;

Queue* barber_queue;
Barber* barbers;
int barber_sems;
Queue* client_queue;
Seats* seats;

int dec(int semid, unsigned short semnum, short sem_flg) {
    struct sembuf buf = {semnum, -1, sem_flg};
    return semop(semid, &buf, 1);
}

int inc(int semid, unsigned short semnum, short sem_flg) {
    struct sembuf buf = {semnum, 1, sem_flg};
    return semop(semid, &buf, 1);
}

void qinit(Queue* q, int size) {
    q->cap = size;
    q->head = 0;
    q->tail = 0;
    q->size = 0;
    q->sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(q->sem, 0, SETVAL, 1);
}

int qput(Queue* q, Person person) {
    dec(q->sem, 0, 0);
    if (q->size == q->cap) {
        inc(q->sem, 0, 0);
        return -1;
    }
    q->elems[q->head] = person;
    q->head = (q->head + 1) % q->cap;
    q->size++;
    inc(q->sem, 0, 0);
    return 0;
}

Person qget(Queue* q) {
    dec(q->sem, 0, 0);
    if (q->size == 0) {
        inc(q->sem, 0, 0);
        return *(volatile Person*) NULL;
    }
    Person p = q->elems[q->tail];
    q->tail = (q->tail + 1) % q->cap;
    q->size--;
    inc(q->sem, 0, 0);
    return p;
}

void qprint(Queue* q) {
    printf("h: %d, t: %d, s: %d\n", q->head, q->tail, q->size);
}


void barber_process(int id, Barber barber) {
    qput(barber_queue, (Person) barber);
    while (1) {
        dec(barber_sems, id, 0);
        printf("barber %d woke up! Currently on seat %d servicing client %d with hairstyle %d\n", id, barbers[id].seat, seats->elems[barbers[id].seat].client.id, seats->elems[barbers[id].seat].client.hairstyle);
        sleep(seats->elems[barbers[id].seat].client.hairstyle);
        printf("barber %d finished barbing client %d and is going back to sleep\n", id, seats->elems[barbers[id].seat].client.id);
        qput(barber_queue, (Person) barber);
        dec(seats->sem, 0, 0);
        seats->elems[barbers[id].seat].is_empty = 1;
        seats->free++;
        inc(seats->sem, 0, 0);
    }
}

void wake_barber(int id) {
    inc(barber_sems, id, 0);
}

void coordinator_process() {
//    printf("starting coordination!\n");
    while (1) {
        if (client_queue->size == 0 || seats->free == 0 || barber_queue->size == 0) continue;
//        qprint(client_queue);
//        qprint(barber_queue);
        Client client = qget(client_queue).client;

        Barber* barber = &barbers[qget(barber_queue).barber.id];
        for (int i = 0; i < SEATS; i++) {
            if (!seats->elems[i].is_empty) continue;
//            printf("coordinator: assigning barber %d to client %d\n", barber->id, client.id);
            dec(seats->sem, 0, 0);
            seats->elems[i].is_empty = 0;

            barber->seat = i;
            seats->elems[i].barber = *barber;

            client.seat = i;
            seats->elems[i].client = client;
            seats->free--;
            inc(seats->sem, 0, 0);
            wake_barber(barber->id);
            break;
        }
    }
}

void client_generator_process() {
    int client_id = 0;
    while (1) {
        sleep(1);
        if (rand() % 2 == 1) {
            Client c = {client_id, rand() % HAIRSTYLES + 1, -1};
            qput(client_queue, (Person) c);
            client_id++;
        }
    }
}

pid_t barber_pids[BARBERS];
pid_t coordinator_pid;
pid_t generator_pid;

void stop(int pid) {

    for (int i = 0; i < BARBERS; i++) {
        kill(barber_pids[i], SIGINT);
    }
    kill(coordinator_pid, SIGINT);
    kill(generator_pid, SIGINT);
    exit(0);
}


int main() {
    srand(time(NULL));

    signal(SIGINT, stop);

    barber_queue = (Queue*) shmat(shmget(IPC_PRIVATE, sizeof(Queue), IPC_CREAT | 0666), NULL, 0);
    qinit(barber_queue, BARBERS + WAITING);

    barbers = (Barber*) shmat(shmget(IPC_PRIVATE, sizeof(Barber) * BARBERS, IPC_CREAT | 0666), NULL, 0);

    client_queue = (Queue*) shmat(shmget(IPC_PRIVATE, sizeof(Queue), IPC_CREAT | 0666), NULL, 0);
    qinit(client_queue, BARBERS + WAITING);

    seats = (Seats*) shmat(shmget(IPC_PRIVATE, sizeof(Seats), IPC_CREAT | 0666), NULL, 0);
    seats->total = SEATS;
    seats->free = SEATS;
    for (int i = 0; i < SEATS; i++) {
        seats->elems[i].is_empty = 1;
    }
    seats->sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    seats->sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(seats->sem, 0, SETVAL, 1);

    barber_sems = semget(IPC_PRIVATE, BARBERS, IPC_CREAT | 0666);

    for (int i = 0; i < BARBERS; i++) {

        semctl(barber_sems, i, SETVAL, 0);
        barber_pids[i] = fork();
        if (barber_pids[i] == 0) {
            Barber barber = {i, -1};
            barbers[i] = barber;
            barber_process(i, barber);
        }
    }

    coordinator_pid = fork();
    if (coordinator_pid == 0) coordinator_process();
    generator_pid = fork();
    if (generator_pid == 0) client_generator_process();

    while(wait(NULL));
    return 0;
}

