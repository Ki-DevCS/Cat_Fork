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
* cat_console.c
* Console implementation.
*/

#include "cat/utility/cat_console.h"
#include "cat/cat_platform.inl"
#include "cat/utility/cat_test.h"


#ifdef _WIN32
#include <io.h>
#include <Windows.h>


cat_implementation_begin;


typedef struct cat_console_s
{
    ptr_t   handle[4];
    int32_t io[4];
} cat_console_t;
static cat_console_t cat_console_main;


static void cat_console_internal_redirect(cat_console_t* const console, bool const in, bool const out, bool const err)
{
    FILE* str = NULL;
    int32_t i = -1, j = -1;

    // redirect input
    i = 0;
    if (in)
    {
        if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE && !console->handle[i])
        {
            // flush buffer, duplicate handle and reopen stream to console
            //j = fprintf(stdin, "\n STDIN =/= DEFAULT \n");
            j = fflush(stdin);
            j = _dup(i);
            str = freopen("CONIN$", "r+", stdin);
            if (str)
            {
                // store values and configure
                console->handle[i] = str;
                console->io[i] = j;
                j = setvbuf(stdin, NULL, _IONBF, 0);
                //j = fprintf(stdin, "\n STDIN == CONSOLE \n");
            }
        }
    }
    else
    {
        if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE && console->handle[i])
        {
            // flush and reopen
            //j = fprintf(stdin, "\n STDIN =/= CONSOLE \n");
            j = fflush(stdin);
            str = freopen("NUL:", "r+", stdin);
            if (str)
            {
                // duplicate handle and reconfigure stream, reset variables
                j = _dup2(console->io[i], i);
                j = setvbuf(stdin, NULL, _IONBF, 0);
                //j = fprintf(stdin, "\n STDIN == DEFAULT \n");
                console->handle[i] = 0;
                console->io[i] = -1;
            }
        }
    }

    // redirect output
    i = 1;
    if (out)
    {
        if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE && !console->handle[i])
        {
            // flush buffer, duplicate handle and reopen stream to console
            //j = fprintf(stdout, "\n STDOUT =/= DEFAULT \n");
            j = fflush(stdout);
            j = _dup(i);
            str = freopen("CONOUT$", "a+", stdout);
            if (str)
            {
                // store values and configure
                console->handle[i] = str;
                console->io[i] = j;
                j = setvbuf(stdout, NULL, _IONBF, 0);
                //j = fprintf(stdout, "\n STDOUT == CONSOLE \n");
            }
        }
    }
    else
    {
        if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE && console->handle[i])
        {
            // flush and reopen
            //j = fprintf(stdout, "\n STDOUT =/= CONSOLE \n");
            j = fflush(stdout);
            str = freopen("NUL:", "a+", stdout);
            if (str)
            {
                // duplicate handle and reconfigure stream, reset variables
                j = _dup2(console->io[i], i);
                j = setvbuf(stdout, NULL, _IONBF, 0);
                //j = fprintf(stdout, "\n STDOUT == DEFAULT \n");
                console->handle[i] = 0;
                console->io[i] = -1;
            }
        }
    }

    // redirect error
    i = 2;
    if (err)
    {
        if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE && !console->handle[i])
        {
            // flush buffer, duplicate handle and reopen stream to console
            //j = fprintf(stderr, "\n STDERR =/= DEFAULT \n");
            j = fflush(stderr);
            j = _dup(i);
            str = freopen("CONOUT$", "a+", stderr);
            if (str)
            {
                // store values and configure
                console->handle[i] = str;
                console->io[i] = j;
                j = setvbuf(stderr, NULL, _IONBF, 0);
                //j = fprintf(stderr, "\n STDERR == CONSOLE \n");
            }
        }
    }
    else
    {
        if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE && console->handle[i])
        {
            // flush and reopen
            //j = fprintf(stderr, "\n STDERR =/= CONSOLE \n");
            j = fflush(stderr);
            str = freopen("NUL:", "a+", stderr);
            if (str)
            {
                // duplicate handle and reconfigure stream, reset variables
                j = _dup2(console->io[i], i);
                j = setvbuf(stderr, NULL, _IONBF, 0);
                //j = fprintf(stderr, "\n STDERR == DEFAULT \n");
                console->handle[i] = 0;
                console->io[i] = -1;
            }
        }
    }
}


