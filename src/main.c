#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "ui.h"
#include "commands.h"


int main(void) {
	// Fixed seed for debugging
	srand(12345);
	
	GameState game;
	char line[256];

	ui_init();
	game_init(&game);

	ui_print("hackterm v0.1");
	ui_print("Type 'help' to get started.");

	while (1) {
		/* Render current state */
		ui_render();

		/* Handle user input, if there is any */
		if (ui_readline(line, sizeof(line)) > 0) {
			if (commands_run(&game, line) == CMD_QUIT) {
				break;
			}
		}

		/* Advance simulation*/
		game_tick(&game);
	}

	game_shutdown(&game);
	ui_shutdown();
	return 0;
}
