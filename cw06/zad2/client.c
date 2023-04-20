#include <sys/ipc.h>
#include "config.h"

mqd_t server_queue;
mqd_t client_queue;
char queue_name[50];

pid_t poller_pid;

int id = -1;
int file_id;

int send_to_server(Message* message) {
    return mq_send(server_queue, (char*) message, sizeof(Message), message->mtype);
}

long receive_from_server(Message* message) {
    return mq_receive(client_queue, (char*) message, sizeof(Message), NULL);
}

void handle_new_message() {
    Message mes;
    receive_from_server(&mes);
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
    mq_close(client_queue);
    mq_unlink(queue_name);
    Message mes = {STOP, "", id, -1, time(NULL)};
    send_to_server(&mes);
    kill(poller_pid, SIGINT);
    exit(0);
}

void init(char* nickname) {
    struct mq_attr attr = {0, 10, sizeof(Message), 0};
    server_queue = mq_open("/server_queue", O_RDWR);

    char buf[3];
    FILE* id_file = fopen("new_id", "r");
    fread(buf, sizeof(char), 3, id_file);
    file_id = (int) strtol(buf, NULL, 10);
    fclose(id_file);
    id_file = fopen("new_id", "w");
    fprintf(id_file, "%d", file_id + 1);
    fclose(id_file);

    sprintf(queue_name, "/client_queue_%d", file_id);
    client_queue = mq_open(queue_name, O_RDWR | O_CREAT | O_EXCL, 0666, &attr);

    struct mq_attr queue_info;
    mq_getattr(client_queue, &queue_info);
//    printf("client queue: %d %ld %ld %lu\n", client_queue, queue_info.mq_msgsize, queue_info.mq_maxmsg, sizeof(Message));

    Message message = {INIT, "", file_id, -1, time(NULL)};
    strcpy(message.contents, nickname);
    if (send_to_server(&message) == -1) {
        perror("initializing");
        mq_close(client_queue);
        mq_unlink(queue_name);
        exit(EXIT_FAILURE);
    }
    Message response = {};
    mq_getattr(client_queue, &queue_info);
//    printf("pending messages: %ld\n", queue_info.mq_curmsgs);
    if (receive_from_server(&response) == -1) {
        perror("receiving initialization response");
        mq_close(client_queue);
        mq_unlink(queue_name);
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
    struct mq_attr queue_info;
    while(1) {
        mq_getattr(client_queue, &queue_info);
        if (queue_info.mq_curmsgs > 0) {
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
    if (strlen(argv[1]) > MAXNICKSIZE) {
        fprintf(stderr, "nickname too long\n");
        return 1;
    }
    init(argv[1]);
    if ((poller_pid = fork()) == 0) {
        listen_to_messages();
    }
    chat_loop();

    return 0;
}
