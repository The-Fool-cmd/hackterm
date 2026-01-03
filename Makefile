
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Use pkg-config to find Lua; fall back to common linker flags if pkg-config
# isn't available on the system. This keeps the Makefile concise and portable.
LUA_CFLAGS := $(shell pkg-config --cflags lua5.3 lua 2>/dev/null || echo)
LUA_LIBS := $(shell pkg-config --libs lua5.3 lua 2>/dev/null || echo -llua -lm -ldl)

CFLAGS += $(LUA_CFLAGS)
LDLIBS := -lncurses $(LUA_LIBS)

SRC = src/main.c src/ui.c src/commands.c src/core_commands.c src/game.c src/server.c src/script.c src/script_api.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: hackterm

hackterm: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) hackterm
