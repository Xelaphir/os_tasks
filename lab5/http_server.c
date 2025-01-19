
#include "http_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "database.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#endif

#define PAGE_HEADER "<html><head><title>Temperature Logger</title></head><body>"
#define PAGE_FOOTER "</body></html>"
#define MAX_BUFFER 1024

#ifdef _WIN32
SOCKET server_socket;
#else
int server_socket;
pthread_t server_thread;
#endif

static int http_port;
static bool running = false;

static char *format_temperature_reading(TemperatureReading reading)
{
    struct tm *timeinfo = localtime(&reading.timestamp);
    char timestamp_str[30];
    strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    char *result = malloc(256);
    if (result == NULL)
    {
        return NULL;
    }

    snprintf(result, 256, "Timestamp: %s, Temperature: %.2f<br>", timestamp_str, reading.temperature);
    return result;
}

static char *build_stats_response(time_t start_time, time_t end_time)
{

    char *result_string = NULL;

    TemperatureStatistics stats = db_get_stats(start_time, end_time);
    struct tm *start_timeinfo = localtime(&stats.start_time);
    struct tm *end_timeinfo = localtime(&stats.end_time);
    char start_time_str[30];
    strftime(start_time_str, sizeof(start_time_str), "%Y-%m-%d %H:%M:%S", start_timeinfo);
    char end_time_str[30];
    strftime(end_time_str, sizeof(end_time_str), "%Y-%m-%d %H:%M:%S", end_timeinfo);

    char *stats_string;
    if (stats.start_time != 0 && stats.end_time != 0)
    {
        stats_string = malloc(512);
        if (stats_string == NULL)
        {
            return NULL;
        }
        snprintf(stats_string, 512, "<h3>Statistics</h3>Start Time: %s<br>End Time: %s<br>Average Temperature: %.2f<br>", start_time_str, end_time_str, stats.average_temperature);
    }
    else
    {
        stats_string = malloc(256);
        if (stats_string == NULL)
        {
            return NULL;
        }
        snprintf(stats_string, 256, "<h3>No Data Available</h3>");
    }

    char *current_temp_string = NULL;
    TemperatureReading current_temp = db_get_current_temp();
    if (current_temp.timestamp != 0)
    {
        current_temp_string = format_temperature_reading(current_temp);
    }

    int response_len = strlen(PAGE_HEADER) + (current_temp_string ? strlen(current_temp_string) : 0) + strlen(stats_string) + strlen(PAGE_FOOTER) + 1;
    result_string = malloc(response_len);

    if (result_string)
    {
        strcpy(result_string, PAGE_HEADER);
        if (current_temp_string)
        {
            strcat(result_string, "<h2>Current Temperature</h2>");
            strcat(result_string, current_temp_string);
            free(current_temp_string);
        }
        strcat(result_string, stats_string);
        free(stats_string);
        strcat(result_string, PAGE_FOOTER);
    }
    return result_string;
}

static int handle_stats_request(int client_socket, time_t start_time, time_t end_time)
{
    char *response = build_stats_response(start_time, end_time);

    if (response == NULL)
    {
        return -1;
    }

#ifdef _WIN32
    send(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
    send(client_socket, response, strlen(response), 0);
#else
    send(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n", 44, 0);
    send(client_socket, response, strlen(response), 0);
#endif

    free(response);
    return 0;
}

static int handle_bad_request(int client_socket)
{
    const char *response = "<html><body><h1>Bad Request</h1></body></html>";
#ifdef _WIN32
    send(client_socket, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n", 50, 0);
    send(client_socket, response, strlen(response), 0);
#else
    send(client_socket, "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n", 50, 0);
    send(client_socket, response, strlen(response), 0);
#endif

    return 0;
}

void handle_client(int client_socket)
{
    char buffer[MAX_BUFFER];
    int bytes_read;

#ifdef _WIN32
    bytes_read = recv(client_socket, buffer, MAX_BUFFER, 0);
#else
    bytes_read = recv(client_socket, buffer, MAX_BUFFER, 0);
#endif

    if (bytes_read <= 0)
    {
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }
    buffer[bytes_read] = '\0';

    char method[16], url[MAX_BUFFER], version[16];
    sscanf(buffer, "%s %s %s", method, url, version);

    if (strcmp(method, "GET") != 0)
    {
        handle_bad_request(client_socket);
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }

    if (strcmp(url, "/") == 0)
    {
        handle_stats_request(client_socket, 0, time(NULL));
    }
    else if (strncmp(url, "/stats", 6) == 0)
    {
        time_t start_time = 0;
        time_t end_time = time(NULL);

        char *start_str = NULL;
        char *end_str = NULL;

        char *params = strchr(url, '?');
        if (params != NULL)
        {
            params++;
            char *token = strtok(params, "&");
            while (token != NULL)
            {
                if (strncmp(token, "start=", 6) == 0)
                {
                    start_str = token + 6;
                }
                else if (strncmp(token, "end=", 4) == 0)
                {
                    end_str = token + 4;
                }
                token = strtok(NULL, "&");
            }
        }
        if (start_str != NULL)
        {
            start_time = atol(start_str);
        }
        if (end_str != NULL)
        {
            end_time = atol(end_str);
        }

        handle_stats_request(client_socket, start_time, end_time);
    }
    else
    {
        handle_bad_request(client_socket);
    }
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

#ifdef _WIN32

unsigned __stdcall http_server_thread(void *arg)
{
    SOCKET client_socket;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    while (running)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addrlen);
        if (client_socket == INVALID_SOCKET)
        {
            if (running)
            {
                fprintf(stderr, "Accept failed: %d\n", WSAGetLastError());
            }
            continue;
        }

        handle_client(client_socket);
    }
    return 0;
}

#else
void *http_server_thread(void *arg)
{
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (running)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addrlen);
        if (client_socket < 0)
        {
            if (running)
            {
                perror("Accept failed");
            }
            continue;
        }
        handle_client(client_socket);
    }
    return NULL;
}

#endif

bool start_http_server(int port)
{
    http_port = port;
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed\n");
        return false;
    }
#endif

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (server_socket == INVALID_SOCKET)
    {
        perror("Socket creation failed");
        WSACleanup();
        return false;
    }
#else
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        return false;
    }
#endif
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(http_port);
#ifdef _WIN32
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        perror("Bind failed");
        closesocket(server_socket);
        WSACleanup();
        return false;
    }
#else
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Bind failed");
        close(server_socket);
        return false;
    }

#endif

#ifdef _WIN32
    if (listen(server_socket, 5) == SOCKET_ERROR)
    {
        perror("Listen failed");
        closesocket(server_socket);
        WSACleanup();
        return false;
    }
#else
    if (listen(server_socket, 5) == -1)
    {
        perror("Listen failed");
        close(server_socket);
        return false;
    }
#endif

    running = true;

#ifdef _WIN32
    unsigned threadID;
    _beginthreadex(NULL, 0, http_server_thread, NULL, 0, &threadID);

#else
    if (pthread_create(&server_thread, NULL, http_server_thread, NULL) != 0)
    {
        perror("Failed to create server thread");
        close(server_socket);
        return false;
    }
#endif

    printf("HTTP Server started on port %d\n", http_port);
    return true;
}
void stop_http_server()
{
    running = false;
#ifdef _WIN32
    if (server_socket != INVALID_SOCKET)
    {
        closesocket(server_socket);
        WSACleanup();
    }
#else
    if (server_socket != -1)
    {
        close(server_socket);
    }
    if (server_thread != 0)
    {
        pthread_join(server_thread, NULL);
    }
#endif
}
