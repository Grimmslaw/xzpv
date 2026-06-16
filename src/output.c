#include <stdarg.h>
#include "xzpv/output.h"

bool GLOBAL_VERBOSE = false;

void setVerbose(const bool verbose) {
    GLOBAL_VERBOSE = verbose;
}

int iffprintf(FILE* restrict file, const char* restrict format, ...) {
    if (!GLOBAL_VERBOSE) return 0;

    va_list args;
    va_start(args, format);
    const int result = vfprintf(file, format, args);
    va_end(args);
    return result;
}

int eprintf(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    const int result = vfprintf(stderr, format, args);
    va_end(args);
    return result;
}

int ifprintf(const char* restrict format, ...) {
    if (!GLOBAL_VERBOSE) return 0;

    va_list args;
    va_start(args, format);
    const int result = vfprintf(stdout, format, args);
    va_end(args);
    return result;
}
