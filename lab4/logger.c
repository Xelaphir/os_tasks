#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_READINGS_PER_DAY (24 * 60 * 60) // Макс. кол-во измерений в сутки
#define MAX_HOURLY_AVG (24 * 31) // Макс. кол-во часовых ср. значений в месяц
#define MAX_DAILY_AVG (365) // Макс. кол-во дневных ср. значений в год

// Глобальные переменные
static TemperatureReading *raw_data_buffer;
static int raw_data_count = 0;
static char raw_data_filename[256];

static TemperatureReading *hourly_avg_buffer;
static int hourly_avg_count = 0;
static char hourly_avg_filename[256];

static TemperatureReading *daily_avg_buffer;
static int daily_avg_count = 0;
static char daily_avg_filename[256];

// Вспомогательные функции
static void write_readings_to_file(const char *filename, TemperatureReading *buffer, int count);
static float calculate_average(TemperatureReading *buffer, int count);
static int get_day_of_year(time_t timestamp);
static int get_hour_of_day(time_t timestamp);
static time_t get_start_of_day(time_t timestamp);
static time_t get_start_of_hour(time_t timestamp);


// Функция инициализации логгера
bool init_logger(const char *raw_data_file, const char *hourly_avg_file, const char *daily_avg_file) {
    if (raw_data_file == NULL || hourly_avg_file == NULL || daily_avg_file == NULL) {
        return false;
    }

    // Инициализируем буфер для "сырых" данных
    raw_data_buffer = malloc(MAX_READINGS_PER_DAY * sizeof(TemperatureReading));
    if (raw_data_buffer == NULL) {
        perror("Failed to allocate memory for raw data buffer");
        return false;
    }
    strncpy(raw_data_filename, raw_data_file, sizeof(raw_data_filename) - 1);
    raw_data_filename[sizeof(raw_data_filename) - 1] = '\0';

    // Инициализируем буфер для часовых средних значений
     hourly_avg_buffer = malloc(MAX_HOURLY_AVG * sizeof(TemperatureReading));
    if (hourly_avg_buffer == NULL) {
        perror("Failed to allocate memory for hourly average buffer");
         free(raw_data_buffer);
         return false;
    }
    strncpy(hourly_avg_filename, hourly_avg_file, sizeof(hourly_avg_filename) - 1);
    hourly_avg_filename[sizeof(hourly_avg_filename) - 1] = '\0';

    // Инициализируем буфер для дневных средних значений
     daily_avg_buffer = malloc(MAX_DAILY_AVG * sizeof(TemperatureReading));
    if (daily_avg_buffer == NULL) {
        perror("Failed to allocate memory for daily average buffer");
        free(raw_data_buffer);
        free(hourly_avg_buffer);
        return false;
    }
    strncpy(daily_avg_filename, daily_avg_file, sizeof(daily_avg_filename) - 1);
    daily_avg_filename[sizeof(daily_avg_filename) - 1] = '\0';

    return true;
}

// Функция добавления измерения температуры
void log_temperature(float temperature) {
    if (raw_data_buffer == NULL) {
        fprintf(stderr, "Error: Logger not initialized.\n");
        return;
    }

    time_t now = time(NULL);
    raw_data_buffer[raw_data_count % MAX_READINGS_PER_DAY].timestamp = now;
    raw_data_buffer[raw_data_count % MAX_READINGS_PER_DAY].temperature = temperature;
    raw_data_count++;

    // Write to file every time there is a new reading
    write_readings_to_file(raw_data_filename, raw_data_buffer, (raw_data_count > MAX_READINGS_PER_DAY) ? MAX_READINGS_PER_DAY : raw_data_count);
}


