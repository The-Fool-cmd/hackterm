#ifndef INCLUDE_UI_VIEW_H_
#define INCLUDE_UI_VIEW_H_

#include <ncurses.h>
#include <stddef.h>

/**
 * @file ui_view.h
 * @brief Simple pluggable view interface for the UI subsystem.
 *
 * This header defines a minimal view interface that allows views to be
 * registered and swapped at runtime. Each view provides lifecycle hooks
 * so it can manage its own state and rendering into the main content
 * window.
 */

/**
 * @brief Opaque view handle.
 */
typedef struct UIView UIView;

/**
 * @brief View render callback.
 *
 * Called by the UI rendering loop to draw the view into the provided
 * content window. The view should not call `wrefresh()` for the whole
 * screen; normal drawing conventions apply.
 *
 * @param win Window allocated for the view's main content area.
 */
typedef void (*ui_view_render_fn)(WINDOW* win);

/**
 * @brief View input handler callback.
 *
 * Called with a single input character or key code. Return non-zero
 * if the view handled the key and no further processing is required.
 *
 * @param ch Input character/key code.
 * @return int 1 if handled, 0 otherwise.
 */
typedef int (*ui_view_input_fn)(int ch);

/**
 * @brief View initialization callback.
 *
 * Called once when the view becomes active. Use to allocate view-local
 * resources if needed.
 */
typedef void (*ui_view_init_fn)(void);

/**
 * @brief View teardown callback.
 *
 * Called when a view is being deactivated or unregistered.
 */
typedef void (*ui_view_teardown_fn)(void);

/**
 * @brief View descriptor struct.
 */
struct UIView {
    const char* id;              /**< unique id / name for the view */
    ui_view_init_fn init;        /**< optional init callback */
    ui_view_teardown_fn teardown;/**< optional teardown callback */
    ui_view_render_fn render;    /**< render callback (required) */
    ui_view_input_fn handle_input;/**< input handler (optional) */
    /**
     * Fetch a completed input line from the view, if available.
     * Called by the UI input wrapper (e.g. `ui_readline_nonblocking`) to
     * retrieve a submitted line (one per call). Return the number of
     * bytes written into `buf`, or 0 if none is available.
     */
    int (*fetch_line)(char* buf, int max);
};

/**
 * @brief Register builtin views and initialize the view registry.
 *
 * This registers the minimal set of views used by the application.
 * It is idempotent and can be called multiple times (subsequent calls
 * do nothing).
 */
void ui_register_builtin_views(void);

/**
 * @brief Render the currently active view into the provided window.
 *
 * Called by `ui_render()` to render the content area. If no view is
 * active, this is a no-op.
 *
 * @param win The window to render into.
 */
void ui_view_render_active(WINDOW* win);

/**
 * @brief Forward input to the active view's handler.
 *
 * If an active view handles the input (returns non-zero), this function
 * will return non-zero as well.
 *
 * @param ch Input character/key code.
 * @return int 1 if handled by the active view, 0 otherwise.
 */
int ui_view_handle_input(int ch);

/**
 * @brief Ask the active view for a completed input line, if any.
 *
 * This delegates to the active view's optional `fetch_line` callback.
 * Returns >0 when a line was written into `buf`.
 */
int ui_view_fetch_line(char* buf, int max);

#endif // INCLUDE_UI_VIEW_H_
