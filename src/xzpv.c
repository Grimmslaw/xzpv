#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>

#include "xzpv/config.h"
#include "xzpv/context.h"
#include "xzpv/files.h"
#include "xzpv/operations.h"
#include "xzpv/output.h"

#define PROG_NAME "xzpv"
#define DECIMAL_BASE 10

/**
 * @brief Prints the usage text for the program and returns the exit status.
 *
 * If the exit status to return represents a good exit, the usage text will be prefaced with a description of the
 * program (as the implication is that it was called with the "-h" option).
 *
 * @param status The status for the program to exit with.
 * @param context The options context to clean up before exiting.
 * @return The status for the main function to exit the program with.
 */
int usage(const int status, Context* context) {
    context_destroy(context);
    if (status == EXIT_SUCCESS) {
        fprintf(stderr, "Usage: %s [options] <target>\n\n", PROG_NAME);
    }

    fprintf(stderr,
        "Options:\n"
        "  -h, --help                           Display this usage text and exit.\n"
        "  -c <level>, --compression <level>    Level of compression (see `man xz`).\n"
        "  -d <dest>, --destination <dest>      Desired path of the archive file.\n"
        "  -v, --verbose                        Verbose output.\n"
        "  -x, --delete                         Delete the target after archiving.\n"
        "  -0, --dry                            Simulate the actions of the command without changing files.\n");

    return status;
}

int main(const int argc, char* argv[]) {
    if (argc > 1) {
        // these are only for package managers, so don't parse them like the other options below
        if (strcmp(argv[1], "--install") == 0) {
            return handleInstallation(argv[0]) ? 0 : 1;
        }
        if (strcmp(argv[1], "--uninstall") == 0) {
            return handleUninstallation(argv[0]) ? 0 : 1;
        }
    }

    // 1. First-Run Auto-Generation Framework Execution Check
    char configFile[4096] = {0};
    getConfigPath(configFile, sizeof(configFile), argv[0]);

    FILE* f = fopen(configFile, "r");
    if (!f) {
        printf("Configuration file not found. Initializing first-run setup...\n");
        if (!handleInstallation(argv[0])) {
            fprintf(stderr, "Warning: Could not automatically generate default configuration.\n");
        }
    } else {
        fclose(f);
    }

    // 2. Parse Core Application CLI Arguments
    int option_index = 0;
    static struct option long_options[] = {
        {"help",        no_argument,        NULL, 'h'},
        {"compression", required_argument,  NULL, 'c'},
        {"dry",         no_argument,        NULL, '0'},
        {"destination", required_argument,  NULL, 'd'},
        {"verbose",     no_argument,        NULL, 'v'},
        {"delete",      no_argument,        NULL, 'x'},
        {NULL,          0,                  NULL,  0 }
    };

    Context* context = context_new();
    if (context == NULL) {
        fprintf(stderr, "Error: CLI options memory unallocated.\n\n");
        return EXIT_FAILURE;
    }

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "hc:d:vx0", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                return usage(EXIT_SUCCESS, context);
            case '0':
                context->dry = 1;
                break;
            case 'c':
                context->compression = (int) strtol(optarg, NULL, DECIMAL_BASE);
                break;
            case 'd':
                snprintf(context->destination, PATH_MAX, "%s", optarg);
                break;
            case 'v':
                context->verbose = 1;
                break;
            case 'x':
                context->delete = 1;
                break;
            default:
                fprintf(stderr, "Error: Unknown option -%c\n\n", opt);
                return usage(EXIT_FAILURE, context);
        }
    }

    // 3. Positional Argument Validation Rules Check
    if (argc - optind < 1) {
        fprintf(stderr, "Error: %s requires a positional argument specifying the target.\n\n", PROG_NAME);
        return usage(EXIT_FAILURE, context);
    }

    snprintf(context->target, PATH_MAX, "%s", argv[optind]);
    context_setCwd(context);

    // 4. Default Destination Name Configuration Setup
    if (context->destination[0] == '\0') {
        char* resolved = resolve_relative(context->target, context->cwd, PATH_MAX);
        if (resolved != NULL) {
            snprintf(context->destination, PATH_MAX, "%s.tar.xz", resolved);
            free(resolved);
        }
    }

    setVerbose(context->verbose == 1);
    context_toString(context, printf);

    // 5. Run Execution Engine Processing Pipeline Tasks
    if (archive(context) < 0) {
        fprintf(stderr, "Failed to archive file(s)/folder(s).\n");
        context_destroy(context);
        return EXIT_FAILURE;
    }

    if (cleanup(context) < 0) {
        fprintf(stderr, "Failed to cleanup target.\n");
        context_destroy(context);
        return EXIT_FAILURE;
    }

    context_destroy(context);
    return EXIT_SUCCESS;
}