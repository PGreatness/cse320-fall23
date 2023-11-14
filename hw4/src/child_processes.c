#include "child_processes.h"

int run_child_process(char* command, char* args[], int num_args)
{
    // create a child processs
    pid_t pid = fork();

    // check if fork() failed
    if (pid < 0)
    {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }

    // check if we are in the child process
    if (pid == 0)
    {
        // close stdout and redirect to stderr
        dup2(STDERR_FILENO, STDOUT_FILENO);
        // start tracing the child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        // execute the command with execvp()
        execvp(command, args);

        // if we get here, execlp() failed
        // perror("execvp() failed");
        exit(EXIT_FAILURE);
    }
    // add the child to the list of children
    child_t *child = spawn_child(pid, 1, args, num_args);
    child_summary(child, STDOUT_FILENO);
    // we are in the parent process
    return pid;
}

void show_child_process(pid_t deet_id, int filenum)
{
    // get the child process
    child_t *child = get_child_by_deet_id(deet_id);


    // check if the child exists
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deet_id);
        return;
    }

    child_summary(child, filenum);
}

void show_all_child_processes(int filenum)
{
    // get the first child
    child_t *child = sentinel.next;

    // iterate over all children
    while (child != &sentinel)
    {
        child_summary(child, filenum);
        child = child->next;
    }

}

int continue_child_process(int deet_id, int filenum)
{
    // get the child process
    child_t *child = get_child_by_deet_id(deet_id);

    // check if the child exists
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deet_id);
        return -1;
    }

    // check if the child is stopped
    if (child->status != PSTATE_STOPPED)
    {
        debug("Child with deet_id %d is not stopped\n", deet_id);
        return -1;
    }

    // continue the child process
    // set_child_status(child, PSTATE_CONTINUING, 0);
    int state = PSTATE_RUNNING;
    if (child->trace)
    {
        ptrace(PTRACE_CONT, child->pid, NULL, NULL);
    }
    else{
        state = PSTATE_CONTINUING;
        kill(child->pid, SIGCONT);
    }
    set_child_status(child, state, -1);
    child_summary(child, filenum);
    return 0;
}

int kill_child_process(int deet_id)
{
    // get the child process
    child_t *child = get_child_by_deet_id(deet_id);

    // check if the child exists
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deet_id);
        return -1;
    }

    // kill the child process
    kill_child(child->pid);
    return 0;
}

int stop_child_process(int deetId)
{
    child_t* child = get_child_by_deet_id_ig(deetId);
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_STOPPED)
    {
        debug("Child with deet_id %d is already stopped\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_STOPPING)
    {
        debug("Child with deet_id %d is already stopping\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_DEAD)
    {
        debug("Child with deet_id %d is not running\n", deetId);
        return -1;
    }
    stop_child(child->pid);
    return 0;
}

void kill_all_child_processes()
{
    // set the shutdown flag
    shutdown = 1;
    kill(getpid(), SIGCHLD);
}

int release_child_process(int deetId)
{
    child_t* child = get_child_by_deet_id_ig(deetId);
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_DEAD)
    {
        debug("Child with deet_id %d is already dead\n", deetId);
        return -1;
    }
    if (!(child->trace))
    {
        debug("Child with deet_id %d is not being traced\n", deetId);
        return -1;
    }
    return release_child(child->pid);
}

void wait_for_child_process(int deetId, int state)
{
    suspend_until_state(deetId, state);
}

int peek_in_process(int deetId, unsigned long addr, int amnt, int filenum)
{
    child_t* child = get_child_by_deet_id_ig(deetId);
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_DEAD)
    {
        debug("Child with deet_id %d is dead\n", deetId);
        return -1;
    }
    if (!(child->trace))
    {
        debug("Child with deet_id %d is not being traced\n", deetId);
        return -1;
    }
    for (int i = 0; i < amnt; i++)
    {
        print_long_as_hex(filenum, addr + (i * 8));
        print_string(filenum, "\t");
        unsigned long data = ptrace(PTRACE_PEEKDATA, child->pid, addr + (i * 8), NULL);
        print_long_as_hex(filenum, data);
        print_string(filenum, "\n");
    }
    return 0;
}

int poke_in_process(int deetId, unsigned long addr, unsigned long value)
{
    child_t* child = get_child_by_deet_id_ig(deetId);
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_DEAD)
    {
        debug("Child with deet_id %d is dead\n", deetId);
        return -1;
    }
    if (!(child->trace))
    {
        debug("Child with deet_id %d is not being traced\n", deetId);
        return -1;
    }
    ptrace(PTRACE_POKEDATA, child->pid, addr, value);
    return 0;
}

int backtrace_child_process(int deetId, int limit, int filenum)
{
    child_t* child = get_child_by_deet_id_ig(deetId);
    if (child == NULL)
    {
        debug("Child with deet_id %d not found\n", deetId);
        return -1;
    }
    if (child->status == PSTATE_DEAD)
    {
        debug("Child with deet_id %d is dead\n", deetId);
        return -1;
    }
    if (!(child->trace))
    {
        debug("Child with deet_id %d is not being traced\n", deetId);
        return -1;
    }
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child->pid, NULL, &regs) < 0)
    {
        debug("Error getting registers for child with deet_id %d\n", deetId);
        return -1;
    }
    // get the current stack frame and the return address stored in that frame
    unsigned long frame = regs.rbp;
    unsigned long ret_addr = regs.rip;
    // keep going until we reach the end of the stack or reach limit
    int i = 0;
    while (frame > 1 && i < limit)
    {
        // print the current frame
        print_long_as_hex(filenum, frame);
        print_string(filenum, "\t");
        print_long_as_hex(filenum, ret_addr);
        print_string(filenum, "\n");
        // get the next frame and return address
        ret_addr = ptrace(PTRACE_PEEKDATA, child->pid, frame + 8, NULL);
        frame = ptrace(PTRACE_PEEKDATA, child->pid, frame, NULL);
        i++;
    }
    return 0;
}