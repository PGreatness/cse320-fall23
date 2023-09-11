#include <stdlib.h>

#include "global.h"
#include "debug.h"

#include "util.h"
#include "compare.h"

/**
 * @brief  Read genetic distance data and initialize data structures.
 * @details  This function reads genetic distance data from a specified
 * input stream, parses and validates it, and initializes internal data
 * structures.
 *
 * The input format is a simplified version of Comma Separated Values
 * (CSV).  Each line consists of text characters, terminated by a newline.
 * Lines that start with '#' are considered comments and are ignored.
 * Each non-comment line consists of a nonempty sequence of data fields;
 * each field iis terminated either by ',' or else newline for the last
 * field on a line.  The constant INPUT_MAX specifies the maximum number
 * of data characters that may be in an input field; fields with more than
 * that many characters are regarded as invalid input and cause an error
 * return.  The first field of the first data line is empty;
 * the subsequent fields on that line specify names of "taxa", which comprise
 * the leaf nodes of a phylogenetic tree.  The total number N of taxa is
 * equal to the number of fields on the first data line, minus one (for the
 * blank first field).  Following the first data line are N additional lines.
 * Each of these lines has N+1 fields.  The first field is a taxon name,
 * which must match the name in the corresponding column of the first line.
 * The subsequent fields are numeric fields that specify N "distances"
 * between this taxon and the others.  Any additional lines of input following
 * the last data line are ignored.  The distance data must form a symmetric
 * matrix (i.e. D[i][j] == D[j][i]) with zeroes on the main diagonal
 * (i.e. D[i][i] == 0).
 *
 * If 0 is returned, indicating data successfully read, then upon return
 * the following global variables and data structures have been set:
 *   num_taxa - set to the number N of taxa, determined from the first data line
 *   num_all_nodes - initialized to be equal to num_taxa
 *   num_active_nodes - initialized to be equal to num_taxa
 *   node_names - the first N entries contain the N taxa names, as C strings
 *   distances - initialized to an NxN matrix of distance values, where each
 *     row of the matrix contains the distance data from one of the data lines
 *   nodes - the "name" fields of the first N entries have been initialized
 *     with pointers to the corresponding taxa names stored in the node_names
 *     array.
 *   active_node_map - initialized to the identity mapping on [0..N);
 *     that is, active_node_map[i] == i for 0 <= i < N.
 *
 * @param in  The input stream from which to read the data.
 * @return 0 in case the data was successfully read, otherwise -1
 * if there was any error.  Premature termination of the input data,
 * failure of each line to have the same number of fields, and distance
 * fields that are not in numeric format should cause a one-line error
 * message to be printed to stderr and -1 to be returned.
 */

