/**
 * @file output.h
 * @brief Utilities and states related to outputting to file streams.
 * @author Wes Rickey
 * @date 2026-06-08
 */

#ifndef XZ_PROG_OUTPUT_H
#define XZ_PROG_OUTPUT_H

#include <stdio.h>

extern bool GLOBAL_VERBOSE;

/**
 * @brief Turns "verbose" mode on or off by updating the `GLOBAL_VERBOSE` variable.
 *
 * @param verbose Whether "verbose" mode should be on (`true`) or off (`false`)
 */
void setVerbose(bool verbose);

/**
 * @brief Writes formatted data to a specified output stream, only if "verbose" mode is currently turned on.
 *
 * @param file The file descriptor describing the stream to print the text to.
 * @param format The text to print, in C-style `printf` format.
 * @param ... A variable number of arguments for formatting the `format` text.
 * @return A positive integer representing the number of characters written to the stream, or a negative integer
 * representing an execution failure (output or encoding).
 */
int iffprintf(FILE* restrict file, const char* restrict format, ...);
/**
 * @brief Writes formatted data to Standard Error (stderr).
 *
 * @param format The text to print, in C-style `printf` format.
 * @param ... A variable number of arguments for formatting the `format` text.
 * @return A positive integer representing the number of characters written to the stream, or a negative integer
 * representing an execution failure (output or encoding).
 */
int eprintf(const char* restrict format, ...);
/**
 * @brief Writes formatted data to Standard Out (stdout), only if "verbose" mode is currently turned on.
 *
 * @param format The text to print, in C-style `printf` format.
 * @param ... A variable number of arguments for formatting the `format` text.
 * @return A positive integer representing the number of characters written to the stream, or a negative integer
 * representing an execution failure (output or encoding).
 */
int ifprintf(const char* restrict format, ...);

#endif //XZ_PROG_OUTPUT_H