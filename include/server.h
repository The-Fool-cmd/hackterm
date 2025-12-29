#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>

#define SERVER_NAME_LEN 32
#define SERVER_MAX_LINKS 16

typedef int ServerId;
#define SERVER_INVALID_ID (-1)

typedef struct {
    ServerId to;
} ServerLink;

typedef struct {
    ServerId id;
    char name[SERVER_NAME_LEN];

    // gameplay stats
    int security;
    int money;

    // topology
    ServerLink links[SERVER_MAX_LINKS];
    int link_count;

    // future-proofing
    int subnet_id;   // unused for now
} Server;

const Server* server_get_linked(
    const Server *s,
    const Server *all,
    int index
);

void server_init(Server *s, ServerId id, const char *name);
int  server_add_link(Server *s, ServerId to);
int server_link_bidirectional(Server *a, Server *b);
ServerId server_generate_random(Server *servers, int *server_count, const char *name);



#endif