# This is a CMake wrapper for PlatformIO project
cmake_minimum_required(VERSION 3.16)
project(CoAP_ESP32_LED)

set(CMAKE_CXX_STANDARD 14)

# Add PlatformIO generated includes
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/CoAP-simple-library
)

# Manually add ESP32 Arduino framework paths for IDE recognition
set(PLATFORMIO_PATH "$ENV{HOME}/.platformio")
if(WIN32)
    set(PLATFORMIO_PATH "$ENV{USERPROFILE}/.platformio")
endif()

# Add ESP32 Arduino includes
if(EXISTS "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32")
    file(GLOB_RECURSE ARDUINO_ESP32_INCLUDES
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/cores/esp32"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/tools/sdk/esp32/include"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/libraries"
    )

    include_directories(
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/cores/esp32"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/tools/sdk/esp32/include/freertos/include"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/tools/sdk/esp32/include/esp_wifi/include"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/tools/sdk/esp32/include/lwip/include/apps"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/tools/sdk/esp32/include/lwip/lwip/src/include"
            "${PLATFORMIO_PATH}/packages/framework-arduinoespressif32/libraries/WiFi/src"
    )
endif()

# Collect source files
file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

file(GLOB_RECURSE HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

# Create dummy target for IDE
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS}
        src/Client.cpp)

# Define some Arduino/ESP32 macros for IDE
target_compile_definitions(${PROJECT_NAME} PRIVATE
        ARDUINO=10819
        ARDUINO_ESP32_DEV
        ESP32
        ESP_PLATFORM
)

# PlatformIO custom targets
add_custom_target(pio_build
        COMMAND platformio run
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building with PlatformIO"
)

add_custom_target(pio_upload
        COMMAND platformio run --target upload
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Uploading with PlatformIO"
)

add_custom_target(pio_clean
        COMMAND platformio run --target clean
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Cleaning PlatformIO build"
)

add_custom_target(pio_monitor
        COMMAND platformio device monitor
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Opening serial monitor"
)