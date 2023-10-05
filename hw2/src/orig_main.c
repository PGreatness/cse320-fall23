#include "orig.h"

void free_all(Course* c);
static struct option_info {
        unsigned int val;
        char *name;
        char chr;
        int has_arg;
        char *argname;
        char *descr;
} option_table[] = {
 {REPORT,         "report",    'r',      no_argument, NULL,
                  "Process input data and produce specified reports."},
 {COLLATE,        "collate",   'c',      no_argument, NULL,
                  "Collate input data and dump to standard output."},
 {FREQUENCIES,    "freqs",     0,        no_argument, NULL,
                  "Print frequency tables."},
 {QUANTILE,      "quants",    0,        no_argument, NULL,
                  "Print quantile information."},
 {SUMMARIES,      "summaries", 0,        no_argument, NULL,
                  "Print quantile summaries."},
 {MOMENTS,        "stats",     0,        no_argument, NULL,
                  "Print means and standard deviations."},
 {COMPOSITES,     "comps",     0,        no_argument, NULL,
                  "Print students' composite scores."},
 {INDIVIDUALS,    "indivs",    0,        no_argument, NULL,
                  "Print students' individual scores."},
 {HISTOGRAMS,     "histos",    0,        no_argument, NULL,
                  "Print histograms of assignment scores."},
 {TABSEP,         "tabsep",    0,        no_argument, NULL,
                  "Print tab-separated table of student scores."},
 {ALLOUTPUT,      "all",       'a',      no_argument, NULL,
                  "Print all reports."},
 {SORTBY,         "sortby",    'k',      required_argument, "key",
                  "Sort by {name, id, score}."},
 {NONAMES,        "nonames",   'n',      no_argument, NULL,
                  "Suppress printing of students' names."},
 {OUTPUT,          "output",   'o',      required_argument, "file",
                  "Specify file to be used for output."},
    {14, NULL, 0, 0, NULL, NULL}
};

static struct option long_options[15];
static char *short_options = "rck:ano:";

static void init_options() {
    for(unsigned int i = 0; i < 15; i++) {
        struct option_info *oip = &option_table[i];
        if(oip->val != i) {
            fprintf(stderr, "Option initialization error\n");
            abort();
        }
        struct option *op = &long_options[i];
        op->name = oip->name;
        op->has_arg = oip->has_arg;
        op->flag = NULL;
        op->val = oip->val;
    }
}

static int report, collate, freqs, quants, summaries, moments,
            student_scores, composite, histograms, tabsep, nonames;

static void usage(char* name);

extern int errors, warnings;

void checkIfAbbreviated(int opt, char optval, char** argv);
void checkPositionalArguments(char* flag, char** argv);

