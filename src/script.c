/**
 * @file script.c
 * @brief Embeds Lua and provides script lifecycle, logging and execution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "script.h"
#include "script_api.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static lua_State* L = NULL;

/* --- Script log (ring buffer) --- */
#define SCRIPT_LOG_MAX_ENTRIES 1024
#define SCRIPT_LOG_MAX_LINE 512

static char* script_log_buf[SCRIPT_LOG_MAX_ENTRIES];
static int script_log_start = 0;
static int script_log_count = 0;

/* internal helper to free buffer */
static void script_log_clear(void) {
    for (int i = 0; i < SCRIPT_LOG_MAX_ENTRIES; i++) {
	free(script_log_buf[i]);
	script_log_buf[i] = NULL;
    }
    script_log_start = 0;
    script_log_count = 0;
}

/**
 * @brief Append a formatted message to the in-memory script log.
 *
 * See `script_log` declaration in `include/script.h` for details.
 */
void script_log(const char* fmt, ...) {
    char tmp[SCRIPT_LOG_MAX_LINE];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);

    char* s = strdup(tmp);
    if (!s) return; /* drop on alloc fail */

    int idx = (script_log_start + script_log_count) % SCRIPT_LOG_MAX_ENTRIES;
    if (script_log_count == SCRIPT_LOG_MAX_ENTRIES) {
	free(script_log_buf[idx]);
	script_log_buf[idx] = s;
	script_log_start = (script_log_start + 1) % SCRIPT_LOG_MAX_ENTRIES;
    } else {
	script_log_buf[idx] = s;
	script_log_count++;
    }
}

/**
 * @brief Retrieve recent script log entries.
 *
 * See `script_log_get_recent` declaration in `include/script.h`.
 */
int script_log_get_recent(const char** out, int max) {
    if (!out || max <= 0) return 0;
    int n = (script_log_count < max) ? script_log_count : max;
    /* start index of the oldest to return */
    int start = (script_log_start + script_log_count - n) % SCRIPT_LOG_MAX_ENTRIES;
    for (int i = 0; i < n; i++) {
	int idx = (start + i) % SCRIPT_LOG_MAX_ENTRIES;
	out[i] = script_log_buf[idx];
    }
    return n;
}

int script_run(const char* path, int argc, char** argv) {
    if (!L || !path) return -1;

    char full[1024];
    if (strchr(path, '/') != NULL) {
	strncpy(full, path, sizeof(full) - 1);
	full[sizeof(full) - 1] = '\0';
    } else {
	snprintf(full, sizeof(full), "./scripts/%s", path);
    }

    /* prepare arg table in Lua as global 'arg' */
    lua_State* sL = L;
    lua_newtable(sL);
    for (int i = 0; i < argc; i++) {
	lua_pushinteger(sL, i + 1);
	lua_pushstring(sL, argv[i]);
	lua_settable(sL, -3);
    }
    lua_setglobal(sL, "arg");

    if (luaL_loadfile(sL, full) != 0) {
	fprintf(stderr, "Lua load error %s: %s\n", full, lua_tostring(sL, -1));
	lua_pop(sL, 1);
	/* clear arg global to avoid leaking */
	lua_pushnil(sL);
	lua_setglobal(sL, "arg");
	return -1;
    }

    if (lua_pcall(sL, 0, LUA_MULTRET, 0) != 0) {
	fprintf(stderr, "Lua runtime error %s: %s\n", full, lua_tostring(sL, -1));
	lua_pop(sL, 1);
	lua_pushnil(sL);
	lua_setglobal(sL, "arg");
	return -1;
    }

    /* clear arg global after successful run */
    lua_pushnil(sL);
    lua_setglobal(sL, "arg");

    return 0;
}

static int load_script_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return 0; /* not present */
    fclose(f);
    if (luaL_dofile(L, path) != 0) {
	fprintf(stderr, "Lua error loading %s: %s\n", path, lua_tostring(L, -1));
	lua_pop(L, 1);
	return -1;
    }
    return 1;
}
/**
 * @brief Initialize scripting subsystem and load default scripts.
 *
 * The function creates a Lua state, registers C helpers, and attempts
 * to load scripts from standard locations.
 */
int script_init(GameState* g) {
    if (!g) return -1;

    /* initialize C-side API state */
    if (script_api_init(g) != 0) return -1;

    L = luaL_newstate();
    if (!L) return -1;
    luaL_openlibs(L);

    /* register C helpers into Lua (creates global `game` table) */
    if (script_api_register(L) != 0) {
	fprintf(stderr, "Warning: failed to register script API\n");
    }

    /* Try to load user scripts from two locations (project and user dir). */
    load_script_file("./scripts/init.lua");
    const char* home = getenv("HOME");
    if (home) {
	char buf[512];
	snprintf(buf, sizeof(buf), "%s/.hackterm/init.lua", home);
	load_script_file(buf);
    }

    /* clear any previous log state */
    script_log_clear();
    return 0;
}

/**
 * @brief Shutdown the scripting subsystem and free resources.
 */
void script_shutdown(void) {
    if (L) {
	lua_close(L);
	L = NULL;
    }
    script_api_shutdown();
    script_log_clear();
}

/**
 * @brief Dispatch an unrecognized command to user scripts.
 *
 * Calls the global `on_command` function if present. The function is
 * called in protected mode and must not abort the program on errors.
 */
int script_handle_command(GameState* g, const char* cmd, int argc, char** argv) {
    (void)g;
    if (!L) return 0;

    lua_getglobal(L, "on_command");
    if (!lua_isfunction(L, -1)) {
	lua_pop(L, 1);
	return 0;
    }

    /* push arguments: command string, args table (argv[1..n-1]) */
    lua_pushstring(L, cmd);
    lua_newtable(L);
    for (int i = 1; i < argc; i++) {
	lua_pushinteger(L, i);
	lua_pushstring(L, argv[i]);
	lua_settable(L, -3);
    }

    if (lua_pcall(L, 2, 1, 0) != 0) {
	fprintf(stderr, "Lua error in on_command: %s\n", lua_tostring(L, -1));
	lua_pop(L, 1);
	return 0;
    }

    int handled = 0;
    if (lua_isboolean(L, -1)) {
	handled = lua_toboolean(L, -1);
    } else if (lua_isnil(L, -1)) {
	handled = 0;
    }
    lua_pop(L, 1);
    return handled ? 1 : 0;
}
