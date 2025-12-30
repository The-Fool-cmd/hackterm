#ifndef COMMANDS_H
#define COMMANDS_H

#include "game.h"

/**
 * @brief Result codes returned by command execution.
 */
typedef enum {
    CMD_OK   = 0, /**< Command executed successfully. */
    CMD_QUIT = 1  /**< Command requests the game to quit. */
} CommandResult;

/**
 * @brief Executes a command entered by the player.
 * 
 * Parses and runs the command given in @p input, affecting the
 * provided game state @p g.
 * 
 * @param g Pointer to the current game state.
 * @param input Null-terminated string containing the player's command.
 * @return A CommandResult indicating success or quit request.
 */
CommandResult commands_run(GameState *g, const char *input);
/* ----- INDIVIDUAL COMMAND HANDLERS -----*/

/**
 * @brief Show a list of available commands.
 */
CommandResult cmd_help(GameState *g, int argc, char **argv);
/**
 * @brief Exit the game.
 */
CommandResult cmd_exit(GameState *g, int argc, char **argv);
/**
 * @brief Print text to the UI.
 */
CommandResult cmd_echo(GameState *g, int argc, char **argv);
/**
 * @brief List servers connected to the current server.
 */
CommandResult cmd_scan(GameState *g, int argc, char **argv);
/**
 * @brief Connect to a server by name.
 */
CommandResult cmd_connect(GameState *g, int argc, char **argv);
/**
 * @brief Save the current game state.
 */
CommandResult cmd_save(GameState *g, int argc, char **argv);

#endif /* COMMANDS_H */
