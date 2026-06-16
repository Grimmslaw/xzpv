#ifndef XZ_PROG_CONFIG_H
#define XZ_PROG_CONFIG_H

#include "context.h"

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #define PATH_SEPARATOR '\\'
#else
    #define PLATFORM_UNIX 1
    #define PATH_SEPARATOR '/'
#endif

#define CONFIG_TOSTRING_INVOCATION "Config{configPath=%s,installDir=%s}\n", \
    config->configPath, config->installDir

/* -- Model -- */
typedef struct {
    char* configPath;
    char* installDir;
} Config;

Config* config_new();
void config_print(const Config* config, Printer printer);
void config_destroy(Config* config);

/* -- Utility -- */
int isElevatedContext(void);
void getConfigPath(char* configPath, size_t maxLength, const char* argv0);
int createDirRecursive(const char* path);
int handleInstallation(const char* argv0);
int handleUninstallation(const char* argv0);

#endif //XZ_PROG_CONFIG_H