#include "track_children.h"

child_t sentinel = {
    .pid = 0,
    .status = PSTATE_NONE,
    .deetId = -2,
    .trace = 0,
    .exit_status = -1,
    .command = NULL,
    .args = NULL,
    .argc = 0,
    .next = &sentinel,
    .prev = &sentinel};

child_t* head = &sentinel;
child_t* tail = &sentinel;

static pid_t next_deet_id = 0;

int find_next_deet_id()
{
    child_t* child = sentinel.next;
    int i = 0;
    int id = -1;
    while (child != &sentinel)
    {
        if (child->status == PSTATE_DEAD)
        {
            int deetId = child->deetId;
            set_child_status(child, PSTATE_NONE, -1);
            free_child(child);
            if (id == -1) id = deetId;
        }
        child = child->next;
        i++;
    }
    if (id != -1) return id;
    int deetId = i;
    next_deet_id = i + 1;
    return deetId;
}

void connect_child(child_t* child)
{
    int child_id = child->deetId;
    child_t* curr = sentinel.next;
    while (curr != &sentinel)
    {
        if (curr->deetId < child_id)
        {
            if (curr->next == &sentinel)
            {
                curr->next = child;
                child->prev = curr;
                child->next = &sentinel;
                sentinel.prev = child;
                return;
            }
            if (curr->next->deetId > child_id)
            {
                child->next = curr->next;
                child->prev = curr;
                curr->next->prev = child;
                curr->next = child;
                return;
            }
        }
        curr = curr->next;
    }
    child_t* n = sentinel.next;
    n->prev = child;
    child->next = n;
    child->prev = &sentinel;
    sentinel.next = child;
}

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
    new_child->deetId = find_next_deet_id();
    connect_child(new_child);
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
    new_child->argc = num_args;
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
    set_child_status(child, PSTATE_KILLED, -1);
    child_summary(child, STDERR_FILENO);
    warn("Killing child %d", killed_pid);
    kill(killed_pid, SIGKILL);
    // ptrace(PTRACE_KILL, killed_pid, NULL, NULL);
    // wait for the child to die from the sigchld handler
    return killed_pid;
}

pid_t stop_child(pid_t pid)
{
    child_t* child = get_child(pid);
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    pid_t stopped_pid = child->pid;
    set_child_status(child, PSTATE_STOPPING, -1);
    child_summary(child, STDERR_FILENO);
    kill(stopped_pid, SIGSTOP);
    return stopped_pid;
}

pid_t release_child(int pid)
{
    child_t* child = get_child(pid);
    if (child == NULL)
    {
        debug("child not found");
        return -1;
    }
    pid_t released_pid = child->pid;
    child->trace = 0;
    set_child_status(child, PSTATE_RUNNING, -1);
    child_summary(child, STDERR_FILENO);
    ptrace(PTRACE_DETACH, released_pid, NULL, NULL);
    return released_pid;

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

int set_child_status(child_t *child, int status, int exit_status)
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
    int old_status = child->status;
    child->status = status;
    child->exit_status = exit_status;
    log_state_change(child->pid, old_status, status, exit_status);
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

child_t* get_child_by_deet_id_ig(int deetId)
{
    // lock the mutex
    child_t *curr = sentinel.next;
    while (curr && curr != &sentinel)
    {
        if (curr->deetId == deetId)
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

void child_summaries_in_state(int state, int filenum)
{
    child_t* child = sentinel.next;
    while (child != &sentinel)
    {
        if (child->status == state)
        {
            child_summary(child, filenum);
        }
        child = child->next;
    }
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
    print_string(filenum, "\t");
    print_int(filenum, child->pid);
    print_string(filenum, "\t");
    print_string(filenum, child->trace ? "T" : "U");
    print_string(filenum, "\t");
    print_string(filenum, status);
    print_string(filenum, "\t");
    if (child->exit_status == -1)
    {
        print_string(filenum, "\t");
    }
    else
    {
        print_string(filenum, "0x");
        print_int_as_hex(filenum, child->exit_status);
        // print_int(filenum, child->exit_status);
        print_string(filenum, "\t");
    }
    for (int i = 0; child->args[i] != NULL; i++)
    {
        print_string(filenum, child->args[i]);
        if (child->args[i + 1] != NULL)
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
    for (int i = 0; i < child->argc; i++)
    {
        free(child->args[i]);
    }
    free(child->args);
    child_t* prev = child->prev;
    child_t* next = child->next;
    prev->next = next;
    next->prev = prev;
    free(child);
    // next_deet_id--;
    // unlock the mutex
}

int decrement_next_deet_id()
{
    next_deet_id--;
    return next_deet_id;
}

int increment_next_deet_id()
{
    next_deet_id++;
    return next_deet_id;
}

int get_next_deet_id()
{
    return next_deet_id;
}
// Path: src/track_children.h