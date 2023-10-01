#include "err_string.h"

int is_valid_template_string(char** tmpt)
{
        char *tmpt2 = *tmpt;
        if (*tmpt2 != '%') return 0;
        char* tmp = tmpt2;
        tmp++;
        int flag = 0;
        int unacc_char = 0;
        int decimal = 0;
        while (*tmp)
        {
                if (*tmp == '\0' || *tmp == '\n' || *tmp == ' ')
                {
                    *tmpt = tmp;
                    return 0;
                }
                switch(*tmp)
                {
                        // these are valid end characters
                        case 'i':
                        case 'd':
                        case 's':
                        case 'c':
                        case 'f':
                        case 'x':
                        case 'p':
                            *tmpt = tmp;
                            return 1;
                        // these are valid flags
                        case '0':
                            // has to be first character after %
                            // if (tmp != tmpt + 1) return 0;
                            if (flag) return -1;
                            flag = 1;
                            break;
                        case '#':
                            // has to be first character after %
                            if (tmp != tmpt2 + 1) return -1;
                            if (flag) return -1;
                            flag = 2;
                            break;
                        case '-':
                            // has to be first character after %
                            if (tmp != tmpt2 + 1) return -1;
                            if (flag) return -1;
                            flag = 3;
                            break;
                        case '+':
                            // has to be first character after %
                            if (tmp != tmpt2 + 1) return -1;
                            if (flag) return -1;
                            flag = 4;
                            break;
                        default:
                            unacc_char = 1;
                            // if it's a digit, it's a width
                            if (*tmp >= '0' && *tmp <= '9') unacc_char = 0;
                            // if it's a period, it's a precision
                            if (*tmp == '.' && !decimal) { unacc_char = 0; decimal = 1; }
                            // if it's a * and there's no flags, it's a width
                            if (*tmp == '*' && !flag) unacc_char = 0;
                            break;
                }
                if (unacc_char)
                {
                    *tmpt = tmp;
                    return -2;
                }
                tmp++;
        }
        return 0;
}

void print_args(char* current, char* end, va_list* ap, FILE* out)
{
    char a[100] = {0};
    int i = 0;
    while (end >= current && (a[i++] = *current++));
    char type = a[i - 1];
    switch (type)
    {
        case 'i':
            fprintf(out, a, va_arg(*ap, int));
            break;
        case 'd':
            fprintf(out, a, va_arg(*ap, int));
            break;
        case 's':
            fprintf(out, a, va_arg(*ap, char*));
            break;
        case 'c':
            fprintf(out, a, va_arg(*ap, int));
            break;
        case 'f':
            fprintf(out, a, (float) va_arg(*ap, double));
            break;
        default:
            fprintf(out, a, "");
            break;
    }
}