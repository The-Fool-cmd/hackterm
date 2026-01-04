#include "ui_internal.h"

WINDOW* header_win = NULL;
WINDOW* output_win = NULL;
WINDOW* input_win = NULL;

int term_rows = 0;
int term_cols = 0;
int header_height = 1;
int input_height = 3;
int prompt_x = 2;
int prompt_y = 1;

char input_buf[INPUT_BUF_SIZE];
int input_len = 0;

char input_history[INPUT_HISTORY_MAX][INPUT_BUF_SIZE];
int history_count = 0;
int history_pos = 0;

char status_buf[256] = "";

char* out_lines[OUT_HISTORY_MAX];
int out_start = 0;
int out_count = 0;
int out_scroll_lines = 0;

void ui_layout(void) {
    getmaxyx(stdscr, term_rows, term_cols);
    int out_height = term_rows - input_height - header_height;
    if (out_height < 1) out_height = 1;

    if (header_win) delwin(header_win);
    if (output_win) delwin(output_win);
    if (input_win) delwin(input_win);

    header_win = newwin(header_height, term_cols, 0, 0);
    output_win = newwin(out_height, term_cols, header_height, 0);
    input_win = newwin(input_height, term_cols, header_height + out_height, 0);

    scrollok(output_win, TRUE);
    keypad(input_win, TRUE);
    nodelay(input_win, TRUE);  // non-blocking input

    // initial refresh
    wrefresh(output_win);
    wrefresh(header_win);
}
