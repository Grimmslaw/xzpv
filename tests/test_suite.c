#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <xzpv/context.h>
#include <xzpv/files.h>

// Test 1: Verify Context Object Lifecycle and Default States
void test_context_create(void) {
    Context* context = context_new();
    assert(context != NULL);

    // Assert modern C23 default flag states match specifications
    assert(context->verbose == 0);
    assert(context->delete == 0);
    assert(context->dry == 0);
    assert(context->compression == 6);

    // Check allocated pointer states
    assert(context->cwd != NULL);
    assert(context->destination != NULL);
    assert(context->target != NULL);

    context_destroy(context);
    printf("[PASS] test_context_creation\n");
}

// Test 2: Verify Relative Path Resolution Mechanisms
void test_path_resolution(void) {
    // Resolve standard paths
    char* resolved = resolve_relative("test.txt", "/tmp", PATH_MAX);
    assert(resolved != NULL);

    // Cross-platform check for path concatenation
#if defined(_WIN32) || defined(_WIN64)
    assert(strstr(resolved, "\\tmp\\test.txt") != NULL || strstr(resolved, "/tmp\\test.txt") != NULL);
#else
    assert(strcmp(resolved, "/tmp/test.txt") == 0);
#endif
    free(resolved);
    printf("[PASS] test_path_resolution\n");
}

int main(void) {
    printf("Running xzpv test execution engine...\n\n");

    test_context_create();
    test_path_resolution();

    printf("\nAll unit tests passed successfully.\n");
    return 0;
}
