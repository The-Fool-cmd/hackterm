#include <string.h>
#include <stdlib.h>

#include "commands.h"
#include "ui.h"

static void cmd_help(void) {
	ui_print("Available commands:");
	ui_print("  help   - show this message");
	ui_print("  exit   - quit hackterm");
}

void commands_run(const char *input) {
	if (strcmp(input, "help") == 0) {
		cmd_help();
		return;
	}

	if (strcmp(input, "exit") == 0) {
		ui_shutdown();
		exit(0);
	}

	ui_print("Unknown command: %s", input);
}
