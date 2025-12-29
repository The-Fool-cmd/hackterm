#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"

static WINDOW *output_win;
static WINDOW *input_win;

static int term_rows, term_cols;
static int input_height = 3;
static int prompt_x = 2;
static int prompt_y = 1;

static void ui_layout(void) {
	getmaxyx(stdscr, term_rows, term_cols);

	int out_height = term_rows - input_height;
	if (out_height < 1) {
		out_height = 1;
	}

	if (output_win) {
		delwin(output_win);
	}
	if (input_win) {
		delwin(input_win);
	}

	output_win = newwin(out_height, term_cols, 0, 0);
	input_win  = newwin(input_height, term_cols, out_height, 0);

	scrollok(output_win, TRUE);
	keypad(input_win, TRUE);
	
	// terminal-like
	wmove(output_win, 0, 0);
	wrefresh(output_win);
}

void ui_init(void) {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(1);

	ui_layout();
}

void ui_shutdown(void) {
	if (output_win) {
		delwin(output_win);
		output_win = NULL;
	}
	if (input_win) {
		delwin(input_win);
		input_win = NULL;
	}
	endwin();
}

void ui_print(const char *fmt, ...) {
	if (!output_win) return;

	char buffer[1024];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	// print and scroll
	wprintw(output_win, "%s\n", buffer);
	wrefresh(output_win);
}

int ui_readline(char *buf, int max) {
	if (!input_win) return 0;

	for (int y = 1; y < input_height - 1; y++) {
		wmove(input_win, y, 1);
		wclrtoeol(input_win);
	}

	// Prompt
	mvwprintw(input_win, prompt_y, 1, "> ");
	wmove(input_win, prompt_y, prompt_x);

	// read
	echo();
	wgetnstr(input_win, buf, max - 1);
	noecho();

	// return length
	return (int)strlen(buf);
}

void ui_render(void) {
	if (!input_win) return;

	// handle terminal resizing
	int r, c;
	getmaxyx(stdscr, r, c);
	if (r != term_rows || c != term_cols) {
		endwin();
		refresh();
		clear();
		ui_layout();
	}

	werase(input_win);
	box(input_win, 0, 0);
	wrefresh(input_win);
}
