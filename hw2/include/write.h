#ifndef HW2_WRITE_H
#define HW2_WRITE_H

/*
 * Type definitions for functions in write.c
 */

void writeprofessor(FILE *fd, Professor *p);
void writeassistant(FILE *fd, Assistant *a);
void writescore(FILE *fd, Score *s);
void writestudent(FILE *fd, Student *s, int nonames);
void writesection(FILE *fd, Section *s, int nonames);
void writeassignment(FILE *fd, Assignment *a);
void writecourse(FILE *fd, Course *c, int nonames);
void writefile(char *f, Course *c, int nonames);

#endif