int read_distance_data(FILE *in) {
    // TO BE IMPLEMENTED
    if (in == NULL) return -1; // error with file given

    char c; // the character to be read
    int field_size = 0; // the initial size of the field
    int taxa_in_line = 0; // the current number of taxa in the line
    int num_rows = 0; // the number of rows
    int rows_remaining = 1; // the number of rows remaining. this is calculated from the num rows
    int first_line = 1; // 1 if we are still looking at the first row of descriptors, 0 if we passed it
    int name_num = 0; // the taxa position and the num to be used in the node_names matrix

    // the row and column of the distance matrix
    int row = 0;
    int col = 0;

    int max_col = 0;

    // get the next char in the file
    while( (c = fgetc(in)) )
    {
        // if we reach the end of the input
        if ( feof(in) )
        {
            // check if there's still rows remaining
            if (rows_remaining > 0)
            {
                debug("rows remaining: %i\n", rows_remaining);
                // perhaps the last row ended before a \n was added, add it in and continue processing
                ungetc('\n', in);
                continue;
            }
            break;
        }
        switch (c)
        {
        // comment line
        case '#':
            // consume all chars until a newline and then break to continue
            while( (c = fgetc(in)) && c != '\n' );
            break;
        // end of line
        case '\n':
            // line ended, add the last taxa to counter
            taxa_in_line++;
            // if this is the first line to be initialized
            if (num_rows == 0)
            {
                num_rows = taxa_in_line;
                rows_remaining = num_rows;
                // the number of taxa is the number of rows - 1
                num_taxa = num_rows - 1;
                num_all_nodes = num_taxa;
                num_active_nodes = num_taxa;
            }
            // check if the number of taxa is not equal to the number of rows
            // if not the same, either too little given or too many, fail
            if (num_rows != taxa_in_line) return -1;
            if (first_line)
            {
                memorycpy(*(node_names + name_num), input_buffer, field_size + 1);
                name_num++;
            }else{
                double n;
                if (validateNum(input_buffer, field_size, &n))
                {
                    debug("number at end got: %lf\n", n);
                    *(*(distances + col) + row) = n;
                    row++;
                    max_col++;
                    col = 0;
                } else {
                    // last field is not a number and not in the first line, fail
                    // TODO: fail
                    debug("NUMBER FAILED");
                }
            }
            // else reset taxa count and field size for next line
            taxa_in_line = 0;
            field_size = 0;
            first_line = 0;
            memoryset(input_buffer, 0, sizeof (input_buffer));
            // check if we went too many rows down
            debug("current row: %i\tremaining rows: %i\n", num_rows - rows_remaining, rows_remaining);
            if (rows_remaining-- <= 0) return -1;
            break;
        // comma (,)
        case ',':
            // check if we have all the data we need
            if (rows_remaining <= 0)
            {
                // consume the rest of the file
                while( (c = fgetc(in)) && !feof(in) );
                break;
            }
            // increment the number of taxa in current row
            taxa_in_line++;
            // empty field, discount
            if (field_size <= 0) continue;
            // add a null delimiter to the end of buffer
            *(input_buffer + field_size + 1) = '\0';
            // debug("GOT HERE with: %s\n", input_buffer);
            // copy over the input buffer to the node_names array from beginning to
            // the size of the field + 1 to account for the null terminator
            if (first_line)
            {
                memorycpy(*(node_names + name_num), input_buffer, field_size + 1);
                NODE named_node;
                named_node.name = *(node_names + name_num);
                *(nodes + name_num) = named_node;
                name_num++;
            }
            // place the number in the distances matrix
            double n;
            if (validateNum(input_buffer, field_size, &n))
            {
                debug("Number got was: %lf\n", n);
                *(*(distances + col) + row) = n;
                col++;
            } else {
                // this is the identifier for the row
                // we need to make sure the order of the taxa is the same as the first line
                if (!first_line && taxa_in_line == 1)
                {
                    // use the row variable to get what the current row is supposed to be
                    // and then compare the name of the node to the name of the node in the
                    // first line
                    // create pointers to avoid losing the original pointers
                    char *real_name = *(node_names + row);
                    char *input_name = input_buffer;
                    if (!compareStrings(real_name, input_name))
                    {
                        debug("NAME FAILED: expected \"%s\", got \"%s\"\n", real_name, input_name);
                        return -1;
                    }
                }
                // if it's not the first line and it's not the first field, fail
                if (!first_line && taxa_in_line != 1)
                {
                    // TODO: fail
                    debug("NUMBER FAILED 2");
                }
            }
            // new field coming up, reset field size
            field_size = 0;
            memoryset(input_buffer, 0, sizeof (input_buffer));
            break;
        // spaces
        case ' ':
            // treat like regular input
        // default case is any chars
        default:
            // have no space to set the character, fail
            // debug("field size: %i\tinput max size: %i\n", field_size, INPUT_MAX);
            if (field_size >= INPUT_MAX) return -1;
            // set the character of the buffer
            // printf("getting character of: %c\n", c);
            *(input_buffer + field_size) = c;
            // increase the size of the field
            field_size++;
            break;
        }
    }

    int i = 0;
    do
    {
        debug("node_names[%i]: %s\n", i, *(node_names + i));
        debug("node_names[%i][0]: %i, %c\n", i, *(*(node_names + i)), *(*(node_names + i)));
        // active_node_map[i] = i for 0 <= i < N
        *(active_node_map + i) = i;
    } while ((*(*(node_names + ++i))));

    // add the checker for matrix conformity
    if (checkIdentityMatrix((double *)distances, num_rows)) return -1;
    // not all rows have been given
    debug("rows remaining: %i\n", rows_remaining);
    if (rows_remaining > 0) return -1;
    return 0;
    abort();
}

