#ifndef HW2_READ_H
#define HW2_READ_H

/*
 * Type definitions for database read functions
 */

Course *readfile(char *root);
Course *readcourse();
Professor *readprofessor();
Assistant *readassistant();
Assignment *readassignments();
Section *readsections(Assignment *a);
Student *readstudents(Assignment *a, Section *sep);
Score *readscores(Assignment *a);
void readgrade(Score *s);
void readweight(Assignment *a);
void readmax(Assignment *a);
void readnorm(Assignment *a);
Surname readsurname();
Name readname();
Atype readatype();
Id readid();
#ifdef MSDOS
int iswhitespace(char c);       /* Correct for buggy ANSI implementation */
#else
int iswhitespace(int c);
#endif
void gobblewhitespace();
void gobbleblanklines();
void advancetoken();
void advanceeol();
int istoken();
int tokensize();
void flushtoken();
int checktoken(char *key);
void expecttoken(char *key);
void expecteof();
void expectnewline();
void pushfile();
void previousfile();

#endif