cat_impl bool cat_console_create(void)
{
    // if console not already open
    HWND handle = GetConsoleWindow();
    bool create = !handle && !cat_console_main.handle[3];
    if (!create)
        return false;

    // allocate and show console
    create = AllocConsole();
    if (!create)
        return false;

    // reset flags
    cat_console_main.handle[0] = cat_console_main.handle[1] = cat_console_main.handle[2] = NULL;
    cat_console_main.io[0] = cat_console_main.io[1] = cat_console_main.io[2] = -1;

    // init flag
    cat_console_main.handle[3] = handle = GetConsoleWindow();

    // disable closing console manually because doing this kills 
    //	the whole application; could also start a new process, 
    //	but then there's also that to manage
    DeleteMenu(GetSystemMenu(handle, FALSE), SC_CLOSE, MF_BYCOMMAND);

    // redirect to new console (in/out, not err)
    cat_console_internal_redirect(&cat_console_main, 1, 1, 0);

    // done
    return true;
}

cat_impl bool cat_console_destroy(void)
{
    HWND const handle = GetConsoleWindow();
    bool release = (cat_console_main.handle[3] == handle) && handle;
    if (!release)
        return false;

    // reset to original standard i/o
    cat_console_internal_redirect(&cat_console_main, 0, 0, 0);

    // delete console instance
    // console will hide when all standard handles are closed
    release = FreeConsole();
    if (!release)
        return false;

    // reset
    cat_console_main.handle[3] = 0;
    return true;
}

cat_impl bool cat_console_config(bool const in, bool const out, bool const err)
{
    HWND const handle = GetConsoleWindow();
    bool const redirect = (cat_console_main.handle[3] == handle) && handle;
    if (!redirect)
        return false;

    // redirect toggle
    cat_console_internal_redirect(&cat_console_main, in, out, err);
    return true;
}

cat_impl bool cat_console_toggle_cursor(bool const visible)
{
    CONSOLE_CURSOR_INFO cursorInfo = { 0 };
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    bool completed = stdHandle && console &&
        GetConsoleCursorInfo(stdHandle, &cursorInfo);
    if (!completed)
        return false;

    cursorInfo.bVisible = visible;
    completed = SetConsoleCursorInfo(stdHandle, &cursorInfo);
    return completed;
}

cat_impl bool cat_console_get_pos(int16_t* const p_x_out, int16_t* const p_y_out)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo = { 0 };
    HANDLE stdHandle = NULL;
    HWND console = NULL;
    bool completed = false;
    assert_or_bail(p_x_out) false;
    assert_or_bail(p_y_out) false;

    stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    console = GetConsoleWindow();
    completed = stdHandle && console &&
        GetConsoleScreenBufferInfo(stdHandle, &screenBufferInfo);
    if (!completed)
        return false;

    *p_x_out = screenBufferInfo.dwCursorPosition.X;
    *p_y_out = screenBufferInfo.dwCursorPosition.Y;
    return true;
}

cat_impl bool cat_console_set_pos(int16_t const x, int16_t const y)
{
    COORD const pos = { x, y };
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    bool const completed = stdHandle && console &&
        SetConsoleCursorPosition(stdHandle, pos);
    return completed;
}

cat_impl bool cat_console_get_color(cat_console_color_t* const p_fg_out, cat_console_color_t* const p_bg_out)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo = { 0 };
    HANDLE stdHandle = NULL;
    HWND console = NULL;
    bool completed = false;
    assert_or_bail(p_fg_out) false;
    assert_or_bail(p_bg_out) false;

    stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    console = GetConsoleWindow();
    completed = stdHandle && console &&
        GetConsoleScreenBufferInfo(stdHandle, &screenBufferInfo);
    if (!completed)
        return false;

    *p_fg_out = (cat_console_color_t)(screenBufferInfo.wAttributes & 0xf);
    *p_bg_out = (cat_console_color_t)(screenBufferInfo.wAttributes >> 4 & 0xf);
    return true;
}

cat_impl bool cat_console_set_color(cat_console_color_t const fg, cat_console_color_t const bg)
{
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    bool const completed = stdHandle && console &&
        SetConsoleTextAttribute(stdHandle, (int16_t)(fg | bg << 4));
    return completed;
}

cat_impl bool cat_console_reset_color(void)
{
    return cat_console_set_color(cat_console_white, cat_console_black);
}

cat_impl bool cat_console_get_pos_color(int16_t* const p_x_out, int16_t* const p_y_out, cat_console_color_t* const p_fg_out, cat_console_color_t* const p_bg_out)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo = { 0 };
    HANDLE stdHandle = NULL;
    HWND console = NULL;
    bool completed = false;
    assert_or_bail(p_x_out) false;
    assert_or_bail(p_y_out) false;
    assert_or_bail(p_fg_out) false;
    assert_or_bail(p_bg_out) false;

    stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    console = GetConsoleWindow();
    completed = stdHandle && console &&
        GetConsoleScreenBufferInfo(stdHandle, &screenBufferInfo);
    if (!completed)
        return false;

    *p_x_out = screenBufferInfo.dwCursorPosition.X;
    *p_y_out = screenBufferInfo.dwCursorPosition.Y;
    *p_fg_out = (cat_console_color_t)(screenBufferInfo.wAttributes & 0xf);
    *p_bg_out = (cat_console_color_t)(screenBufferInfo.wAttributes >> 4 & 0xf);
    return true;
}

