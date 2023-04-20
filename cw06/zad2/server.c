#include "config.h"

mqd_t server_queue;
mqd_t clients_queues[MAXCLIENTS];
int active_clients_mask[MAXCLIENTS];
char clients_nicknames[MAXCLIENTS][MAXNICKSIZE];

Message out_message;

int send_to_client(Message* message) {
    return mq_send(clients_queues[message->receiver_id], (char*) message, sizeof(Message), message->mtype);

}

long receive_from_client(Message* message) {
    return mq_receive(server_queue, (char*) message, sizeof(Message), NULL);
}

void stop(int sig) {
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (active_clients_mask[i]) {
            Message mes = {STOP_SV, "", -1, i, time(NULL)};
            send_to_client(&mes);
        }
    }
    mq_close(server_queue);
    usleep(300);
    mq_unlink("/server_queue");
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
                    sprintf(buf, "/client_queue_%d", in_message->sender_id);
                    mqd_t sender_id = mq_open(buf, O_RDWR);
                    clients_queues[i] = sender_id;
                    active_clients_mask[i] = 1;
                    strcpy(clients_nicknames[i], in_message->contents);

                    out_message = (Message) {INIT_SV, "", -1, i, time(NULL)};
                    if (send_to_client(&out_message) == -1) perror("send_to_client");
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
    struct mq_attr attr = {0, 10, sizeof(Message), 0};
    server_queue = mq_open("/server_queue", O_RDWR | O_CREAT, 0666, &attr);
    if (server_queue == -1) {
        perror("mq_open");
        mq_close(server_queue);
        mq_unlink("/server_queue");
        exit(EXIT_FAILURE);
    }

    FILE* id_file = fopen("new_id", "w");
    fprintf(id_file, "100");
    fclose(id_file);

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