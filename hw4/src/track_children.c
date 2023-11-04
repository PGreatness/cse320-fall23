#include "track_children.h"

child_t sentinel = {
    .pid = 0,
    .status = PSTATE_NONE,
    .deetId = -1,
    .trace = 0,
    .command = NULL,
    .args = NULL,
    .next = NULL};

static pid_t next_deet_id = 0;

child_t* spawn_child(pid_t pid, int trace, char *args[], int num_args)
{
    child_t *new_child = malloc(sizeof(child_t));
    if (new_child == NULL)
    {
        debug("malloc failed");
        exit(1);
    }
    new_child->pid = pid;
    new_child->status = PSTATE_NONE;
    new_child->deetId = next_deet_id++;
    new_child->next = NULL;
    new_child->trace = trace;
    new_child->args = malloc(sizeof(char*) * (num_args + 1));
    if (new_child->args == NULL)
    {
        debug("malloc of args failed");
        exit(1);
    }
    for (int i = 0; i < num_args; i++)
    {
        new_child->args[i] = malloc(sizeof(char) * (strlen(args[i]) + 1));
        if (new_child->args[i] == NULL)
        {
            debug("malloc of args[%d] failed", i);
            exit(1);
        }
        strncpy(new_child->args[i], args[i], strlen(args[i]) + 1);
    }
    child_t *last = get_last_child();
    if (last == NULL)
    {
        sentinel.next = new_child;
    }
    else
    {
        last->next = new_child;
    }
    return new_child;
}

pid_t kill_child(pid_t pid)
{
    child_t *child = get_child(pid);
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    pid_t killed_pid = child->pid;
    child->pid = 0;
    child->status = PSTATE_KILLED;
    child->deetId = -1;
    child->trace = 0;
    next_deet_id--;
    return killed_pid;
}

int remove_all_dead_children()
{
    int count = 0;
    child_t *prev = &sentinel;
    child_t *curr = sentinel.next;
    while (curr != NULL)
    {
        if (curr->pid == 0)
        {
            prev->next = curr->next;
            for (int i = 0; curr->args[i] != NULL; i++)
            {
                free(curr->args[i]);
            }
            free(curr);
            curr = prev->next;
            count++;
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
    return count;
}

child_t* get_child(pid_t pid)
{
    child_t *curr = sentinel.next;
    while (curr != NULL)
    {
        if (curr->pid == pid)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

int get_child_status(pid_t pid)
{
    child_t *child = get_child(pid);
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    return child->status;
}

int set_child_status_by_pid(pid_t pid, int status)
{
    child_t *child = get_child(pid);
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    child->status = status;
    return 0;
}

int set_child_status(child_t *child, int status)
{
    if (child == NULL)
    {
        debug("child is null");
        return -1;
    }
    child->status = status;
    return 0;
}

child_t* get_child_by_deet_id(pid_t deetId)
{
    child_t *curr = sentinel.next;
    while (curr != NULL)
    {
        if (curr->deetId == deetId)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

child_t* get_last_child()
{
    child_t *curr = sentinel.next;
    child_t *last = NULL;
    while (curr != NULL)
    {
        last = curr;
        curr = curr->next;
    }
    return last;
}

// Path: src/track_children.h