#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum {
    CMD_OK = 0,
    CMD_QUIT = 1
} CommandResult;

CommandResult commands_run(const char *input);

#endif
