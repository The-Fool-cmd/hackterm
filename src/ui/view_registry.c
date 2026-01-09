/*
 * view_registry.c
 *
 * Simple registry for UI views. The registry holds pointers for each
 * view enum (VIEW_HOME..VIEW_QUIT) and exposes helper functions used by
 * the renderer and input loop.
 */

#include <ncurses.h>
#include <string.h>

#include "ui_internal.h"
#include "ui_view.h"
#include "ui.h"

/* Static array indexed by ui_view_t enum value. NULL = no view registered */
static const UIView* view_table[VIEW_COUNT] = { 0 };
static int views_registered = 0;

/* Forward declarations for builtin views (implemented in src/ui/views) */
void view_home_render(WINDOW* win);
int view_home_input(int ch);

/* Terminal view implemented in separate file */
void view_terminal_render(WINDOW* win);
int view_terminal_input(int ch);
int view_terminal_fetch_line(char* buf, int max);

void view_settings_render(WINDOW* win);
int view_settings_input(int ch);

void view_city_render(WINDOW* win);
int view_city_input(int ch);

void view_quit_render(WINDOW* win);
int view_quit_input(int ch);

/* Define static descriptors */
static const UIView home_view = {
    .id = "home",
    .init = NULL,
    .teardown = NULL,
    .render = view_home_render,
    .handle_input = view_home_input,
};

static const UIView terminal_view = {
    .id = "terminal",
    .init = NULL,
    .teardown = NULL,
    .render = view_terminal_render,
    .handle_input = view_terminal_input,
    .fetch_line = view_terminal_fetch_line,
};

static const UIView settings_view = {
    .id = "settings",
    .init = NULL,
    .teardown = NULL,
    .render = view_settings_render,
    .handle_input = view_settings_input,
};

static const UIView city_view = {
    .id = "city",
    .init = NULL,
    .teardown = NULL,
    .render = view_city_render,
    .handle_input = view_city_input,
};

static const UIView quit_view = {
    .id = "quit",
    .init = NULL,
    .teardown = NULL,
    .render = view_quit_render,
    .handle_input = view_quit_input,
};

/**
 * @brief Register builtin views into the table.
 *
 * This maps the `ui_view_t` enum values to the static descriptors
 * above. It's intentionally simple — future work may add dynamic
 * registration by id/name.
 */
void ui_register_builtin_views(void) {
    if (views_registered) return;

    view_table[VIEW_HOME] = &home_view;
    view_table[VIEW_TERMINAL] = &terminal_view;
    view_table[VIEW_SETTINGS] = &settings_view;
    view_table[VIEW_CITY] = &city_view;
    view_table[VIEW_QUIT] = &quit_view;

    views_registered = 1;
}

/**
 * @brief Render the currently active view.
 */
void ui_view_render_active(WINDOW* win) {
    if ((size_t)current_view >= VIEW_COUNT) return;
    const UIView* v = view_table[current_view];
    if (!v || !v->render) return;
    v->render(win);
}

/**
 * @brief Forward input to the active view's handler.
 */
int ui_view_handle_input(int ch) {
    if ((size_t)current_view >= VIEW_COUNT) return 0;
    const UIView* v = view_table[current_view];
    if (!v || !v->handle_input) return 0;
    return v->handle_input(ch);
}

/**
 * @brief Ask the active view for a completed input line, if any.
 *
 * Delegates to the active view's optional `fetch_line` callback.
 */
int ui_view_fetch_line(char* buf, int max) {
    if ((size_t)current_view >= VIEW_COUNT) return 0;
    const UIView* v = view_table[current_view];
    if (!v || !v->fetch_line) return 0;
    return v->fetch_line(buf, max);
}

/* Builtin views are implemented in separate compilation units under
 * src/ui/views/. The registry holds pointers to their descriptors
 * defined earlier in this file. */
