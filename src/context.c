#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "xzpv/context.h"
#include "xzpv/output.h"

Context* context_new() {
    Context* context = calloc(1, sizeof(Context));
    if (context == NULL) {
        eprintf("Failed to initialize context object.\n");
        return NULL;
    }

    context->cwd = malloc(PATH_MAX);
    context->destination = malloc(PATH_MAX);
    context->target = malloc(PATH_MAX);
    if (context->cwd == NULL || context->destination == NULL || context->target == NULL) {
        eprintf("Failed to allocate memory for context string buffers.\n");
        context_destroy(context);
        return NULL;
    }

    context->cwd[0] = '\0';
    context->destination[0] = '\0';
    context->target[0] = '\0';

    context->verbose = 0;
    context->delete = 0;
    context->dry = 0;
    context->compression = DEFAULT_COMPRESSION_LEVEL;

    return context;
}

void context_toString(const Context* context, const Printer printer) {
    printer(CONTEXT_TOSTRING_INVOCATION);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void context_setCwd(Context* context) {
    if (context == NULL || context->cwd == NULL) {
        eprintf("Could not set cwd because context or buffer was NULL.\n");
        return;
    }

    if (getcwd(context->cwd, PATH_MAX) == NULL) {
        context->cwd[0] = '\0';
    }
}

void context_destroy(Context* context) {
    if (context == NULL) return;
    free(context->cwd);
    free(context->destination);
    free(context->target);
    free(context);
}
