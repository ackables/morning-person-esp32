#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "routineData.h"
#include "esp_log.h"
#include "routine_parser.h"
#include "jsonParse.h"


static const char *TAG = "ROUTINE_PARSER";

void parse_homeCoords(jparse_ctx_t *jctx) {
    if (json_obj_get_object(jctx, "homeCoords") == OS_SUCCESS) {
        // latitude & longitude as floats
        json_obj_get_float(jctx, "lat", &homeCoords.lat);
        json_obj_get_float(jctx, "lon", &homeCoords.lon);
        json_obj_leave_object(jctx);
        ESP_LOGI(TAG, "Home coords: %f, %f",
                 homeCoords.lat, homeCoords.lon);
    } else {
        ESP_LOGW(TAG, "Missing 'homeCoords' object");
    }
}

void parse_tasks(jparse_ctx_t *jctx) {
    int num_tasks = 0;
    if (json_obj_get_array(jctx, "tasks", &num_tasks) == OS_SUCCESS) {
        task_count = (num_tasks > MAX_TASKS) ? MAX_TASKS : num_tasks;
        for (int i = 0; i < task_count; ++i) {
            if (json_arr_get_object(jctx, i) == OS_SUCCESS) {
                json_obj_get_int   (jctx, "id",   &task_list[i].id);
                json_obj_get_string(jctx, "name",
                                    task_list[i].name,
                                    sizeof(task_list[i].name));
                json_obj_get_int   (jctx, "time", &task_list[i].time);
                json_arr_leave_object(jctx);

                ESP_LOGI(TAG, "Parsed Task %d: %s (%d min)",
                         task_list[i].id,
                         task_list[i].name,
                         task_list[i].time);
            } else {
                ESP_LOGW(TAG, "tasks[%d] is not an object", i);
            }
        }
        json_obj_leave_array(jctx);
    } else {
        ESP_LOGW(TAG, "Missing 'tasks' array");
    }
}

void parse_days(jparse_ctx_t *jctx) {
    if (json_obj_get_object(jctx, "days") == OS_SUCCESS) {
        day_count = NUM_DAYS;
        for (Weekday wd = SUNDAY; wd < NUM_DAYS; ++wd) {
            if (json_obj_get_object(jctx, weekday_names[wd]) == OS_SUCCESS) {
                Day *d = &day_list[wd];
                // fill in the name & enum
                strncpy(d->name, weekday_names[wd], sizeof(d->name) - 1);
                d->weekday = wd;
                d->task_count = 0;

                // parse the task_id array
                int cnt = 0;
                if (json_obj_get_array(jctx, "task_id", &cnt) == OS_SUCCESS) {
                    d->task_count = (cnt > MAX_TASKS) ? MAX_TASKS : cnt;
                    for (int i = 0; i < d->task_count; ++i) {
                        json_arr_get_int(jctx, i, &d->task_id[i]);
                    }
                    json_obj_leave_array(jctx);
                }

                // parse the dest object
                if (json_obj_get_object(jctx, "dest") == OS_SUCCESS) {
                    // nested coords object
                    if (json_obj_get_object(jctx, "destCoords") == OS_SUCCESS) {
                        json_obj_get_float(jctx, "lat",
                                           &d->dest.coords.lat);
                        json_obj_get_float(jctx, "lon",
                                           &d->dest.coords.lon);
                        json_obj_leave_object(jctx);
                    }
                    json_obj_get_int(jctx, "travelTime",
                                     &d->dest.travelTime);
                    json_obj_leave_object(jctx);
                }

                // final scalar
                json_obj_get_int(jctx, "destArrival", &d->destArrival);
                json_obj_leave_object(jctx);

                ESP_LOGI(TAG, "Parsed %s: %d tasks, arrival=%d",
                         d->name, d->task_count, d->destArrival);
            } else {
                ESP_LOGW(TAG, "days.%s not found",
                         weekday_names[wd]);
            }
        }
        json_obj_leave_object(jctx);
    } else {
        ESP_LOGW(TAG, "Missing 'days' object");
    }
}

void load_and_parse_routine(const char *path) {
    // 1) Slurp file into memory
    FILE *f = fopen(path, "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open %s", path);
        return;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(len + 1);
    if (!buf) {
        ESP_LOGE(TAG, "Out of memory");
        fclose(f);
        return;
    }
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    // 2) Kick off the parser
    jparse_ctx_t jctx;
    if (json_parse_start(&jctx, buf, len) != OS_SUCCESS) {
        ESP_LOGE(TAG, "json_parse_start failed");
        free(buf);
        return;
    }

    // 3) Walk each section
    parse_homeCoords(&jctx);
    parse_tasks    (&jctx);
    parse_days     (&jctx);

    // 4) Tear down
    json_parse_end(&jctx);
    free(buf);
}