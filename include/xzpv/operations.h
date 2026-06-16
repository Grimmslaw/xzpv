/**
 * @file operations.h
 * @brief Defines the essential operations of the command.
 * @author Wes Rickey
 * @date 2026-06-08
 */

#ifndef XZ_PROG_OPERATIONS_H
#define XZ_PROG_OPERATIONS_H

#include "context.h"

#if defined(_WIN32) || defined(_WIN64)
#define popen _popen
#define pclose _pclose
#endif

/**
 * @brief   Archives the target file or directory to the indicated destination (both specified by the given running
 *          context).
 *
 * This function executes the following:
 *  1. Tars the file or directory. [`tar cf`]
 *  2. Initializes a visual representation of the current progress of the archival process. [`pv -s` with `du -sk`
 *      piped into `awk '{print $1}'` as its argument]
 *  3. Begins archiving the `tar`red file or directory using XZ compression. [`xz`]
 *  4. Updates the visual representation of progress initialized in step #2 until the archiving is complete.
 *
 * @param context The current running context.
 * @return `0` if the archiving operation is successful, or a negative integer if an error occurs at any point.
 */
int archive(const Context* context);

/**
 * Deletes the original target file or directory, if `delete` is set in the context.
 *
 * @param context The current running context.
 * @return  `0` if the cleanup process was successful (or did not happen), or a negative integer if any error occurred
 *          in the process.
 */
int cleanup(const Context* context);

#endif //XZ_PROG_OPERATIONS_H