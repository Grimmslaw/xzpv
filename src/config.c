#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xzpv/config.h"
#include "xzpv/output.h"

#if PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
    #include <shlobj.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <pwd.h>
    #include <limits.h>
#endif

Config* config_new() {
    Config* config = calloc(1, sizeof(Config));
    if (config == NULL) {
        eprintf("Failed to initialize config object.\n");
        return NULL;
    }

    config->configPath = malloc(PATH_MAX);
    config->installDir = malloc(PATH_MAX);
    if (config->configPath == NULL || config->installDir == NULL) {
        eprintf("Failed to allocate memory for config string buffers.\n");
        config_destroy(config);
        return NULL;
    }

    config->configPath[0] = '\0';
    config->installDir[0] = '\0';

    return config;
}

void config_print(const Config* config, const Printer printer) {
    printer(CONFIG_TOSTRING_INVOCATION);
}

void config_destroy(Config* config) {
    if (config == NULL) return;
    free(config->configPath);
    free(config->installDir);
    free(config);
}

static int getExeDirectory(char* directory, size_t maxLength, const char* argv0) {
#if PLATFORM_WINDOWS
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
        char* lastSlash = strrchr(exePath, PATH_SEPARATOR);
        if (lastSlash) {
            *lastSlash = '\0';
            snprintf(directory, maxLength, "%s", exePath);
            return 1;
        }
    }
#elif defined(__linux__)
    char exePath[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (length != -1) {
        exePath[length] = '\0';
        char* lastSlash = strrchr(exePath, PATH_SEPARATOR);
        if (lastSlash) {
            *lastSlash = '\0';
            snprintf(directory, maxLength, "%s", exePath);
            return 1;
        }
    }
#else   // maxOS / BSD / Generic Unix Fallback using argv[0]
    if (argv0 && argv0[0] == PATH_SEPARATOR) {  // i.e., it's an absolute path
        char temp[PATH_MAX];
        snprintf(temp, sizeof(temp), "%s", argv0);
        char* lastSlash = strrchr(temp, PATH_SEPARATOR);
        if (lastSlash) {
            *lastSlash = '\0';
            snprintf(directory, maxLength, "%s", temp);
            return 1;
        }
    }
#endif
    return 0;
}

int isElevatedContext(void) {
#if PLATFORM_WINDOWS
    BOOL isAdmin = FALSE;
    PSID adminGroupSid = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroupSid)) {
        CheckTokenMembership(NULL, adminGroupSid, &isAdmin);
        FreeSid(adminGroupSid);
    }
    return isAdmin;
#else
    return geteuid() == 0;
#endif
}

void getConfigPath(char* configPath, size_t maxLength, const char* argv0) {
    char exeDir[4096] = {0};

    // 1. Portable Mode
    if (getExeDirectory(exeDir, sizeof(exeDir), argv0)) {
        snprintf(configPath, maxLength, "%s%cxzpv.conf", exeDir, PATH_SEPARATOR);
        // check if it actually exists, locally
        FILE* f = fopen(configPath, "r");
        if (f) {
            fclose(f);
            return;
        }
    }

    // 2. Standard OS Mode
#if PLATFORM_WINDOWS
    char basePath[MAX_PATH];
    if (isElevatedContext()) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, basePath))) {
            snprintf(configPath, maxLength, "%s\\xzpv\\config.ini", basePath);
            return;
        }
    } else {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, basePath))) {
            snprintf(configPath, maxLength, "%s\\xzpv\\config.ini", basePath);
            return;
        }
    }
#else
    if (isElevatedContext()) {
        snprintf(configPath, maxLength, "/etc/xzpv/xzpv.conf");
        return;
    }

    const char* xdgConfig = getenv("XDG_CONFIG_HOME");
    if (xdgConfig && strlen(xdgConfig) > 0) {
        snprintf(configPath, maxLength, "%s/xzpv/xzpv.conf", xdgConfig);
        return;
    }
    const char* home = getenv("HOME");
    if (!home) {
        const struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    if (home) {
        snprintf(configPath, maxLength, "%s/.config/xzpv/xzpv.conf", home);
        return;
    }
#endif
    // 3. Critical fallback
    snprintf(configPath, maxLength, ".%cxzpv.conf", PATH_SEPARATOR);
}

int createDirRecursive(const char* path) {
    char initial[4096];
    char* buffer = NULL;

    snprintf(initial, sizeof(initial), "%s", path);
    const size_t length = strlen(initial);
    if (length == 0) return 0;

    if (initial[length - 1] == PATH_SEPARATOR) {
        initial[length - 1] = '\0';
    }

    for (buffer = initial + 1; *buffer; buffer++) {
        if (*buffer == PATH_SEPARATOR) {
            *buffer = '\0';
#if PLATFORM_WINDOWS
            _mkdir(initial);
#else
            mkdir(initial, 0755);
#endif
            *buffer = PATH_SEPARATOR;
        }
    }

#if PLATFORM_WINDOWS
    return _mkdir(initial) == 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(initial, 0755) == 0 || errno == EEXIST;
#endif
}

static void getParentDirectory(const char* filePath, char* parentDirectory, size_t maxLength) {
    snprintf(parentDirectory, maxLength, "%s", filePath);
    char* lastSlash = strrchr(parentDirectory, PATH_SEPARATOR);
    if (lastSlash) {
        *lastSlash = '\0';
    }
}

int handleInstallation(const char* argv0) {
    char configPath[4096] = {0};
    char configDir[4096] = {0};

    getConfigPath(configPath, sizeof(configPath), argv0);
    getParentDirectory(configPath, configDir, sizeof(configDir));
    ifprintf("Installing configuration to: %s\n", configPath);

    // check if configuration already exists to prevent overwriting user data
    FILE* check = fopen(configPath, "r");
    if (check) {
        fclose(check);
        ifprintf("Configuration file already exists. Skipping deployment to preserve settings.\n");
        return 1;
    }

    // ensure target directory structure exists
    if (!createDirRecursive(configDir)) {
        eprintf("Error: Failed to create configuration directory: %s\n", configDir);
        return 0;
    }

    // write default template configuration
    FILE* configFile = fopen(configPath, "w");
    if (!configFile) {
        eprintf("Error: Cannot write to configuration. (Are you root/admin?)\n");
        return 0;
    }

    fprintf(configFile, "# xzpv Configuration File\n");
    fprintf(configFile, "# Generated automatically during installation\n\n");
    fprintf(configFile, "[general]\n");
    fprintf(configFile, "todo = true\n");

    fclose(configFile);
    ifprintf("Successfully installed default configuration template.\n");
    return 1;
}

int handleUninstallation(const char* argv0) {
    char configPath[512];
    getConfigPath(configPath, sizeof(configPath), argv0);

    ifprintf("Uninstalling configuration from: %s\n", configPath);

    // check if file exists
    FILE* configFile = fopen(configPath, "r");
    if (!configFile) {
        ifprintf("No configuration file found at target path. Nothing to clean up.\n");
        return 1;
    }
    fclose(configFile);

    // delete the configuration file
    if (remove(configPath) != 0) {
        eprintf("Error: Failed to delete configuration file. Check system permissions.\n");
        return 0;
    }

    ifprintf("Successfully removed configuration data.\n");
    return 1;
}
