#include "cat/utility/cat_test.h"
#include <stdio.h>

#define MAX_TESTS 128

/* Internal test registry storage */
static cat_test g_tests[MAX_TESTS];
static int32_t g_test_count = 0;

static int32_t g_tests_passed = 0;
static int32_t g_tests_failed = 0;


/* cat_register_test() Add a test to the registry */
void cat_register_test(const char* name, cat_test_fn fn)
{
    if (g_test_count >= MAX_TESTS)
    {
        cstr_t str = "cat_test: ERROR: Maximum number of tests (%d) exceeded!\n" + MAX_TESTS;
        cat_console_debug_print(str);
        return;
    }

    g_tests[g_test_count].name = name;
    g_tests[g_test_count].fn = fn;
    g_test_count++;
}

/* cat_run_all_tests() Executes all registered tests and prints results */
void cat_run_all_tests(void)
{
    //char buffer[256];

    cat_console_debug_print("Running tests...\n");

    g_tests_passed = 0;
    g_tests_failed = 0;

    for (int i = 0; i < g_test_count; i++)
    {
        const cat_test* t = &g_tests[i];
        cat_test_fn fn = t->fn;
        int result = fn();

        if (result == 0)
        {
            cat_console_debug_print("[PASS] ");
            //cat_console_debug_print(t->name);
            cat_console_debug_print("\n");
            g_tests_passed++;
        }
        else
        {
            cat_console_debug_print("[FAIL] ");
            cat_console_debug_print(t->name);
            cat_console_debug_print(" (code: ");

            // Convert result to string manually
            char codebuf[12];
            snprintf(codebuf, sizeof(codebuf), "%d", result);
            cat_console_debug_print(codebuf);
            cat_console_debug_print(")\n");

            g_tests_failed++;
        }
    }
}


/* cat_print_summary() Prints a final pass/fail report*/
void cat_print_summary(void)
{
    cat_console_debug_print("\n========== Test Summary ==========\n");
    //cat_console_debug_print("Total tests : %d\n" + g_test_count);
    //cat_console_debug_print("Passed      : %d\n" +  g_tests_passed);
    //cat_console_debug_print("Failed      : %d\n" + g_tests_failed);

    if (g_tests_failed > 0)
        cat_console_debug_print("Some tests failed.\n");
    else
        cat_console_debug_print("All tests passed!\n");

    cat_console_debug_print("==================================\n");
}