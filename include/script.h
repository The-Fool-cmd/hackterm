#ifndef INCLUDE_SCRIPT_H_
#define INCLUDE_SCRIPT_H_

#include "game.h"

/**
 * @brief Initialize the scripting subsystem.
 *
 * The scripting subsystem may keep a reference to the provided GameState.
 * User scripts must not mutate the game state directly; they should
 * request actions through the exposed API functions.
 *
 * @param g Pointer to the current GameState.
 * @return 0 on success, non-zero on failure.
 */
int script_init(GameState* g);

/**
 * @brief Shutdown the scripting subsystem and release resources.
 */
void script_shutdown(void);

/**
 * @brief Allow scripts to handle an unrecognized command.
 *
 * This function calls the script-level handler (for example, a global
 * `on_command` function) in protected mode. It should not longjmp or
 * otherwise abort the main program.
 *
 * @param g Current GameState (read-only for scripts).
 * @param cmd Command name.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 1 if the command was handled by scripts, 0 otherwise.
 */
int script_handle_command(GameState* g, const char* cmd, int argc, char** argv);

/**
 * @brief Append a formatted message to the in-memory script log.
 *
 * The message is formatted using the printf family and stored in a
 * bounded ring buffer. Long messages are truncated. The stored strings
 * are owned by the logging subsystem and must not be freed by callers.
 *
 * @param fmt printf-style format string.
 */
void script_log(const char* fmt, ...);

/**
 * @brief Retrieve recent script log entries.
 *
 * The caller provides an array of pointers `out` with capacity `max`.
 * The function fills `out` with pointers to internal strings and returns
 * the number of entries written. The caller must not free these pointers.
 *
 * @param out Destination array for pointers to log entries.
 * @param max Maximum number of entries to retrieve.
 * @return Number of entries written into `out`.
 */
int script_log_get_recent(const char** out, int max);

/**
 * Execute a script file with arguments.
 * If `path` contains a directory separator it is used as-is; otherwise
 * the implementation will try `./scripts/<path>`.
 *
 * @param path script filename or relative name
 * @param argc number of arguments in argv
 * @param argv array of argument strings (not including the script name)
 * @return 0 on success, non-zero on error
 */
/**
 * @brief Execute a script file with arguments.
 *
 * If `path` contains a directory separator it is used as-is; otherwise
 * the implementation will try `./scripts/<path>`.
 *
 * A global Lua table `arg` is provided while the script runs and cleared
 * afterwards. Errors are printed to stderr.
 *
 * @param path Script filename or relative name.
 * @param argc Number of arguments in argv.
 * @param argv Array of argument strings (not including the script name).
 * @return 0 on success, non-zero on error.
 */
int script_run(const char* path, int argc, char** argv);

#endif  // INCLUDE_SCRIPT_H_
