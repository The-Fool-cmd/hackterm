#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include "server.h"
#include "core_result.h"

#define MAX_SERVERS 512
#define MAX_ACTIONS 256

/**
 * @brief An enum for Action types.
 * 
 * Contains all the different types of actions.
 */
typedef enum {
    ACTION_CONNECT,
    ACTION_SCAN,
    ACTION_DOWNLOAD,
    ACTION_NOP,
} ActionType;

/**
 * @brief Represents an action.
 * 
 * Contains all the queued up actions and their count.
 */
typedef struct {
    ActionType type; /**< Type of action. */
    int target_server; /**< The server the action should run on. */
    int value; /**< Optional: The value/argument for the action. */
} Action;

/**
 * @brief An array of all queued actions.
 * 
 * Contains all the queued up actions and their count.
 */
typedef struct {
    Action actions[MAX_ACTIONS]; /**< Array of all queued up actions. */
    int count; /**< The number of queued up actions. */
} ActionQueue;

/**
 * @brief Represents the overall state of the game.
 * 
 * Contains all servers in the game, the total number of servers,
 * and information about the player's current and home server.
 */
typedef struct {
    Server servers[MAX_SERVERS]; /**< Array of all servers in the game. */
    int server_count;            /**< Number of servers currently in use. */

    ServerId current_server; /**< ID of the server the player is currently connected to. */
    ServerId home_server;    /**< ID of the player's home server. */

    int tick;   /**< Current tick number. */

    ActionQueue queue; /**< Queued actions. */
} GameState;

/* ---------------- LIFECYCLE ---------------- */

/**
 * @brief Initializes the gamestate.
 * 
 * Sets up all internal structures and prepares the game state.
 * 
 * @param g Pointer to the GameState to initialize.
 */
void game_init(GameState *g);

/**
 * @brief Shuts down the game in a controlled manner.
 * 
 * Frees resources and performs any necessary cleanup.
 * 
 * @param g Pointer to the GameState to shut down.
 */
void game_shutdown(GameState *g);

/* ---------------- HELPERS ---------------- */

/**
 * @brief Returns the server the player is currently connected to.
 * 
 * @param g Pointer to the GameState.
 * @return Pointer to the current Server, or NULL if not connected.
 */
Server *game_get_current_server(GameState *g);

/**
 * @brief Returns the server with the specified ID.
 * 
 * @param g Pointer to the GameState.
 * @param id ID of the server to retrieve.
 * @return Pointer to the Server with the given ID, or NULL if not found.
 */
Server *game_get_server(GameState *g, ServerId id);

/* ---------------- COMMANDS ---------------- */

/**
 * @brief Scans for servers connected to the current server.
 * 
 * @param g Pointer to the GameState.
 * @param out Array to store the found server IDs.
 * @param max Maximum number of entries that can be written to @p out.
 * @return Number of servers written to @p out.
 */
int game_scan(const GameState *g, ServerId *out, int max);

/**
 * @brief Connects the player to a server.
 * 
 * @param g Pointer to the GameState.
 * @param to ID of the server to connect to.
 * @return CORE_OK on success, otherwise a CoreResult error code.
 */
CoreResult game_connect(GameState *g, ServerId to);

/**
 * @brief Saves the current game state to a file.
 * 
 * @param g Pointer to the GameState.
 * @param filename Path to the file where the state should be saved.
 * @returns 
 */
bool game_save(const GameState *g, const char *filename);
/**
 * @brief Simulates one tick.
 * 
 * @param g Pointer to the GameState
 */
void game_tick(GameState *g);

#endif /* GAME_H */
