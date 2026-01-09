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
extern WINDOW* sidebar_win;

/* sidebar/menu state */
extern int sidebar_width;
extern const char** menu_items;
extern int menu_count;
extern int selected_menu;

/* current view shown in the main area */
typedef enum {
	VIEW_HOME = 0,
	VIEW_TERMINAL,
	VIEW_SETTINGS,
	VIEW_CITY,
	VIEW_QUIT,
	VIEW_COUNT
} ui_view_t;

extern ui_view_t current_view;

extern int term_rows;
extern int term_cols;
extern int header_height;
extern int input_height;
extern int prompt_x;
extern int prompt_y;


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
void ui_redraw_output_no_update(void);
#endif /* SRC_UI_UI_INTERNAL_H_ */
