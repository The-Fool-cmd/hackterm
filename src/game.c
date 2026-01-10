#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "server.h"
#include "core_result.h"
#include "cJSON.h"
#include "generator.h"

void game_generate_network(GameState* g) {
    /* Preserve behavior: use HACKTERM_SEED if set, else 0 to use global RNG */
    const char* seed_env = getenv("HACKTERM_SEED");
    unsigned int seed = 0;
    if (seed_env) seed = (unsigned int)atoi(seed_env);
    generator_generate_city(g, seed);
}

void game_init(GameState* g) {
    if (!g) return;

    g->server_count = 0;

    /* create home server */
    server_init(&g->servers[0], 0, "home");
    g->server_count = 1;
    g->home_server = 0;
    g->current_server = 0;
    g->tick = 0;

    // generate demo network
    game_generate_network(g);
}

/* Sends the shutdown signal */
void game_shutdown(GameState* g) {
    (void)g;
    /* nothing yet */
}

/* helper functions*/

/* Returns a pointer to the server with ServerId */
Server* game_get_server(GameState* g, ServerId id) {
    if (!g) return NULL;
    if (id < 0 || id >= g->server_count) return NULL;
    return &g->servers[id];
}

/* Returns a pointer to the server the player is connected to */
Server* game_get_current_server(GameState* g) {
    return game_get_server(g, g->current_server);
}

/* game commands */

/* Returns an array of linked servers*/
int game_scan(const GameState* g, ServerId* out, int max) {
    if (!g || !out) return 0;

    const Server* curr = &g->servers[g->current_server];
    int count = curr->link_count;

    if (count > max) count = max;

    for (int i = 0; i < count; i++) {
	out[i] = curr->links[i].to;  // just indices
    }

    return count;
}

CoreResult game_connect(GameState* g, ServerId to) {
    if (!g) return CORE_ERR_INVALID_ARG;

    Server* curr = &g->servers[g->current_server];
    for (int i = 0; i < curr->link_count; i++) {
	if (curr->links[i].to == to) {
	    g->current_server = to;
	    return CORE_OK;
	}
    }
    return CORE_ERR_NOT_LINKED;
}

bool game_save(const GameState* g, const char* filename) {
    if (!g || !filename) return false;

    char tmpfile[512];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", filename);

    cJSON* root = cJSON_CreateObject();
    if (!root) return false;
    cJSON_AddNumberToObject(root, "version", 1);

    cJSON* game = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "game", game);
    cJSON_AddNumberToObject(game, "server_count", g->server_count);
    cJSON_AddNumberToObject(game, "home_server", g->home_server);
    cJSON_AddNumberToObject(game, "current_server", g->current_server);

    cJSON* servers = cJSON_CreateArray();
    cJSON_AddItemToObject(game, "servers", servers);

    for (int i = 0; i < g->server_count; i++) {
        const Server* s = &g->servers[i];
        cJSON* sObj = cJSON_CreateObject();
        cJSON_AddNumberToObject(sObj, "id", s->id);
        cJSON_AddStringToObject(sObj, "name", s->name);
        cJSON_AddNumberToObject(sObj, "security", s->security);
        cJSON_AddNumberToObject(sObj, "money", s->money);
        /* store string type name under the stable "type" key */
        cJSON_AddStringToObject(sObj, "type", server_type_to_string(s->type));
        cJSON_AddNumberToObject(sObj, "subnet", s->subnet_id);

        cJSON* links = cJSON_CreateArray();
        for (int j = 0; j < s->link_count; j++) {
            cJSON_AddItemToArray(links, cJSON_CreateNumber(s->links[j].to));
        }
        cJSON_AddItemToObject(sObj, "links", links);

        cJSON* sArr = cJSON_CreateArray();
        for (int j = 0; j < s->service_count; j++) {
            cJSON* svc = cJSON_CreateObject();
            cJSON_AddStringToObject(svc, "name", s->services[j].name);
            cJSON_AddNumberToObject(svc, "port", s->services[j].port);
            cJSON_AddNumberToObject(svc, "vuln", s->services[j].vuln_level);
            cJSON_AddItemToArray(sArr, svc);
        }
        cJSON_AddItemToObject(sObj, "services", sArr);

        cJSON_AddItemToArray(servers, sObj);
    }

    char* out = cJSON_PrintUnformatted(root);
    if (!out) { cJSON_Delete(root); return false; }

    FILE* f = fopen(tmpfile, "w");
    if (!f) { free(out); cJSON_Delete(root); return false; }
    fwrite(out, 1, strlen(out), f);
    fwrite("\n", 1, 1, f);
    fclose(f);
    free(out);
    cJSON_Delete(root);

    if (rename(tmpfile, filename) != 0) {
        remove(tmpfile);
        return false;
    }
    return true;
}

