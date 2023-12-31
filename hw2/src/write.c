
/*
 * Write out grade database to ASCII file.
 */

#include <stddef.h>
#include <stdio.h>
#include "global.h"
#include "gradedb.h"
#include "write.h"
#include "error.h"

void writeprofessor(FILE* fd, Professor* p)
{
        fprintf(fd, " PROFESSOR %s, %s\n", p->surname, p->name);
}

void writeassistant(FILE* fd, Assistant* a)
{
        if (a == NULL) return;
        fprintf(fd, " ASSISTANT %s, %s\n", a->surname, a->name);
}

void writescore(FILE* fd, Score* s)
{
        fprintf(fd, "   SCORE %s", s->asgt->name);
        if(s->flag == VALID) {
                fprintf(fd, " %f", s->grade);
        } else {
                switch(s->subst) {
                case USERAW:
                        fprintf(fd, " USERAW %f", s->grade);
                        break;
                case USENORM:
                        switch(s->asgt->npolicy) {
                        case QUANTILE:
                          fprintf(fd, " USENORM %f", s->qnorm);
                          break;
                        case LINEAR:
                          fprintf(fd, " USENORM %f", s->lnorm);
                          break;
                        case SCALE:
                          fprintf(fd, " USENORM %f", s->snorm);
                          break;
                        case RAW:
                          break;
                        }
                        break;
                case USELIKEAVG:
                        fprintf(fd, " USELIKEAVG");
                        break;
                case USECLASSAVG:
                        fprintf(fd, " USECLASSAVG");
                        break;
                }
                fprintf(fd, " %s", s->code);
        }
        fprintf(fd, "\n");
}

void writestudent(FILE* fd, Student* s, int nm)
{
        Score *sp;
        fprintf(fd, "  STUDENT %s %s%s %s\n", s->id, nm ? "" : s->surname, nm ? "": ",", nm ? "" : s->name);
        for(sp = s->rawscores; sp != NULL; sp = sp->next)
                writescore(fd, sp);
}

void writesection(FILE* fd, Section* s, int nonames)
{
        Student *sp;
        fprintf(fd, " SECTION %s\n", s->name);
        writeassistant(fd, s->assistant);
        for(sp = s->roster; sp != NULL; sp = sp->next)
                writestudent(fd, sp, nonames);
}

void writeassignment(FILE* fd, Assignment *a)
{
        fprintf(fd, " ASSIGNMENT %s %s\n", a->name, a->atype);
        if(a->wpolicy == WEIGHT)
                fprintf(fd, "  WEIGHT %f\n", a->weight);
        if(a->npolicy != RAW) {
                switch(a->npolicy) {
                case LINEAR:
                        fprintf(fd, "  NORMALIZE LINEAR: %f, %f",
                               a->mean, a->stddev);
                        break;
                case QUANTILE:
                        fprintf(fd, "  NORMALIZE QUANTILE");
                        break;
                case SCALE:
                        fprintf(fd, "  NORMALIZE SCALE %f", a->scale);
                case RAW:
                        break;
                }
                fprintf(fd, " %s\n", a->ngroup == BYCLASS ? "BYCLASS": "BYSECTION");
        }
        if(a->max != 0) {
          fprintf(fd, "  MAXIMUM %f\n", a->max);
        }
}

void writecourse(FILE* fd, Course* c, int nm)
{
        Assignment *ap;
        Section *sp;
        fprintf(fd, "COURSE %s %s\n", c->number, c->title);
        writeprofessor(fd, c->professor);
        for(ap = c->assignments; ap != NULL; ap = ap->next)
                writeassignment(fd, ap);
        for(sp = c->sections; sp != NULL; sp = sp->next)
                writesection(fd, sp, nm);
}

void writefile(char* f, Course* c, int nm)
{
        FILE *fd;
        if((fd = fopen(f, "w")) == NULL) {
                error("Can't write file: %s\n", f);
                return;
        }
        writecourse(fd, c, nm);
        fclose(fd);
}

