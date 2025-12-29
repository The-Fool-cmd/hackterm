#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "commands.h"
#include "ui.h"

static void cmd_help(void) {
	ui_print("Available commands:");
	ui_print("  help   - show this message");
	ui_print("  exit | quit  - quit hackterm");
	ui_print("  echo <text...> - print text");
}

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


CommandResult commands_run(const char *input) {
	char buffer[1024];
	char *argv[100];

	strncpy(buffer, input, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	int argc = split_args(buffer, argv, 100);
	if (argc == 0) {
		return CMD_OK;
	}

	if (strcmp(argv[0], "help") == 0) {
		cmd_help();
		return CMD_OK;
	}

	if (strcmp(argv[0], "exit") == 0 || strcmp(argv[0], "quit") == 0) {
		return CMD_QUIT;
	}

	if (strcmp(argv[0], "echo") == 0) {
		if (argc == 1) {
			ui_print("");
		} else {
			// print original without echo
			const char *msg = strstr(input, "echo");
			const char *p = msg ? msg + 4 : input;
			while (*p == ' ') p++;
			ui_print("%s", p);
		}
		return CMD_OK;
	}

	ui_print("Unknown command: %s", argv[0]);
	ui_print("Type 'help' for a list of commands.");
	return CMD_OK;
}
