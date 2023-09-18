#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "debug.h"

int main(int argc, char **argv)
{
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    int read_data = read_distance_data(stdin);

    // no global_options set, output edge data
    if (global_options == 0)
    {
        build_taxonomy(stdout);
        return EXIT_SUCCESS;
    }
    // -m flag set, output matrix data
    if (global_options == MATRIX_OPTION)
    {
        int matrix = 0;
        build_taxonomy(NULL);
        emit_distance_matrix(stdout);
        return EXIT_SUCCESS;
    }
    // -n flag set, output newick data
    if (global_options == NEWICK_OPTION)
    {
        if (build_taxonomy(NULL)) emit_newick_format(stdout);
        return EXIT_SUCCESS;
    }
    // TO BE IMPLEMENTED
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
