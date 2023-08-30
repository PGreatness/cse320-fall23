#include <stdlib.h>

#include "global.h"
#include "debug.h"

#include "compare.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv)
{
    if (argc < 1) return -1; // should never happen
    if (argc == 1) return 0; // only program called, return success for validation
    
    for (int i = 2; i <= argc; i++)
    {
        // help option is called
        if (compareStrings(*(argv + i - 1), "-h"))
        {
            if (i != 2) return -1; // help option must be the first option
            global_options = global_options | HELP_OPTION;
            return 0; // validation success, help requested
        }

        // matrix mode requested
        if (compareStrings(*(argv + i - 1), "-m"))
        {
            // already set newick mode
            if (global_options != 0) return -1;
            global_options = global_options | MATRIX_OPTION;
            continue;
        }

        // newick mode requested
        if (compareStrings(*(argv + i - 1), "-n"))
        {
            // already set matrix mode
            if (global_options != 0) return -1;
            global_options = global_options | NEWICK_OPTION;
            continue;
        }

        if (compareStrings(*(argv + i - 1), "-o"))
        {
            // not in newick mode
            // validation fails
            if ((global_options & NEWICK_OPTION) != NEWICK_OPTION) return -1;
            // no file name given
            if (i == argc) return -1;
            outlier_name = *(argv + i++);
            continue;
        }

        // improper argument given, fail validation
        return -1;

    }
    return 0; // all validation checks passed, return success
}
