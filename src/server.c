#include <string.h>

#include "server.h"

void server_init(Server *s, ServerId id, const char *name) {
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

int server_add_link(Server *s, ServerId to) {
    if (!s) return -1;
    if (s->link_count >= SERVER_MAX_LINKS) return -1;

    // prevent duplicate links
    for (int i = 0; i < s->link_count; i++) {
        if (s->links[i].to == to) {
            return 0; // already linked
        }
    }

    s->links[s->link_count].to = to;
    s->link_count++;

    return 0;
}

int server_link_bidirectional(Server *a, Server *b) {
    if (!a || !b) return -1;
    if (a->id == b->id) return -1;

    if (server_add_link(a, b->id) != 0) return -1;
    if (server_add_link(b, a->id) != 0) return -1;
   
}
