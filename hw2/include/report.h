#ifndef HW2_REPORT_H
#define HW2_REPORT_H

#include <stdio.h>
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "debug.h"

#ifdef MSDOS
#include <time.h>
#else
#include <sys/types.h>
#include <time.h>
#endif

/**
 * The array of quantiles to be computed and scores to be reported.
*/

extern float quantiles[];
extern float scores[];


/*
 * Type definitions for database report functions
 */

void reportparams(FILE* fd, char* fn, Course* c);
void reportfreqs(FILE* fd, Stats* s);
float interpolatequantile(Freqs* fp, int n, float q);
void reportquantilesummaries(FILE* fd, Stats* s);
void reportquantiles(FILE* fd, Stats* s);
void reportmoments(FILE* fd, Stats* s);
void reportscores(FILE* fd, Course* c, int nonames);
void reportcomposites(FILE* fd, Course* c, int nonames);
void reporthistos(FILE* fd, Course* c, Stats* s);
void histo(FILE* fd, int bins[], float min, float max, int cnt);
void reporttabs(FILE* fd, Course* c, int nonames);

#endif