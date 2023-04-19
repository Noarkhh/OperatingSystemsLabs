#ifndef ZAD1_CONFIG_H
#define ZAD1_CONFIG_H

#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define SERVERID 21
#define MSGCNTSIZE 256
#define MAXCLIENTS 128

enum ClientCommand {
    ZERO, STOP, LIST, TO_ALL, TO_ONE, INIT
};

enum ServerCommand {
    ZERO_SV, INIT_SV, MSG_SV, STOP_SV, LIST_SV
};

typedef struct Message {
    long mtype;
    char contents[MSGCNTSIZE];
    int sender_id;
    int receiver_id;
    time_t time;
} Message;


#endif //ZAD1_CONFIG_H
