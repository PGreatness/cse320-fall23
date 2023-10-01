#include "free.h"

void free_freqs(Freqs* f)
{
    if (f == NULL) return;
    while (f != NULL)
    {
        Freqs* tmp = f->next;
        free(f);
        f = tmp;
    }
}

void free_classstats(Classstats* c)
{
    if (c == NULL) return;
    while(c != NULL)
    {
        Classstats* tmp = c->next;
        Freqs* f = c->freqs;
        if (f != NULL)
        {
            free_freqs(f);
            c->freqs = NULL;
        }
        Sectionstats* s = c->sstats;
        if (s != NULL)
        {
            free_sectionstats(s);
            c->sstats = NULL;
        }
        free(c);
        c = tmp;
    }
}

void free_stats(Stats* s)
{
    if (s == NULL) return;
    if (s->cstats != NULL)
    {
        free_classstats(s->cstats);
        s->cstats = NULL;
    }
    free(s);
}

void free_professor(Professor* p)
{
    if (p == NULL) return;
    if (p->name != NULL)
    {
        free(p->name);
        p->name = NULL;
    }
    if (p->surname != NULL)
    {
        free(p->surname);
        p->surname = NULL;
    }
    free(p);
}

void free_assignments(Assignment* a)
{
    if (a == NULL) return;
    while (a != NULL)
    {
        Assignment* temp = a;
        a = a->next;
        if (temp->name != NULL)
        {
            free(temp->name);
            temp->name = NULL;
        }
        if (temp->atype != NULL)
        {
            free(temp->atype);
            temp->atype = NULL;
        }
        free(temp);
    }
}

void free_sections(Section* s)
{
    if (s == NULL) return;
    while (s != NULL)
    {
        Section* tmp = s->next;
        if (s->name != NULL)
        {
            free(s->name);
            s->name = NULL;
        }
        if (s->assistant != NULL)
        {
            Assistant* ast = s->assistant;
            if (ast->name != NULL)
            {
                free(ast->name);
                ast->name = NULL;
            }
            if (ast->surname != NULL)
            {
                free(ast->surname);
                ast->surname = NULL;
            }
            free(ast);
            s->assistant = NULL;
        }
        free(s);
        s = tmp;
    }
}

void free_sectionstats(Sectionstats* s)
{
    if (s == NULL) return;
    while (s != NULL)
    {
        Sectionstats* tmp = s->next;
        if (s->freqs != NULL)
        {
            free_freqs(s->freqs);
            s->freqs = NULL;
        }
        free(s);
        s = tmp;
    }
}

void free_scores(Score* s)
{
    if (s == NULL) return;
    while (s != NULL)
    {
        Score* tmp = s->next;
        if (s->code != NULL)
        {
            free(s->code);
            s->code = NULL;
        }
        free(s);
        s = tmp;
    }
}

void free_students(Student* std)
{
    if (std == NULL) return;
    while (std != NULL)
    {
        Student* tmp = std->cnext;
        if (std->name != NULL)
        {
            free(std->name);
            std->name = NULL;
        }
        if (std->surname != NULL)
        {
            free(std->surname);
            std->surname = NULL;
        }
        if (std->id != NULL)
        {
            free(std->id);
            std->id = NULL;
        }
        if (std->rawscores != NULL)
        {
            free_scores(std->rawscores);
            std->rawscores = NULL;
        }
        if (std->normscores != NULL)
        {
            free_scores(std->normscores);
            std->normscores = NULL;
        }
        free(std);
        std = tmp;
    }
}

void free_all(Course* c)
{
    if (c->number != NULL)
    {
        free(c->number);
        c->number = NULL;
    }
    if (c->title != NULL)
    {
        free(c->title);
        c->title = NULL;
    }
    if (c->professor != NULL)
    {
        free_professor(c->professor);
        c->professor = NULL;
    }
    if (c->assignments != NULL)
    {
        free_assignments(c->assignments);
        c->assignments = NULL;
    }
    if (c->sections != NULL)
    {
        free_sections(c->sections);
        c->sections = NULL;
    }
    if (c->roster != NULL)
    {
        free_students(c->roster);
        c->roster = NULL;
    }
    free(c);
}