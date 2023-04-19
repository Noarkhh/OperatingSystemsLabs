#include "config.h"

key_t server_msq;
key_t clients_msqs[MAXCLIENTS];
int active_clients_mask[MAXCLIENTS];

Message out_message;

int send_to_client(Message* message) {
    return msgsnd(clients_msqs[message->receiver_id], message, sizeof(Message) - sizeof(long), 0);
}

long receive_from_client(Message* message) {
    return msgrcv(server_msq, message, sizeof(Message) - sizeof(long), -10, 0);
}

void process_message(Message* in_message) {
    switch (in_message->mtype) {
        case INIT:
            printf("processing init\n");
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (!active_clients_mask[i]) {
                    clients_msqs[i] = in_message->sender_id;
                    active_clients_mask[i] = 1;
                    out_message = (Message) {INIT_SV, "", -1, i, time(NULL)};
                    send_to_client(&out_message);
                    break;
                }
            }
            out_message = (Message) {INIT_SV, "", -1, -1, time(NULL)};
            break;
        case STOP:
            printf("processing stop\n");
            break;
        case LIST:
            printf("processing list\n");
            out_message = (Message) {LIST_SV, "listing...\n", -1, in_message->sender_id, time(NULL)};
            send_to_client(&out_message);
            break;
        case TO_ALL:
            printf("processing to all\n");
            for (int i = 0; i < MAXCLIENTS; i++) {
                if (active_clients_mask[i]) {
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
}

int main() {
    server_msq = msgget(SERVERID, IPC_CREAT | 0666);
    Message mes;
    while (1) {
        receive_from_client(&mes);
        process_message(&mes);
    }
//    msgctl(msq_id, IPC_RMID, NULL);
    return 0;
}