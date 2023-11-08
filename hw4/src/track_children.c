#include "track_children.h"

child_t sentinel = {
    .pid = 0,
    .status = PSTATE_NONE,
    .deetId = -2,
    .trace = 0,
    .exit_status = -1,
    .command = NULL,
    .args = NULL,
    .next = &sentinel,
    .prev = &sentinel};

child_t* head = &sentinel;
child_t* tail = &sentinel;

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
    new_child->deetId = -1;
    new_child->deetId = next_deet_id++;
    new_child->next = &sentinel;
    new_child->prev = &sentinel;
    new_child->exit_status = -1;
    new_child->trace = trace;
    new_child->args = calloc(num_args + 1, sizeof(char*));
    if (new_child->args == NULL)
    {
        debug("malloc of args failed");
        exit(1);
    }
    for (int i = 0; i < num_args; i++)
    {
        new_child->args[i] = calloc(strlen(args[i]) + 1, sizeof(char));
        if (new_child->args[i] == NULL)
        {
            debug("malloc of args[%d] failed", i);
            exit(1);
        }
        strncpy(new_child->args[i], args[i], strlen(args[i]) + 1);
    }
    child_t* prev_sent = sentinel.prev;
    prev_sent->next = new_child;
    sentinel.prev = new_child;
    new_child->prev = prev_sent;
    new_child->next = &sentinel;
    if (head == &sentinel)
    {
        head = new_child;
    }
    tail = new_child;
    new_child->status = PSTATE_RUNNING;
    log_state_change(new_child->pid, PSTATE_NONE, PSTATE_RUNNING, 0);
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
    debug("Got here with %p!", child);
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    pid_t killed_pid = child->pid;
    set_child_status(child, PSTATE_KILLED);
    warn("Killing child %d", killed_pid);
    kill(killed_pid, SIGKILL);
    // ptrace(PTRACE_KILL, killed_pid, NULL, NULL);
    // wait for the child to die from the sigchld handler
    return killed_pid;
}

int remove_all_dead_children()
{
    int count = 0;
    child_t *prev = &sentinel;
    child_t *curr = sentinel.next;
    while (curr != &sentinel)
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
    child_t *curr = sentinel.next;
    while (curr != &sentinel)
    {
        if (curr->pid == pid)
        {
            // unlock the mutex
            return curr;
        }
        curr = curr->next;
    }
    // unlock the mutex
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
    debug("locked in set_child_status\n");
    // log_error("attempting to enter sem_wait");
    if (child == NULL)
    {
        debug("child is null");
        // unlock the mutex
        debug("unlocked in set_child_status if\n");
        return -1;
    }
    info("child is %p", child);
    info("child->pid: %d", child->pid);
    info("child->status: %d", child->status);
    info("status: %d", status);
    log_state_change(child->pid, child->status, status, 0);
    child->status = status;
    // unlock the mutex
    debug("unlocked in set_child_status\n");
    return 0;
}

child_t* get_child_by_deet_id(pid_t deetId)
{
    // lock the mutex
    child_t *curr = sentinel.next;
    while (curr && curr != &sentinel)
    {
        if (curr->deetId == deetId && curr->exit_status == -1)
        {
            // unlock the mutex
            return curr;
        }
        curr = curr->next;
    }
    // unlock the mutex
    return NULL;
}

child_t* get_last_child()
{
    return tail;
}

void child_summary(child_t* child, int filenum)
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
    print_int(filenum, child->deetId);
    print_string(filenum, " \t");
    print_int(filenum, child->pid);
    print_string(filenum, " \t");
    print_string(filenum, child->trace ? "T" : "U");
    print_string(filenum, " \t");
    print_string(filenum, status);
    print_string(filenum, " \t");
    if (child->exit_status == -1)
    {
        print_string(filenum, " \t");
    }
    else
    {
        print_int(filenum, child->exit_status);
        print_string(filenum, " \t");
    }
    for (int i = 0; child->args[i] != NULL; i++)
    {
        print_string(filenum, child->args[i]);
        print_string(filenum, " ");
    }
    print_string(filenum, "\n");
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
    if (child == NULL)
    {
        debug("child is null");
        // unlock the mutex
        return;
    }
    child->deetId = -1;
    for (int i = 0; child->args[i] != NULL; i++)
    {
        free(child->args[i]);
    }
    free(child->args);
    child_t* prev = child->prev;
    child_t* next = child->next;
    prev->next = next;
    free(child);
    next_deet_id--;
    // unlock the mutex
}
// Path: src/track_children.h