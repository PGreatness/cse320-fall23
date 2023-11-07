#include "track_children.h"

child_t sentinel = {
    .pid = 0,
    .status = PSTATE_NONE,
    .deetId = -1,
    .trace = 0,
    .exit_status = -1,
    .command = NULL,
    .args = NULL,
    .next = NULL};

static pid_t next_deet_id = 0;

pthread_mutex_t child_lock = PTHREAD_MUTEX_INITIALIZER;

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
    new_child->exit_status = -1;
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

int remove_dead_child(child_t* child)
{
    child_t *prev = &sentinel;
    child_t *next = child->next;
    while (prev->next && prev->next != child)
    {
        prev = prev->next;
    }
    if (prev->next == NULL)
    {
        debug("child not found");
        return -1;
    }
    prev->next = next;
    for (int i = 0; child->args[i] != NULL; i++)
    {
        free(child->args[i]);
    }
    free(child);
    return 0;
}

pid_t kill_child(pid_t pid)
{
    child_t *child = get_child(pid);
    debug("Got here!");
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    pid_t killed_pid = child->pid;
    set_child_status(child, PSTATE_KILLED);
    ptrace(PTRACE_KILL, killed_pid, NULL, NULL);
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
        if (curr->status == PSTATE_DEAD)
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
    // lock the mutex
    debug("In getchild\n");
    if (pthread_mutex_lock(&child_lock) != 0)
    {
        debug("pthread_mutex_lock failed");
        exit(1);
    }
    debug("Got lock\n");
    child_t *curr = sentinel.next;
    while (curr != NULL)
    {
        if (curr->pid == pid && curr->exit_status == -1)
        {
            // unlock the mutex
            if (pthread_mutex_unlock(&child_lock) != 0)
            {
                debug("pthread_mutex_unlock failed");
                exit(1);
            }
            return curr;
        }
        curr = curr->next;
    }
    // unlock the mutex
    if (pthread_mutex_unlock(&child_lock) != 0)
    {
        debug("pthread_mutex_unlock failed");
        exit(1);
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
    // lock the mutex
    // fprintf(stderr,"In set_child_status\n");
    if (pthread_mutex_lock(&child_lock) != 0)
    {
        debug("pthread_mutex_lock failed");
        exit(1);
    }
    debug("locked in set_child_status\n");
    if (child == NULL)
    {
        debug("child is null");
        // unlock the mutex
        if (pthread_mutex_unlock(&child_lock) != 0)
        {
            debug("pthread_mutex_unlock failed");
            exit(1);
        }
        debug("unlocked in set_child_status if\n");
        return -1;
    }
    log_state_change(child->pid, child->status, status, 0);
    child->status = status;
    // unlock the mutex
    debug("unlocked in set_child_status\n");
    if (pthread_mutex_unlock(&child_lock) != 0)
    {
        debug("pthread_mutex_unlock failed");
        exit(1);
    }
    return 0;
}

child_t* get_child_by_deet_id(pid_t deetId)
{
    // lock the mutex
    if (pthread_mutex_lock(&child_lock) != 0)
    {
        debug("pthread_mutex_lock failed");
        exit(1);
    }
    child_t *curr = sentinel.next;
    while (curr != NULL)
    {
        if (curr->deetId == deetId && curr->exit_status == -1)
        {
            // unlock the mutex
            if (pthread_mutex_unlock(&child_lock) != 0)
            {
                debug("pthread_mutex_unlock failed");
                exit(1);
            }
            return curr;
        }
        curr = curr->next;
    }
    // unlock the mutex
    if (pthread_mutex_unlock(&child_lock) != 0)
    {
        debug("pthread_mutex_unlock failed");
        exit(1);
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

void child_summary(child_t* child, FILE* stream)
{
    char *status;
    switch (child->status)
    {
        case PSTATE_NONE:
            status = "none";
            break;
        case PSTATE_RUNNING:
            status = "running";
            break;
        case PSTATE_STOPPING:
            status = "stopping";
            break;
        case PSTATE_STOPPED:
            status = "stopped";
            break;
        case PSTATE_CONTINUING:
            status = "continuing";
            break;
        case PSTATE_KILLED:
            status = "killed";
            break;
        case PSTATE_DEAD:
            status = "dead";
            break;
        default:
            status = "unknown";
            break;
    }
    fprintf(stream, "%d\t%d\t%c\t%s\t", child->deetId, child->pid, child->trace ? 'T' : 'U', status);
    child->exit_status == -1 ? fprintf(stream, "\t") : fprintf(stream, "0x%d\t", child->exit_status);
    for (int i = 0; child->args[i] != NULL; i++)
    {
        fprintf(stream, "%s ", child->args[i]);
    }
    fprintf(stream, "\n");
}

void set_exit_status(child_t *child, int status)
{
    if (child == NULL)
    {
        debug("child is null");
        return;
    }
    child->exit_status = status;
}

void free_child(child_t* child)
{
    // lock the mutex
    if (pthread_mutex_lock(&child_lock) != 0)
    {
        debug("pthread_mutex_lock failed");
        exit(1);
    }
    if (child == NULL)
    {
        debug("child is null");
        // unlock the mutex
        if (pthread_mutex_unlock(&child_lock) != 0)
        {
            debug("pthread_mutex_unlock failed");
            exit(1);
        }
        return;
    }
    for (int i = 0; child->args[i] != NULL; i++)
    {
        free(child->args[i]);
    }
    free(child);
    // unlock the mutex
    if (pthread_mutex_unlock(&child_lock) != 0)
    {
        debug("pthread_mutex_unlock failed");
        exit(1);
    }
}
// Path: src/track_children.h