#include "ui_internal.h"

WINDOW* header_win = NULL;
WINDOW* output_win = NULL;
WINDOW* input_win = NULL;
WINDOW* sidebar_win = NULL;

int term_rows = 0;
int term_cols = 0;
int header_height = 1;
int input_height = 3;
int prompt_x = 2;
int prompt_y = 1;

/* sidebar/menu state */
int sidebar_width = 20;
const char* menu_items_static[] = { "Home", "Terminal", "Settings", "City", "Quit" };
const char** menu_items = menu_items_static;
int menu_count = sizeof(menu_items_static) / sizeof(menu_items_static[0]);
int selected_menu = 0;

/* current view */
ui_view_t current_view = VIEW_HOME;

char status_buf[256] = "";

char* out_lines[OUT_HISTORY_MAX];
int out_start = 0;
int out_count = 0;
int out_scroll_lines = 0;

void ui_layout(void) {
    getmaxyx(stdscr, term_rows, term_cols);
    /* make the output area span the full height under the header so views
     * can fill the available space. The input is rendered as an overlay
     * at the bottom of the screen. */
    int out_height = term_rows - header_height;
    if (out_height < 1) out_height = 1;

    if (header_win) delwin(header_win);
    if (output_win) delwin(output_win);
    if (input_win) delwin(input_win);
    if (sidebar_win) delwin(sidebar_win);

    /* determine sidebar width (clamp for small terminals) */
    if (term_cols < 60) sidebar_width = 12;
    else sidebar_width = 20;
    if (sidebar_width > term_cols - 20) sidebar_width = term_cols / 4;

    header_win = newwin(header_height, term_cols, 0, 0);
    /* make sidebar extend from header to bottom of terminal */
    int sidebar_height = term_rows - header_height;
    if (sidebar_height < 1) sidebar_height = 1;
    sidebar_win = newwin(sidebar_height, sidebar_width, header_height, 0);
    output_win = newwin(out_height, term_cols - sidebar_width, header_height, sidebar_width);

    /* Only create the input overlay when the Terminal view is active.
     * This avoids keeping an input window around for other views and
     * reduces window ownership complexity. */
    if (current_view == VIEW_TERMINAL) {
        int input_starty = term_rows - input_height;
        if (input_starty < header_height + 1) input_starty = header_height + 1;
        input_win = newwin(input_height, term_cols - sidebar_width, input_starty, sidebar_width);
        keypad(input_win, TRUE);
        nodelay(input_win, TRUE);  // non-blocking input (for compatibility)
    } else {
        input_win = NULL;
    }

    scrollok(output_win, TRUE);

    // initial refresh
    wrefresh(sidebar_win);
    wrefresh(output_win);
    wrefresh(header_win);
}
