/**
 * @file script_api.c
 * @brief Lua-facing C helpers for script access to the game state.
 */

#include "script_api.h"

#include <string.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "core_commands.h"
#include "game.h"
#include "script.h"

static GameState* g_state = NULL;

static const char* core_result_to_string(CoreResult r) {
    switch (r) {
	case CORE_OK:
	    return "OK";
	case CORE_ERR_NOT_FOUND:
	    return "NOT_FOUND";
	case CORE_ERR_NOT_LINKED:
	    return "NOT_LINKED";
	case CORE_ERR_FILE:
	    return "FILE_ERROR";
	case CORE_ERR_INVALID_ARG:
	    return "INVALID_ARG";
	default:
	    return "UNKNOWN";
    }
}

/* game.scan() -> table of server names (array-style) */
static int l_game_scan(lua_State* L) {
    if (!g_state) {
	lua_pushnil(L);
	lua_pushstring(L, "no game state");
	return 2;
    }
    int n = 0;
    ServerId* ids = core_scan(g_state, &n);
    lua_newtable(L);
    for (int i = 0; i < n; i++) {
	ServerId id = ids[i];
	Server* s = game_get_server(g_state, id);
	lua_pushinteger(L, i + 1);
	if (s)
	    lua_pushstring(L, s->name);
	else
	    lua_pushnil(L);
	lua_settable(L, -3);
    }
    return 1;
}

/* game.connect(name) -> true | false, errmsg, code */
static int l_game_connect(lua_State* L) {
    if (!g_state) {
	lua_pushboolean(L, 0);
	lua_pushstring(L, "no game state");
	return 2;
    }
    const char* name = lua_tostring(L, 1);
    if (!name) {
	lua_pushboolean(L, 0);
	lua_pushstring(L, "expected string argument");
	return 2;
    }
    ServerId out = -1;
    CoreResult cr = core_connect(g_state, name, &out);
    if (cr == CORE_OK) {
	lua_pushboolean(L, 1);
	return 1;
    }
    lua_pushboolean(L, 0);
    lua_pushstring(L, core_result_to_string(cr));
    lua_pushinteger(L, (int)cr);
    return 3;
}

/* game.save(path) -> true | false, errmsg, code */
static int l_game_save(lua_State* L) {
    if (!g_state) {
	lua_pushboolean(L, 0);
	lua_pushstring(L, "no game state");
	return 2;
    }
    const char* path = lua_tostring(L, 1);
    if (!path) {
	lua_pushboolean(L, 0);
	lua_pushstring(L, "expected string argument");
	return 2;
    }
    CoreResult cr = core_save(g_state, path);
    if (cr == CORE_OK) {
	lua_pushboolean(L, 1);
	return 1;
    }
    lua_pushboolean(L, 0);
    lua_pushstring(L, core_result_to_string(cr));
    lua_pushinteger(L, (int)cr);
    return 3;
}

/* game.get_current() -> { id = <num>, name = <str> } or nil
 * Returns a small table describing the current server. */
static int l_game_get_current(lua_State* L) {
    if (!g_state) {
	lua_pushnil(L);
	return 1;
    }
    Server* s = game_get_current_server(g_state);
    if (!s) {
	lua_pushnil(L);
	return 1;
    }
    lua_newtable(L);
    lua_pushstring(L, "id");
    lua_pushinteger(L, s->id);
    lua_settable(L, -3);
    lua_pushstring(L, "name");
    lua_pushstring(L, s->name);
    lua_settable(L, -3);
    lua_pushstring(L, "security");
    lua_pushinteger(L, s->security);
    lua_settable(L, -3);
    lua_pushstring(L, "money");
    lua_pushinteger(L, s->money);
    lua_settable(L, -3);
    return 1;
}

/* game.list_servers() -> table of server names (index -> name)
 * Returns all servers present in the game state for informational purposes.
 */
static int l_game_list_servers(lua_State* L) {
    if (!g_state) {
	lua_pushnil(L);
	lua_pushstring(L, "no game state");
	return 2;
    }
    lua_newtable(L);
    for (int i = 0; i < g_state->server_count; i++) {
	lua_pushinteger(L, i + 1);
	lua_pushstring(L, g_state->servers[i].name);
	lua_settable(L, -3);
    }
    return 1;
}

static const luaL_Reg game_funcs[] = {{"scan", l_game_scan}, {"connect", l_game_connect},
                                      {"get_current", NULL}, {"list_servers", NULL},
                                      {"save", l_game_save}, {NULL, NULL}};

/* script.log(...): concatenate tostring(...) of all args with spaces and store */
static int l_script_log(lua_State* L) {
    int n = lua_gettop(L);
    if (n == 0) {
        return 0;
    }

    char tmp[1024];
    size_t off = 0;
    for (int i = 1; i <= n; i++) {
    	size_t len;
    	luaL_tolstring(L, i, &len); /* pushes string repr */
    	const char* s = lua_tostring(L, -1);
    	if (!s) {
    	    s = "(null)";
    	}
    	size_t copy = len;
    	if (off + copy + 2 >= sizeof(tmp)) {
    	    /* truncate */
    	    copy = sizeof(tmp) - off - 2;
    	}
    	memcpy(tmp + off, s, copy);
    	off += copy;
    	if (i < n && off + 1 < sizeof(tmp)) {
    	    tmp[off++] = ' ';
    	}
    	lua_pop(L, 1); /* remove tostring result */
    	if (off + 1 >= sizeof(tmp)) {
    	    break;
    	}
    }
    tmp[off] = '\0';
    script_log("%s", tmp);
    return 0;
}

int script_api_init(GameState* g) {
    if (!g) {
        return -1;
    }
    g_state = g;
    return 0;
}

int script_api_register(lua_State* L) {
    if (!L) {
        return -1;
    }
    /* fill in dynamic entries for readability (separate to keep functions
     * declared close to implementation) */
    luaL_newlib(L, game_funcs);
    /* register additional helpers */
    lua_pushcfunction(L, l_game_get_current);
    lua_setfield(L, -2, "get_current");
    lua_pushcfunction(L, l_game_list_servers);
    lua_setfield(L, -2, "list_servers");
    lua_setglobal(L, "game");

    /* register script table with log() */
    lua_newtable(L);
    lua_pushcfunction(L, l_script_log);
    lua_setfield(L, -2, "log");
    lua_setglobal(L, "script");
    return 0;
}

void script_api_shutdown(void) {
    g_state = NULL;
}
