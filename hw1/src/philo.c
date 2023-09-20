#include <stdlib.h>

#include "global.h"
#include "debug.h"

#include "util.h"
#include "compare.h"

#define DEFTHIS 1
#undef DEFTHIS

int taxa_helper();
int find_newick_form(NODE *node, NODE* prev, NODE *head, FILE *out);
int find_newick(NODE* node, NODE* prev, NODE* head);

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
    if (in == NULL)
    {
        fprintf(stderr, "ERROR: no file given\n");
        return -1;
    }// error with file given

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
                // debug("rows remaining: %i\n", rows_remaining);
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
            if (num_rows != taxa_in_line)
            {
                fprintf(stderr, "ERROR: number of taxa is not equal to the number of rows\n");
                return -1;
            }
            if (first_line)
            {
                memorycpy(*(node_names + name_num), input_buffer, field_size + 1);
                NODE named_node;
                named_node.name = *(node_names + name_num);
                *(named_node.neighbors) = NULL;
                *(named_node.neighbors + 1) = NULL;
                *(named_node.neighbors + 2) = NULL;
                *(nodes + name_num) = named_node;
                name_num++;
            }else{
                double n;
                if (validateNum(input_buffer, field_size, &n))
                {
                    // debug("number at end got: %lf\n", n);
                    *(*(distances + col) + row) = n;
                    row++;
                    max_col++;
                    col = 0;
                } else {
                    // last field is not a number and not in the first line, fail
                    fprintf(stderr, "ERROR: last field is not a number and not in the first line\n");
                    // debug("NUMBER FAILED");
                    return -1;
                }
            }
            // else reset taxa count and field size for next line
            taxa_in_line = 0;
            field_size = 0;
            first_line = 0;
            memoryset(input_buffer, 0, sizeof (input_buffer));
            // check if we went too many rows down
            // debug("current row: %i\tremaining rows: %i\n", num_rows - rows_remaining, rows_remaining);
            if (rows_remaining-- <= 0)
            {
                fprintf(stderr, "ERROR: too many rows given\n");
                return -1;
            }
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
            // // debug("GOT HERE with: %s\n", input_buffer);
            // copy over the input buffer to the node_names array from beginning to
            // the size of the field + 1 to account for the null terminator
            if (first_line)
            {
                memorycpy(*(node_names + name_num), input_buffer, field_size + 1);
                NODE named_node;
                named_node.name = *(node_names + name_num);
                *(named_node.neighbors) = NULL;
                *(named_node.neighbors + 1) = NULL;
                *(named_node.neighbors + 2) = NULL;
                *(nodes + name_num) = named_node;
                name_num++;
            }
            // place the number in the distances matrix
            double n;
            if (validateNum(input_buffer, field_size, &n))
            {
                // debug("Number got was: %lf\n", n);
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
                        // debug("NAME FAILED: expected \"%s\", got \"%s\"\n", real_name, input_name);
                        fprintf(stderr, "ERROR: name of taxa \"%s\" is not the same as in first line\n", input_name);
                        return -1;
                    }
                }
                // if it's not the first line and it's not the first field, fail
                if (!first_line && taxa_in_line != 1)
                {
                    fprintf(stderr, "ERROR: last field is not a number and not in the first line\n");
                    // debug("NUMBER FAILED 2");
                    return -1;
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
            // // debug("field size: %i\tinput max size: %i\n", field_size, INPUT_MAX);
            if (field_size >= INPUT_MAX)
            {
                fprintf(stderr, "ERROR: field size is greater than input max size\n");
                return -1;
            }
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
        // debug("node_names[%i]: %s\n", i, *(node_names + i));
        // debug("node_names[%i][0]: %i, %c\n", i, *(*(node_names + i)), *(*(node_names + i)));
        // active_node_map[i] = i for 0 <= i < N
        *(active_node_map + i) = i;
    } while ((*(*(node_names + ++i))));

    // add the checker for matrix conformity
    if (checkIdentityMatrix((double *)distances, num_rows))
    {
        fprintf(stderr, "ERROR: matrix is not symmetric\n");
        return -1;
    }
    // not all rows have been given
    // debug("rows remaining: %i\n", rows_remaining);
    if (rows_remaining > 0)
    {
        fprintf(stderr, "ERROR: not all rows have been given\n");
        return -1;
    }
    return 0;
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
 * tree that is output.  The node to be used as the outlier will be
 * determined as follows:  If the global variable "outlier_name" is
 * non-NULL, then the leaf node having that name will be used as
 * the outlier.  If the value of "outlier_name" is NULL, then the
 * leaf node having the greatest total distance to the other leaves
 * will be used as the outlier.
 *
 * @param out  Stream to which to output a rooted tree represented in
 * Newick format.
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.  If the global variable "outlier_name" is
 * non-NULL, then it is an error if no leaf node with that name exists
 * in the tree.
 */
