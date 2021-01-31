//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

// Code for invoking Doom

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <shellapi.h>

#else

#include <sys/wait.h>
#include <unistd.h>

#endif

#include <memory>
#include <vector>
#include <string>

#include "textscreen.h"
#include "../elib/elib.h"
#include "../elib/m_argv.h"
#include "../elib/qstring.h"
#include "execute.h"
#include "mode.h"
#include "m_misc.h"

using args_t = std::vector<qstring>;

struct execute_context_s
{
    args_t args;
};

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.

static char *TempFile(const char *s)
{
    const char *tempdir;

#ifdef _WIN32
    // Check the TEMP environment variable to find the location.

    tempdir = getenv("TEMP");

    if (tempdir == nullptr)
    {
        tempdir = ".";
    }
#else
    // In Unix, just use /tmp.

    tempdir = "/tmp";
#endif

    return M_StringJoin(tempdir, DIR_SEPARATOR_S, s, nullptr);
}

static int ArgumentNeedsEscape(const char *arg)
{
    for (const char *p = arg; *p != '\0'; ++p)
    {
        if (isspace(*p))
        {
            return 1;
        }
    }

    return 0;
}

// Arguments passed to the setup tool should be passed through to the
// game when launching a game.  Calling this adds all arguments from
// myargv to the output context.

void PassThroughArguments(execute_context_t *context)
{
    for (int i = 1; i < myargc; ++i)
    {
        if (ArgumentNeedsEscape(myargv[i]))
        {
            AddCmdLineParameter(context, "\"%s\"", myargv[i]);
        }
        else
        {
            AddCmdLineParameter(context, "%s", myargv[i]);
        }
    }
}

execute_context_t *NewExecuteContext(void)
{
    return new execute_context_s {};    
}

void AddCmdLineParameter(execute_context_t *context, const char *s, ...)
{
    va_list args;
    va_start(args, s);
    context->args.push_back(std::move(qstring().vprintf(s, args)));
    va_end(args);
}

#if defined(_WIN32)

boolean OpenFolder(const char *path)
{
    // "If the function succeeds, it returns a value greater than 32."
    return intptr_t(ShellExecute(nullptr, "open", path, nullptr, nullptr, SW_SHOWDEFAULT)) > 32;
}

// Wait for the specified process to exit.  Returns the exit code.
static unsigned int WaitForProcessExit(HANDLE subprocess)
{
    DWORD exit_code;

    for (;;)
    {
        WaitForSingleObject(subprocess, INFINITE);

        if (!GetExitCodeProcess(subprocess, &exit_code))
        {
            return -1;
        }

        if (exit_code != STILL_ACTIVE)
        {
            return exit_code;
        }
    }
}

static void ConcatWCString(std::wstring &wstr, const char *value)
{
    const int clen = (int)(strlen(value));
    const int size_needed = MultiByteToWideChar(CP_UTF8, 0, value, clen, nullptr, 0);
    
    std::unique_ptr<wchar_t []> upwstrTo { new wchar_t [ size_needed + 1 ] };
    memset(upwstrTo.get(), 0, (size_needed+1) * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, value, clen, upwstrTo.get(), size_needed);

    wstr += upwstrTo.get();
}

//
// Build the command line string, a wide character string of the form:
//
// "program" "arg"...
//
static std::wstring BuildCommandLine(const char *program, execute_context_t *context)
{
    std::wstring result;
    wchar_t exe_path[MAX_PATH];

    // Get the path to this .exe file.
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);

    result.push_back(L'"');

    // Copy the path part of the filename (including ending \)
    // into the result buffer:
    wchar_t *sep = wcsrchr(exe_path, L'\\');
    if(sep != nullptr)
    {
        result.append(exe_path, sep - exe_path + 1);
    }

    // Concatenate the name of the program:
    ConcatWCString(result, program);

    // End of program name, start of arguments:
    result += L"\"";
 
    for(const qstring &arg : context->args)
    {
        result += L" ";
        ConcatWCString(result, arg.constPtr());
    }
    
    return result;
}

