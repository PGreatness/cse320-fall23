#ifndef UTIL_HW1
#define UTIL_HW1
#endif

/**
 * Copies `size` bytes from `src` to `dest`
 * @returns a pointer to the next free byte location. may be unsafe
*/
char *memorycpy(char *dest, char *src, int size);

/**
 * Sets `size` bytes of `src` to `c`
 * @returns a pointer to the next free byte location. may be unsafe
*/
char *memoryset(char *src, int c, int size);

/**
 * Validates the source string as a number. The source string is taken up until `size` bytes.
 * Only the double `num` is changed at the end of this function.
 * 
 * @param src a pointer to the beginning of the string representation of a number. This
 * number can be a double, meaning it can have a decimal. Only one decimal can be present
 * in order to be a valid string
 * @param size the amount of bytes of the `src` to look at. If the size given is more than
 * the size of the source string, undefined behavior will occur
 * @param num a pointer to a double. This double's value will be changed to be value of
 * the number that is in `src`
 * @returns a `1` if everything is works, else `0` for any failures. `num` will not be set
 * in case of failure
*/
int validateNum(char *src, int size, double *num);

/**
 * Checks to see whether the given matrix is compliant with the assignment's requirements.
 * This means that the matrix has every value along the left-right diagonal as 0 and that
 * each of the values has a corresponding value i.e `matrix[i][j] == matrix[j][i]`
 * @param matrix a pointer to the start of the matrix
 * @param n the size of the matrix. Note that the size is normal counting order (1, 2, 3,...)
 * @returns `0` if fail, else `1`
*/
int checkIdentityMatrix(double *matrix, int n);

/**
 * Stringifies the given number and places it into the given buffer. The buffer must be
 * large enough to hold the stringified number. The number is stringified in the way that
 * it was given.
 * @param buffer a pointer to the start of the buffer
 * @param num the number to stringify
 * @param pos the position to start writing the stringified number
 * @param addNull whether or not to add a null terminator at the end of the stringified number
 * @returns the number of bytes written to the buffer
*/
int stringifyNumber(char *buffer, int num, int pos, int addNull);
