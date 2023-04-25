#include "config.h"

key_t server_msq;
key_t client_msq;

pid_t poller_pid;

int id = -1;

int send_to_server(Message* message) {
    return msgsnd(server_msq, message, sizeof(Message) - sizeof(long), 0);
}

long receive_from_server(Message* message, long type) {
    return msgrcv(client_msq, message, sizeof(Message) - sizeof(long), type, 0);
}

void handle_new_message() {
    Message mes;
    receive_from_server(&mes, -20);
    switch (mes.mtype) {
        case STOP_SV:
            printf("\rServer stopped.\n");
            kill(getppid(), SIGINT);
            break;
        case MSG_SV:
            printf("\r%sNew message from %d:\n%s\n", ctime(&mes.time), mes.sender_id, mes.contents);
            break;
        case LIST_SV:
            printf("\r === LIST OF CLIENTS ===\n%s\n", mes.contents);
            break;
    }
}

void stop(int sig) {
    msgctl(client_msq, IPC_RMID, NULL);
    Message mes = {STOP, "", id, -1, time(NULL)};
    send_to_server(&mes);
    kill(poller_pid, SIGINT);
    exit(0);
}

void init(char* nickname) {
    server_msq = msgget(SERVERID, 0);
    client_msq = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    Message message = {INIT, "", client_msq, -1, time(NULL)};
    strcpy(message.contents, nickname);
    if (send_to_server(&message) == -1) {
        perror("initializing");
        exit(EXIT_FAILURE);
    }
    Message response = {};
    if (receive_from_server(&response, -10) == -1) {
        perror("receiving initialization response");
        exit(EXIT_FAILURE);
    }
    id = response.receiver_id;

    struct sigaction sa;
    sa.sa_handler = stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
}

int execute_command(char* cmd, char* args) {
    char* ptr;
    char* token;

    if (!cmd) {
        return 2;
    }
    if (strcmp(cmd, "LIST") == 0) {
        Message mes = {LIST, "", id, 0, time(NULL)};
        send_to_server(&mes);
        return 0;

    } else if (strcmp(cmd, "2ALL") == 0) {
        if (strlen(args) > MAXMSGSIZE) {
            fprintf(stderr, "message too long\n");
            return -1;
        }
        token = args;
        if (token == NULL) {
            fprintf(stderr, "empty message\n");
            return -1;
        }
        Message mes = {TO_ALL, "", id, -1, time(NULL)};
        while (token != NULL) {
            strcat(mes.contents, token);
            token = strtok(NULL, " \n");
            if (token != NULL) strcat(mes.contents, " ");
        }
        send_to_server(&mes);
        return 0;

    } else if (strcmp(cmd, "2ONE") == 0) {
        int receiver_id = (int) strtol(args, &ptr, 10);
        if (receiver_id < 0 || receiver_id > MAXCLIENTS || strcmp(ptr, "") != 0) {
            fprintf(stderr, "bad receiver id\n");
            return -1;
        }
        token = strtok(NULL, " \n");
        if (token == NULL) {
            fprintf(stderr, "empty message\n");
            return -1;
        }
        Message mes = {TO_ONE, "", id, receiver_id, time(NULL)};
        while (token != NULL) {
            strcat(mes.contents, token);
            token = strtok(NULL, " \n");
            if (token != NULL) strcat(mes.contents, " ");
        }
        send_to_server(&mes);
        return 0;

    } else if (strcmp(cmd, "STOP") == 0) {
        if (args != NULL) {
            fprintf(stderr, "bad arguments\n");
            return -1;
        }
        Message mes = {STOP, "", id, 0, time(NULL)};
        send_to_server(&mes);
        raise(SIGINT);
    } else {
        fprintf(stderr, "invalid command\n");
        return -1;
    }
    return 0;
}

void listen_to_messages() {
    struct msqid_ds queue_info;
    while(1) {
        msgctl(client_msq, IPC_STAT, &queue_info);
        if (queue_info.msg_qnum > 0) {
            handle_new_message();
        }
        usleep(10);
    }
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
        usleep(300);
        printf("]>> ");
    }
}

int main(int argc, char** argv) {
    if (argc > 2) {
        fprintf(stderr, "bad arguments\n");
        return 1;
    }
    if (argc == 2 && strlen(argv[1]) > MAXNICKSIZE) {
        fprintf(stderr, "nickname too long\n");
        return 1;
    }
    init(argc == 2 ? argv[1] : "");
    if ((poller_pid = fork()) == 0) {
        listen_to_messages();
    }
    chat_loop();

    return 0;
}
