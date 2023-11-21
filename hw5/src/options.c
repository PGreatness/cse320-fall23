#include "student/options.h"

const command_line_args_t STUDENT_OPTIONS[] = {
    {'p', NULL, "Port number to listen to", 1, 1},
    {'h', NULL, "The hostname to connect as a client", 0, 1},
    {'q', NULL, "Enable quiet mode", 0, 0}
};

const char* STUDENT_SHORT_OPTIONS = "p:h:q";

const struct option STUDENT_LONG_OPTIONS[] = {
    {NULL, 0, NULL, 0}
};