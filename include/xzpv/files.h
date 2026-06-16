/**
 * @file files.h
 * @brief Operations specific to files and file systems.
 * @author Wes Rickey
 * @date 2026-06-08
 */

#ifndef XZ_PROG_FILES_H
#define XZ_PROG_FILES_H

#include <stddef.h>

#define INITIAL_PATH_SIZE 1024

/**
 * @brief   Initializes a string on the heap representing the full path of the current working directory (that the
 *          command is running within).
 *
 * @return  The current working directory as a string, or `NULL` if an error occurs allocating memory or resolving the
 *          directory name.
 */
char* resolve_cwd(void);

/**
 * @brief   Initializes a string on the heap representing the combination of the given relative `pathname` with the
 *          given `basepath`, resolving any special path portions (e.g., `.`, `..`, `~`, etc.).
 * @param pathname  The path to be appended to the end of `basepath` after resolving any special path portions.
 * @param basepath  The path onto which `pathname` will be appended to.
 * @param max_size  The maximum size of the string to allocate on the heap for the final path.
 * @return The full combined path, or `NULL` if any error occurs in this function's execution.
 */
char* resolve_relative(const char* pathname, const char* basepath, size_t max_size);

/**
 * @brief Determines whether the given `pathname` represents a file or directory that actually exists on the filesystem.
 *
 * @param pathname The full path to check.
 * @return `1` if the given path represents a file or directory that actually exists on the filesystem, `0` otherwise.
 */
int path_exists(const char* pathname);

/**
 * @brief Traverses the tree represented by the given path, removing all files and directories, recursively.
 *
 * This function uses `nftw` to traverse the files, and, importantly uses the following flags:
 *  - FTW_DEPTH: Files are visited with a bottom-up approach (i.e., subdirectories are visited first), meaning a
 *      directory's contents are deleted first, followed by the (now-empty) directory, itself.
 *  - FTW_MOUNT: The traversal does not cross a mount point/stays on the same filesystem.
 *  - FTW_PHYS: The traversal does not follow symlinks, which means that orphaned links may be left.
 *
 * @param pathname The full path to the file or directory which should be deleted.
 * @return `0` if the removal succeeded, otherwise a negative integer.
 */
int rmtree(const char* pathname);

#endif //XZ_PROG_FILES_H