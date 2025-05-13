#ifndef __JSONPARSE_H
#define __JSONPARSE_H

#include <stdio.h>
#include <string.h>
// #include "/Users/bensmith/Documents/ENGR213_214/morning-person-esp32/managed_components/espressif__json_parser/"
#include "esp_log.h"
#include "routineData.h"
#include "json_parser.h"

extern Task task_list[MAX_TASKS];
extern int task_count;

void parse_tasks(jparse_ctx_t *jctx);
void load_and_parse_json_file(const char *path);

#endif