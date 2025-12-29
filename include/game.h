#ifndef GAME_H
#define GAME_H

#include "server.h"

#define MAX_SERVERS 64

typedef struct {
    Server servers[MAX_SERVERS];
    int server_count;

    ServerId current_server;
    ServerId home_server;
} GameState;

/* lifecycle */
void game_init(GameState *g);
void game_shutdown(GameState *g);

/* helpers */
Server *game_get_current_server(GameState *g);
Server *game_get_server(GameState *g, ServerId id);

#endif
