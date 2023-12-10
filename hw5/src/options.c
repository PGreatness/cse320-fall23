#include "student/options.h"

const command_line_args_t STUDENT_OPTIONS[] = {
    {'p', NULL, "Port number to listen to", 1, 1},
};

const char* STUDENT_SHORT_OPTIONS = "p:";

const struct option STUDENT_LONG_OPTIONS[] = {
    {NULL, 0, NULL, 0}
};