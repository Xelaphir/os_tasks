#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "logger.h"
#include "usb_port.h"

int main()
{
    if (!init_logger("raw_temperature.txt", "hourly_average.txt", "daily_average.txt"))
    {
        fprintf(stderr, "Failed to initialize logger.\n");
        return 1;
    }

    printf("Logger initialized. Starting to read temperature.\n");

    // Set random seed
    srand(time(NULL));
    time_t last_hourly_process_time = 0;
    time_t last_daily_process_time = 0;

    while (1)
    {
        float temperature = read_temperature_from_usb();
        log_temperature(temperature);

        time_t now = time(NULL);
        if (now - last_hourly_process_time >= 3600)
        {
            process_hourly_average();
            last_hourly_process_time = now;
        }

        if (now - last_daily_process_time >= 24 * 3600)
        {
            process_daily_average();
            last_daily_process_time = now;
        }

#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }
    close_logger();
    return 0;
}
