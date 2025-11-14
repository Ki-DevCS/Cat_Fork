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

/*
* cat.c
* Main test file.
*/

#include "cat/cat.h"


extern int cat_time_test(void);
extern int cat_console_test(void);
extern int cat_memory_test(void);
extern int cat_thread_test(void);

//Aeris Test
extern int AerisTasks_Test(void);

cat_noinl int cat_test_all(int const argc, cstr_t const* const argv[])
{
    unused2(argc, argv);
    cat_register_test("Time Test",        cat_time_test);
    cat_register_test("Console Test",     cat_console_test);
    cat_register_test("Memory Test",      cat_memory_test);
    cat_register_test("Aeris Tasks Test", AerisTasks_Test);
    cat_register_test("Thread Test",      cat_thread_test);

    cat_run_all_tests();
    cat_print_summary();

    return 0;
}
