#include <ncurses.h>
#include <string.h>

#include "../ui_internal.h"
#include "ui_view.h"
#include "ui.h"

void view_city_render(WINDOW* win) {
    if (!win) return;
    werase(win);
    if (has_colors()) wbkgd(win, COLOR_PAIR(3));
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "City view - overview:");
    mvwprintw(win, 3, 4, "(placeholder) City center, population: 12345");
    wrefresh(win);
}

int view_city_input(int ch) {
    (void)ch; return 0;
}