/* Load using cJSON for robustness */
bool game_load(GameState* g, const char* filename) {
    if (!g || !filename) return false;
    FILE* f = fopen(filename, "r");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return false; }
    char* buf = malloc(sz + 1);
    if (!buf) { fclose(f); return false; }
    if (fread(buf, 1, sz, f) != (size_t)sz) { free(buf); fclose(f); return false; }
    buf[sz] = '\0';
    fclose(f);

    cJSON* root = cJSON_Parse(buf);
    free(buf);
    if (!root) return false;

    cJSON* game = cJSON_GetObjectItem(root, "game");
    if (!game) { cJSON_Delete(root); return false; }

    cJSON* sc = cJSON_GetObjectItem(game, "server_count");
    cJSON* hs = cJSON_GetObjectItem(game, "home_server");
    cJSON* cs = cJSON_GetObjectItem(game, "current_server");
    cJSON* servers = cJSON_GetObjectItem(game, "servers");
    if (!sc || !servers) { cJSON_Delete(root); return false; }

    int server_count = (int)sc->valuedouble;
    int home_server = hs ? (int)hs->valuedouble : 0;
    int current_server = cs ? (int)cs->valuedouble : 0;

    if (server_count <= 0 || server_count > MAX_SERVERS) { cJSON_Delete(root); return false; }

    g->server_count = 0;
    g->home_server = home_server;
    g->current_server = current_server;
    g->tick = 0;

    int arr_len = cJSON_GetArraySize(servers);
    for (int i = 0; i < arr_len; i++) {
        cJSON* sObj = cJSON_GetArrayItem(servers, i);
        if (!sObj) continue;
        cJSON* jid = cJSON_GetObjectItem(sObj, "id");
        cJSON* jname = cJSON_GetObjectItem(sObj, "name");
        cJSON* jsec = cJSON_GetObjectItem(sObj, "security");
        cJSON* jmoney = cJSON_GetObjectItem(sObj, "money");
        cJSON* jtype = cJSON_GetObjectItem(sObj, "type");
        cJSON* jsub = cJSON_GetObjectItem(sObj, "subnet");

        int id = jid ? (int)jid->valuedouble : -1;
        const char* name = jname && jname->valuestring ? jname->valuestring : NULL;
        int security = jsec ? (int)jsec->valuedouble : 1;
        int money = jmoney ? (int)jmoney->valuedouble : 0;
        ServerType role_val = SERVER_TYPE_UNKNOWN;
        if (jtype && cJSON_IsString(jtype) && jtype->valuestring) {
            role_val = server_type_from_string(jtype->valuestring);
        }
        int subnet = jsub ? (int)jsub->valuedouble : -1;

        if (id < 0 || id >= MAX_SERVERS) continue;

        server_init(&g->servers[id], id, name);
        g->servers[id].security = security;
        g->servers[id].money = money;
        g->servers[id].type = role_val;
        g->servers[id].subnet_id = subnet;

        /* links */
        cJSON* jlinks = cJSON_GetObjectItem(sObj, "links");
        if (jlinks && cJSON_IsArray(jlinks)) {
            int ln = cJSON_GetArraySize(jlinks);
            for (int li = 0; li < ln; li++) {
                cJSON* item = cJSON_GetArrayItem(jlinks, li);
                if (item && cJSON_IsNumber(item)) {
                    server_add_link(&g->servers[id], (int)item->valuedouble);
                }
            }
        }

        /* services */
        cJSON* jsvcs = cJSON_GetObjectItem(sObj, "services");
        if (jsvcs && cJSON_IsArray(jsvcs)) {
            int sn = cJSON_GetArraySize(jsvcs);
            for (int si = 0; si < sn && si < MAX_SERVICES_PER_SERVER; si++) {
                cJSON* svc = cJSON_GetArrayItem(jsvcs, si);
                if (!svc) continue;
                cJSON* sname = cJSON_GetObjectItem(svc, "name");
                cJSON* sport = cJSON_GetObjectItem(svc, "port");
                cJSON* svuln = cJSON_GetObjectItem(svc, "vuln");
                const char* sname_s = sname && sname->valuestring ? sname->valuestring : "";
                int port = sport ? (int)sport->valuedouble : 0;
                int vuln = svuln ? (int)svuln->valuedouble : 0;
                g->servers[id].services[g->servers[id].service_count].port = port;
                g->servers[id].services[g->servers[id].service_count].vuln_level = vuln;
                strncpy(g->servers[id].services[g->servers[id].service_count].name, sname_s, SERVICE_NAME_LEN - 1);
                g->servers[id].services[g->servers[id].service_count].name[SERVICE_NAME_LEN - 1] = '\0';
                g->servers[id].service_count++;
            }
        }

        if (id >= 0) {
            if (id >= g->server_count) g->server_count = id + 1;
        }
    }

    cJSON_Delete(root);
    return true;
}

void game_tick(GameState* g) {
    /* Placeholder*/
    g->tick++;
}

void action_queue_push(GameState* g, Action a) {
    if (g->queue.count < MAX_ACTIONS) {
	g->queue.actions[g->queue.count++] = a;
    } else {
	// Error, reached max actions
	printf("[debug] action queue full, action dropped!");
    }
}

Action action_queue_pop(GameState* g) {
    Action a = {.type = ACTION_NOP};
    if (g->queue.count == 0) return a;

    a = g->queue.actions[0];
    // shift remaining actoins
    for (int i = 1; i < g->queue.count; i++) {
	g->queue.actions[i - 1] = g->queue.actions[i];
    }
    g->queue.count--;
    return a;
}
