#include "core_commands.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "game.h"

/* --- Connect --- */
CoreResult core_connect(GameState* g, const char* server_name, ServerId* out_target) {
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

    if (game_connect(g, target) != CORE_OK) {
	if (out_target) *out_target = target;
	return CORE_ERR_NOT_LINKED;
    }

    if (out_target) *out_target = target;
    return CORE_OK;
}

/* --- Scan --- */
ServerId* core_scan(GameState* g, int* out_count) {
    static ServerId buf[16];  // max 16 connections
    if (!g || !out_count) return NULL;

    int n = game_scan(g, buf, 16);
    *out_count = n;
    return buf;
}

/* --- Echo --- */
char* core_echo(int argc, char** argv) {
    if (argc <= 1 || !argv) {
	return strdup("");
    }

    /* Calculate total length (including space separators and final NUL) */
    size_t len = 0;
    for (int i = 1; i < argc; i++) {
        len += strlen(argv[i]) + 1; /* +1 for space or NUL */
    }

    char* result = malloc(len);
    if (!result) {
        return NULL;
    }

    size_t pos = 0;
    for (int i = 1; i < argc; i++) {
        const char* sep = (i < argc - 1) ? " " : "";
        int written = snprintf(result + pos, len - pos, "%s%s", argv[i], sep);
        if (written < 0) {
            /* encoding error; ensure string is terminated */
            result[pos] = '\0';
            break;
        }
        /* Move position forward but never past allocated size */
        if ((size_t)written >= len - pos) {
            /* truncated */
            pos = len - 1;
            result[pos] = '\0';
            break;
        }
        pos += (size_t)written;
    }

    return result;
}

/* --- Save --- */
CoreResult core_save(GameState* g, const char* file) {
    if (!g || !file) return CORE_ERR_INVALID_ARG;
    if (game_save(g, file)) return CORE_OK;
    return CORE_ERR_FILE;
}
