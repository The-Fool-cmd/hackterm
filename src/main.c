#include <string.h>

#include "ui.h"
#include "commands.h"

int main(void) {
	char line[256];

	ui_init();
	ui_print("hackterm v0.1");
	ui_print("Type 'help' to get started.");

	while (1) {
		ui_render();

		if (ui_readline(line, sizeof(line)) > 0) {
			if (commands_run(line) == CMD_QUIT) {
				break;
			}
		}
	}

	ui_shutdown();
	return 0;
}
