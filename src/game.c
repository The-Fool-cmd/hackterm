#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "server.h"

void game_generate_network(GameState *g) {
    if (!g) return;

    int num_new_servers = 250; // hardcoded for now
    ServerId new_id;

    // first server always connects to home
    char name_buf[32];
    snprintf(name_buf, sizeof(name_buf), "srv%d", 1);
    new_id = server_generate_random(g->servers, &g->server_count, name_buf);

    // link to home
    server_add_link(&g->servers[new_id], 0);
    server_add_link(&g->servers[0], new_id);

    for (int i = 0; i < num_new_servers; i++) {
        snprintf(name_buf, sizeof(name_buf), "srv%d", i+i);
        ServerId new_id = server_generate_random(g->servers, &g->server_count, name_buf);

        // link to one/two existing servers randomly
        int link_to = rand() % g->server_count;
        server_add_link(&g->servers[new_id], link_to);
        server_add_link(&g->servers[link_to], new_id);

        // second link for redundancy, maybe
        if (g->server_count > 2 && (rand() % 2)) {
            int extra_link = rand() % g->server_count;
            if (extra_link != new_id) {
                server_add_link(&g->servers[new_id], extra_link);
                server_add_link(&g->servers[extra_link], new_id);
            }
        }
    }
}

void game_init(GameState *g) {
    if (!g) return;

    g->server_count = 0;

    /* create home server */
    server_init(&g->servers[0], 0, "home");
    g->server_count = 1;
    g->home_server = 0;
    g->current_server = 0;

    // generate demo network
    game_generate_network(g);
}

/* Sends the shutdown signal */
void game_shutdown(GameState *g) {
    (void)g;
    /* nothing yet */
}

/* helper functions*/

/* Returns a pointer to the server with ServerId */
Server *game_get_server(GameState *g, ServerId id) {
    if (!g) return NULL;
    if (id < 0 || id >= g->server_count) return NULL;
    return &g->servers[id];
}

/* Returns a pointer to the server the player is connected to */
Server *game_get_current_server(GameState *g) {
    return game_get_server(g, g->current_server);
}

/* game commands */

/* Returns an array of linked servers*/
int game_scan(const GameState *g, ServerId *out, int max) {
    if (!g || !out) return 0;

    const Server *curr = &g->servers[g->current_server];
    int count = curr->link_count;

    if (count > max) count = max;

    for (int i = 0; i < count; i++) {
        out[i] = curr->links[i].to; // just indices
    }

    return count;
}

int game_connect(GameState *g, ServerId to) {
    if (!g) return 0;

    Server *curr = &g->servers[g->current_server];
    for (int i = 0; i < curr->link_count; i++) {
        if (curr->links[i].to == to) {
            g->current_server = to;
            return 1;
        }
    }
    return 0;
}

void game_save(const GameState *g, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "%d %d %d\n", g->server_count, g->home_server, g->current_server);

    for (int i = 0; i < g->server_count; i++) {
        const Server *s = &g->servers[i];
        fprintf(f, "%d %s %d %d %d\n", s->id, s->name, s->security, s->money, s->link_count);
        for (int j = 0; j < s->link_count; j++) {
            fprintf(f, "%d ", s->links[j].to);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}