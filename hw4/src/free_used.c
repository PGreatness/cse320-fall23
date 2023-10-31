#include "free_used.h"

void free_args(char* args[], int size)
{
    for (int i = 0; i < size; i++)
    {
        free(args[i]);
    }
}