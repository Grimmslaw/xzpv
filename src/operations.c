#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WIN32) && !defined(_WIN64)
    #include <sys/wait.h>
#endif
#include "xzpv/files.h"
#include "xzpv/operations.h"
#include "xzpv/output.h"

/**
 * @brief   Executes the command--which may or may not have self-updating output--represented by the given text by
 *          opening a pipe.
 * @param command The text of the command to execute
 * @return  `0` if the command executed successfully, or a negative integer if an error occurs initializing or calling
 *          the command.
 */
static int executeCommand(const char* command) {
    ifprintf("Executing: %s\n", command);
    char buffer[256];
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        eprintf("Failed to open pipe for command execution workflow.\n");
        return -1;
    }

    // Read stdout stream contents from the piped task layout in real-time
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
        fflush(stdout);
    }

    int status = pclose(pipe);
#if defined(_WIN32) || defined(_WIN64)
    return (status == 0) ? 0 : -1;
#else
    if (WIFEXITED(status)) {
        const int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            eprintf("Subprocess pipeline failed with system exit code: %d\n", exit_code);
            return -1;
        }
        return 0;
    }
    return -1;
#endif
}

int archive(const Context* context) {
    char* command = NULL;
#if defined(_WIN32) || defined(_WIN64)
    // Windows Fallback: Simple native system execution layout
    // NOTE: Windows requires external zip/tar tooling installations to handle pipeline tokens
    int allocation = asprintf(&command, "tar -caf \"%s\" \"%s\"", context->destination, context->target);
#else
    const int allocation = asprintf(&command,
        "tar cf - \"%s\" 2>/dev/null "
        "| pv -s $(du -sk \"%s\" | awk '{print $1 * 1024}') "
        "| xz -%hu >\"%s\"",
        context->target, context->target, (unsigned short) context->compression, context->destination);
#endif

    if (allocation < 0) {
        eprintf("Failed to initialize command string configuration context.\n");
        return -1;
    }

    int result = 0;
    if (!context->dry) {
        if (executeCommand(command) < 0) {
            eprintf("Failed to execute data pipeline operation.\n");
            result = -1;
        } else if (!path_exists(context->destination)) {
            eprintf("Failed to complete archiving: Target destination file missing.\n");
            result = -1;
        }
    } else {
        ifprintf("[Dry] Executing command: %s\n", command);
    }

    free(command);
    return result;
}

int cleanup(const Context* context) {
    if (context->delete) {
        if (!context->dry) {
            return rmtree(context->target);
        }

        ifprintf("[Dry] Deleting original target.\n");
    }

    return 0;
}
