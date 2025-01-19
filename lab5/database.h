#ifndef DATABASE_H
#define DATABASE_H

#include <time.h>
#include <stdbool.h>

// Структура для хранения измерения температуры
typedef struct
{
    time_t timestamp;
    float temperature;
} TemperatureReading;

typedef struct
{
    time_t start_time;
    time_t end_time;
    float average_temperature;
} TemperatureStatistics;

bool db_init(const char *db_file);
void db_close();
void db_insert_raw_data(time_t timestamp, float temperature);
void db_insert_hourly_avg(time_t timestamp, float average_temperature);
void db_insert_daily_avg(time_t timestamp, float average_temperature);
TemperatureReading db_get_current_temp();
TemperatureStatistics db_get_stats(time_t start_time, time_t end_time);

#endif
