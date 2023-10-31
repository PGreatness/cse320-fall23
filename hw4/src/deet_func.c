#include "deet_func.h"

int count_and_add_args(char* buffer, char* args[], int* num_args)
{
    char* token = strtok(buffer, " ");
    // consume the first token (the command)
    token = strtok(NULL, " ");
    int i = 0;
    while (token != NULL)
    {
        // copy the token into the args array
        size_t size = strlen(token) + 1;
        args[i] = (char*)malloc(sizeof(char) * size);
        if (args[i] == NULL)
        {
            fprintf(stderr, "Error: initialization of args[%d] in count_and_add_args() failed.\n", i);
            exit(1);
        }
        strncpy(args[i], token, size);
        i++;
        token = strtok(NULL, " ");
    }
    *num_args = i;
    return 0;
}

int ask_for_input(FILE* stream, char*args[], int* num_args)
{
    size_t size = MAX_LINE;
    char* buffer = (char*)malloc(size);
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: initialization of buffer in input failed.\n");
        exit(1);
    }
    printf("deet> ");
    // flush the buffer
    fflush(stdout);
    size_t chars_read = getline(&buffer, &size, stream);
    if (chars_read == -1)
    {
        free(buffer);
        fprintf(stderr, "Error: getline() failed.\n");
        return -1;
    }
    if (chars_read < 2)
    {
        free(buffer);
        debug("No input detected.\n");
        return -2;
    }
    if (chars_read > MAX_LINE)
    {
        free(buffer);
        fprintf(stderr, "Error: input too long.\n");
        return -1;
    }
    // compare the buffer with the commands available
    // if the command is valid, call the corresponding function
    // if the command is invalid, print an error message

    char* buffer_token = strtok(buffer, " ");
    if (strlen(buffer_token) <= 1)
    {
        free(buffer);
        return -2;
    }
    buffer_token = strtok(buffer_token, "\n");

    for (int i = 0; i < NUM_CMDS; i++)
    {
        debug("Comparing %s with %s\n", buffer_token, commands[i].name);

        size_t command_length = strlen(commands[i].name);
        if (strlen(buffer_token) != command_length)
            continue;
        if (strncasecmp(buffer_token, commands[i].name, command_length) == 0)
        {
            debug("Command %s found.\n", commands[i].name);
            // count the number of arguments
            count_and_add_args(buffer, args, num_args);
            free(buffer);
            return commands[i].command;
        }
    }
    return -2;
}

void print_help()
{
    fprintf(stdout, "Available commands:\n");
    for (int i = 0; i < NUM_CMDS; i++)
    {
        char* arg_range = (char*)malloc(sizeof(char) * 20);
        if (arg_range == NULL)
        {
            fprintf(stderr, "Error: initialization of arg_range in print_help() failed.\n");
            exit(1);
        }
        if (commands[i].min_args == commands[i].max_args)
        {
            sprintf(arg_range, "%d", commands[i].min_args);
        }
        else if (commands[i].max_args == -1)
        {
            sprintf(arg_range, ">=%d", commands[i].min_args);
        }
        else if (commands[i].min_args == -1)
        {
            sprintf(arg_range, "<=%d", commands[i].max_args);
        }
        else
        {
            sprintf(arg_range, "%d-%d", commands[i].min_args, commands[i].max_args);
        }

        fprintf(stdout, "%s (%s args) -- %s\n", commands[i].name, arg_range, commands[i].desc);
        free(arg_range);
    }
}
