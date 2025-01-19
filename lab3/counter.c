#include "counter.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define SLEEP(ms) Sleep(ms)
#define GET_PID() GetCurrentProcessId()
#define FORK() _spawnl(_P_NOWAIT, _pgmptr, _pgmptr, NULL)
#define WAITPID(pid) WaitForSingleObject((HANDLE)pid, INFINITE);
#define GET_TIME(buf, buflen)                                                                                                                          \
    {                                                                                                                                                  \
        SYSTEMTIME st;                                                                                                                                 \
        GetLocalTime(&st);                                                                                                                             \
        snprintf(buf, buflen, "%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds); \
    }
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#define SLEEP(ms) usleep(ms * 1000)
#define GET_PID() getpid()
#define FORK() fork()
#define WAITPID(pid) waitpid(pid, NULL, 0)
#define GET_TIME(buf, buflen)                                                                                                                                                  \
    {                                                                                                                                                                          \
        struct timeval tv;                                                                                                                                                     \
        struct tm *tm;                                                                                                                                                         \
        gettimeofday(&tv, NULL);                                                                                                                                               \
        tm = localtime(&tv.tv_sec);                                                                                                                                            \
        snprintf(buf, buflen, "%04d-%02d-%02d %02d:%02d:%02d.%03ld", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 1000); \
    }
#endif

volatile int counter = 0;
volatile int child1_running = 0;
volatile int child2_running = 0;

void log_message(FILE *log, const char *message)
{
    if (log)
    {
        fprintf(log, "%s\n", message);
        fflush(log);
    }
    else
    {
        printf("Failed to open the log file.\n");
    }
}

void child_process(int child_num, FILE *log)
{
    char time_buf[64];
    GET_TIME(time_buf, sizeof(time_buf));

    char message_start[256];
    sprintf(message_start, "Child %d, PID: %d, Start Time: %s", child_num, GET_PID(), time_buf);
    log_message(log, message_start);

    if (child_num == 1)
    {
        counter += 10;
    }
    else
    {
        counter *= 2;
        SLEEP(2000);
        counter /= 2;
    }

    GET_TIME(time_buf, sizeof(time_buf));
    char message_end[256];
    sprintf(message_end, "Child %d, PID: %d, End Time: %s, Counter: %d", child_num, GET_PID(), time_buf, counter);
    log_message(log, message_end);

    fclose(log);
    exit(0);
}

void main_loop(FILE *log)
{
    char time_buf[64];
    int loop_count = 0;

    while (1)
    {
        SLEEP(300);
        counter++;

        loop_count++;
        if (loop_count % (1000 / 300) == 0)
        {
            GET_TIME(time_buf, sizeof(time_buf));
            char log_line[256];
            sprintf(log_line, "Time: %s, PID: %d, Counter: %d", time_buf, GET_PID(), counter);
            log_message(log, log_line);
        }

        if (loop_count % (3000 / 300) == 0)
        {

            if (!child1_running)
            {
                int pid1 = FORK();
                if (pid1 == 0)
                {
                    FILE *child_log = fopen(LOG_FILE, "a");
                    child_process(1, child_log);
                }
                else if (pid1 > 0)
                {
                    child1_running = 1;
                }
                else
                {
                    log_message(log, "Failed to fork child process 1");
                }
            }
            else
            {
                log_message(log, "Child 1 is still running, skip start.");
            }
            if (!child2_running)
            {
                int pid2 = FORK();
                if (pid2 == 0)
                {
                    FILE *child_log = fopen(LOG_FILE, "a");
                    child_process(2, child_log);
                }
                else if (pid2 > 0)
                {
                    child2_running = 1;
                }
                else
                {
                    log_message(log, "Failed to fork child process 2");
                }
            }
            else
            {
                log_message(log, "Child 2 is still running, skip start.");
            }
        }

        int status;

        if (child1_running)
        {
            int res = WAITPID(((pid_t)-1));
            if (res > 0)
            {
                child1_running = 0;
            }
        }
        if (child2_running)
        {
            int res = WAITPID(((pid_t)-1));
            if (res > 0)
            {
                child2_running = 0;
            }
        }
    }
}