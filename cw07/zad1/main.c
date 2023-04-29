#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
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
    int sem;
    Seat elems[SEATS];
} Seats;

typedef struct Queue {
    int cap;
    int head;
    int tail;
    int size;
    int sem;
    Person elems[BARBERS + WAITING];
} Queue;


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

Queue* barber_queue;
Barber* barbers;
int barber_sems;
Queue* client_queue;
Seats* seats;

void barber_process(int id, Barber barber) {
    qput(barber_queue, (Person) barber);
    while (1) {
        dec(barber_sems, id, 0);
        printf("barber %d woke up! Currently on seat %d servicing client %d with hairstyle %d\n", id, barbers[id].seat, seats->elems[barbers[id].seat].client.id, seats->elems[barbers[id].seat].client.hairstyle);
        sleep(seats->elems[barbers[id].seat].client.hairstyle + 1);
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
    printf("starting coordination!\n");
    while (1) {
        if (client_queue->size == 0 || seats->free == 0 || barber_queue->size == 0) continue;
//        qprint(client_queue);
//        qprint(barber_queue);
        Client client = qget(client_queue).client;

        Barber* barber = &barbers[qget(barber_queue).barber.id];
        for (int i = 0; i < SEATS; i++) {
            if (!seats->elems[i].is_empty) continue;
            printf("coordinator: assigning barber %d to client %d\n", barber->id, client.id);
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
            Client c = {client_id, rand() % 3, -1};
            qput(client_queue, (Person) c);
            client_id++;
        }
    }
}

int main() {
    srand(time(NULL));

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
    semctl(seats->sem, 0, SETVAL, 1);

    barber_sems = semget(IPC_PRIVATE, BARBERS, IPC_CREAT | 0666);
    pid_t barber_pids[BARBERS];

    for (int i = 0; i < BARBERS; i++) {

        semctl(barber_sems, i, SETVAL, 0);
        barber_pids[i] = fork();
        if (barber_pids[i] == 0) {
            Barber barber = {i, -1};
            barbers[i] = barber;
            barber_process(i, barber);
        }
    }

    pid_t coordinator_pid = fork();
    if (coordinator_pid == 0) coordinator_process();
    pid_t generator_pid = fork();
    if (generator_pid == 0) client_generator_process();

    while(wait(NULL));
//    sleep(100);
//    sleep(2);
//    sleep(1);
//    printf("barber queue size: %d\n", barber_queue->size);
//    struct sembuf buf  = {0, 1, 0};
//    semop(barber_sems, &buf, 1);
//    usleep(100);
//    buf.sem_num = 1;
//    semop(barber_sems, &buf, 1);
//    usleep(100);
//    printf("%d\n", barber_queue->elems[1].barber.id);
//    Barber b = qget(barber_queue).barber;
//    printf("barber %d\n", b.id);
//    qprint(barber_queue);
//    usleep(50);
//    qget(barber_queue);
//    qget(barber_queue);
//    b = qget(barber_queue).barber;
//    printf("barber %d\n", b.id);
//    qprint(barber_queue);
//    usleep(300);

    for (int i = 0; i < BARBERS; i++) {
        kill(barber_pids[i], SIGINT);
    }
    kill(coordinator_pid, SIGINT);
    kill(generator_pid, SIGINT);
    return 0;
}

