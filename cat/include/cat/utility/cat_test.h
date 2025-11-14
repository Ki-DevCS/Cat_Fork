////////////////////////////////////////////////////////////////////////////////
/// Copyright 2025 Daniel S. Buckstein
/// 
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
/// 
///     http://www.apache.org/licenses/LICENSE-2.0
/// 
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
////////////////////////////////////////////////////////////////////////////////


#ifndef _CAT_TEST_H
#define _CAT_TEST_H


#include "cat/cat_platform.h"
#include "cat/utility/cat_console.h"


cat_interface_begin;


typedef int(*cat_test_fn)(void);

typedef struct cat_test
{
    const char* name;
    cat_test_fn fn;
} cat_test;

/* Register a test. Call this before cat_run_all_tests(). */
cat_decl void cat_register_test(const char* name, cat_test_fn fn);

/* Run all registered tests and track pass/fail. */
cat_decl void cat_run_all_tests(void);

/* Print a pass/fail summary at the end. */
cat_decl void cat_print_summary(void);

// Assertion helpers, returns current line number.
#define CAT_ASSERT(cond) \
    do { if (!(cond)) return __LINE__; } while (0)

#define CAT_ASSERT_EQ(a, b) \
    do { if ((a) != (b)) return __LINE__; } while (0)

#define CAT_ASSERT_NEQ(a, b) \
    do { if ((a) == (b)) return __LINE__; } while (0)


cat_interface_end;


#endif // #ifndef _CAT_TEST_H_