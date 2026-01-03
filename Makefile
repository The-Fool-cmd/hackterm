CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lncurses

SRC = src/main.c src/ui.c src/commands.c src/core_commands.c src/game.c src/server.c
OBJ = $(SRC:.c=.o)

hackterm: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJ) hackterm
