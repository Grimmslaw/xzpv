#define _GNU_SOURCE

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xzpv/files.h"
#include "xzpv/output.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
    #include <direct.h>
    #include <windows.h>

    #define F_OK 0
    #define access _access
    #define PATH_MAX MAX_PATH
#else
    #include <limits.h>
    #include <unistd.h>
    #include <ftw.h>
#endif

char* resolve_cwd() {
    char* buffer = malloc(INITIAL_PATH_SIZE);
    if (buffer == NULL) {
        eprintf("Failed to allocate memory for cwd.\n");
        return NULL;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (_getcwd(buffer, INITIAL_PATH_SIZE) == NULL) {
#else
    if (getcwd(buffer, INITIAL_PATH_SIZE) == NULL) {
#endif
        eprintf("Failed to get cwd.\n");
        free(buffer);
        return NULL;
    }
    return buffer;
}

char* resolve_relative(const char* pathname, const char* basepath, size_t max_size) {
    char* combined_path = malloc(max_size);
    if (combined_path == NULL) {
        eprintf("Failed to allocate memory for combined pathname.\n");
        return NULL;
    }

    char* resolved_path = malloc(max_size);
    if (resolved_path == NULL) {
        eprintf("Failed to allocate memory for relative pathname.\n");
        free(combined_path);
        return NULL;
    }

#if defined(_WIN32) || defined(_WIN64)
    snprintf(combined_path, max_size, "%s\\%s", basepath, pathname);
    if (_fullpath(resolved_path, combined_path, max_size) == NULL) {
        strncpy(resolved_path, combined_path, max_size - 1);
        resolved_path[max_size - 1] = '\0';
    }
#else
    snprintf(combined_path, max_size, "%s/%s", basepath, pathname);
    char* sysResolved = realpath(combined_path, NULL);
    if (sysResolved != NULL) {
        snprintf(resolved_path, max_size, "%s", sysResolved);
        free(sysResolved);
    } else {
        snprintf(resolved_path, max_size, "%s", combined_path);
    }
#endif

    free(combined_path);
    return resolved_path;
}

int path_exists(const char* pathname) {
    return access(pathname, F_OK) == 0;
}

#if !defined(_WIN32) && !defined(_WIN64)
static int rm_fn(const char* pathname, [[maybe_unused]] const struct stat* buffer, [[maybe_unused]] int type, [[maybe_unused]] struct FTW* ftwb) {
    if (remove(pathname) < 0) {
        eprintf("Failed to delete %s.\n", pathname);
        return -1;
    }
    return 0;
}
#endif

int rmtree(const char* pathname) {
#if defined(_WIN32) || defined(_WIN64)
    SHFILEOPSTRUCTA fileOp = {
        .hwnd = NULL,
        .wFunc = FO_DELETE,
        .pFrom = pathname,
        .pTo = NULL,
        .fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
        .fAnyOperationsAborted = NULL,
        .hNameMappings = NULL,
        .lpszProgressTitle = NULL
    };
    char windowsPath[PATH_MAX + 2] = {0};
    snprintf(windowsPath, PATH_MAX, "%s", pathname);
    fileOp.pFrom = windowsPath;

    if (SHFileOperationA(&fileOp) != 0) {
        eprintf("Failed to complete Windows file deletion tree.\n");
        return -1;
    }
    return 0;
#else
    if (nftw(pathname, rm_fn, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0) {
        eprintf("Failed to complete file traversal.\n");
        return -1;
    }
    return 0;
#endif
}
