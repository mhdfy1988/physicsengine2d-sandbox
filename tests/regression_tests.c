#include <stdio.h>
#include "../include/physics.h"
#include "regression_tests.h"

typedef int (*RegressionTestFn)(void);

typedef struct {
    const char* name;
    RegressionTestFn fn;
} RegressionTestEntry;

int main(void) {
    int passed = 0;
    int total = 0;
    static const RegressionTestEntry kTests[] = {
#define REGRESSION_TEST_ENTRY(fn) { #fn, fn },
        REGRESSION_TEST_LIST(REGRESSION_TEST_ENTRY)
#undef REGRESSION_TEST_ENTRY
    };
    int i;
    int test_count = (int)(sizeof(kTests) / sizeof(kTests[0]));

    for (i = 0; i < test_count; i++) {
        total++;
        if (!kTests[i].fn()) {
            printf("\nResult: FAIL (%d/%d) at %s\n", passed, total, kTests[i].name);
            return 1;
        }
        passed++;
    }

    printf("\nResult: PASS (%d/%d)\n", passed, total);
    return 0;
}
