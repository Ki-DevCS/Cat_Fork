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


extern void cat_time_test(void);
extern void cat_console_test(void);
extern void cat_memory_test(void);
extern void cat_thread_test(void);

//Aeris Test
extern void AerisTasks_Test(void);

cat_noinl int cat_test_all(int const argc, char const* const argv[])
{
    unused2(argc, argv);
    cat_time_test();
    cat_console_test();
    cat_memory_test();

	//Aeris Test
    AerisTasks_Test();

    cat_thread_test();
    return 0;
}
