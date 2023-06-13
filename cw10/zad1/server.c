#include "config.h"

int server_socket;
struct sockaddr_in server_address = {AF_INET, 5000, {LOCALADDR}};

struct sockaddr_in clients_addresses[MAXCLIENTS];
int active_clients_mask[MAXCLIENTS];
char clients_nicknames[MAXCLIENTS][MAXNICKSIZE];

Message out_message;

ssize_t send_to_client(Message* message) {
    return sendto(server_socket, message, sizeof(Message), 0,
                  (struct sockaddr*) &clients_addresses[message->receiver_id], sizeof(struct sockaddr_in));
}

long receive_from_client(Message* message) {
//    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    return read(server_socket, message, sizeof(Message));
}

void stop(int sig) {
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (active_clients_mask[i]) {
            Message mes = {STOP_SV, "", -1, i, time(NULL)};
            send_to_client(&mes);
        }
    }
    close(server_socket);
    exit(0);
}

void log_message(Message* mes) {
    FILE* logging_file = fopen("logs.txt", "a");
    fprintf(logging_file, "%s\t%ld\t\t%d\t\t%d\t\t\t%s\n", strtok(ctime(&mes->time), "\n"), mes->mtype, mes->sender_id, mes->receiver_id, mes->contents);
    fclose(logging_file);
}

void process_message(Message* in_message) {
    char list[MAXMSGSIZE] = "";
    char buf[50];
    switch (in_message->mtype) {
        case INIT:
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (!active_clients_mask[i]) {
                    clients_addresses[i] = in_message->sender_address;
                    active_clients_mask[i] = 1;
                    strcpy(clients_nicknames[i], in_message->contents);

                    out_message = (Message) {INIT_SV, "", -1, i, time(NULL)};
                    send_to_client(&out_message);
                    break;
                }
            }
            out_message = (Message) {INIT_SV, "", -1, -1, time(NULL)};
            break;
        case STOP:
            active_clients_mask[in_message->sender_id] = 0;
            break;
        case LIST:
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (active_clients_mask[i]) {
                    sprintf(buf, "ID: %d |name: %s\n", i, clients_nicknames[i]);
                    strcat(list, buf);
                }
            }
            out_message = (Message) {LIST_SV, "", -1, in_message->sender_id, time(NULL)};
            strcpy(out_message.contents, list);
            send_to_client(&out_message);
            break;
        case TO_ALL:
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (active_clients_mask[i] && i != in_message->sender_id) {
                    out_message = (Message) {MSG_SV, "", in_message->sender_id, i, time(NULL)};
                    strcpy(out_message.contents, in_message->contents);
                    send_to_client(&out_message);
                }
            }
            break;
        case TO_ONE:
            out_message = (Message) {MSG_SV, "", in_message->sender_id, in_message->receiver_id, time(NULL)};
            strcpy(out_message.contents, in_message->contents);
            send_to_client(&out_message);
            break;
    }
    log_message(in_message);
}

int main() {
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        perror("binding server socket");
        close(server_socket);
        return EXIT_FAILURE;
    }

    struct sigaction sa;
    sa.sa_handler = stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    FILE* logging_file = fopen("logs.txt", "a");
    time_t init_time = time(NULL);
    fprintf(logging_file, "======================== %s ========================\n", strtok(ctime(&init_time), "\n"));
    fprintf(logging_file, "date\t\t\t\t\t\ttype\tsender\treceiver\tcontents\n");
    fclose(logging_file);

    Message mes;
    while (1) {
        receive_from_client(&mes);
        process_message(&mes);
    }
}