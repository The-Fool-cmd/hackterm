#ifndef CORE_COMMANDS_H
#define CORE_COMMANDS_H

#include "game.h"
#include "core_result.h"

/**
 * @file core_commands.h
 * @brief Core game command logic for HackTerm.
 *
 * These functions implement the game logic for commands.
 * They do NOT print anything or handle the terminal.
 * Terminal commands and Lua wrappers call these functions.
 */

/* --- Connect --- */
/**
 * @brief Attempt to connect to a server by name.
 * 
 * @param g Pointer to GameState.
 * @param server_name Name of the server to connect to.
 * @param out_target If non-NULL, will contain the index of the server connected to.
 * @return CORE_OK on success, otherwise a CoreResult error code.
 */
CoreResult core_connect(GameState *g, const char *server_name, ServerId *out_target);

/* --- Scan --- */
/**
 * @brief Get a list of directly connected servers from the current server.
 * 
 * @param g Pointer to GameState.
 * @param out_count Number of servers returned.
 * @return Pointer to a static array of ServerIds (max 16). Do not free.
 */
ServerId* core_scan(GameState *g, int *out_count);

/* --- Echo --- */
/**
 * @brief Concatenate arguments into a single string.
 * 
 * @param argc Number of arguments (including command itself at argv[0]).
 * @param argv Array of arguments.
 * @return Heap-allocated string containing concatenated arguments. Caller must free().
 */
char* core_echo(int argc, char **argv);

/* --- Save --- */
/**
 * @brief Save the current game state to a file.
 * 
 * @param g Pointer to GameState.
 * @param file Filename to save to.
 * @return CORE_OK on success, otherwise a CoreResult error code.
 */
CoreResult core_save(GameState *g, const char *file);

/* --- Additional commands can follow the same pattern --- */
#endif