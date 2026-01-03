#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "game.h"
#include "core_commands.h"
#include "commands.h"
#include "ui.h"

#define MAX_ARGS 100

/**
 * @brief Represents a shell command.
 *
 * Contains the command name, help text, and a pointer to the handler function.
 *
 * @param name The name of the command (e.g., "help", "exit").
 * @param help A short help string describing the command.
 * @param handler Function pointer to the command's implementation.
 */
typedef struct {
    const char *name;   /**< Command name. */
    const char *help;   /**< Help text for the command. */
    CommandResult (*handler)(GameState *g, int argc, char **argv); /**< Command handler function. */
} Command;


/**
 * @brief Show a list of available commands.
 */
static CommandResult cmd_help(GameState *g, int argc, char **argv);
/**
 * @brief Exit the game.
 */
static CommandResult cmd_exit(GameState *g, int argc, char **argv);
/**
 * @brief Print text to the UI.
 */
static CommandResult cmd_echo(GameState *g, int argc, char **argv);
/**
 * @brief List servers connected to the current server.
 */
static CommandResult cmd_scan(GameState *g, int argc, char **argv);
/**
 * @brief Connect to a server by name.
 */
static CommandResult cmd_connect(GameState *g, int argc, char **argv);
/**
 * @brief Save the current game state.
 */
static CommandResult cmd_save(GameState *g, int argc, char **argv);

static const Command commands[] = {
	{"help",		"show this message",						cmd_help},
	{"exit",		"quit hackterm",							cmd_exit},
	{"echo",		"print text",								cmd_echo},
	{"scan",		"list servers connected to current server", cmd_scan},
	{"connect",		"connect to a linked server",				cmd_connect},
	{"save",		"save the game",							cmd_save},
};

static const int command_count = sizeof(commands)/sizeof(commands[0]);

static int split_args(char *s, char** argv, int max_args) {
	int argc = 0;
	while (*s) {
		while (isspace((unsigned char)*s)) s++;
		if (!*s) break;
		if (argc >= max_args) break;
		argv[argc++] = s;
		while (*s && !isspace((unsigned char)*s)) s++;
		if (*s) *s++ = '\0';
	}
	return argc;
}

static CommandResult cmd_help(GameState *g, int argc, char **argv) {
	(void)g; (void)argc; (void)argv;

	Server *cur = game_get_current_server(g);
	if (cur) {
		ui_print("Connected to: %s", cur->name);
	}

	ui_print("Available commands:");
	for (int i = 0; i < command_count; i++) {
		ui_print("	%-10s - %s", commands[i].name, commands[i].help);
	}
	return CMD_OK;
}

static CommandResult cmd_exit(GameState *g, int argc, char **argv) {
	(void)g; (void)argc; (void)argv;
	return CMD_QUIT;
}

static CommandResult cmd_echo(GameState *g, int argc, char **argv) {
    (void)g;
    char *s = core_echo(argc, argv);
    if (!s) {
        ui_print("");
        return CMD_OK;
    }
    ui_print("%s", s);
    free(s);
    return CMD_OK;
}

static CommandResult cmd_scan(GameState *g, int argc, char **argv) {
    (void)argc; (void)argv;
    int n = 0;
    ServerId *connections = core_scan(g, &n);
    ui_print("Connected servers:");
    for (int i = 0; i < n; i++) {
        ServerId id = connections[i];
        Server *s = game_get_server(g, id);
        if (s) ui_print("  %s", s->name);
        else ui_print("  <unknown> (%d)", id);
    }
    return CMD_OK;
}

static CommandResult cmd_connect(GameState *g, int argc, char **argv) {
    if (argc < 2) {
        ui_print("Usage: connect <server_name>");
        return CMD_OK;
    }
    ServerId out_target = -1;
    CoreResult cr = core_connect(g, argv[1], &out_target);
    if (cr == CORE_OK) {
        Server *s = game_get_server(g, out_target);
        ui_print("Connected to %s.", s ? s->name : argv[1]);
    } else if (cr == CORE_ERR_NOT_FOUND) {
        ui_print("Server '%s' not found.", argv[1]);
    } else if (cr == CORE_ERR_NOT_LINKED) {
        Server *s = game_get_server(g, out_target);
        ui_print("Cannot connect to %s: not directly linked.", s ? s->name : argv[1]);
    } else if (cr == CORE_ERR_INVALID_ARG) {
        ui_print("Invalid argument to connect.");
    } else {
        ui_print("Cannot connect to %s: error (%d).", argv[1], cr);
    }

    return CMD_OK;
}

static CommandResult cmd_save(GameState *g, int argc, char **argv) {
    const char *file = (argc > 1) ? argv[1] : "save.save";
    CoreResult cr = core_save(g, file);
    if (cr == CORE_OK) {
        ui_print("Game saved to %s", file);
    } else if (cr == CORE_ERR_FILE) {
        ui_print("Failed to save game to %s: file error.", file);
    } else if (cr == CORE_ERR_INVALID_ARG) {
        ui_print("Failed to save game: invalid arguments.");
    } else {
        ui_print("Failed to save game to %s: error (%d)", file, cr);
    }
    return CMD_OK;
}

// --- Command handler ---
CommandResult commands_run(GameState *g, const char *input) {
    char buffer[1024];
    char *argv[MAX_ARGS];

    strncpy(buffer, input, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    int argc = split_args(buffer, argv, MAX_ARGS);
    if (argc == 0) return CMD_OK;

    for (int i = 0; i < command_count; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            return commands[i].handler(g, argc, argv);
        }
    }

    ui_print("Unknown command: %s", argv[0]);
    ui_print("Type 'help' for a list of commands.");
    return CMD_OK;
}
