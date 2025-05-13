#ifndef ROUTINE_PARSER_H
#define ROUTINE_PARSER_H

#include "jsonParse.h"
#include "routineData.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Parse the "homeCoords" object from JSON into homeCoords global.
 * @param jctx JSON parser context
 */
void parse_homeCoords(jparse_ctx_t *jctx);

/**
 * @brief Parse the top-level "tasks" array into task_list[] and task_count.
 * @param jctx JSON parser context
 */
void parse_tasks(jparse_ctx_t *jctx);

/**
 * @brief Parse the "days" object into day_list[] and day_count.
 * @param jctx JSON parser context
 */
void parse_days(jparse_ctx_t *jctx);

/**
 * @brief Load a JSON file from path and parse into all routineData structures.
 * @param path Filesystem path to the JSON file
 */
void load_and_parse_routine(const char *path);

#ifdef __cplusplus
}
#endif

#endif // ROUTINE_PARSER_H
