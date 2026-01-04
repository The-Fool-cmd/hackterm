#include <ncurses.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdio.h>

#include "ui.h"
#include "ui_internal.h"
#include "commands.h"

int ui_readline_nonblocking(char* buf, int max) {
    if (!input_win) return 0;

    int ch = wgetch(input_win);  // non-blocking
    if (ch == ERR) {
        return 0;  // no input yet
    }

    if (ch == KEY_MOUSE) {
        MEVENT me;
        if (getmouse(&me) == OK) {
            if (me.bstate & BUTTON4_PRESSED) {
                if (output_win) {
                    out_scroll_lines += 3;
                    ui_redraw_output();
                }
                return 0;
            } else if (me.bstate & BUTTON5_PRESSED) {
                if (output_win) {
                    out_scroll_lines -= 3;
                    if (out_scroll_lines < 0) out_scroll_lines = 0;
                    ui_redraw_output();
                }
                return 0;
            }
        }
    }

    if (ch == '\n' || ch == '\r') {  // Enter pressed
        input_buf[input_len] = '\0';
        strncpy(buf, input_buf, max);
        int len = input_len;

        if (len > 0) {
            if (history_count == 0 || strcmp(input_history[(history_count - 1) % INPUT_HISTORY_MAX], input_buf) != 0) {
                strncpy(input_history[history_count % INPUT_HISTORY_MAX], input_buf, INPUT_BUF_SIZE - 1);
                input_history[history_count % INPUT_HISTORY_MAX][INPUT_BUF_SIZE - 1] = '\0';
                history_count++;
            }
            history_pos = history_count;
        }

        input_len = 0;
        memset(input_buf, 0, sizeof(input_buf));
        return len;
    } else if (ch == KEY_BACKSPACE || ch == 127) {
        if (input_len > 0) {
            input_len--;
            input_buf[input_len] = '\0';
        }
    } else if (ch == '\t') {
        /* Autocomplete against available commands. Find the current token. */
        int pos = input_len - 1;
        while (pos >= 0 && !isspace((unsigned char)input_buf[pos])) pos--;
        int start = pos + 1;
        int token_len = input_len - start;
        if (token_len <= 0) {
            return 0; /* nothing to complete */
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
        return 0;
    } else if (ch == KEY_UP) {
        if (history_count > 0 && history_pos > 0) {
            history_pos--;
            int idx = history_pos % INPUT_HISTORY_MAX;
            strncpy(input_buf, input_history[idx], INPUT_BUF_SIZE - 1);
            input_buf[INPUT_BUF_SIZE - 1] = '\0';
            input_len = strlen(input_buf);
        }
    } else if (ch == KEY_DOWN) {
        if (history_count > 0 && history_pos < history_count - 1) {
            history_pos++;
            int idx = history_pos % INPUT_HISTORY_MAX;
            strncpy(input_buf, input_history[idx], INPUT_BUF_SIZE - 1);
            input_buf[INPUT_BUF_SIZE - 1] = '\0';
            input_len = strlen(input_buf);
        } else if (history_pos >= history_count - 1) {
            history_pos = history_count;
            input_buf[0] = '\0';
            input_len = 0;
        }
    } else if (ch == KEY_PPAGE) {
        if (output_win) {
            int h, w;
            getmaxyx(output_win, h, w);
            (void)h; (void)w;
            out_scroll_lines += h;
            if (out_scroll_lines < 0) out_scroll_lines = 0;
        }
    } else if (ch == KEY_NPAGE) {
        if (output_win) {
            int h, w;
            getmaxyx(output_win, h, w);
            (void)h; (void)w;
            out_scroll_lines -= h;
            if (out_scroll_lines < 0) out_scroll_lines = 0;
        }
    } else if (ch >= 32 && ch <= 126) {
        if (input_len < INPUT_BUF_SIZE - 1) {
            input_buf[input_len++] = (char)ch;
            input_buf[input_len] = '\0';
        }
    }

    /* redraw current input line */
    for (int y = 1; y < input_height - 1; y++) {
        wmove(input_win, y, 1);
        wclrtoeol(input_win);
    }
    mvwprintw(input_win, prompt_y, 1, "> %s", input_buf);
    /* place cursor after the printed prompt (print_x + length_of"> ") */
    wmove(input_win, prompt_y, 1 + prompt_x + input_len);
    wrefresh(input_win);

    return 0;
}
