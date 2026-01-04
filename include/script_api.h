/**
 * @file script_api.h
 * @brief C functions exposed to Lua scripts.
 */

#ifndef INCLUDE_SCRIPT_API_H_
#define INCLUDE_SCRIPT_API_H_

#include "game.h"

/* Forward-declare lua_State so callers of this header don't need Lua
 * includes unless they register functions directly. */
typedef struct lua_State lua_State;

/**
 * @brief Initialize the C-side script API.
 *
 * This stores the provided GameState pointer for later use by the
 * Lua-facing helper functions.
 *
 * @param g Pointer to the current GameState.
 * @return 0 on success, non-zero on error.
 */
int script_api_init(GameState* g);

/**
 * @brief Register the script-facing helpers into a Lua state.
 *
 * Registers a single global root table `ht` that contains domain namespaces
 * used by scripts (for example `ht.net` and `ht.log`). The function also
 * creates backward-compatible aliases (`game` -> `ht.net`, `script` ->
 * `ht.log`) so existing scripts continue to work. Prefer using the `ht` root
 * in new scripts.
 *
 * The `ht` shape (subject to extension) will look like:
 *
 * ht = {
 *   net = { scan, connect, get_current, list_servers, save },
 *   log = { info },
 * }
 *
 * @param L Lua state to register into.
 * @return 0 on success, non-zero on error.
 */
int script_api_register(lua_State* L);

/**
 * @brief Shutdown the script API and release resources.
 */
void script_api_shutdown(void);

#endif  // INCLUDE_SCRIPT_API_H_
