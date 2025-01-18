#include "bg_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif

BackgroundProcess* BackgroundProcess_create() {

    BackgroundProcess *process = (BackgroundProcess*) malloc(sizeof(BackgroundProcess));

    if (process == NULL) {
       return NULL;
    }

    #ifdef _WIN32
        memset(&process->processInfo, 0, sizeof(process->processInfo));
    #else
        process->pid = 0;
    #endif

    return process;

    }

void BackgroundProcess_destroy(BackgroundProcess *process)
{

    #ifdef _WIN32
        if (process->processInfo.hProcess != NULL) {
            CloseHandle(process->processInfo.hProcess);
            CloseHandle(process->processInfo.hThread);
        }
    #endif

    free(process);
}


ProcessResult BackgroundProcess_execute(BackgroundProcess *process, const char *command) {
    ProcessResult result;
    result.success = 0;
    result.exitCode = -1;

    #ifdef _WIN32
        STARTUPINFOA startupInfo;
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);


        char* cmd_arr = _strdup(command);
        if(cmd_arr == NULL){
            fprintf(stderr, "Failed to allocate memory for command.\n");
            return result;
        }

        BOOL creationResult = CreateProcessA(
            NULL,
            cmd_arr,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &startupInfo,
            &process->processInfo);


        free(cmd_arr);

        if (!creationResult) {
            fprintf(stderr, "CreateProcess failed: %lu\n", GetLastError());
            return result;
        }

        WaitForSingleObject(process->processInfo.hProcess, INFINITE);

        DWORD exitCode;
        if (!GetExitCodeProcess(process->processInfo.hProcess, &exitCode)) {
            fprintf(stderr, "GetExitCodeProcess failed: %lu\n", GetLastError());
            return result;
        }
        result.exitCode = (int)exitCode;
        result.success = 1;

    #else
        pid_t pid = fork();
        process->pid = pid;

        if (pid == -1) {
            perror("fork failed");
            return result;
        } else if (pid == 0) {
        if (execl("/bin/sh", "sh", "-c", command, (char *)NULL) == -1)
            {
            perror("execl failed");
            exit(1);
            }
        } else {
            int status;
            if(waitpid(pid, &status, 0) == -1)
            {
                perror("waitpid failed");
                return result;
            }
            if (WIFEXITED(status)) {
                result.exitCode = WEXITSTATUS(status);
                result.success = 1; // true
            } else {
                result.exitCode = -1;
            }
        }
    #endif
    return result;
}

void printResult(const char* testName, const ProcessResult result) {
    if (result.success) {
        printf("  %s Success, exit code: %d\n", testName, result.exitCode);
    } else {
        printf("  %s Failed.\n", testName);
    }
}