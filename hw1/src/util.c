#include "util.h"

char *memorycpy(char *dest, char *src, int size)
{
    if (size < 1) return dest;
    int i = 0;
    do
    {
        *(dest + i) = *(src + i);
        i++;
    } while (i < size);
    return dest + i;
}

char *memoryset(char *src, int c, int size)
{
    if (size < 1) return src;
    int i = 0;
    do
    {
        *(src + i++) = c;
    } while (i < size);
    return src + i;
}

int validateNum(char *src, int size, double *num)
{
    double decimal = 0;
    int i = 0;
    double n = 0;
    if (size < 1) return 0; // fail if size is too small
    do
    {
        char c = *(src + i);
        // c is not a number
        if (c > '9' || c < '0')
        {
            // c is a decimal, check if decimal already used
            if (c == '.')
            {
                // decimal was already used, fail
                if (decimal) return 0;
                // set the flag for decimals to true so it's done correctly
                decimal = 1.0;
                i++;
                continue;
            }
            // invalid character, fail
            return 0;
        }
        // get the numerical representation of c
        c -= '0';
        if (decimal)
        {
            decimal *= 10;
            n += (c / decimal);
        }
        else
        {
            n = (n * 10) + c;
        }
        i++;
    } while (i < size);
    *num = n;
    return 1;
}

int checkIdentityMatrix(double *matrix, int n)
{
    int i = 0;
    // check the left to right diagonal of the matrix
    // all values in the diagonal should be equal to 0
    while ( i < n && !( *(matrix + ( i * n + i)) ) && i++);
    // diagonals are not all 0, fail
    if (i < n) return 0;

    i = 0;

    // check if each of the values have a corresponding value in the matrix
    // if they do not, it breaks the rules and we can discard it
    while ( i < n)
    {
        int j = 0;
        while ( j < i )
        {
            if ( (*(matrix + (j * n + i)) != *(matrix + (i * n + j))) ) return 0;
            j++;
        }
        i++;
    }
    
    return 1;
}

int stringifyNumber(char *buffer, int num, int pos, int addNull)
{
    if (num < 10)
    {
        *(buffer + pos) = num + '0';
        if (addNull) *(buffer + pos + 1) = '\0';
        return (addNull ? pos + 1 : pos);
    }
    int i = num;
    int j = 1;
    while (i > 10)
    {
        i /= 10;
        j *= 10;
    }
    *(buffer + pos) = i + '0';
    return stringifyNumber(buffer, num % j, pos + 1, addNull);
}
