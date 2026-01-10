#include <string.h>
#include <stdlib.h>

#include "server.h"
#include "game.h"
#include "core_result.h"

#include <ctype.h>

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
    s->service_count = 0;
    // unused as of now
    s->subnet_id = -1;
    s->type = SERVER_TYPE_UNKNOWN;
}

const char* server_type_to_string(ServerType t) {
    switch (t) {
        case SERVER_TYPE_ISP: return "isp";
        /* POP removed */
        case SERVER_TYPE_AREA: return "area";
        case SERVER_TYPE_NEIGHBORHOOD: return "neighborhood";
        case SERVER_TYPE_BUILDING: return "building_switch";
        case SERVER_TYPE_FLOOR: return "floor_switch";
        case SERVER_TYPE_ROUTER: return "distribution_router";
        case SERVER_TYPE_TOR: return "access_switch";
        case SERVER_TYPE_RACK: return "rack_switch";
        case SERVER_TYPE_BACKBONE: return "backbone";
        case SERVER_TYPE_USER: return "apartment_router"; 
        case SERVER_TYPE_HOST: return "host"; 
        /* legacy logical roles removed */
        default: return "unknown";
    }
}

ServerType server_type_from_string(const char* s) {
    if (!s) return SERVER_TYPE_UNKNOWN;
    /* compare lowercased */
    char buf[32];
    int i = 0;
    while (s[i] && i < (int)sizeof(buf)-1) { buf[i] = (char)tolower((unsigned char)s[i]); i++; }
    buf[i] = '\0';

    if (strcmp(buf, "isp") == 0) return SERVER_TYPE_ISP;
    if (strcmp(buf, "area") == 0) return SERVER_TYPE_AREA;
    if (strcmp(buf, "neighborhood") == 0) return SERVER_TYPE_NEIGHBORHOOD;
    /* accept both new display strings and legacy short names for compatibility */
    if (strcmp(buf, "building") == 0 || strcmp(buf, "building_switch") == 0) return SERVER_TYPE_BUILDING;
    if (strcmp(buf, "floor") == 0 || strcmp(buf, "floor_switch") == 0) return SERVER_TYPE_FLOOR;
    if (strcmp(buf, "distribution_router") == 0 || strcmp(buf, "distribution") == 0 || strcmp(buf, "router") == 0) return SERVER_TYPE_ROUTER;
    if (strcmp(buf, "access_switch") == 0 || strcmp(buf, "tor") == 0 || strcmp(buf, "toswitch") == 0) return SERVER_TYPE_TOR;
    if (strcmp(buf, "rack") == 0 || strcmp(buf, "rack_switch") == 0) return SERVER_TYPE_RACK;
    if (strcmp(buf, "backbone") == 0) return SERVER_TYPE_BACKBONE;
    if (strcmp(buf, "user") == 0 || strcmp(buf, "apartment_router") == 0 || strcmp(buf, "cpe") == 0) return SERVER_TYPE_USER;
    if (strcmp(buf, "host") == 0) return SERVER_TYPE_HOST;
    /* Accept legacy logical role names and map them to HOST so older
     * exports remain loadable even though those logical types were removed.
     */
    if (strcmp(buf, "web") == 0) return SERVER_TYPE_HOST;
    if (strcmp(buf, "app") == 0) return SERVER_TYPE_HOST;
    if (strcmp(buf, "db") == 0) return SERVER_TYPE_HOST;
    if (strcmp(buf, "cache") == 0) return SERVER_TYPE_HOST;
    if (strcmp(buf, "infra") == 0) return SERVER_TYPE_HOST;
    if (strcmp(buf, "honeypot") == 0) return SERVER_TYPE_HOST;

    return SERVER_TYPE_UNKNOWN;
}

/* Generates a random server and returns its Id */
ServerId server_generate_random(Server* servers, int* server_count, const char* name) {
    if (!servers || !server_count || *server_count >= MAX_SERVERS) return SERVER_INVALID_ID;
    ServerId id = *server_count;
    server_init(&servers[id], id, name);

    // Random stats for testing
    servers[id].security = 1 + rand() % 10;  // 1-10
    servers[id].money = 100 + rand() % 900;  // 100-999

    /* Default generated servers are generic hosts. */
    servers[id].type = SERVER_TYPE_HOST;
    /* Specialized roles (web/app/db/etc.) were removed to keep backend types consistent
     * with the simplified topology-focused model.
     */

    /* Simple service pool to give servers some variety */
    struct { int port; const char* name; int base_vuln; } pool[] = {
        {22, "ssh", 1},
        {80, "http", 2},
        {443, "https", 1},
        {3306, "mysql", 4},
        {6379, "redis", 6},
        {11211, "memcached", 6},
        {21, "ftp", 5},
        {8080, "http-alt", 3},
    };
    const int pool_n = sizeof(pool) / sizeof(pool[0]);

    /* decide number of services (0..2) */
    int svc_count = rand() % 3;
    if (svc_count > MAX_SERVICES_PER_SERVER) svc_count = MAX_SERVICES_PER_SERVER;
    for (int i = 0; i < svc_count; i++) {
        int pick = rand() % pool_n;
        servers[id].services[i].port = pool[pick].port;
        servers[id].services[i].vuln_level = pool[pick].base_vuln + (rand() % 3); /* small variance */
        strncpy(servers[id].services[i].name, pool[pick].name, SERVICE_NAME_LEN - 1);
        servers[id].services[i].name[SERVICE_NAME_LEN - 1] = '\0';
    }
    servers[id].service_count = svc_count;

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
