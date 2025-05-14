#include "time_sync.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/time.h>
#include <time.h>

static const char *TAG = "TIME_SYNC";

// Called by the SNTP module when the first time sync happens
static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time is synchronized");
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");

    // Poll mode: query pool.ntp.org once per hour (CONFIG_LWIP_SNTP_UPDATE_DELAY)
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // Use the public NTP pool
    sntp_setservername(0, "pool.ntp.org");

    // Register our notification callback
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    // Start the SNTP service
    sntp_init();
}

void obtain_time(void)
{
    initialize_sntp();

    // Wait up to 10 seconds for the first time sync
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 5;
    while (timeinfo.tm_year < (2020 - 1900) && ++retry <= retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    if (timeinfo.tm_year < (2020 - 1900)) {
        ESP_LOGW(TAG, "Failed to synchronize time");
    } else {
        ESP_LOGI(TAG, "Current date/time: %s", asctime(&timeinfo));
    }
}