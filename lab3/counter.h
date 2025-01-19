#ifndef COUNTER_H
#define COUNTER_H

#include <stdio.h>

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

#define LOG_FILE "app.log"

void log_message(FILE *log, const char *message);
void child_process(int child_num, FILE *log);
void main_loop(FILE *log);

extern volatile int counter;
extern volatile int child1_running;
extern volatile int child2_running;

#endif