#include <string.h>

#include "game.h"
#include "server.h"


void game_init(GameState *g) {
    if (!g) return;

    g->server_count = 0;

    /* create home server */
    server_init(&g->servers[0], 0, "home");
    g->server_count = 1;

    g->home_server = 0;
    g->current_server = 0;
}

void game_shutdown(GameState *g) {
    (void)g;
    /* nothing yet */
}

Server *game_get_server(GameState *g, ServerId id) {
    if (!g) return NULL;
    if (id < 0 || id >= g->server_count) return NULL;
    return &g->servers[id];
}

Server *game_get_current_server(GameState *g) {
    return game_get_server(g, g->current_server);
}
