#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "game.h"
#include "commands.h"
#include "ui.h"

#define MAX_ARGS 100

typedef struct {
	const char *name;
	const char *help;
	CommandResult (*handler)(GameState *g, int argc, char **argv);
} Command;

static Command commands[] = {
	{"help",		"show this message",						cmd_help},
	{"exit",		"quit hackterm",							cmd_exit},
	{"quit",		"quit hackterm",							cmd_exit},
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
	(void)g;

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
    if (argc == 1) {
        ui_print("");
    } else {
        // Print everything after the command name
        char *msg = argv[1];
        for (int i = 2; i < argc; i++) {
            ui_print("%s ", argv[i]);
            msg = argv[i]; // advance to last argument
        }
        ui_print("%s", msg);
    }
    return CMD_OK;
}

static CommandResult cmd_scan(GameState *g, int argc, char **argv) {
    (void)argc; (void)argv;
    ServerId connections[16];
    int n = game_scan(g, connections, 16);

    ui_print("Connected servers:");
    for (int i = 0; i < n; i++) {
        ui_print("  %s", g->servers[connections[i]].name);
    }
    return CMD_OK;
}

static CommandResult cmd_connect(GameState *g, int argc, char **argv) {
    if (argc < 2) {
        ui_print("Usage: connect <server_name>");
        return CMD_OK;
    }

    ServerId target = -1;
    for (int i = 0; i < g->server_count; i++) {
        if (strcmp(g->servers[i].name, argv[1]) == 0) {
            target = i;
            break;
        }
    }

    if (target == -1) {
        ui_print("Server '%s' not found.", argv[1]);
        return CMD_OK;
    }

    if (game_connect(g, target)) {
        ui_print("Connected to %s.", g->servers[target].name);
    } else {
        ui_print("Cannot connect to %s: not directly linked.", g->servers[target].name);
    }

    return CMD_OK;
}

static CommandResult cmd_save(GameState *g, int argc, char **argv) {
    const char *file = (argc > 1) ? argv[1] : "save.save";
    game_save(g, file);
    ui_print("Game saved to %s", file);
    return CMD_OK;
}

// --- Main dispatcher ---
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
