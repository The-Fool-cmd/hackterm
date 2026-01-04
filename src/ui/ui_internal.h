#ifndef SRC_UI_UI_INTERNAL_H_
#define SRC_UI_UI_INTERNAL_H_

#include <ncurses.h>

#define INPUT_BUF_SIZE 256
#define INPUT_HISTORY_MAX 64
#define OUT_HISTORY_MAX 4096
#define OUT_LINE_MAX 1024

/* shared state between ui implementation files */
extern WINDOW* header_win;
extern WINDOW* output_win;
extern WINDOW* input_win;

extern int term_rows;
extern int term_cols;
extern int header_height;
extern int input_height;
extern int prompt_x;
extern int prompt_y;

extern char input_buf[INPUT_BUF_SIZE];
extern int input_len;

extern char input_history[INPUT_HISTORY_MAX][INPUT_BUF_SIZE];
extern int history_count;
extern int history_pos;

extern char status_buf[256];

/* output scrollback */
extern char* out_lines[OUT_HISTORY_MAX];
extern int out_start;
extern int out_count;
extern int out_scroll_lines;

/* internal helpers used across files */
void ui_layout(void);

/**
 * @internal
 * @brief Redraw the output (scrollback) window.
 *
 * This is an internal helper used by input handling and after layout
 * changes (e.g. terminal resize) to refresh the output window contents
 * from the in-memory scrollback buffer. Not part of the public API.
 */
void ui_redraw_output(void);
void ui_redraw_output(void);

#endif /* SRC_UI_UI_INTERNAL_H_ */
