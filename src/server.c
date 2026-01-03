#include <string.h>
#include <stdlib.h>

#include "server.h"
#include "game.h"
#include "core_result.h"

/* Initialises a server with a given name and sets default values */
void server_init(Server* s, ServerId id, const char* name) {
    if (!s) return;

    s->id = id;

    if (name) {
	strncpy(s->name, name, SERVER_NAME_LEN - 1);
	s->name[SERVER_NAME_LEN - 1] = '\0';
    } else {
	s->name[0] = '\0';
    }

    // Ser defaults
    s->security = 1;
    s->money = 0;

    // topology
    s->link_count = 0;
    // unused as of now
    s->subnet_id = -1;
}

/* Generates a random server and returns its Id */
ServerId server_generate_random(Server* servers, int* server_count, const char* name) {
    if (!servers || !server_count || *server_count >= MAX_SERVERS) return SERVER_INVALID_ID;
    ServerId id = *server_count;
    server_init(&servers[id], id, name);

    // Random stats for testing
    servers[id].security = 1 + rand() % 10;  // 1-10
    servers[id].money = 100 + rand() % 900;  // 100-999

    (*server_count)++;
    return id;
}

/* Links first server to second */
CoreResult server_add_link(Server* s, ServerId to) {
    if (!s) return CORE_ERR_INVALID_ARG;
    if (s->link_count >= SERVER_MAX_LINKS) return CORE_ERR_UNKNOWN;

    // prevent duplicate links
    for (int i = 0; i < s->link_count; i++) {
	if (s->links[i].to == to) {
	    return CORE_OK;  // already linked, treat as success
	}
    }

    s->links[s->link_count].to = to;
    s->link_count++;

    return CORE_OK;
}

/* Untested bidirectional link */
CoreResult server_link_bidirectional(Server* a, Server* b) {
    if (!a || !b) return CORE_ERR_INVALID_ARG;
    if (a->id == b->id) return CORE_ERR_INVALID_ARG;

    if (server_add_link(a, b->id) != CORE_OK) return CORE_ERR_UNKNOWN;
    if (server_add_link(b, a->id) != CORE_OK) return CORE_ERR_UNKNOWN;

    return CORE_OK;
}

/* Returns an array of asdfasfdasdfa*/
const Server* server_get_linked(const Server* s, const Server* all, int index) {
    if (!s || index < 0 || index >= s->link_count) return NULL;

    ServerId id = s->links[index].to;
    return &all[id];
}
