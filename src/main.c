#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "game.h"
#include "ui.h"
#include "commands.h"
#include "script.h"

#define TPS 10
#define MS_PER_TICK (1000 / TPS)

uint64_t current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)(ts.tv_nsec / 1000000);
}

int main(void) {
    // Time related variable
    uint64_t last_tick = current_time_ms();

    // Fixed seed for debugging
    srand(12345);

    // GameState
    GameState game;
    char line[256];

    // Initialise UI and GameState
    ui_init();
    game_init(&game);
    /* Initialize scripting subsystem. */
    if (script_init(&game) != 0) {
	ui_print("Warning: scripting subsystem failed to initialize");
    }

    ui_print("hackterm v0.1");
    ui_print("Type 'help' to get started.");

    while (1) {
	/* Render current state */
	ui_render();

	/* Handle user input, if there is any */
	if (ui_readline_nonblocking(line, sizeof(line)) > 0) {
	    if (commands_run(&game, line) == CMD_QUIT) {
		break;
	    }
	}

    uint64_t now = current_time_ms();
    if (now - last_tick >= MS_PER_TICK) {
	    /* Advance simulation*/
	    game_tick(&game);
	    last_tick += MS_PER_TICK;
	}
    }

    /* Shutdown scripting subsystem before tearing down game state. */
    script_shutdown();

    game_shutdown(&game);
    ui_shutdown();
    return 0;
}
