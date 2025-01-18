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


int main(int argc, char** argv){
    BackgroundProcess *process = BackgroundProcess_create();

    if(process == NULL)
    {
      fprintf(stderr, "Create process failed\n");
      return 1;
    }


    printf("Running process with 'ls -l' in Linux or 'dir' in Windows:\n");
    #if defined(_WIN32) || defined(_WIN64)
        ProcessResult result2 = BackgroundProcess_execute(process, "dir");
    #else
        ProcessResult result2 = BackgroundProcess_execute(process, "ls -l");
    #endif
    printResult("Check directory result:", result2);


    printf("Running process with non-existent command:\n");
    ProcessResult result_nonex = BackgroundProcess_execute(process, "nonexistentcommand");
    printResult("Non existing result", result_nonex);

    BackgroundProcess_destroy(process);
    
    return 0;
}
