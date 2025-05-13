  /* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include <stdlib.h>
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"

extern "C" {
    #include "jsonParse.h"
}

/* Entry point ----------------------------------------------------------------*/

const char path[] = "/spiffs/testData.json";

extern "C" const char* esp_err_to_name(esp_err_t code);

extern "C" void app_main(void) {

  esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = true
};

esp_err_t ret = esp_vfs_spiffs_register(&conf);
if (ret != ESP_OK) {
    ESP_LOGE("SPIFFS", "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
} else if(ret == ESP_OK) {
    ESP_LOGI("SPIFFS", "SPIFFS mounted successfully");
}
// FILE *f = fopen("/spiffs/testData.json", "r");
// if (f) {
//     char line[128];
//     printf("Contents of testData.json:\n");
//     while (fgets(line, sizeof(line), f)) {
//         printf("%s", line);
//     }
//     fclose(f);
// }

  load_and_parse_json_file(path);
  printf("Task count = %d\n", task_count);

}