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
    int read_data;
    if (( read_data = read_distance_data(stdin) ))
    {
        fprintf(stderr, "Error: read_distance_data() failed\n");
        return EXIT_FAILURE;
    }

    // no global_options set, output edge data
    if (global_options == 0)
    {
        if (build_taxonomy(stdout))
        {
            fprintf(stderr, "Error: build_taxonomy() failed\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    // -m flag set, output matrix data
    if (global_options == MATRIX_OPTION)
    {
        if (build_taxonomy(NULL))
        {
            fprintf(stderr, "Error: build_taxonomy() failed while attempting to create a matrix\n");
            return EXIT_FAILURE;
        }
        if (emit_distance_matrix(stdout))
        {
            fprintf(stderr, "Error: emit_distance_matrix() failed\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    // -n flag set, output newick data
    if (global_options == NEWICK_OPTION)
    {
        if (build_taxonomy(NULL))
        {
            fprintf(stderr, "Error: build_taxonomy() failed while attempting to create a newick tree\n");
            return EXIT_FAILURE;
        }
        if (emit_newick_format(stdout))
        {
            fprintf(stderr, "Error: emit_newick_format() failed\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
