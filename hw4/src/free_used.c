#include "free_used.h"

void free_args(char* args[], int size)
{
    for (int i = 0; i < size; i++)
    {
        free(args[i]);
    }
}

void free_children()
{
    child_t *curr = sentinel.next;
    while (curr != NULL)
    {
        child_t *next = curr->next;
        kill_child(curr->pid);
        free_child(curr);
        curr = next;
    }
}