int orig_main(int argc,char* argv[])
{
        Course *c;
        Stats *s;
        char optval;
        int (*compare)() = comparename;
        FILE* f = stdout;

        fprintf(stderr, BANNER);
        init_options();
        char positional_arguments_satisfied = 0;
        if(argc <= 1) usage(argv[0]);
        while(optind < argc) {
            if((optval = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
                debug("GOT: %c, %i\n", optval, optind);
                switch(optval) {
                case 'r': case REPORT:
                    #ifdef STRICT_MATCHING
                    // check if it is abbreviated and exit
                    checkIfAbbreviated(REPORT, optval, argv);
                    #endif
                    // argument satisfied by collate
                    if (positional_arguments_satisfied != 0)
                    {
                        fprintf(stderr, "You must supply either %s or %s as the first argument.\n\n",
                                option_table[REPORT].name, option_table[COLLATE].name);
                        usage(argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    positional_arguments_satisfied = 'r';
                    report++; break;
                case 'c': case COLLATE:
                    #ifdef STRICT_MATCHING
                    // check if the argument has been abbreviated
                    checkIfAbbreviated(COLLATE, optval, argv);
                    #endif
                    // if collate is not the first option, it will fail
                    if (positional_arguments_satisfied != 0)
                    {
                        fprintf(stderr, "You must supply either %s or %s as the first argument.\n\n",
                                option_table[REPORT].name, option_table[COLLATE].name);
                        usage(argv[0]);
                        exit(EXIT_FAILURE);
                    }
                    positional_arguments_satisfied = 'c';
                    collate++; break;
                case TABSEP:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(TABSEP, optval, argv);
                    #endif
                    tabsep++; break;
                case 'n': case NONAMES:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(NONAMES, optval, argv);
                    #endif
                    nonames++; break;
                case 'k': case SORTBY:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(NONAMES, optval, argv);
                    #endif
                    if(!strcmp(optarg, "name"))
                        compare = comparename;
                    else if(!strcmp(optarg, "id"))
                        compare = compareid;
                    else if(!strcmp(optarg, "score"))
                        compare = comparescore;
                    else {
                        fprintf(stderr,
                                "Option '%s' requires argument from {name, id, score}.\n\n",
                                option_table[SORTBY].name);
                        usage(argv[0]);
                    }
                    break;
                case FREQUENCIES:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(FREQUENCIES, optval, argv);
                    #endif
                    ++freqs; break;
                case QUANTILE:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(QUANTILE, optval, argv);
                    #endif
                    quants++; break;
                case SUMMARIES:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(SUMMARIES, optval, argv);
                    #endif
                    summaries++; break;
                case MOMENTS:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(MOMENTS, optval, argv);
                    #endif
                    moments++; break;
                case COMPOSITES:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(COMPOSITES, optval, argv);
                    #endif
                    composite++; break;
                case INDIVIDUALS:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(INDIVIDUALS, optval, argv);
                    #endif
                    student_scores++; break;
                case HISTOGRAMS:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(HISTOGRAMS, optval, argv);
                    #endif
                    histograms++; break;
                case 'a': case ALLOUTPUT:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(ALLOUTPUT, optval, argv);
                    #endif
                    freqs++; quants++; summaries++; moments++;
                    composite++; student_scores++; histograms++; tabsep++;
                    break;
                case '?':
                    usage(argv[0]);
                    break;
                case 'o': case OUTPUT:
                    checkPositionalArguments(&positional_arguments_satisfied, argv);
                    #ifdef STRICT_MATCHING
                    checkIfAbbreviated(OUTPUT, optval, argv);
                    #endif
                    f = fopen(optarg, "w");
                    if(f == NULL) {
                        fprintf(stderr, "Unable to open output file '%s'.\n\n", optarg);
                        usage(argv[0]);
                    }
                    break;
                default:
                    break;
                }
            } else {
                break;
            }
        }
        debug("First: %s\n", argv[optind]);
        if(optind == argc) {
                fprintf(stderr, "No input file specified.\n\n");
                usage(argv[0]);
        }
        char *ifile = argv[optind];
        if(report == collate) {
                fprintf(stderr, "Exactly one of '%s' or '%s' is required.\n\n",
                        option_table[REPORT].name, option_table[COLLATE].name);
                usage(argv[0]);
        }

        fprintf(stderr, "Reading input data...\n");
        c = readfile(ifile);
        if(errors) {
            // if (c != NULL) free_course(c);
            if (c != NULL) free_all(c);
           printf("%d error%s found, so no computations were performed.\n",
                  errors, errors == 1 ? " was": "s were");
           exit(EXIT_FAILURE);
        }

        fprintf(stderr, "Calculating statistics...\n");
        s = statistics(c);
        if(s == NULL)
        {
            if (c != NULL) free_all(c);
            if (s != NULL) free_stats(s);
            fatal("There is no data from which to generate reports.");
        }
        normalize(c, s);
        composites(c);
        sortrosters(c, comparename);
        checkfordups(c->roster);
        if (collate)
        {
            fprintf(stderr, "Dumping collated data...\n");
            writecourse(f, c, FALSE);
            // if (c != NULL) free_course(c);
            if (c != NULL)
                free_all(c);
            if (s != NULL)
                free_stats(s);
            if (f != NULL)
                fclose(f);
            exit(errors ? EXIT_FAILURE : EXIT_SUCCESS);
        }
        sortrosters(c, compare);

        fprintf(stderr, "Producing reports...\n");
        if(report) reportparams(f, ifile, c);
        if(moments) reportmoments(f, s);
        if(composite) reportcomposites(f, c, nonames);
        if(freqs) reportfreqs(f, s);
        if(quants) reportquantiles(f, s);
        if(summaries) reportquantilesummaries(f, s);
        if(histograms) reporthistos(f, c, s);
        if(student_scores) reportscores(f, c, nonames);
        if(tabsep) reporttabs(f, c, nonames);

        fprintf(stderr, "\nProcessing complete.\n");
        fprintf(stderr,"%d warning%s issued.\n", warnings+errors,
               warnings+errors == 1? " was": "s were");
        // if (c != NULL) free_course(c);
        if (c != NULL) free_all(c);
        if (s != NULL) free_stats(s);
        if (f != NULL) fclose(f);
        exit(errors ? EXIT_FAILURE : EXIT_SUCCESS);
}

void usage(char* name)
{
        struct option_info *opt;

        fprintf(stderr, "Usage: %s [options] <data file>\n", name);
        fprintf(stderr, "Valid options are:\n");
        for(unsigned int i = 0; i < 14; i++) {
                opt = &option_table[i];
                char optchr[5] = {' ', ' ', ' ', ' ', '\0'};
                if(opt->chr)
                  sprintf(optchr, "-%c, ", opt->chr);
                char arg[32];
                if(opt->has_arg)
                    sprintf(arg, " <%.10s>", opt->argname);
                else
                    sprintf(arg, "%.13s", "");
                fprintf(stderr, "\t%s--%-10s%-13s\t%s\n",
                            optchr, opt->name, arg, opt->descr);
                opt++;
        }
        exit(EXIT_FAILURE);
}

void checkIfAbbreviated(int opt, char optval, char** argv)
{
    int valid_flag = 0;
    // check if the argument has been abbreviated
    if (option_table[opt].chr == optval)
        valid_flag++;
    if (!valid_flag)
    {
        char *option = argv[optind - 1];
        if (option[0] == '-' && option[1] == '-')
        {
            char *option_name = option + 2;
            if (strcmp(option_name, option_table[opt].name) != 0)
            {
                fprintf(stderr, "Unknown option '%s'. Did you mean '--%s'?\n",
                        argv[optind - 1],
                        option_table[opt].name);
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void checkPositionalArguments(char* flag, char** argv)
{
    if (*flag == 0)
    {
        fprintf(stderr, "Exactly one of '%s' or '%s' is required.\n\n",
                        option_table[REPORT].name, option_table[COLLATE].name);
                usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}
