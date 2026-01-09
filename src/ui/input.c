#include <ncurses.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

#include "ui.h"
#include "ui_internal.h"
#include "commands.h"
#include "ui_view.h"

int ui_readline_nonblocking(char* buf, int max) {
    /* Read global input from stdscr so views can receive keys even when
     * the terminal's input window is an overlay. stdscr is set to
     * non-blocking in ui_init(). */
    int ch = getch();
    if (ch == ERR) return 0;

    if (ch == KEY_MOUSE) {
        MEVENT me;
        if (getmouse(&me) == OK) {
            /* Only treat explicit left-clicks as sidebar selection. This
             * prevents scroll-wheel (BUTTON4/BUTTON5) events from being
             * interpreted as clicks that change the active view. */
            if (me.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED)) {
                if (me.x < sidebar_width && me.y >= header_height && me.y < term_rows) {
                    int rel = me.y - header_height;
                    /* menu items are printed starting at line 1 inside the sidebar box */
                    if (rel >= 1 && rel <= menu_count) {
                        selected_menu = rel - 1;
                        /* map selected menu to view if applicable */
                        if (selected_menu < VIEW_COUNT) current_view = (ui_view_t)selected_menu;
                        ui_set_status("Selected: %s", menu_items[selected_menu]);
                        /* recreate windows/layout for the selected view */
                        ui_layout();
                        ui_redraw_output();
                        ui_render();
                        return 0;
                    }
                }
            }
            if (me.bstate & BUTTON4_PRESSED) {
                /* Only scroll output when the Terminal view is active.
                 * Other views ignore wheel events (they can be handled by
                 * view-specific mouse handlers in a future refactor). */
                if (output_win && current_view == VIEW_TERMINAL) {
                    out_scroll_lines += 3;
                    /* batch redraw to avoid flicker of the input overlay */
                    if (input_win) {
                        ui_redraw_output_no_update();
                        /* terminal overlay redraw (no update) */
                        extern void view_terminal_redraw_input_no_update(void);
                        view_terminal_redraw_input_no_update();
                        doupdate();
                    } else {
                        ui_redraw_output();
                    }
                }
                return 0;
            } else if (me.bstate & BUTTON5_PRESSED) {
                /* Only scroll output when the Terminal view is active. */
                if (output_win && current_view == VIEW_TERMINAL) {
                    out_scroll_lines -= 3;
                    if (out_scroll_lines < 0) out_scroll_lines = 0;
                    if (input_win) {
                        ui_redraw_output_no_update();
                        extern void view_terminal_redraw_input_no_update(void);
                        view_terminal_redraw_input_no_update();
                        doupdate();
                    } else {
                        ui_redraw_output();
                    }
                }
                return 0;
            }
        }
    }

    /* Give the active view the first chance to handle the input. Views
     * may return non-zero to indicate the key was handled. This keeps
     * per-view input logic local to the view implementation. */
    if (ui_view_handle_input(ch)) {
        /* If the view handled the input, it may have produced a completed
         * line. Ask the active view for a line and return it to the caller
         * if present. */
        int got = ui_view_fetch_line(buf, max);
        if (got > 0) return got;
        return 0;
    }

    /* If not handled by a view, some global keys (page up/down) are
     * still useful to affect the output scrollback. Handle them here. */
    if (ch == KEY_PPAGE) {
        /* Only handle PageUp when Terminal view is active so other views
         * can choose their own behavior for the key. Views had the first
         * chance to handle the key via ui_view_handle_input(), so this
         * code is a fallback that affects scrollback only for Terminal. */
        if (output_win && current_view == VIEW_TERMINAL) {
            int h, w;
            getmaxyx(output_win, h, w);
            (void)w;
            out_scroll_lines += h;
            if (out_scroll_lines < 0) out_scroll_lines = 0;
            if (input_win) {
                ui_redraw_output_no_update();
                extern void view_terminal_redraw_input_no_update(void);
                view_terminal_redraw_input_no_update();
                doupdate();
            } else {
                ui_redraw_output();
            }
        }
        return 0;
    } else if (ch == KEY_NPAGE) {
        /* Only handle PageDown when Terminal view is active. */
        if (output_win && current_view == VIEW_TERMINAL) {
            int h, w;
            getmaxyx(output_win, h, w);
            (void)w;
            out_scroll_lines -= h;
            if (out_scroll_lines < 0) out_scroll_lines = 0;
            if (input_win) {
                ui_redraw_output_no_update();
                extern void view_terminal_redraw_input_no_update(void);
                view_terminal_redraw_input_no_update();
                doupdate();
            } else {
                ui_redraw_output();
            }
        }
        return 0;
    }

    /* Nothing handled or produced; return 0. */
    return 0;
}
