#ifndef HW4_COMMANDS_H
#define HW4_COMMANDS_H

typedef struct cmds
{
    int command;
    char* name;
    char* desc;
    int min_args;
    int max_args;
} COMMANDS;

#define CMD_HELP 0
#define CMD_QUIT 1
#define CMD_SHOW 2
#define CMD_RUN 3
#define CMD_STOP 4
#define CMD_CONT 5
#define CMD_RELEASE 6
#define CMD_WAIT 7
#define CMD_KILL 8
#define CMD_PEEK 9
#define CMD_POKE 10
#define CMD_BT 11

#define NUM_CMDS 12

#endif //HW4_COMMANDS_H