#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "EPD_7in5_V2.h"  // From Waveshare C driver

void app_main(void) {
    printf("Starting EPD demo...\n");

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();

    // Create black and white image buffer
    UBYTE *black_image = (UBYTE *)malloc(EPD_7IN5_V2_WIDTH * EPD_7IN5_V2_HEIGHT / 8);
    if (black_image == NULL) {
        printf("Failed to allocate memory for image.\n");
        return;
    }


    // Draw a black bar at the top
    for (int i = 0; i < EPD_7IN5_V2_WIDTH / 8 * 20; i++) {
        black_image[i] = 0x00; // black pixels
    }

    EPD_7IN5_V2_Display(black_image);
    DEV_Delay_ms(2000);
    EPD_7IN5_V2_Sleep();

    free(black_image);
}