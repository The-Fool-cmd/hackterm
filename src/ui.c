#include <ncurses.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"

static WINDOW *output_win;
static WINDOW *input_win;

void ui_init(void) {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(1);

	int h, w;
	getmaxyx(stdscr, h, w);

	output_win = newwin(h - 3, w, 0, 0);
	input_win  = newwin(3, w, h - 3, 0);

	scrollok(output_win, TRUE);
	box(input_win, 0, 0);

	wrefresh(output_win);
	wrefresh(input_win);
}

void ui_shutdown(void) {
	endwin();
}

void ui_print(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vw_printw(output_win, fmt, args);
	va_end(args);

	wprintw(output_win, "\n");
	wrefresh(output_win);
}

int ui_readline(char *buf, int max) {
	memset(buf, 0, max);

	mvwprintw(input_win, 1, 1, "> ");
	wclrtoeol(input_win);
	wrefresh(input_win);

	echo();
	wgetnstr(input_win, buf, max - 1);
	noecho();

	return strlen(buf);
}

void ui_render(void) {
	box(input_win, 0, 0);
	wrefresh(input_win);
}
