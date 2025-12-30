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

#endif /* COMMANDS_H */
