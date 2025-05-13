idf_component_register(
    SRCS "routineData.c"
         "routineData.h"
         "jsonParse.c"
         "jsonParse.h"
         "routine_parser.c"
         "routine_parser.h"
    INCLUDE_DIRS "." 
    REQUIRES arduino espressif__json_parser spiffs
)