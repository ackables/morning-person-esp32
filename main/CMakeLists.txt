idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino esp32-waveshare-epd routineScheduling spiffs time_sync nvs_flash
)