cat_impl bool cat_console_set_pos_color(int16_t const x, int16_t const y, cat_console_color_t const fg, cat_console_color_t const bg)
{
    COORD const pos = { x, y };
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    bool const completed = stdHandle && console &&
        SetConsoleCursorPosition(stdHandle, pos) &&
        SetConsoleTextAttribute(stdHandle, (int16_t)(fg | bg << 4));
    return completed;
}

cat_impl bool cat_console_get_size(int16_t* const p_w_out, int16_t* const p_h_out)
{
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo = { 0 };
    HANDLE stdHandle = NULL;
    HWND console = NULL;
    bool completed = false;
    assert_or_bail(p_w_out) false;
    assert_or_bail(p_h_out) false;

    stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    console = GetConsoleWindow();
    completed = stdHandle && console &&
        GetConsoleScreenBufferInfo(stdHandle, &screenBufferInfo);
    if (!completed)
        return false;

    *p_w_out = screenBufferInfo.dwSize.X;
    *p_h_out = screenBufferInfo.dwSize.Y;
    return true;
}

cat_impl bool cat_console_set_size(int16_t const w, int16_t const h)
{
    COORD const sz = { w, h };
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    bool const completed = stdHandle && console &&
        SetConsoleScreenBufferSize(stdHandle, sz);
    return completed;
}

cat_impl bool cat_console_clear(void)
{
    // simple clear
    //system("cls");
    // help to avoid using system("cls"): https://docs.microsoft.com/en-us/windows/console/clearing-the-screen 
    CONSOLE_SCREEN_BUFFER_INFO buffer = { 0 };
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    COORD const coord = { 0, 0 };
    DWORD sz = 0, write = 0;
    bool completed = stdHandle && console &&
        GetConsoleScreenBufferInfo(stdHandle, &buffer);
    if (!completed)
        return false;

    sz = (buffer.dwSize.X * buffer.dwSize.Y);
    completed = FillConsoleOutputCharacterA(stdHandle, ' ', sz, coord, &write) &&
        GetConsoleScreenBufferInfo(stdHandle, &buffer) &&
        FillConsoleOutputAttribute(stdHandle, buffer.wAttributes, sz, coord, &write) &&
        SetConsoleCursorPosition(stdHandle, coord);
    return completed;
}

cat_impl bool cat_console_draw_test_patch(void)
{
    int16_t x = 0, y = 0, w = 0, h = 0;
    cat_console_color_t fg = 0, bg = 0;
    HANDLE const stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND const console = GetConsoleWindow();
    bool const exists = stdHandle && console;
    if (!exists)
        return false;

    // test all colors and shifts
    for (y = 0; y < 16; ++y)
    {
        for (x = 0; x < 16; ++x)
        {
            fg = (cat_console_color_t)y;
            bg = (cat_console_color_t)x;
            cat_console_set_color(fg, bg);
            cat_console_set_pos(x * 2, y);
            printf("%"PRIx32, (int32_t)x);
            cat_console_set_pos_color(x * 2 + 1, y, fg, bg);
            printf("%"PRIx32, (int32_t)y);
        }
    }
    cat_console_get_pos(&x, &y);
    cat_console_get_color(&fg, &bg);
    cat_console_get_pos_color(&x, &y, &fg, &bg);
    cat_console_get_size(&w, &h);
    cat_console_reset_color();
    printf("XY=(%"PRIi32", %"PRIi32") WH=(%"PRIi32", %"PRIi32") \n", (int32_t)x, (int32_t)y, (int32_t)w, (int32_t)h);
    return true;
}

cat_impl int cat_console_debug_print(cstr_t const format, ...)
{
    char str[256] = { 0 };
    int result = 0;
    va_list args = NULL;

    // fill buffer with formatted arguments
    va_start(args, format);
    result = _vsnprintf(str, sizeof(str), format, args);
    va_end(args);

    // internal print
    OutputDebugStringA(str);
    return result;
}


cat_implementation_end;


#else // #ifdef _WIN32
#error Console not implemented for platform.
#endif // #else // #ifdef _WIN32


#include "cat/utility/cat_time.h"


cat_implementation_begin;


cat_noinl int cat_console_test(void)
{
    cat_console_clear();
    CAT_ASSERT(cat_console_draw_test_patch());
    cat_platform_sleep(cat_platform_time_rate());
    return 0;
}


cat_implementation_end;