#include "core_commands.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* --- Connect --- */
CoreResult core_connect(GameState *g, const char *server_name, ServerId *out_target) {
    if (!g || !server_name) return CORE_ERR_INVALID_ARG;

    ServerId target = -1;
    for (int i = 0; i < g->server_count; i++) {
        if (strcmp(g->servers[i].name, server_name) == 0) {
            target = i;
            break;
        }
    }

    if (target == -1) {
        if (out_target) *out_target = -1;
        return CORE_ERR_NOT_FOUND;
    }

    if (game_connect(g, target) != 0) {
        if (out_target) *out_target = target;
        return CORE_ERR_NOT_LINKED;
    }

    if (out_target) *out_target = target;
    return CORE_OK;
}

/* --- Scan --- */
ServerId* core_scan(GameState *g, int *out_count) {
    static ServerId buf[16]; // max 16 connections
    if (!g || !out_count) return NULL;

    int n = game_scan(g, buf, 16);
    *out_count = n;
    return buf;
}

/* --- Echo --- */
char* core_echo(int argc, char **argv) {
    if (argc <= 1 || !argv) {
        return strdup("");
    }

    // Calculate total length
    size_t len = 0;
    for (int i = 1; i < argc; i++) {
        len += strlen(argv[i]) + 1; // +1 for space or null
    }

    char *result = malloc(len);
    if (!result) return NULL;

    result[0] = '\0';
    for (int i = 1; i < argc; i++) {
        strcat(result, argv[i]);
        if (i < argc - 1) strcat(result, " ");
    }

    return result;
}

/* --- Save --- */
CoreResult core_save(GameState *g, const char *file) {
    if (!g || !file) return CORE_ERR_INVALID_ARG;
    if (game_save(g, file)) return CORE_OK;
    return CORE_ERR_FILE;
}
