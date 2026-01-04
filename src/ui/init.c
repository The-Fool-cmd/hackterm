#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_internal.h"

void ui_init(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_BLACK, COLOR_CYAN);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
    }

    /* enable mouse events (wheel, clicks) so terminals send KEY_MOUSE */
    mouseinterval(0);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    ui_layout();
}

void ui_shutdown(void) {
    if (header_win) { delwin(header_win); header_win = NULL; }
    if (output_win) { delwin(output_win); output_win = NULL; }
    if (input_win) { delwin(input_win); input_win = NULL; }
    endwin();
    /* free output buffer */
    for (int i = 0; i < out_count; i++) {
        int idx = (out_start + i) % OUT_HISTORY_MAX;
        free(out_lines[idx]);
        out_lines[idx] = NULL;
    }
    out_start = 0;
    out_count = 0;
    out_scroll_lines = 0;
}
