/**
 * @file script_api.h
 * @brief C functions exposed to Lua scripts.
 */

#ifndef SCRIPT_API_H
#define SCRIPT_API_H

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
int script_api_init(GameState *g);

/**
 * @brief Register the script-facing helpers into a Lua state.
 *
 * Creates a global `game` table and other helpers used by scripts.
 *
 * @param L Lua state to register into.
 * @return 0 on success, non-zero on error.
 */
int script_api_register(lua_State *L);

/**
 * @brief Shutdown the script API and release resources.
 */
void script_api_shutdown(void);

#endif /* SCRIPT_API_H */
