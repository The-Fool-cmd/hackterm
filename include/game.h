#ifndef GAME_H
#define GAME_H

#include "server.h"

#define MAX_SERVERS 512

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
 * @return 0 on success, non-zero on failure.
 */
int game_connect(GameState *g, ServerId to);

/**
 * @brief Saves the current game state to a file.
 * 
 * @param g Pointer to the GameState.
 * @param filename Path to the file where the state should be saved.
 */
void game_save(const GameState *g, const char *filename);

#endif /* GAME_H */
