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
    int total = out_count;
    if (total == 0) {
        wrefresh(output_win);
        return;
    }
    int visible = h;
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
        mvwprintw(output_win, i, 0, "%.*s", w - 1, line);
    }
    wrefresh(output_win);
}

/* non-static wrapper exported to other ui modules */
void ui_redraw_output(void) {
    redraw_output();
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
