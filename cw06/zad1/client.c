#include "config.h"

key_t server_msq;
key_t client_msq;

int id = -1;

int send_to_server(Message* message) {
    return msgsnd(server_msq, message, sizeof(Message) - sizeof(long), 0);
}

long receive_from_server(Message* message, long type) {
    return msgrcv(client_msq, message, sizeof(Message) - sizeof(long), type, 0);
}

void init() {
    server_msq = msgget(SERVERID, 0);
    client_msq = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    Message message = {INIT, "aa", client_msq, 0, time(NULL)};
    if (send_to_server(&message) == -1) {
        perror("initializing");
        exit(EXIT_FAILURE);
    }
    Message response = {INIT, "aa", client_msq, 0, time(NULL)};
    printf("sent %d %lu\n", client_msq, sizeof(message));
    sleep(2);
    if (receive_from_server(&response, -10) == -1) {
        perror("receiving initialization response");
        exit(EXIT_FAILURE);
    }
    perror("");
    printf("%ld, %s, %d, %d\n", response.mtype, response.contents, response.sender_id, response.receiver_id);
    id = response.receiver_id;
    printf("%d\n", id);
}


int execute_command(char* cmd, char* args) {
    char* ptr;

    if (!cmd) {
        return 2;
    }
    if (strcmp(cmd, "LIST") == 0) {
        Message mes = {LIST, "", id, 0, time(NULL)};
        send_to_server(&mes);
//        receive_from_server(&mes, LIST_SV);
//        printf("%s", mes.contents);
        return 0;

    } else if (strcmp(cmd, "2ALL") == 0) {
        if (strlen(args) > MSGCNTSIZE) {
            fprintf(stderr, "message too long\n");
            return -1;
        }
        Message mes = {TO_ALL, "", id, 0, time(NULL)};
        strcpy(mes.contents, args);
        send_to_server(&mes);
        return 0;

    } else if (strcmp(cmd, "2ONE") == 0) {
        int receiver_id = (int) strtol(args, &ptr, 10);
        if (receiver_id < 0 || receiver_id > MAXCLIENTS) {
            fprintf(stderr, "bad receiver id\n");
            return -1;
        }
        Message mes = {TO_ONE, "", id, receiver_id, time(NULL)};
        strcpy(mes.contents, ptr);
        send_to_server(&mes);
        return 0;

    } else if (strcmp(cmd, "STOP") == 0) {
        if (args != NULL) {
            fprintf(stderr, "bad arguments\n");
            return -1;
        }
        Message mes = {STOP, "", id, 0, time(NULL)};
        send_to_server(&mes);
    } else {
        fprintf(stderr, "invalid command\n");
        return -1;
    }
    return 0;
}


void chat_loop() {
    char buffer[50];
    char* cmd;
    char* args;

    printf("]>> ");
    while (fgets(buffer, 50, stdin)) {
        cmd = strtok(buffer, " \n");
        if (cmd != NULL) args = strtok(NULL, " \n");
        execute_command(cmd, args);
        struct msqid_ds queue_info;
        msgctl(client_msq, IPC_STAT, &queue_info);
        Message mes;
        usleep(1000);
        while (queue_info.msg_qnum > 0) {
            receive_from_server(&mes, -20);
            printf("%s", mes.contents);

            msgctl(client_msq, IPC_STAT, &queue_info);
        }
        printf("]>> ");
    }
}

int main() {
    init();
    chat_loop();

    return 0;
}
