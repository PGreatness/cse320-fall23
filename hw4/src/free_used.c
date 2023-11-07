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
    child_t* child = sentinel.next;
    while (child != NULL)
    {
        child_t* next = child->next;
        kill_child_process(child->deetId);
        child = next;
    }
}