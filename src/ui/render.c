#include <ncurses.h>
#include <stdarg.h>
#include <string.h>

#include "ui.h"
#include "ui_internal.h"

void ui_render(void) {
    if (!input_win) return;

    int r, c;
    getmaxyx(stdscr, r, c);
    if (r != term_rows || c != term_cols) {
        /* Recompute layout for new terminal size without clearing the screen
         * to preserve the scrollback buffer and avoid visual jumps. ui_layout
         * will recreate windows sized for the new terminal and we'll redraw
         * the output and input below. */
        ui_layout();
        ui_redraw_output();
    }

    /* draw header/status */
    werase(header_win);
    if (has_colors()) {
        wattron(header_win, COLOR_PAIR(1));
    }
    mvwprintw(header_win, 0, 1, "%s", status_buf[0] ? status_buf : "hackterm");
    if (has_colors()) {
        wattroff(header_win, COLOR_PAIR(1));
    }
    wrefresh(header_win);

    /* draw input */
    werase(input_win);
    if (has_colors()) {
        wbkgd(input_win, COLOR_PAIR(2));
    }
    box(input_win, 0, 0);
    mvwprintw(input_win, prompt_y, 1, "> %s", input_buf);
    /* place cursor after the prompt */
    wmove(input_win, prompt_y, 1 + prompt_x + input_len);
    wrefresh(input_win);
}

void ui_set_status(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(status_buf, sizeof(status_buf), fmt, ap);
    va_end(ap);
    if (header_win) {
        werase(header_win);
        if (has_colors()) wattron(header_win, COLOR_PAIR(1));
        mvwprintw(header_win, 0, 1, "%s", status_buf[0] ? status_buf : "hackterm");
        if (has_colors()) wattroff(header_win, COLOR_PAIR(1));
        wrefresh(header_win);
    }
}