void process_hourly_average() {
  if (raw_data_buffer == NULL || hourly_avg_buffer == NULL) {
        fprintf(stderr, "Error: Logger not initialized.\n");
        return;
    }

    time_t now = time(NULL);
    time_t hour_start = get_start_of_hour(now);
    int reading_count = 0;
    float sum = 0.0f;

    // Collect readings from the current hour
    for(int i=0; i<((raw_data_count > MAX_READINGS_PER_DAY) ? MAX_READINGS_PER_DAY : raw_data_count); ++i) {
        time_t reading_time = raw_data_buffer[i % MAX_READINGS_PER_DAY].timestamp;
        if(reading_time >= hour_start && reading_time < (hour_start + 3600)) {
             sum += raw_data_buffer[i % MAX_READINGS_PER_DAY].temperature;
             reading_count++;
        }
    }

    // If there were readings, calculate and store the average
    if(reading_count > 0) {
         float avg = sum / reading_count;
        hourly_avg_buffer[hourly_avg_count % MAX_HOURLY_AVG].timestamp = hour_start;
        hourly_avg_buffer[hourly_avg_count % MAX_HOURLY_AVG].temperature = avg;
         hourly_avg_count++;
        // write to file every hour
        write_readings_to_file(hourly_avg_filename, hourly_avg_buffer, (hourly_avg_count > MAX_HOURLY_AVG) ? MAX_HOURLY_AVG : hourly_avg_count);

    }

}



void process_daily_average() {
    if(raw_data_buffer == NULL || daily_avg_buffer == NULL) {
        fprintf(stderr, "Error: Logger not initialized.\n");
        return;
    }
    time_t now = time(NULL);
    time_t day_start = get_start_of_day(now);
     float sum = 0.0f;
    int reading_count = 0;

   // Collect readings from the current day
   for(int i=0; i<((raw_data_count > MAX_READINGS_PER_DAY) ? MAX_READINGS_PER_DAY : raw_data_count); ++i) {
        time_t reading_time = raw_data_buffer[i % MAX_READINGS_PER_DAY].timestamp;
        if(reading_time >= day_start && reading_time < (day_start + 24*3600)) {
            sum += raw_data_buffer[i % MAX_READINGS_PER_DAY].temperature;
            reading_count++;
        }
    }

    if(reading_count > 0) {
        float avg = sum / reading_count;
        int day_index = get_day_of_year(now) - 1;
        daily_avg_buffer[day_index].timestamp = day_start;
        daily_avg_buffer[day_index].temperature = avg;
         daily_avg_count++;

         write_readings_to_file(daily_avg_filename, daily_avg_buffer, (daily_avg_count > MAX_DAILY_AVG) ? MAX_DAILY_AVG : daily_avg_count);
    }

}

// Функция закрытия логгера
void close_logger() {
    if (raw_data_buffer != NULL) {
        free(raw_data_buffer);
    }
     if (hourly_avg_buffer != NULL) {
        free(hourly_avg_buffer);
    }
    if (daily_avg_buffer != NULL) {
         free(daily_avg_buffer);
    }
}

// Вспомогательные функции
// Запись данных в файл
static void write_readings_to_file(const char *filename, TemperatureReading *buffer, int count) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%ld,%f\n", buffer[i].timestamp, buffer[i].temperature);
    }

    fclose(fp);
}
// Функция получения дня года
static int get_day_of_year(time_t timestamp) {
    struct tm *timeinfo = localtime(&timestamp);
    return timeinfo->tm_yday + 1;
}

// Функция получения часа дня
static int get_hour_of_day(time_t timestamp) {
    struct tm *timeinfo = localtime(&timestamp);
    return timeinfo->tm_hour;
}

// Функция получения начала дня
static time_t get_start_of_day(time_t timestamp) {
    struct tm *timeinfo = localtime(&timestamp);
    timeinfo->tm_hour = 0;
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;
    return mktime(timeinfo);
}

static time_t get_start_of_hour(time_t timestamp) {
    struct tm *timeinfo = localtime(&timestamp);
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;
    return mktime(timeinfo);
}

