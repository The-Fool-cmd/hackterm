#include <ncurses.h>
#include <string.h>

#include "../ui_internal.h"
#include "ui_view.h"
#include "ui.h"

void view_home_render(WINDOW* win) {
    if (!win) return;
    werase(win);
    if (has_colors()) wbkgd(win, COLOR_PAIR(3));
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Welcome to hackterm - Home view");
    mvwprintw(win, 3, 2, "Use the sidebar to switch views.");
    wrefresh(win);
}

int view_home_input(int ch) {
    (void)ch;
    return 0;
}
