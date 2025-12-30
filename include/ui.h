#ifndef UI_H
#define UI_H

#include <stdarg.h>

/* ---------------- LIFECYCLE ---------------- */

/**
 * @brief Initializes the user interface subsystem.
 * 
 * Prepares internal structures and any required resources.
 */
void ui_init(void);

/**
 * @brief Shuts down the user interface subsystem.
 * 
 * Frees resources and performs cleanup.
 */
void ui_shutdown(void);

/* ---------------- INPUT/OUTPUT ---------------- */

/**
 * @brief Prints formatted text to the UI output.
 * 
 * Works like printf, supporting format specifiers.
 * 
 * @param fmt Format string.
 * @param ... Additional arguments as required by the format string.
 */
void ui_print(const char *fmt, ...);

/**
 * @brief Reads a line of input from the user.
 * 
 * @param buf Buffer to store the input line.
 * @param max Maximum number of characters to read (including null terminator).
 * @return Number of characters read, not including the null terminator.
 */
int ui_readline(char *buf, int max);

/* ---------------- RENDERING ---------------- */

/**
 * @brief Renders the current state of the UI to the screen.
 * 
 * Should be called once per frame or after any state changes.
 */
void ui_render(void);

#endif /* UI_H */
