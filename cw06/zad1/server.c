#include "config.h"

key_t server_msq;
key_t clients_msqs[MAXCLIENTS];
int active_clients_mask[MAXCLIENTS];
char clients_nicknames[MAXCLIENTS][MAXNICKSIZE];

Message out_message;
FILE* logging_file;

int send_to_client(Message* message) {
    return msgsnd(clients_msqs[message->receiver_id], message, sizeof(Message) - sizeof(long), 0);
}

long receive_from_client(Message* message) {
    return msgrcv(server_msq, message, sizeof(Message) - sizeof(long), -10, 0);
}

void stop(int sig) {
    for (int i = 0; i < MAXCLIENTS; i++) {
        if (active_clients_mask[i]) {
            Message mes = {STOP_SV, "", -1, i, time(NULL)};
            send_to_client(&mes);
        }
    }
    msgctl(server_msq, IPC_RMID, NULL);
    exit(0);
}

void log_message(Message* mes) {
    fprintf(logging_file, "%s\t%ld\t%d\t%d\t%s\n", ctime(&mes->time), mes->mtype, mes->sender_id, mes->receiver_id, mes->contents);
}

void process_message(Message* in_message) {
    switch (in_message->mtype) {
        case INIT:
            printf("processing init\n");
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (!active_clients_mask[i]) {
                    clients_msqs[i] = in_message->sender_id;
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
            printf("processing stop\n");
            active_clients_mask[in_message->sender_id] = 0;
            break;
        case LIST:
            printf("processing list\n");
            char list[MAXMSGSIZE] = "";
            char buf[50];
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (active_clients_mask[i]) {
                    sprintf(buf, "ID: %d name: %s\n", i, clients_nicknames[i]);
                    strcat(list, buf);
                }
            }
            out_message = (Message) {LIST_SV, "", -1, in_message->sender_id, time(NULL)};
            strcpy(out_message.contents, list);
            send_to_client(&out_message);
            break;
        case TO_ALL:
            printf("processing to all\n");
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (active_clients_mask[i] && i != in_message->sender_id) {
                    out_message = (Message) {MSG_SV, "", in_message->sender_id, i, time(NULL)};
                    strcpy(out_message.contents, in_message->contents);
                    send_to_client(&out_message);
                }
            }
            break;
        case TO_ONE:
            printf("processing to one\n");
            out_message = (Message) {MSG_SV, "", in_message->sender_id, in_message->receiver_id, time(NULL)};
            strcpy(out_message.contents, in_message->contents);
            send_to_client(&out_message);
            break;
    }
    log_message(in_message);
}

int main() {
    server_msq = msgget(SERVERID, IPC_CREAT | 0666);
    struct sigaction sa;
    sa.sa_handler = stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    logging_file = fopen("logs.txt", "w");

    time_t init_time = time(NULL);
    fprintf(logging_file, "============ %s ============\n", ctime(&init_time));
    fprintf(logging_file, "date\t\ttype\tsender_id\treceiver_id\tcontents\n");

    Message mes;
    while (1) {
        receive_from_client(&mes);
        process_message(&mes);
    }
//    msgctl(msq_id, IPC_RMID, NULL);
    return 0;
}