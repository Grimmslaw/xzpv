/**
 * @file context.h
 * @brief Modeling and functionality for context established by CLI options.
 * @author Wes Rickey
 * @date 2026-06-08
 */

#ifndef XZ_PROG_CONTEXT_H
#define XZ_PROG_CONTEXT_H

#define INITIAL_FILENAME_LENGTH 1024
#define DEFAULT_COMPRESSION_LEVEL ((unsigned int) 6)
#define CONTEXT_TOSTRING_INVOCATION "Context{verbose=%d,delete=%d,compression=%hu,cwd=%s,destination=%s,target=%s}\n", \
    context->verbose, context->delete, context->compression, context->cwd, context->destination, context->target


/**
 * @brief Representation of a function that prints text in a `printf`-style manner.
 */
typedef int (*Printer)(const char* restrict, ...);

/**
 * @struct Context
 * @brief Represents the persistent context established by the CLI options.
 */
typedef struct {
    unsigned int verbose : 1;       /**< Whether to print extra debugging information. */
    unsigned int delete : 1;        /**< Whether to delete the target files/folders after creating the compressed file. */
    unsigned int dry : 1;           /**< Print everything that happens in execution but make no actual file/folder changes. */
    unsigned int compression : 6;   /**< The level of compression to apply to the files/folders (1-9). */
    char* cwd;                      /**< The current working directory the command is being called in. */
    char* destination;              /**< The path (including desired file name) of the final compressed file. */
    char* target;                   /**< The file or folder to compress. */
} Context;

/**
 * @brief Constructs a new, blank Context and initializes all relevant memory.
 *
 * @return A blank Context.
 */
Context* context_new();

/**
 * @brief Prints a textual representation of the data making up the given context, using the given print function.
 *
 * @param context The context to print.
 * @param printer The function to use for printing.
 */
void context_toString(const Context* context, Printer printer);

/**
 * @brief Sets the `cwd` property of the current running context to the current working directory the command is
 * running in.
 *
 * @param context The current running context.
 */
void context_setCwd(Context* context);

/**
 * @brief Destroys the given context, freeing the memory of all of its components (and, finally, of itself).
 *
 * @param context The current running context.
 */
void context_destroy(Context* context);

#endif //XZ_PROG_CONTEXT_H