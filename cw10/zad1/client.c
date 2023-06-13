#include "config.h"

struct sockaddr_in server_address = {AF_INET, 0, {LOCALADDR}};
struct sockaddr_in client_address = {AF_INET, 0, {LOCALADDR}};
int client_socket;

pid_t poller_pid;

int id = -1;

ssize_t send_to_server(Message* message) {
    return write(client_socket, message, sizeof(Message));
}

long receive_from_server(Message* message) {
    return read(client_socket, message, sizeof(Message));
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
    Message mes = {STOP, "", id, -1, time(NULL)};
    send_to_server(&mes);
    close(client_socket);
    kill(poller_pid, SIGINT);
    exit(0);
}

void init(char* nickname) {
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(client_socket, (struct sockaddr*) &client_address, sizeof(client_address)) == -1) {
        perror("binding client socket");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    socklen_t sockaddr_len = sizeof(client_address);
    getsockname(client_socket, (struct sockaddr*) &client_address, &sockaddr_len);
    printf("client socket address: %s:%d\n", inet_ntoa(client_address.sin_addr), client_address.sin_port);

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("client socket connecting to server socket");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    Message message = {INIT, "", -1, -1, time(NULL), client_address};
    strcpy(message.contents, nickname);
    if (send_to_server(&message) == -1) {
        perror("initializing");
        exit(EXIT_FAILURE);
    }
    Message response = {};
    if (receive_from_server(&response) == -1) {
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
    while(1) {
        recv(client_socket, NULL, 1, MSG_PEEK);
        handle_new_message();
//        msgctl(client_msq, IPC_STAT, &queue_info);
//        if (queue_info.msg_qnum > 0) {
//        }
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
    char* ptr;
    if (argc != 4) {
        fprintf(stderr, "bad arguments\n");
        return EXIT_FAILURE;
    }
    if (strlen(argv[1]) > MAXNICKSIZE) {
        fprintf(stderr, "nickname too long\n");
        return EXIT_FAILURE;
    }
    if (inet_aton(argv[2], &server_address.sin_addr) == 0) {
        perror("invalid ip address");
        return EXIT_FAILURE;
    }
    int server_port = (int) strtol(argv[3], &ptr, 10);
    if (server_port < 0 || server_port > 65535 || strcmp(ptr, "") != 0) {
        fprintf(stderr, "invalid port\n");
        return EXIT_FAILURE;
    }
    server_address.sin_port = server_port;

    init(argv[1]);
    if ((poller_pid = fork()) == 0) {
        listen_to_messages();
    }
    chat_loop();

    return 0;
}
