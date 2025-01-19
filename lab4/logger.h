#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>
#include <stdbool.h>

// Структура для хранения измерения температуры
typedef struct {
    time_t timestamp;
    float temperature;
} TemperatureReading;

// Прототипы функций
bool init_logger(const char *raw_data_file, const char *hourly_avg_file, const char *daily_avg_file);
void log_temperature(float temperature);
void process_hourly_average();
void process_daily_average();
void close_logger();

#endif