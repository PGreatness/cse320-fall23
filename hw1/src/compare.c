#include <stdlib.h>

#include "compare.h"

int compareStrings(char * str1, char * str2)
{
    while(
        (*str1 != '\0' || *str2 != '\0') // check if a null char has been reached
        &&
        *(str1++) == *(str2++) // check if the two strs have the same beginning char
                               // and then set start to next char in str
        );
    // check the char before the currrent head char (because
    // the while loop will cause the strs to start at the next char in the str).
    // that char is the offending char that stopped the while loop
    if (*(--str1) != *(--str2)) return 0;
    // reached end of both strings without issue
    return 1;
}