/**
 * @brief  Emit a representation of the phylogenetic tree in Newick
 * format to a specified output stream.
 * @details  This function emits a representation in Newick format
 * of a synthesized phylogenetic tree to a specified output stream.
 * See (https://en.wikipedia.org/wiki/Newick_format) for a description
 * of Newick format.  The tree that is output will include for each
 * node the name of that node and the edge distance from that node
 * its parent.  Note that Newick format basically is only applicable
 * to rooted trees, whereas the trees constructed by the neighbor
 * joining method are unrooted.  In order to turn an unrooted tree
 * into a rooted one, a root will be identified according by the
 * following method: one of the original leaf nodes will be designated
 * as the "outlier" and the unique node adjacent to the outlier
 * will serve as the root of the tree.  Then for any other two nodes
 * adjacent in the tree, the node closer to the root will be regarded
 * as the "parent" and the node farther from the root as a "child".
 * The outlier node itself will not be included as part of the rooted
 * tree that is output.
 *
 * @param out  Stream to which to output a rooted tree represented in
 * Newick format.
 * @param x  Pointer to the leaf node to be regarded as the "outlier".
 * The unique node adjacent to the outlier will be the root of the tree
 * that is output.  The outlier node itself will not be part of the tree
 * that is emitted.
 */
void emit_newick_format(FILE *out) {
    // TO BE IMPLEMENTED
    abort();
}

/**
 * @brief  Emit the synthesized distance matrix as CSV.
 * @details  This function emits to a specified output stream a representation
 * of the synthesized distance matrix resulting from the neighbor joining
 * algorithm.  The output is in the same CSV form as the program input.
 * The number of rows and columns of the matrix is equal to the value
 * of num_all_nodes at the end of execution of the algorithm.
 * The submatrix that consists of the first num_leaves rows and columns
 * is identical to the matrix given as input.  The remaining rows and columns
 * contain estimated distances to internal nodes that were synthesized during
 * the execution of the algorithm.
 *
 * @param out  Stream to which to output a CSV representation of the
 * synthesized distance matrix.
 */
void emit_distance_matrix(FILE *out) {
    // TO BE IMPLEMENTED
    abort();
}

/**
 * @brief  Build a phylogenetic tree using the distance data read by
 * a prior successful invocation of read_distance_data().
 * @details  This function assumes that global variables and data
 * structures have been initialized by a prior successful call to
 * read_distance_data(), in accordance with the specification for
 * that function.  The "neighbor joining" method is used to reconstruct
 * phylogenetic tree from the distance data.  The resulting tree is
 * an unrooted binary tree having the N taxa from the original input
 * as its leaf nodes, and if (N > 2) having in addition N-2 synthesized
 * internal nodes, each of which is adjacent to exactly three other
 * nodes (leaf or internal) in the tree.  As each internal node is
 * synthesized, information about the edges connecting it to other
 * nodes is output.  Each line of output describes one edge and
 * consists of three comma-separated fields.  The first two fields
 * give the names of the nodes that are connected by the edge.
 * The third field gives the distance that has been estimated for
 * this edge by the neighbor-joining method.  After N-2 internal
 * nodes have been synthesized and 2*(N-2) corresponding edges have
 * been output, one final edge is output that connects the two
 * internal nodes that still have only two neighbors at the end of
 * the algorithm.  In the degenerate case of N=1 leaf, the tree
 * consists of a single leaf node and no edges are output.  In the
 * case of N=2 leaves, then no internal nodes are synthesized and
 * just one edge is output that connects the two leaves.
 *
 * Besides emitting edge data (unless it has been suppressed),
 * as the tree is built a representation of it is constructed using
 * the NODE structures in the nodes array.  By the time this function
 * returns, the "neighbors" array for each node will have been
 * initialized with pointers to the NODE structure(s) for each of
 * its adjacent nodes.  Entries with indices less than N correspond
 * to leaf nodes and for these only the neighbors[0] entry will be
 * non-NULL.  Entries with indices greater than or equal to N
 * correspond to internal nodes and each of these will have non-NULL
 * pointers in all three entries of its neighbors array.
 * In addition, the "name" field each NODE structure will contain a
 * pointer to the name of that node (which is stored in the corresponding
 * entry of the node_names array).
 *
 * @param out  If non-NULL, an output stream to which to emit the edge data.
 * If NULL, then no edge data is output.
 */
void build_taxonomy(FILE *out) {
    // TO BE IMPLEMENTED
    abort();
}