int emit_newick_format(FILE *out) {
    NODE *outlier_node = NULL;
    if (outlier_name != NULL)
    {
        // check if outlier_name exists in the node_names array
        int i = 0;
        int found = 0;
        while (i < num_taxa)
        {
            if (compareStrings(*(node_names + i), outlier_name))
            {
                // found the outlier, set it to the outlier node
                outlier_node = nodes + i;
                found = 1;
                break;
            }
            i++;
        }
        if (!found)
        {
            // outlier_name does not exist in the node_names array
            fprintf(stderr, "ERROR: outlier_name does not exist in the node_names array\n");
            return -1;
        }
    }
    if (outlier_node != NULL)
    {
        find_newick_form(outlier_node, NULL, outlier_node, out);
    }else
    {
        // find the outlier node by finding the node with the greatest total distance
        // this node can be found by finding the node with the greatest row sum
        // and it muust be a leaf node (matrix [0,num_taxa))
        int i = 0;
        int max_row = 0;
        double max_row_sum = 0;
        while (i < num_taxa)
        {
            double row_sum = 0;
            int j = 0;
            while (j < num_taxa)
            {
                row_sum += *(*(distances + i) + j);
                j++;
            }
            if (row_sum > max_row_sum)
            {
                max_row_sum = row_sum;
                max_row = i;
            }
            i++;
        }
        // find_newick_form
        // find_newick_form(nodes + max_row, NULL, nodes + max_row, out);
        find_newick(nodes + max_row, NULL, nodes + max_row);
        printf("\n");
    }
    // abort();
    return 0;
}

int find_newick(NODE* node, NODE* prev, NODE* head)
{
    if (node == NULL) return 0;
    NODE *parent = (NODE*) *(node->neighbors);
    NODE *n1 = (NODE*) *(node->neighbors + 1);
    NODE *n2 = (NODE*) *(node->neighbors + 2);

    if (parent != NULL)
    {
        printf("(");
        find_newick(parent, node, head);
        *(node->neighbors) = NULL;
    }

    if (node == head) return 0;
    // simple null check
    if (n1 != NULL)
    {
        // check if n1 is either the head or the previous node
        if (n1 != head && n1 != prev)
        {
            // n1 is unique, print it out in the form of (n1:dist1)
            double dist1 = *(*(distances + (node - nodes)) + (n1 - nodes));
            printf("%s:%.2lf", n1->name, dist1);
        }
    }
    // do the same for n2
    if (n2 != NULL)
    {
        if (n2 != head && n2 != prev)
        {
            double dist2 = *(*(distances + (node - nodes)) + (n2 - nodes));
            printf(",%s:%.2lf", n2->name, dist2);
        }
    }
    // if both n1 and n2 are null, then this is a leaf node
    if (n1 == NULL && n2 == NULL)
    {
        // print out the name and distance of the node only if it's not the head node or the previous node
        if (node != head && node != prev)
        {
            double dist = *(*(distances + (node - nodes)) + (prev ? (prev - nodes) : 0));
            printf("%s:%.2lf", node->name, dist);
        }
        // print out a comma if this is not the head node but the previous node
        if (node != head && node == prev)
        {
            printf(",");
        }
    }
    // print out the name and distance of the node only if it's not the head node or the previous node
    if (node != head && node != prev)
    {
        double dist = *(*(distances + (node - nodes)) + (prev ? (prev - nodes) : 0));
        printf(")%s:%.2lf", node->name, dist);
    }
    if (parent != head && (n1 == prev || n2 == prev) && (n1 != head && n2 != head))
    {
        printf(",");
    }
    return 0;
}

