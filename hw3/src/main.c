#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    // double* ptr = sf_malloc(sizeof(double));

	void *x = sf_malloc(112);
	void *y = sf_realloc(x, 50);

    sf_free(y);
    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
