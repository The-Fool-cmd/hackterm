#include <ncurses.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

#include "ui.h"
#include "../ui_internal.h"
#include "commands.h"
#include "ui_view.h"

/* Pending completed line produced when Enter is pressed. */
static char pending_line[INPUT_BUF_SIZE];
static int pending_len = 0;

/* Terminal-owned input state */
static char input_buf[INPUT_BUF_SIZE];
static int input_len = 0;
static char input_history[INPUT_HISTORY_MAX][INPUT_BUF_SIZE];
static int history_count = 0;
static int history_pos = 0;

/* Forward declaration so the immediate redraw helper can call the
 * no-update variant without implicit declaration warnings. */
void view_terminal_redraw_input_no_update(void);

void view_terminal_render(WINDOW* win) {
    /* Terminal view uses the existing scrollback renderer which writes
     * directly into the output window. We still box the area here so the
     * terminal has a border. */
    if (!win) return;
    box(win, 0, 0);
    /* Draw output into backing buffer but don't call doupdate here; the
     * top-level renderer will batch the input overlay and call doupdate. */
    ui_redraw_output_no_update();
}

static void terminal_redraw_input(void) {
    if (!input_win) return;
    /* use the immediate refresh path by default */
    view_terminal_redraw_input_no_update();
    doupdate();
}

/* Redraw the input overlay into the curses backing buffer but do not
 * call doupdate(); this allows callers to batch the output + input
 * redraw and then update the physical screen once to avoid flicker. */
void view_terminal_redraw_input_no_update(void) {
    if (!input_win) return;
    werase(input_win);
    if (has_colors()) wbkgd(input_win, COLOR_PAIR(2));
    box(input_win, 0, 0);
    mvwprintw(input_win, prompt_y, 1, "> %s", input_buf);
    wmove(input_win, prompt_y, 1 + prompt_x + input_len);
    wnoutrefresh(input_win);
}

int view_terminal_input(int ch) {
    /* Handle basic line editing for the terminal prompt. Return 1 when the
     * key was handled. If Enter is pressed we stash the line into
     * `pending_line` so the input wrapper can consume it. */
    if (ch == ERR) return 0;

    if (ch == '\n' || ch == '\r') {
        input_buf[input_len] = '\0';
        /* copy into pending buffer */
        pending_len = (int)strnlen(input_buf, INPUT_BUF_SIZE - 1);
        if (pending_len > 0) {
            strncpy(pending_line, input_buf, INPUT_BUF_SIZE - 1);
            pending_line[INPUT_BUF_SIZE - 1] = '\0';
            /* history push */
            if (history_count == 0 || strcmp(input_history[(history_count - 1) % INPUT_HISTORY_MAX], input_buf) != 0) {
                strncpy(input_history[history_count % INPUT_HISTORY_MAX], input_buf, INPUT_BUF_SIZE - 1);
                input_history[history_count % INPUT_HISTORY_MAX][INPUT_BUF_SIZE - 1] = '\0';
                history_count++;
            }
            history_pos = history_count;
        } else {
            /* empty line -> no pending */
            pending_len = 0;
        }
        /* reset input buffer */
        input_len = 0;
        input_buf[0] = '\0';
        terminal_redraw_input();
        return 1;
    } else if (ch == KEY_BACKSPACE || ch == 127) {
        if (input_len > 0) {
            input_len--;
            input_buf[input_len] = '\0';
            terminal_redraw_input();
        }
        return 1;
    } else if (ch == '\t') {
        /* Autocomplete against available commands. Find the current token. */
        int pos = input_len - 1;
        while (pos >= 0 && !isspace((unsigned char)input_buf[pos])) pos--;
        int start = pos + 1;
        int token_len = input_len - start;
        if (token_len <= 0) {
            return 1; /* nothing to complete */
        }

        char token[INPUT_BUF_SIZE];
        int copy_len = token_len < (int)sizeof(token) - 1 ? token_len : (int)sizeof(token) - 1;
        memcpy(token, &input_buf[start], copy_len);
        token[copy_len] = '\0';

        int matches = 0;
        const char* last_match = NULL;
        for (int i = 0; i < commands_count(); i++) {
            const char* name = commands_name(i);
            if (!name) continue;
            if (strncasecmp(name, token, token_len) == 0) {
                matches++;
                if (matches == 1) last_match = name;
                if (matches > 100) break;
            }
        }

        if (matches == 0) {
            ui_print("(no matches)");
        } else if (matches == 1 && last_match) {
            /* complete the token with the remaining characters */
            const char* rest = last_match + token_len;
            while (*rest && input_len < INPUT_BUF_SIZE - 1) {
                input_buf[input_len++] = *rest++;
            }
            input_buf[input_len] = '\0';
        } else {
            ui_print("Matches:");
            int shown = 0;
            for (int i = 0; i < commands_count(); i++) {
                const char* name = commands_name(i);
                if (!name) continue;
                if (strncasecmp(name, token, token_len) == 0) {
                    ui_print("  %s", name);
                    if (++shown >= 50) break;
                }
            }
        }
        terminal_redraw_input();
        return 1;
    } else if (ch == KEY_UP) {
        if (history_count > 0 && history_pos > 0) {
            history_pos--;
            int idx = history_pos % INPUT_HISTORY_MAX;
            strncpy(input_buf, input_history[idx], INPUT_BUF_SIZE - 1);
            input_buf[INPUT_BUF_SIZE - 1] = '\0';
            input_len = strlen(input_buf);
            terminal_redraw_input();
        }
        return 1;
    } else if (ch == KEY_DOWN) {
        if (history_count > 0 && history_pos < history_count - 1) {
            history_pos++;
            int idx = history_pos % INPUT_HISTORY_MAX;
            strncpy(input_buf, input_history[idx], INPUT_BUF_SIZE - 1);
            input_buf[INPUT_BUF_SIZE - 1] = '\0';
            input_len = strlen(input_buf);
            terminal_redraw_input();
        } else if (history_pos >= history_count - 1) {
            history_pos = history_count;
            input_buf[0] = '\0';
            input_len = 0;
            terminal_redraw_input();
        }
        return 1;
    } else if (ch >= 32 && ch <= 126) {
        if (input_len < INPUT_BUF_SIZE - 1) {
            input_buf[input_len++] = (char)ch;
            input_buf[input_len] = '\0';
            terminal_redraw_input();
        }
        return 1;
    }

    return 0;
}

int view_terminal_fetch_line(char* buf, int max) {
    if (pending_len <= 0) return 0;
    int copy = pending_len < max - 1 ? pending_len : max - 1;
    memcpy(buf, pending_line, copy);
    buf[copy] = '\0';
    /* clear pending */
    pending_len = 0;
    pending_line[0] = '\0';
    return copy;
}
