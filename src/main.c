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
/* Target frame rate for UI rendering (frames per second). */
#define FPS 60
#define MS_PER_FRAME (1000 / FPS)

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

    /* Try to load JSON save first (save.json). If it fails, initialize a new game. */
    if (!game_load(&game, "save.json")) {
        game_init(&game);
    }
    /* Initialize scripting subsystem. */
    if (script_init(&game) != 0) {
	ui_print("Warning: scripting subsystem failed to initialize");
    }

    ui_print("hackterm v0.1");
    ui_print("Type 'help' to get started.");

    while (1) {
        /* Frame start timestamp */
        uint64_t frame_start = current_time_ms();

        /* Render current state */
        ui_render();

        /* Handle user input, if there is any */
        if (ui_readline_nonblocking(line, sizeof(line)) > 0) {
            if (commands_run(&game, line) == CMD_QUIT) {
                break;
            }
        }

        /* Advance simulation: catch up by running ticks until caught up */
        uint64_t now = current_time_ms();
        while (now - last_tick >= MS_PER_TICK) {
            game_tick(&game);
            last_tick += MS_PER_TICK;
            now = current_time_ms();
        }

        /* Cap render loop to target FPS to avoid burning CPU */
        uint64_t frame_end = current_time_ms();
        int64_t elapsed = (int64_t)(frame_end - frame_start);
        int64_t sleep_ms = MS_PER_FRAME - elapsed;
        if (sleep_ms > 0) {
            struct timespec ts;
            ts.tv_sec = sleep_ms / 1000;
            ts.tv_nsec = (sleep_ms % 1000) * 1000000L;
            nanosleep(&ts, NULL);
        }
    }

    /* Shutdown scripting subsystem before tearing down game state. */
    script_shutdown();

    game_shutdown(&game);
    ui_shutdown();
    return 0;
}
