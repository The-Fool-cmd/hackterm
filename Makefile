
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Use pkg-config to find Lua; fall back to common linker flags if pkg-config
# isn't available on the system. This keeps the Makefile concise and portable.
LUA_CFLAGS := $(shell pkg-config --cflags lua5.3 lua 2>/dev/null || echo)
LUA_LIBS := $(shell pkg-config --libs lua5.3 lua 2>/dev/null || echo -llua -lm -ldl)

CFLAGS += $(LUA_CFLAGS)
LDLIBS := -lncurses $(LUA_LIBS)

SRC = src/main.c src/ui/state.c src/ui/init.c src/ui/view_registry.c src/ui/output.c src/ui/input.c src/ui/render.c src/ui/views/terminal.c src/ui/views/home.c src/ui/views/settings.c src/ui/views/city.c src/ui/views/quit.c src/commands.c src/core_commands.c src/game.c src/server.c src/script.c src/script_api.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: hackterm

hackterm: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

# Generate documentation using Doxygen (requires doxygen installed)
.PHONY: docs
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile && echo "Doxygen finished. Open docs/html/index.html"; \
	else \
		echo "doxygen not found. Please install doxygen to build docs."; \
	fi

.PHONY: docs-serve
docs-serve: docs
	@if command -v python3 >/dev/null 2>&1; then \
		echo "Serving docs at http://localhost:8000"; \
		( cd docs/html && python3 -m http.server 8000 >/dev/null 2>&1 & ); \
		sleep 0.2; \
		if command -v xdg-open >/dev/null 2>&1; then \
			xdg-open http://localhost:8000 >/dev/null 2>&1 & \
		elif command -v sensible-browser >/dev/null 2>&1; then \
			sensible-browser http://localhost:8000 >/dev/null 2>&1 & \
		else \
			python3 -m webbrowser -t "http://localhost:8000" >/dev/null 2>&1 & \
		fi; \
	else \
		echo "python3 not found; please install Python 3 to serve docs."; exit 1; \
	fi

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) hackterm
	rm -rf docs
