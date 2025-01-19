#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

static sqlite3 *db;

bool db_init(const char *db_file) {
     int rc = sqlite3_open(db_file, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return false;
    }
    char *error_message = 0;
    const char *create_raw_table_sql = "CREATE TABLE IF NOT EXISTS raw_temperature ("
                                        "timestamp INTEGER PRIMARY KEY,"
                                        "temperature REAL);";
    rc = sqlite3_exec(db, create_raw_table_sql, NULL, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating raw table: %s\n", error_message);
        sqlite3_free(error_message);
        sqlite3_close(db);
        return false;
    }

     const char *create_hourly_table_sql = "CREATE TABLE IF NOT EXISTS hourly_avg ("
                                        "timestamp INTEGER PRIMARY KEY,"
                                        "temperature REAL);";
      rc = sqlite3_exec(db, create_hourly_table_sql, NULL, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating hourly table: %s\n", error_message);
         sqlite3_free(error_message);
         sqlite3_close(db);
        return false;
    }

    const char *create_daily_table_sql = "CREATE TABLE IF NOT EXISTS daily_avg ("
                                        "timestamp INTEGER PRIMARY KEY,"
                                        "temperature REAL);";
    rc = sqlite3_exec(db, create_daily_table_sql, NULL, 0, &error_message);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating daily table: %s\n", error_message);
         sqlite3_free(error_message);
        sqlite3_close(db);
        return false;
    }

    return true;
}
void db_close() {
    if (db) {
        sqlite3_close(db);
    }
}
void db_insert_raw_data(time_t timestamp, float temperature) {
    char *error_message = 0;
    sqlite3_stmt *stmt;
     int rc = sqlite3_prepare_v2(db, "INSERT INTO raw_temperature (timestamp, temperature) VALUES (?, ?)", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
         return;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)timestamp);
    sqlite3_bind_double(stmt, 2, (double)temperature);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void db_insert_hourly_avg(time_t timestamp, float average_temperature) {
   char *error_message = 0;
     sqlite3_stmt *stmt;
     int rc = sqlite3_prepare_v2(db, "INSERT INTO hourly_avg (timestamp, temperature) VALUES (?, ?)", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
         return;
    }
     sqlite3_bind_int64(stmt, 1, (sqlite3_int64)timestamp);
    sqlite3_bind_double(stmt, 2, (double)average_temperature);

    rc = sqlite3_step(stmt);
     if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert hourly average data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void db_insert_daily_avg(time_t timestamp, float average_temperature) {
    char *error_message = 0;
     sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, "INSERT INTO daily_avg (timestamp, temperature) VALUES (?, ?)", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
         return;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)timestamp);
    sqlite3_bind_double(stmt, 2, (double)average_temperature);

    rc = sqlite3_step(stmt);
     if (rc != SQLITE_DONE) {
         fprintf(stderr, "Failed to insert daily average data: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

TemperatureReading db_get_current_temp() {
    TemperatureReading reading = {0, 0.0f};
     sqlite3_stmt *stmt;
     int rc = sqlite3_prepare_v2(db, "SELECT timestamp, temperature FROM raw_temperature ORDER BY timestamp DESC LIMIT 1", -1, &stmt, NULL);
      if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return reading;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        reading.timestamp = (time_t)sqlite3_column_int64(stmt, 0);
        reading.temperature = (float)sqlite3_column_double(stmt, 1);
    }
    sqlite3_finalize(stmt);
    return reading;
}

TemperatureStatistics db_get_stats(time_t start_time, time_t end_time) {
  TemperatureStatistics stats = {0, 0, 0.0f};
     sqlite3_stmt *stmt;
     int rc = sqlite3_prepare_v2(db, "SELECT MIN(timestamp), MAX(timestamp), AVG(temperature) FROM raw_temperature WHERE timestamp >= ? AND timestamp <= ?", -1, &stmt, NULL);
     if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return stats;
    }
      sqlite3_bind_int64(stmt, 1, (sqlite3_int64)start_time);
      sqlite3_bind_int64(stmt, 2, (sqlite3_int64)end_time);


    if (sqlite3_step(stmt) == SQLITE_ROW) {
        stats.start_time = (time_t)sqlite3_column_int64(stmt, 0);
        stats.end_time = (time_t)sqlite3_column_int64(stmt, 1);
        stats.average_temperature = (float)sqlite3_column_double(stmt, 2);
    }
     sqlite3_finalize(stmt);

    return stats;
}
