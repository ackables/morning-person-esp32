/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include <stdlib.h>
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "routine_parser.h"
#include "routineData.h"
#include "time_sync.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <WiFi.h>
#include "nvs_flash.h"
#include "wifi_creds.h"

extern "C" {
    #include "jsonParse.h"
    extern const char* esp_err_to_name(esp_err_t code);
    extern const char* ssid;
    extern const char* pass;
}

static const char *TAG = "MAIN";

/* Entry point ----------------------------------------------------------------*/
const char path[] = "/spiffs/testData.json";

extern "C" void app_main(void) {

    // Mount SPIFFS
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "SPIFFS mounted successfully");


    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated/erased or needs erasing to upgrade
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);


        // Start Arduino WiFi 
    WiFi.begin(ssid, pass);
    ESP_LOGI(TAG, "Connecting WiFi '%s'…", ssid);
    // block until connected
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        ESP_LOGI(TAG, ".");
    }
    ESP_LOGI(TAG, "WiFi connected, IP: %s", WiFi.localIP().toString().c_str());

    // Now that we have an IP, sync time
    obtain_time();

    // // Parse JSON, start display task…
    // load_and_parse_routine(path);
    // xTaskCreatePinnedToCore(display_task_loop, "disp", 8*1024, NULL, 1, NULL, 1);

    // Parse JSON into data structures
    load_and_parse_routine(path);

    // Display summary
    ESP_LOGI(TAG, "Total tasks parsed: %d", task_count);
    for (int i = 0; i < task_count; ++i) {
        ESP_LOGI(TAG, "Task[%d]: id=%d, name='%s', time=%d",
                 i,
                 task_list[i].id,
                 task_list[i].name,
                 task_list[i].time);
    }
    ESP_LOGI(TAG, "Total days parsed: %d", day_count);
    for (int wd = MONDAY; wd < NUM_DAYS; ++wd) {
        Day &d = day_list[wd];
        ESP_LOGI(TAG, "Day[%s]: taskCount=%d, destArrival=%d, travelTime=%d, coords={%f, %f}", 
                 weekday_names[wd],
                 d.task_count,
                 d.destArrival,
                 d.dest.travelTime,
                 d.dest.coords.lat,
                 d.dest.coords.lon);
        for (int j = 0; j < d.task_count; ++j) {
            int tid = d.task_id[j];
            if (tid >= 0 && tid < task_count) {
                Task &t = task_list[tid];
                ESP_LOGI(TAG, "  -> [%d] %s (%d min)", tid, t.name, t.time);
            }
        }
    }

    // Optionally, trigger EPD update or UI here
}