static int ExecuteCommand(const char *program, execute_context_t *context)
{
    STARTUPINFOW startup_info;
    PROCESS_INFORMATION proc_info;
    std::wstring command;
    int result = 0;

    command = BuildCommandLine(program, context);

    // Invoke the program:

    memset(&proc_info, 0, sizeof(proc_info));
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    // make mutable copy
    std::unique_ptr<wchar_t []> upCmdLine { new wchar_t [command.length() + 1] };
    wcsncpy(upCmdLine.get(), command.c_str(), command.length() + 1);

    if (!CreateProcessW(nullptr, upCmdLine.get(),
                        nullptr, nullptr, FALSE, 0, nullptr, nullptr,
                        &startup_info, &proc_info))
    {
        result = -1;
    }
    else
    {
        // Wait for the process to finish, and save the exit code.

        result = WaitForProcessExit(proc_info.hProcess);

        CloseHandle(proc_info.hProcess);
        CloseHandle(proc_info.hThread);
    }

    return result;
}

#else

boolean OpenFolder(const char *path)
{
    char *cmd;
    int result;

#if defined(__MACOSX__)
    cmd = M_StringJoin("open \"", path, "\"", nullptr);
#else
    cmd = M_StringJoin("xdg-open \"", path, "\"", nullptr);
#endif
    result = system(cmd);
    free(cmd);

    return result == 0;
}

// Given the specified program name, get the full path to the program,
// assuming that it is in the same directory as this program is.

static char *GetFullExePath(const char *program)
{
    char *result;
    const char *sep;
    size_t result_len;
    unsigned int path_len;

    sep = strrchr(myargv[0], DIR_SEPARATOR);

    if (sep == nullptr)
    {
        result = M_StringDuplicate(program);
    }
    else
    {
        path_len = sep - myargv[0] + 1;
        result_len = strlen(program) + path_len + 1;
        result = static_cast<char *>(malloc(result_len));

        M_StringCopy(result, myargv[0], result_len);
        result[path_len] = '\0';

        M_StringConcat(result, program, result_len);
    }

    return result;
}

static int ExecuteCommand(const char *program, execute_context_t *context)
{
    pid_t childpid;
    int result;
    

    childpid = fork();

    if (childpid == 0) 
    {
        // This is the child.  Execute the command.

        const size_t numargs = 2 + context->args.size();
        std::unique_ptr<const char *[]> upArgs { new const char * [numargs] };

        size_t idx = 0;
        upArgs[idx++] = GetFullExePath(program);
        for(const qstring &str : context->args)
            upArgs[idx++] = str.constPtr();
        upArgs[idx] = nullptr;

        execvp(upArgs[0], upArgs.get());

        exit(0x80);
    }
    else
    {
        // This is the parent.  Wait for the child to finish, and return
        // the status code.

        waitpid(childpid, &result, 0);

        if (WIFEXITED(result) && WEXITSTATUS(result) != 0x80) 
        {
            return WEXITSTATUS(result);
        }
        else
        {
            return -1;
        }
    }
}

#endif

int ExecuteDoom(execute_context_t *context)
{
    int result;
    
    // Run Doom

    result = ExecuteCommand(GetExecutableName(), context);

    // Destroy context
    delete context;

    return result;
}

static void TestCallback(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(data))
{
#if DEADCODE
    execute_context_t *exec;
    char *main_cfg;
    char *extra_cfg;
    txt_window_t *testwindow;

    testwindow = TXT_MessageBox("Starting Doom",
                                "Starting Doom to test the\n"
                                "settings.  Please wait.");
    TXT_DrawDesktop();

    // Save temporary configuration files with the current configuration

    main_cfg = TempFile("tmp.cfg");
    extra_cfg = TempFile("extratmp.cfg");

    //M_SaveDefaultsAlternate(main_cfg, extra_cfg);

    // Run with the -testcontrols parameter

    exec = NewExecuteContext();
    AddCmdLineParameter(exec, "-testcontrols");
    AddCmdLineParameter(exec, "-config \"%s\"", main_cfg);
    AddCmdLineParameter(exec, "-extraconfig \"%s\"", extra_cfg);
    ExecuteDoom(exec);

    TXT_CloseWindow(testwindow);

    // Delete the temporary config files

    remove(main_cfg);
    remove(extra_cfg);
    free(main_cfg);
    free(extra_cfg);
#endif
}

txt_window_action_t *TestConfigAction(void)
{
    txt_window_action_t *test_action;
    
    test_action = TXT_NewWindowAction('t', "Test");
    TXT_SignalConnect(test_action, "pressed", TestCallback, nullptr);

    return test_action;
}

