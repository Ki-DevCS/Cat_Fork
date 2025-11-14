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
* cat_time.c
* Time implementation.
*/

#include "cat/utility/cat_time.h"
#include "cat/cat_platform.inl"


#define CAT_PLATFORM_TIME_USE_WIN


#if (defined _WIN32 && defined CAT_PLATFORM_TIME_USE_WIN)
#define CAT_PLATFORM_TIME_WIN 1
#endif // #if (defined _WIN32 && defined CAT_PLATFORM_TIME_USE_WIN)

#ifdef CAT_PLATFORM_TIME_WIN
#include <Windows.h>
#else // #ifdef CAT_PLATFORM_TIME_WIN
#include <time.h>
#define NS_PER_S 1000000000
#endif // #else // #ifdef CAT_PLATFORM_TIME_WIN


cat_implementation_begin;


cat_impl cat_time_rate_t cat_platform_time_rate(void)
{
#ifdef CAT_PLATFORM_TIME_WIN
    LARGE_INTEGER pf = { 0 };
    if (!QueryPerformanceFrequency(&pf))
        return 0;
    return pf.LowPart;
#else // #ifdef CAT_PLATFORM_TIME_WIN
    return NS_PER_S;
#endif // #else // #ifdef CAT_PLATFORM_TIME_WIN
}

cat_impl cat_time_t cat_platform_time(void)
{
#ifdef CAT_PLATFORM_TIME_WIN
    LARGE_INTEGER pc = { 0 };
    if (!QueryPerformanceCounter(&pc))
        return 0;
    return pc.QuadPart;
#else // #ifdef CAT_PLATFORM_TIME_WIN
    struct timespec ts = { 0 };
    if (timespec_get(&ts, TIME_UTC) != TIME_UTC)
        return 0;
    return (ts.tv_sec * NS_PER_S + ts.tv_nsec);
#endif // #else // #ifdef CAT_PLATFORM_TIME_WIN
}

cat_impl void cat_platform_sleep(cat_time_t const duration)
{
    cat_time_t const t = cat_platform_time() + duration;
    while (cat_platform_time() < t);
}


#include "cat/utility/cat_console.h"


cat_noinl int cat_time_test(void)
{
    cat_time_rate_t const volatile t_rate = cat_platform_time_rate();
    cat_time_t const volatile t0 = cat_platform_time();
    cat_time_t volatile dt = 0;

    cat_platform_sleep(t_rate);
    {
        dt = cat_platform_time() - t0;
        printf("\nTime: \n    platform rate=%"PRIu32" t0=%"PRIi64" dt=%"PRIi64, t_rate, t0, dt);
    }
    cat_platform_sleep(t_rate);
    return 0;
}


cat_implementation_end;