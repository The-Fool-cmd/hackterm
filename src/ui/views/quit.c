#include <ncurses.h>
#include <string.h>

#include "../ui_internal.h"
#include "ui_view.h"
#include "ui.h"

void view_quit_render(WINDOW* win) {
    if (!win) return;
    werase(win);
    if (has_colors()) wbkgd(win, COLOR_PAIR(3));
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Quit selected - press Enter in input to exit.");
    wrefresh(win);
}

int view_quit_input(int ch) {
    (void)ch; return 0;
}
