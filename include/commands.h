#ifndef INCLUDE_COMMANDS_H_
#define INCLUDE_COMMANDS_H_

#include "game.h"

/**
 * @brief Result codes returned by command execution.
 */
typedef enum {
    CMD_OK = 0,  /**< Command executed successfully. */
    CMD_QUIT = 1 /**< Command requests the game to quit. */
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
CommandResult commands_run(GameState* g, const char* input);

/**
 * @brief Return the number of built-in commands.
 *
 * This returns the number of commands available in the interactive
 * command table. It is intended for use by UI code (autocomplete,
 * command listings) and should remain inexpensive to call.
 *
 * @return The number of registered commands.
 */
int commands_count(void);

/**
 * @brief Get the name of a command by index.
 *
 * Returns the null-terminated command name for the command at the
 * given index (0..commands_count()-1). If the index is out of range
 * the function returns NULL.
 *
 * @param idx Index of the command to query.
 * @return Pointer to a static null-terminated string with the command
 *         name, or NULL if idx is invalid.
 */
const char* commands_name(int idx);
#endif  // INCLUDE_COMMANDS_H_
