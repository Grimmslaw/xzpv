cmake_minimum_required(VERSION 3.15)
project(xzpv VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 1. Project Layout Setup
# gather source files
set(SOURCES 
        src/xzpv.c
        src/context.c
        src/operations.c
        src/files.c
        src/output.c
        src/config.c)
# define final executable
add_executable(xzpv ${SOURCES})
target_include_directories(xzpv PUBLIC include)

# 2. Windows Specific API Linking
if (WIN32)
    target_link_libraries(xzpv PRIVATE shell32)
endif()

# package manager installation rules
include(GNUInstallDirs)

# install binary to standard system binary dir
install(TARGETS xzpv
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
