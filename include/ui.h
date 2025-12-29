#ifndef UI_H
#define UI_H

void ui_init(void);
void ui_shutdown(void);

void ui_print(const char *fmt, ...);
int  ui_readline(char *buf, int max);

void ui_render(void);

#endif
