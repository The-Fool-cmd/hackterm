#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "ui.h"
#include "ui_internal.h"

static void out_push(const char* s) {
    char* copy = malloc(OUT_LINE_MAX);
    if (!copy) return;
    strncpy(copy, s, OUT_LINE_MAX - 1);
    copy[OUT_LINE_MAX - 1] = '\0';

    if (out_count < OUT_HISTORY_MAX) {
        int idx = (out_start + out_count) % OUT_HISTORY_MAX;
        out_lines[idx] = copy;
        out_count++;
    } else {
        free(out_lines[out_start]);
        out_lines[out_start] = copy;
        out_start = (out_start + 1) % OUT_HISTORY_MAX;
    }
    /* new output resets scroll to bottom */
    out_scroll_lines = 0;
}

static void redraw_output(void) {
    if (!output_win) return;
    int h, w;
    getmaxyx(output_win, h, w);
    werase(output_win);
    /* draw a border around the output area so terminal view has a box */
    box(output_win, 0, 0);
    int total = out_count;
    if (total == 0) {
        wrefresh(output_win);
        return;
    }
    /* content area is inside the box.
     * If the terminal input overlay is present, reserve that many rows
     * at the bottom so scrollback lines are not drawn underneath it.
     * Otherwise allow views to use the full area. */
    int reserved_bottom = 0;
    if (input_win && current_view == VIEW_TERMINAL) {
        reserved_bottom = input_height;
    }
    int content_h = h - 2 - reserved_bottom;
    int content_w = w - 2;
    if (content_h < 1 || content_w < 1) {
        wrefresh(output_win);
        return;
    }
    int visible = content_h;
    if (visible > total) visible = total;
    if (out_scroll_lines < 0) out_scroll_lines = 0;
    if (out_scroll_lines > total - visible) out_scroll_lines = total - visible;

    int first = total - visible - out_scroll_lines;
    if (first < 0) first = 0;
    for (int i = 0; i < visible; i++) {
        int logical = first + i;
        int idx = (out_start + logical) % OUT_HISTORY_MAX;
        const char* line = out_lines[idx];
        if (!line) continue;
        /* write inside the box with 1-row/1-col offset */
        mvwprintw(output_win, 1 + i, 1, "%.*s", content_w, line);
    }
    wrefresh(output_win);
}

/* redraw into curses' backing buffer but do not call doupdate(). This
 * allows callers to redraw both output and other windows (e.g. the
 * terminal input overlay) and then call doupdate() once to avoid
 * flicker. */
static void redraw_output_no_update(void) {
    if (!output_win) return;
    int h, w;
    getmaxyx(output_win, h, w);
    werase(output_win);
    box(output_win, 0, 0);
    int total = out_count;
    if (total == 0) {
        wnoutrefresh(output_win);
        return;
    }
    int reserved_bottom = 0;
    if (input_win && current_view == VIEW_TERMINAL) {
        reserved_bottom = input_height;
    }
    int content_h = h - 2 - reserved_bottom;
    int content_w = w - 2;
    if (content_h < 1 || content_w < 1) {
        wnoutrefresh(output_win);
        return;
    }
    int visible = content_h;
    if (visible > total) visible = total;
    if (out_scroll_lines < 0) out_scroll_lines = 0;
    if (out_scroll_lines > total - visible) out_scroll_lines = total - visible;

    int first = total - visible - out_scroll_lines;
    if (first < 0) first = 0;
    for (int i = 0; i < visible; i++) {
        int logical = first + i;
        int idx = (out_start + logical) % OUT_HISTORY_MAX;
        const char* line = out_lines[idx];
        if (!line) continue;
        mvwprintw(output_win, 1 + i, 1, "%.*s", content_w, line);
    }
    wnoutrefresh(output_win);
}

/* non-static wrapper exported to other ui modules */
void ui_redraw_output(void) {
    redraw_output();
}

void ui_redraw_output_no_update(void) {
    redraw_output_no_update();
}

void ui_print(const char* fmt, ...) {
    if (!output_win) return;

    char buffer[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    out_push(buffer);
    ui_redraw_output();
}
