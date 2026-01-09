#include <ncurses.h>
#include <stdarg.h>
#include <string.h>

#include "ui.h"
#include "ui_internal.h"
#include "ui_view.h"

void ui_render(void) {
    /* render should work even when `input_win` is NULL (non-terminal views)
     * so do not early-return here. */

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

    /* draw sidebar menu (left) */
    if (sidebar_win) {
        werase(sidebar_win);
        if (has_colors()) wbkgd(sidebar_win, COLOR_PAIR(3));
        box(sidebar_win, 0, 0);
        for (int i = 0; i < menu_count; i++) {
            int y = 1 + i;
            if (i == selected_menu) {
                wattron(sidebar_win, A_REVERSE);
            }
            mvwprintw(sidebar_win, y, 2, "%s", menu_items[i]);
            if (i == selected_menu) {
                wattroff(sidebar_win, A_REVERSE);
            }
        }
        wrefresh(sidebar_win);
    }

    /* draw main/output area depending on the active view */
    if (output_win) {
        /* delegate rendering to the registered view implementation */
        ui_view_render_active(output_win);
    }

    /* draw input overlay: delegate to terminal view when active. We
     * avoid referencing input_buf or input_len here — the Terminal view
     * owns its input state and knows how to draw itself into the
     * backing buffer. */
    if (input_win) {
        if (current_view == VIEW_TERMINAL) {
            extern void view_terminal_redraw_input_no_update(void);
            /* terminal view already rendered the output via
             * ui_view_render_active; ask it to draw the input overlay into
             * the backing buffer and flush once. */
            view_terminal_redraw_input_no_update();
            doupdate();
            curs_set(1);
        } else {
            /* Redraw output on top so the area where the input overlay
             * would be is covered without clearing stdscr (avoids flicker). */
            if (header_win) wnoutrefresh(header_win);
            if (sidebar_win) wnoutrefresh(sidebar_win);
            if (output_win) {
                touchwin(output_win);
                wnoutrefresh(output_win);
            }
            doupdate();
            curs_set(0);
        }
    } else {
        /* input_win not present: ensure output covers the space */
        if (header_win) wnoutrefresh(header_win);
        if (sidebar_win) wnoutrefresh(sidebar_win);
        if (output_win) {
            touchwin(output_win);
            wnoutrefresh(output_win);
        }
        doupdate();
        curs_set(0);
    }
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
