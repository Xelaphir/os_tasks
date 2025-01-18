#ifndef BG_PROC_TESTING_H
#define BG_PROC_TESTING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

typedef struct
{
    int exitCode;
    int success;
} ProcessResult;

typedef struct
{
    #if defined(_WIN32) || defined(_WIN64)
        PROCESS_INFORMATION processInfo;
    #else
        pid_t pid;
    #endif
} BackgroundProcess;

ProcessResult BackgroundProcess_execute(BackgroundProcess *process, const char* command);
BackgroundProcess* BackgroundProcess_create();
void BackgroundProcess_destroy(BackgroundProcess *process);

void printResult(const char* testName, const ProcessResult result);

#endif