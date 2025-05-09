#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    printf("🎉 Morning Person is running!\n");

    while (true) {
        printf("Tick...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1000 ms
    }
}