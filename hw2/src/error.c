/*
 * Error handling routines
 */

#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "err_string.h"

int errors;
int warnings;
int dbflag = 1;

void fatal(char* fmt, ...)
{
	va_list ap;
	int count = 0;
	char* p = fmt;
	while (*p && ((count = count + (*p == '%')) > -1)) p++;
	FILE* tmp = tmpfile();
	fprintf(tmp, "\nFatal error: ");
	va_start(ap, fmt);
	while (*fmt)
	{
		if (*fmt == '%' && count)
		{
			char* p = fmt;
			int valid;
			if ( (valid = is_valid_template_string(&p)) != 1 )
			{
				if (valid == 0)
				{
					fprintf(stderr, "Error at %p on char %c\n", p, *p);
				}
				if (valid == -1)
				{
					fprintf(stderr, "Error with flags\n");
				}
				if (valid == -2)
				{
					fprintf(stderr, "Unknown character at %p: %c\n", p, *p);
				}
				return -1;
			}
			count--;
			char *t = fmt;
			print_args(t, p, &ap, tmp);
			fmt = p;
		} else {
			fprintf(tmp, "%c", *fmt);
		}
		fmt++;
	}
	va_end(ap);
	if (count > 0)
	{
		fprintf(stderr, "Too few arguments to fatal\n");
	} else {
		char c;
		rewind(tmp);
		while ((c = fgetc(tmp)) != EOF)
		{
			fputc(c, stderr);
		}
	}
	fclose(tmp);
	exit(1);
}

void error(fmt, a1, a2, a3, a4, a5, a6)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
{
	fprintf(stderr, "\nError: ");
	fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
	errors++;
}

void warning(fmt, a1, a2, a3, a4, a5, a6)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
{
	fprintf(stderr, "\nWarning: ");
	fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
	warnings++;
}

void debug(fmt, a1, a2, a3, a4, a5, a6)
char *fmt, *a1, *a2, *a3, *a4, *a5, *a6;
{
	if(!dbflag) return;
	fprintf(stderr, "\nDebug: ");
	fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	fprintf(stderr, "\n");
}
