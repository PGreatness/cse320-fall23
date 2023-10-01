#ifndef HW2_FREE_H
#define HW2_FREE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "version.h"
#include "global.h"
#include "gradedb.h"
#include "stats.h"
#include "read.h"
#include "write.h"
#include "normal.h"
#include "sort.h"
#include "error.h"
#include "report.h"

void free_professor(Professor* p);
void free_assignments(Assignment* a);
void free_sections(Section* s);
void free_students(Student* s);
void free_scores(Score* s);
void free_all(Course* c);
void free_stats(Stats* s);
void free_sectionstats(Sectionstats* s);
void free_classstats(Classstats* s);

#endif