int find_newick_form(NODE *node, NODE *prev, NODE *head, FILE *out)
{
    NODE* node_parent = (NODE*) *(node->neighbors);
    if (node_parent) fprintf(out, "(");
    if (*(node->neighbors)) find_newick_form(*(node->neighbors), node, head, out);

    NODE* n1 = (NODE*) *(node->neighbors + 1);
    NODE* n2 = (NODE*) *(node->neighbors + 2);
    if (n1 && n2)
    {
        // internal node
        // distance from node to children
        double dist1 = *(*(distances + (node - nodes)) + (n1 - nodes));
        double dist2 = *(*(distances + (node - nodes)) + (n2 - nodes));

        if (prev == n1 || prev == n2)
        {
            // will be handled by previous recursion
            if (prev == n1)
            {
                fprintf(out, "%s:%.2lf)", n2->name, dist2);
            }else{
                fprintf(out, "%s:%.2lf)", n1->name, dist1);
            }
        }else{
            fprintf(out, "%s:%.2lf,%s:%.2lf)", n1->name, dist1, n2->name, dist2);
        }
    }
    double dist = *(*(distances + (node - nodes)) + (prev ? (prev - nodes) : 0));
    NODE* head_parent = (NODE*) *(head->neighbors);
    if (node != head) head_parent == node ? fprintf(out, "%s:%.02lf\n", node->name, dist) : fprintf(out, "%s:%.02lf,", node->name, dist);
    return 0;
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
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int emit_distance_matrix(FILE *out) {
    int row_names = 0;
    // emit the first ',' character in the first row
    if (fprintf(out, ",") < 1) return -1;
    while (row_names < num_all_nodes)
    {
        if (fprintf(out, "%s", *(node_names + row_names)) < 1) return -1;
        if (row_names < num_all_nodes - 1)
        {
            if (fprintf(out, ",") < 1) return -1;
        }
        row_names++;
    }
    if (fprintf(out, "\n") < 1) return -1;
    int current_row = 0;
    while (current_row < num_all_nodes)
    {
        int current_col = 0;
        if (fprintf(out, "%s,", *(node_names + current_row)) < 1) return -1;
        while (current_col < num_all_nodes)
        {
            if (fprintf(out, "%.2lf", *(*(distances + current_row) + current_col)) < 1) return -1;
            if (current_col < num_all_nodes - 1)
            {
                if (fprintf(out, ",") < 1) return -1;
            }
            current_col++;
        }
        if (fprintf(out, "\n") < 1) return -1;
        current_row++;
    }
    // abort();
    return 0;
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
 * @return 0 in case the output is successfully emitted, otherwise -1
 * if any error occurred.
 */
int build_taxonomy(FILE *out) {

    if (num_taxa < 1 || num_active_nodes < 1)
    {
        fprintf(stderr, "ERROR: no taxa given\n");
        return -1;
    }// error with file given

    if (num_taxa == 1 || num_active_nodes == 1)
    {
        // only one taxa given, no need to build tree
        return 0;
    }

    int result = taxa_helper(out);
    if (result == -1)
    {
        fprintf(stderr, "ERROR: taxa_helper failed\n");
        return -1;
    }
    return 0;
}

int taxa_helper(FILE* out)
{
    #ifdef DEFTHIS
    // print out the distances matrix
    sprintf(input_buffer, "%i_iteration.csv", num_all_nodes);
    FILE *distances_file = fopen(input_buffer, "w");
    emit_distance_matrix(distances_file);
    #endif
    if (num_active_nodes < 3)
    {
        // number of active nodes is less than 3, cannot build tree
        if (num_active_nodes < 2)
        {
            // only one taxa given, no need to build tree
            return 0;
        }
        // 2 taxa given, just need to join them
        // get the two active nodes
        int node1 = *(active_node_map);
        int node2 = *(active_node_map + 1);
        // get the nodes
        NODE *n1 = nodes + node1;
        NODE *n2 = nodes + node2;
        // set the neighbors
        *(n1->neighbors + 1) = n2;
        *(n2->neighbors + 1) = n1;

        if (out != NULL)
        {
            // emit the edge data
            if (fprintf(out, "%i,%i,%.2lf\n", node2, node1, *(*(distances + node1) + node2)) < 1) return -1;
        }
        return 1;
    }


    // set up the iterators to go through each row and column of the distances matrix
    int row = 0, col = 0;

    // first reset row_sums
    while (row < num_all_nodes)
    {
        *(row_sums + row) = 0;
        row++;
    }

    // reset row
    row = 0;

    while (row < num_active_nodes)
    {
        // get the row that is listed as active in the active node map
        int row_in_map = *(active_node_map + row);
        // calculate the row sum
        double row_sum = 0;
        while (col < num_all_nodes)
        {
            int add_col = 0;
            int col_iter = 0;
            // check to see if this column is an active column
            while (col_iter < num_active_nodes)
            {
                if (*(active_node_map + col_iter) == col)
                {
                    add_col = 1;
                    break;
                }
                col_iter++;
            }

            // if the column was included in the active nodes, add it to the row sum
            if (add_col)
            {
                row_sum += *(*(distances + row_in_map) + col);
            }
            // increment column
            col++;
        }
        // set the row sum to correctly show the sum of the row, disregarding
        // any of the columns that were removed in previous iterations of this
        // function
        *(row_sums + row_in_map) = row_sum;
        // proceed to next row
        row++;
        // reset the column
        col = 0;
    }

    // at this point, row_sums contain the sum of each row without the columns
    // that are no longer active. now we need to find the q values

    // reset the row and column iterators
    row = 0, col = 0;

    // the minimum q value, initialized to the value at the first row and second column
    double min_q = *(*distances + 1);
    // the row and column of the minimum q value in the distance matrix
    int min_row = 0, min_col = 1;
    // the index in the active node map that corresponds to the minimum row, initialized to 0
    int chosen_active_node1 = min_row;
    // the index in the active node map that corresponds to the minimum column, initialized to 1
    int chosen_active_node2 = min_col;

    // iterate through the rows that matter, meaning rows that are in the active node map
    while (row < num_active_nodes)
    {
        // the row that is listed as active in the active node map
        int row_in_map = *(active_node_map + row);
        // iterate through all the columns in that row
        while (col < num_all_nodes)
        {
            int include_col = 0;
            int col_iter = 0;
            // check to see if this column is an active column
            while (col_iter < num_active_nodes)
            {
                if (*(active_node_map + col_iter) == col)
                {
                    include_col = 1;
                    break;
                }
                col_iter++;
            }
            // column and row are the same (meaning on the diagonal), skip
            if (col == row_in_map) include_col = 0;
            if (include_col)
            {
                // this column matters, find the q value
                // get the current distance
                double current_distance = *(*(distances + row_in_map) + col);
                // get the row sum of the row
                double row_sum = *(row_sums + row_in_map);
                // get the row sum of the column
                double col_sum = *(row_sums + col);
                // get the number of active nodes - 2
                double n2 = num_active_nodes - 2;
                // calculate the q value
                double q_value = (n2 * current_distance) - row_sum - col_sum;
                // if the q value is less than the current minimum, set the minimum to the q value
                if (q_value < min_q)
                {
                    min_q = q_value;
                    min_row = row_in_map;
                    min_col = col;
                    chosen_active_node1 = row;
                    chosen_active_node2 = col_iter;
                }
            }
            // increment column
            col++;
        }
        // increment row
        row++;
        // set column to row + 1 to avoid checking the same row and column
        col = row + 1;
    }

    // at this point, i have the minimum q value and the row and column of that q value

    // create a new node that will join the two nodes located at the min_col and min_row
    NODE *u = nodes + num_all_nodes;
    // get node f, the node at the min_row
    NODE *f = nodes + min_row;
    // get node g, the node at the min_col
    NODE *g = nodes + min_col;

    // set the neighbors of u

    *(u->neighbors + 1) = f;
    *(u->neighbors + 2) = g;
    if (num_active_nodes - 1 < 3)
    {
        // the single edge added as the last step should be stored
        // as neighbor[0] of both of the nodes being connected
        *(f->neighbors) = u;
        *(g->neighbors) = u;
    }

    // set the parents of f and g
    *(f->neighbors) = u;
    *(g->neighbors) = u;

    // set the name of the node in node_names
    sprintf(*(node_names + num_all_nodes), "#%d", num_all_nodes);
    // set the name of the node in the node struct
    u->name = *(node_names + num_all_nodes);

    // update the distances matrix to incorporate u
    /**
     * Formula copied from docs:
     * D'(u, k) = D'(k, u) =
     *  if k = u then 0
     * else if k = f then D'(u, k) = (D(f, g) + (S(f) - S(g)) / (N - 2)) / 2
     * else if k = g then D'(u, k) = (D(f, g) + (S(g) - S(f)) / (N - 2)) / 2
     * else D'(u, k) = (D(f, k) + D(g, k) - D(f, g)) / 2
     */

    // we only need to update the distances matrix for the new node, u
    row = num_all_nodes, col = 0;

    // iterate through the columns of the new node
    while (col < num_all_nodes)
    {
        int col_iter = 0;
        int include_col = 0;
        // check to see if this column is an active column
        while (col_iter < num_active_nodes)
        {
            if (*(active_node_map + col_iter) == col)
            {
                include_col = 1;
                break;
            }
            col_iter++;
        }
        // if the column is not an active column, skip
        if (!include_col)
        {
            col++;
            continue;
        }
        // if the column is the same as the row, set the distance to 0
        if (col == row)
        {
            *(*(distances + row) + col) = 0;
            col++;
            continue;
        }

        // get D(f,g)
        double dfg = *(*(distances + min_row) + min_col);
        // get the row sum of f and g
        double sf = *(row_sums + min_row);
        double sg = *(row_sums + min_col);
        // get the number of active nodes - 2
        double n2 = num_active_nodes - 2;

        // if the column is the same as f, set the distance to the first formula
        if (col == min_row)
        {
            debug("at min_row: %i, setting to %.2lf\n", col, (dfg + (sf - sg) / n2) / 2);
            double d = (dfg + (sf - sg) / n2) / 2;
            *(*(distances + row) + col) = d;
            // set the value for the symmetric position as the same
            *(*(distances + col) + row) = d;
            col++;
            continue;
        }

        // if the column is the same as g, set the distance to the second formula
        if (col == min_col)
        {
            debug("at min_col: %i, setting to %.2lf\n", col, (dfg + (sg - sf) / n2) / 2);
            double d = (dfg + (sg - sf) / n2) / 2;
            *(*(distances + row) + col) = d;
            // set the value for the symmetric position as the same
            *(*(distances + col) + row) = d;
            col++;
            continue;
        }

        // set the distance to the third formula
        double dfk = *(*(distances + min_row) + col);
        double dgk = *(*(distances + min_col) + col);

        debug("at col: %i, setting to %.2lf\n", col, (dfk + dgk - dfg) / 2);
        double d = (dfk + dgk - dfg) / 2;
        *(*(distances + row) + col) = d;
        // set the value for the symmetric position as the same
        *(*(distances + col) + row) = d;
        col++;
    }

    // print out the edge data if out is not null
    if (out != NULL)
    {
        // print out f, u, value of D(f, u)
        fprintf(out, "%i,%i,%.2lf\n", min_row, num_all_nodes, *(*(distances + min_row) + num_all_nodes));
        // print out g, u, value of D(g, u)
        fprintf(out, "%i,%i,%.2lf\n", min_col, num_all_nodes, *(*(distances + min_col) + num_all_nodes));
    }

    // inactivate f and g
    // set f to the new node in the active node map
    *(active_node_map + chosen_active_node1) = num_all_nodes++;
    // set g to the last active node in the active node map
    *(active_node_map + chosen_active_node2) = *(active_node_map + --num_active_nodes);

    // recurse
    return taxa_helper(out);
}
