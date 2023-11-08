#include "deet_func.h"

int suppress = 0;

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
            debug("Error: initialization of args[%d] in count_and_add_args() failed.\n", i);
            exit(1);
        }
        strncpy(args[i], token, size);
        i++;
        token = strtok(NULL, " ");
    }
    if (i != 0)
    {
        if (args[i - 1][strlen(args[i - 1]) - 1] == '\n')
            args[i - 1][strlen(args[i - 1]) - 1] = '\0';
    }
    args[i] = NULL;
    *num_args = i;
    return 0;
}

void print_help()
{
    int file = STDOUT_FILENO;
    print_string(file, "Available commands:\n");
    for (int i = 0; i < NUM_CMDS; i++)
    {
        print_string(file, commands[i].name);
        print_string(file, " (");
        if (commands[i].min_args == commands[i].max_args)
        {
            print_int(file, commands[i].min_args);
        }
        else if (commands[i].max_args == -1)
        {
            print_string(file, ">=");
            print_int(file, commands[i].min_args);
        }
        else if (commands[i].min_args == -1)
        {
            print_string(file, "<=");
            print_int(file, commands[i].max_args);
        }
        else
        {
            print_int(file, commands[i].min_args);
            print_string(file, "-");
            print_int(file, commands[i].max_args);
        }
        print_string(file, " args) -- ");
        print_string(file, commands[i].desc);
        print_string(file, "\n");
    }
}

int get_input(FILE* stream, char* args[], int* num_args)
{
    size_t size = MAX_LINE;
    char* buffer = (char*)malloc(size);
    if (buffer == NULL)
    {
        debug("Error: initialization of buffer in input failed.\n");
        exit(1);
    }
    // flush the buffers
    if (!shutdown)
        log_prompt();
    unblock_signal(SIGCHLD);
    if (!suppress)
        print_string(STDOUT_FILENO, "deet> ");
    // read the input
    int chars_read = getline(&buffer, &size, stream);
    if (buffer[0] == '\0')
    {
        // treat this as a quit
        args[0] = 0;
        *num_args = -1;
        free(buffer);
        return -2;
    }
    if (chars_read == -1)
    {
        args[0] = 0;
        *num_args = -1;
        free(buffer);
        debug("Error: getline() failed");
        return -2;
    }
    if (chars_read < 2)
    {
        args[0] = 0;
        *num_args = 0;
        free(buffer);
        debug("No input detected.\n");
        return -2;
    }
    if (chars_read > MAX_LINE)
    {
        args[0] = 0;
        *num_args = 0;
        free(buffer);
        debug("Error: input too long.\n");
        return -1;
    }
    log_input(buffer);
    // compare the buffer with the commands available
    // if the command is valid, call the corresponding function
    // if the command is invalid, print an error message

    char* tmp_buffer = (char*)malloc(sizeof(char) * chars_read);
    if (tmp_buffer == NULL)
    {
        debug("Error: initialization of tmp_buffer in input failed.\n");
        exit(1);
    }
    strncpy(tmp_buffer, buffer, chars_read);
    char* buffer_token = strtok(buffer, " ");
    if (strlen(buffer_token) <= 1)
    {
        free(buffer);
        return -2;
    }
    buffer_token = strtok(buffer_token, "\n");

    for (int i = 0; i < NUM_CMDS; i++)
    {
        // debug("Comparing %s with %s\n", buffer_token, commands[i].name);

        size_t command_length = strlen(commands[i].name);
        if (strlen(buffer_token) != command_length)
            continue;
        if (strncasecmp(buffer_token, commands[i].name, command_length) == 0)
        {
            debug("Command %s found.\n", commands[i].name);
            // count the number of arguments
            count_and_add_args(tmp_buffer, args, num_args);
            free(buffer);
            free(tmp_buffer);
            return commands[i].command;
        }
    }
    // copy the command into args[0]
    args[0] = (char*)malloc(sizeof(char) * (strlen(buffer_token) + 1));
    if (args[0] == NULL)
    {
        debug("Error: initialization of args[0] in input failed.\n");
        exit(1);
    }
    strncpy(args[0], buffer_token, strlen(buffer_token) + 1);
    // make num_args = 1
    *num_args = 1;
    free(buffer);
    free(tmp_buffer);
    return -2;
}