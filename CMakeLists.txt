# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS components/arduino components/esp32-waveshare-epd routineScheduling components/time_sync)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(morning_person_esp32)