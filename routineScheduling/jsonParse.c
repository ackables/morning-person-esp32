#include <stdio.h>
#include <string.h>
// #include "/Users/bensmith/Documents/ENGR213_214/morning-person-esp32/managed_components/espressif__json_parser/include/json_parser.h"
#include "esp_log.h"
#include "routineData.h"
#include "jsonParse.h"
#include "json_parser.h"

extern Task task_list[MAX_TASKS];
extern int task_count;

void parse_tasks(jparse_ctx_t *jctx){
    int num_tasks = 0;
    if (json_obj_get_array(jctx, "tasks", &num_tasks) != 0) return;

    // for (int i = 0; i < num_tasks && i < MAX_TASKS; i++) {
    //     if (json_arr_get_object(jctx, i) == 0) {
    //         json_obj_get_string(jctx, "name", task_list[i].name, sizeof(task_list[i].name));
    //         json_obj_get_int(jctx, "time", &task_list[i].time);
    //         json_obj_leave_object(jctx);
    //         task_count++;
    //     }
    // }
    for (int i = 0; i < num_tasks && i < MAX_TASKS; i++) {
        if (json_arr_get_object(jctx, i) == 0) {
            if (json_obj_get_string(jctx, "name", task_list[i].name, sizeof(task_list[i].name)) != 0) {
                ESP_LOGW("JSON", "Failed to read 'name' for task %d", i);
            }

            if (json_obj_get_int(jctx, "time", &task_list[i].time) != 0) {
                ESP_LOGW("JSON", "Failed to read 'time' for task %d", i);
            }

            ESP_LOGI("JSON", "Parsed Task %d: %s (%d min)", i, task_list[i].name, task_list[i].time);

            task_count++;
            json_arr_leave_object(jctx);
            
        } 
        else {
            ESP_LOGW("JSON", "Element %d is not a valid object", i);
        }
    }
    printf("Num tasks: %d\n", num_tasks);
    json_obj_leave_array(jctx);
}

void load_and_parse_json_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("Failed to open JSON file: %s\n", path);
        return;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    // Read file into buffer
    char *json_buf = malloc(filesize + 1);
    if (!json_buf) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return;
    }

    fread(json_buf, 1, filesize, file);
    json_buf[filesize] = '\0'; // Null-terminate the string
    fclose(file);

    // Parse JSON
    jparse_ctx_t jctx;
    if (json_parse_start(&jctx, json_buf, strlen(json_buf)) != 0) {
        printf("Failed to start JSON parsing.\n");
        free(json_buf);
        return;
    }

    ESP_LOGI("JSON", "Raw JSON data:\n%s", json_buf);

    parse_tasks(&jctx);  // Your task parser

    json_parse_end(&jctx);
    free(json_buf);

    // Print parsed tasks
    for (int i = 0; i < task_count; i++) {
        printf("Task %d: %s (%d minutes)\n", i + 1, task_list[i].name, task_list[i].time);
    }
}