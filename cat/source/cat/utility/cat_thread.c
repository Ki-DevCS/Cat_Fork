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
* cat_thread.c
* Thread management implementation.
*/


#include "cat/utility/cat_thread.h"
#include "cat/cat_platform.inl"

#include <threads.h>
#ifdef _WIN32
#include <Windows.h>
#else // #ifdef _WIN32
#endif // #else // #ifdef _WIN32


cat_implementation_begin;

#pragma region Aeris Task System - Definitions
typedef struct AerisTask
{
    void (*func)(void* data);
    void* data;
}
AerisTask;

#define AERIS_TASK_QUEUE_CAPACITY 64

typedef struct AerisTaskQueue
{
    AerisTask tasks[AERIS_TASK_QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;

    mtx_t lock;
    cnd_t signal;
}
AerisTaskQueue;

static AerisTaskQueue g_taskQueue;
static thrd_t g_workerThread;
static int g_taskSystemRunning = 0;
#pragma endregion


static int cat_thrd_internal_entry_point(cat_thread_params_t const* const p_thread_params)
{
    assert_or_bail(p_thread_params) 1;
    assert_or_bail(p_thread_params->func) 1;
    assert_or_bail((p_thread_params->argc == 0) || p_thread_params->argv) 1;
    return p_thread_params->func(p_thread_params->argc, p_thread_params->argv);
}


cat_impl int cat_thrd_create(thrd_t* const p_thread_out, cat_thread_params_t const* const p_thread_params)
{
    assert_or_bail(p_thread_out) thrd_error;
    assert_or_bail(p_thread_params) thrd_error;
    return thrd_create(p_thread_out, &cat_thrd_internal_entry_point, (void*)p_thread_params);
}

cat_impl bool cat_thread_rename(cstr_t const name)
{
    bool result = false;
    assert_or_bail(name) false;
#ifdef _WIN32
    // Thread rename: 
    // https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2015/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
    {
#pragma warning(push)
#pragma warning(disable: 4820 6320 6322)// suppress warnings
#pragma pack(push, 8) // 8-bit alignment
	    // name change structure: 
	    // declare a data structure that can be used internally
        struct tagTHREADNAME_INFO
        {
            DWORD  type;    // reserved (must be 0x1000)
            LPCSTR name;    // name string (provided by caller)
            DWORD  threadID;// thread ID (-1 for calling thread)
            DWORD  flags;   // reserved (must be zero)
        } const info = { 0x1000, name, (DWORD)(-1), 0 };
#pragma pack(pop)
        __try
        {
            // attempt name change
            DWORD const exception = 0x406D1388;
            RaiseException(exception, 0,
                (sizeof(info) / sizeof(ULONG_PTR)),
                (ULONG_PTR const*)(&info));
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            // unhandled exception
        }
        result = true;
#pragma warning(pop)
    }
#else // #ifdef _WIN32
    // Not supported.
#endif // #else // #ifdef _WIN32
    return result;
}


#include "cat/utility/cat_time.h"
#include "cat/utility/cat_console.h"


static int cat_thread_test_func(size_t const argc, void* const argv[])
{
    int result = 0;
    thrd_t const* p_thrd = NULL;
    cstr_t thrd_name = NULL;
    int print_count = 0;

    assert_or_bail((argc == 3) && argv && argv[0] && argv[1] && argv[2]) 1;
    p_thrd = (thrd_t const*)argv[0];
    thrd_name = (cstr_t)argv[1];
    print_count = *(int const*)argv[2];
    result |= !cat_thread_rename(thrd_name);
    printf("\nThread: \n    thrd_name=\"%s\" id=%"PRIu32, thrd_name, p_thrd->_Tid);
    while ((print_count > 0) != 0)
    {
        if ((print_count % 1000) == 0)
            printf("\n    print_count=%"PRIi32, print_count);
        --print_count;
    }
    return result;
}

static int thrd_test_func(void* const arg)
{
    unused(arg);
    printf("\n%s", __FUNCTION__);
    cat_platform_sleep(cat_platform_time_rate());
    return 0;
}

cat_noinl void cat_thread_test(void)
{
    thrd_t thrd = { 0 };
    int thrd_res = 0;
    int print_count = 10000;
    void* const args[] = {
        &thrd,       // thread object
        __FUNCTION__,// thread name
        &print_count,// print count
    };
    cat_thread_params_t const params = {
        &cat_thread_test_func, array_count(args), args
    };

    cat_console_clear();
    {
        thrd_res = thrd_create(&thrd, &thrd_test_func, NULL);
        assert_or_bail(thrd_res == thrd_success);
        thrd_join(thrd, &thrd_res);
    }
    {
        thrd_res = cat_thrd_create(&thrd, &params);
        assert_or_bail(thrd_res == thrd_success);
        thrd_join(thrd, &thrd_res);
    }
    cat_platform_sleep(cat_platform_time_rate());
}


cat_implementation_end;