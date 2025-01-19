#include "counter.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{

    FILE *log = fopen(LOG_FILE, "a");
    if (!log)
    {
        printf("Failed to open log file. Program will exit.\n");
        return 1;
    }

    char time_buf[64];
#ifdef _WIN32
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    }
#else
    {
        struct timeval tv;
        struct tm *tm;
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d:%02d.%03ld", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 1000);
    }
#endif

    char message_start[256];
    sprintf(message_start, "Main process, PID: %d, Start Time: %s",
#ifdef _WIN32
            GetCurrentProcessId()
#else
            getpid()
#endif
                ,
            time_buf);

    log_message(log, message_start);

#ifdef _WIN32
    char input[256];
    FORK();
    while (1)
    {
        printf("Enter new value for counter: ");
        if (fgets(input, sizeof(input), stdin))
        {
            if (strncmp(input, "exit", 4) == 0)
            {
                break;
            }
            else
            {
                int new_counter = atoi(input);
                if (new_counter != 0)
                {
                    counter = new_counter;
                    printf("Counter set to: %d \n", counter);
                }
                else
                {
                    printf("Error in input value\n");
                }
            }
        }
    }
#else
    pid_t pid = fork();
    if (pid == 0)
    {
        char input[256];
        while (1)
        {
            printf("Enter new value for counter: ");
            if (fgets(input, sizeof(input), stdin))
            {
                if (strncmp(input, "exit", 4) == 0)
                {
                    break;
                }
                else
                {
                    int new_counter = atoi(input);
                    if (new_counter != 0)
                    {
                        counter = new_counter;
                        printf("Counter set to: %d \n", counter);
                    }
                    else
                    {
                        printf("Error in input value\n");
                    }
                }
            }
        }
        exit(0);
    }
    else
    {
        main_loop(log);
    }
#endif
    fclose(log);
    return 0;
}