/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <time.h> 
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "utility/EPD_7in5_V2.h"
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
#include <vector>

extern "C" {
    #include "jsonParse.h"
    #include <stdlib.h>
    #include <time.h> 
    extern const char* esp_err_to_name(esp_err_t code);
    extern const char* ssid;
    extern const char* pass;
}

static const char *TAG = "MAIN";

/* Entry point ----------------------------------------------------------------*/
const char path[] = "/spiffs/testData.json";

#include <vector>     // for std::vector

extern "C" void display_task(void *pv){
    const int W = EPD_7IN5_V2_WIDTH;
    const int H = EPD_7IN5_V2_HEIGHT;

    // small vertical gaps
    const int gap1 = 2;
    const int gap2 = 5;

    // clock dims for "HH:MM:SS"
    const int CLK_CHARS = 8;
    const int CLK_W     = (Font24.Width * CLK_CHARS + 7) & ~7;
    const int CLK_H     = Font24.Height;

    // height of (dayName + gap1 + date + gap2 + clock)
    const int centerH = Font24.Height + gap1 + Font16.Height + gap2 + CLK_H;
    // top of that block, to vertically center
    const int yStart  = (H - centerH)/2;

    // columns at very top
    const int COL_W   = Font16.Width * 12; // ~12 chars wide
    const int X_LEFT  = 20;
    const int X_RIGHT = W - 20 - COL_W;
    const int yCol    = 4; // 4px down from top for headers

    // buffers
    UBYTE *fullBuf = (UBYTE*)malloc((W/8)*H);
    UBYTE *clkBuf  = (UBYTE*)malloc((CLK_W/8)*CLK_H);
    if (!fullBuf || !clkBuf) {
        ESP_LOGE(TAG, "display_task: OOM");
        vTaskDelete(NULL);
    }

    int lastIdxNext      = -1;
    bool lastWithinCutoff = false;
    int lastDayOfMonth   = -1;

    for (;;) {
        time_t now; struct tm tmnow;
        time(&now);
        localtime_r(&now, &tmnow);
        int nowMin    = tmnow.tm_hour*60 + tmnow.tm_min;
        int todayWd   = tmnow.tm_wday;
        int todayDate = tmnow.tm_mday;

        // build today’s start times
        Day &dToday = day_list[todayWd];
        int nT = dToday.task_count;
        std::vector<int> startT(nT);
        int tarr = dToday.destArrival * 60;
        for (int k = nT-1; k >= 0; --k) {
            tarr -= task_list[dToday.task_id[k]].time;
            startT[k] = tarr;
        }

        // find next index and cutoff
        int idxNext = 0;
        while (idxNext < nT && startT[idxNext] <= nowMin) {
            ++idxNext;
        }
        bool withinCutoff = nowMin <= (dToday.destArrival*60 + 15);

        // only full–refresh when day-of-month, idxNext or cutoff changes
        if (idxNext       != lastIdxNext
         || withinCutoff != lastWithinCutoff
         || todayDate    != lastDayOfMonth)
        {
            lastIdxNext      = idxNext;
            lastWithinCutoff = withinCutoff;
            lastDayOfMonth   = todayDate;

            // --- FULL SCREEN REFRESH ---
            EPD_7IN5_V2_Init();
            Paint_NewImage(fullBuf, W, H, ROTATE_0, WHITE);
            Paint_Clear(WHITE);

            // 1) draw centered Day name + Date
            char dateBuf[20];
            snprintf(dateBuf, sizeof(dateBuf),
                     "%02d/%02d/%04hu",
                     tmnow.tm_mon+1,
                     tmnow.tm_mday,
                     tmnow.tm_year+1900);
            const char *dayName = weekday_names[todayWd];
            int dayW  = strlen(dayName)*Font24.Width;
            int dateW = strlen(dateBuf)*Font16.Width;
            // day
            Paint_DrawString_EN(
              (W - dayW)/2,
              yStart,
              dayName,
              &Font24,
              BLACK, WHITE
            );
            // date
            Paint_DrawString_EN(
              (W - dateW)/2,
              yStart + Font24.Height + gap1,
              dateBuf,
              &Font16,
              BLACK, WHITE
            );

            // 2) split into prev/todo (and roll forward/back if empty)
            std::vector<int> prevIdx, todoIdx;
            for (int i = 0; i < nT; ++i) {
                if (startT[i] + 15 < nowMin) prevIdx.push_back(i);
                else                           todoIdx.push_back(i);
            }
            // ... roll todo to tomorrow if empty ...
            int todoDay = todayWd, prevDay = todayWd;
            std::vector<int> startTodo = startT, startPrev = startT;

            if (todoIdx.empty()) {
                todoDay = (todayWd + 1) % NUM_DAYS;
                Day &d2 = day_list[todoDay];
                startTodo.resize(d2.task_count);
                int m2 = d2.destArrival * 60;
                for (int k = d2.task_count-1; k>=0; --k) {
                    m2 -= task_list[d2.task_id[k]].time;
                    startTodo[k] = m2;
                }
                todoIdx.clear();
                for (int j = 0; j < d2.task_count; ++j) todoIdx.push_back(j);
            }
            if (prevIdx.empty()) {
                prevDay = (todayWd + NUM_DAYS - 1) % NUM_DAYS;
                Day &d1 = day_list[prevDay];
                startPrev.resize(d1.task_count);
                int m1 = d1.destArrival * 60;
                for (int k = d1.task_count-1; k>=0; --k) {
                    m1 -= task_list[d1.task_id[k]].time;
                    startPrev[k] = m1;
                }
                prevIdx.clear();
                for (int j = 0; j < d1.task_count; ++j) prevIdx.push_back(j);
            }

            // 3) draw column headers
            char bufTitle[32];
            // left
            snprintf(bufTitle, sizeof(bufTitle),
                     "Todo for %s", weekday_names[todoDay]);
            Paint_DrawString_EN(
              X_LEFT, yCol,
              bufTitle,
              &Font16,
              BLACK, WHITE
            );
            // right (right‐aligned)
            snprintf(bufTitle, sizeof(bufTitle),
                     "Previous for %s", weekday_names[prevDay]);
            int tW = strlen(bufTitle)*Font16.Width;
            Paint_DrawString_EN(
              X_RIGHT + COL_W - tW,
              yCol,
              bufTitle,
              &Font16,
              BLACK, WHITE
            );

            // 4) draw up to 5 tasks under each (white on black)
            const int LINE_H = Font16.Height + 4;
            int yList = yCol + Font16.Height + 4;
            char lineBuf[64];

            // todo
            for (int i = 0; i < std::min((int)todoIdx.size(), 5); ++i) {
                int ti = todoIdx[i];
                int hh = startTodo[ti]/60, mm = startTodo[ti]%60;
                const char *nm = (todoDay==todayWd)
                  ? task_list[dToday.task_id[ti]].name
                  : task_list[day_list[todoDay].task_id[ti]].name;
                snprintf(lineBuf, sizeof(lineBuf),
                         "%02d:%02d %s", hh, mm, nm);
                int tw = strlen(lineBuf)*Font16.Width;
                if (X_LEFT + tw > W) {
                    size_t mc = (W - X_LEFT)/Font16.Width;
                    lineBuf[mc] = '\0';
                }
                Paint_DrawString_EN(
                  X_LEFT,
                  yList + i*LINE_H,
                  lineBuf,
                  &Font16,
                  WHITE, BLACK
                );
            }

            // previous (right-aligned) – no truncation, full string shifted left as needed
            for (int i = 0; i < std::min((int)prevIdx.size(), 5); ++i) {
                int ti = prevIdx[i];
                int hh = startPrev[ti] / 60, mm = startPrev[ti] % 60;
                const char* nm = task_list[ day_list[prevDay].task_id[ti] ].name;

                // build the full entry
                snprintf(lineBuf, sizeof(lineBuf), "%02d:%02d %s", hh, mm, nm);

                // measure its pixel width
                int tw = strlen(lineBuf) * Font16.Width;

                // right-align it within the COL_W box
                int xPrev = X_RIGHT + (COL_W - tw);

                // draw it
                Paint_DrawString_EN(
                  xPrev,
                  yList + i * LINE_H,
                  lineBuf,
                  &Font16,
                  WHITE,
                  BLACK
                );
            }

            // 5) optional “Current Task” if next <15min away
            if (withinCutoff && idxNext < nT) {
                int when = startT[idxNext];
                if (when>nowMin && when-nowMin<=15) {
                    int cx = (W - Font16.Width*12)/2;
                    int cy = yCol + Font16.Height + 4;
                    Paint_DrawString_EN(
                      cx, cy,
                      "Current Task",
                      &Font16,
                      BLACK, WHITE
                    );
                    snprintf(lineBuf,sizeof(lineBuf),
                             "%02d:%02d %s",
                             when/60, when%60,
                             task_list[dToday.task_id[idxNext]].name);
                    Paint_DrawString_EN(
                      cx, cy+LINE_H,
                      lineBuf,
                      &Font16,
                      WHITE, BLACK
                    );
                }
            }

            // push the full‐screen update
            EPD_7IN5_V2_Display(fullBuf);
            EPD_7IN5_V2_Sleep();
        }

        // --- PARTIAL (every second) : redraw only the clock ---
        {
            char clkText[16];
            snprintf(clkText,sizeof(clkText),
                     "%02d:%02d:%02d",
                     tmnow.tm_hour,
                     tmnow.tm_min,
                     tmnow.tm_sec);

            EPD_7IN5_V2_Init_Part();
            Paint_SelectImage(clkBuf);
            Paint_NewImage(clkBuf, CLK_W, CLK_H, 0, WHITE);
            Paint_Clear(WHITE);
            // black text on white background for clock
            Paint_DrawString_EN(
              (CLK_W - CLK_CHARS*Font24.Width)/2,
              0,
              clkText,
              &Font24,
              WHITE, BLACK
            );

            int x0 = ((W - CLK_W)/2) & ~7;
            int y0 = yStart + Font24.Height + gap1 + Font16.Height + gap2;
            EPD_7IN5_V2_Display_Part(
              clkBuf,
              x0,    y0,
              x0+CLK_W-1,
              y0+CLK_H-1
            );
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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
    setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();
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
    const int width  = EPD_7IN5_V2_WIDTH;
    const int height = EPD_7IN5_V2_HEIGHT;
    // 1 bit per pixel → width/8 bytes per scanline
    UBYTE *buffer = (UBYTE*)malloc((width / 8) * height);


    // set up all GPIO pins & SPI for the e-paper
    if (DEV_Module_Init() != 0) {
      ESP_LOGE(TAG, "DEV Module init failed");
      return;
    }

    Paint_NewImage(buffer, width, height, ROTATE_0, WHITE);
    Paint_Clear(WHITE);
    Paint_DrawString_EN(20, 60, "Good Morning!", &Font16, WHITE, BLACK);

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();
    EPD_7IN5_V2_Display(buffer);
    EPD_7IN5_V2_Sleep();

    free(buffer);



xTaskCreatePinnedToCore(
    display_task,  // the only display worker
    "disp",
    8*1024,
    NULL,
    1,
    NULL,
    1
);
    // Optionally, trigger EPD update or UI here
}
