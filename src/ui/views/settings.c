#include <ncurses.h>
#include <string.h>

#include "../ui_internal.h"
#include "ui_view.h"
#include "ui.h"

void view_settings_render(WINDOW* win) {
    if (!win) return;
    werase(win);
    if (has_colors()) wbkgd(win, COLOR_PAIR(3));
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Settings view - configuration options:");
    mvwprintw(win, 3, 4, "(placeholder)");
    wrefresh(win);
}

int view_settings_input(int ch) {
    (void)ch; return 0;
}
