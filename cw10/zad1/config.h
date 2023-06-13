#ifndef ZAD1_CONFIG_H
#define ZAD1_CONFIG_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define SERVERID 21
#define MAXMSGSIZE 1028
#define MAXCLIENTS 128
#define MAXNICKSIZE 32
#define LOCALADDR 0x0100007f

enum ClientCommand {
    ZERO, STOP, LIST, TO_ALL, TO_ONE, INIT
};

enum ServerCommand {
    ZERO_SV, INIT_SV, MSG_SV, STOP_SV, LIST_SV
};

typedef struct Message {
    long mtype;
    char contents[MAXMSGSIZE];
    int sender_id;
    int receiver_id;
    time_t time;
    struct sockaddr_in sender_address;
} Message;


#endif //ZAD1_CONFIG_H
