#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <esp_camera.h>
#include <numeric>
#include "sensor.hpp"
#include "../fec.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int frame;

void OV2640_c::setup() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 7;
    config.fb_count = 2;
    ESP_ERROR_CHECK(esp_camera_init(&config));
    sensor_t *sensor = esp_camera_sensor_get();
    sensor->set_framesize(sensor, config.frame_size);
    sensor->set_saturation(sensor, 0);
    frame = 0;
}

void OV2640_c::loop() {
    vTaskDelay(10);
    camera_fb_t *fb = esp_camera_fb_get();
    typedef uint8_t id;
    const size_t CHUNK_DATA_SIZE = 1000;
    const size_t crc_count = 4;
    struct chunk_params {
        uint8_t k;
        uint8_t m;
    };
    typedef std::array<uint8_t, CHUNK_DATA_SIZE> chunk_data;
    struct chunk_t {
        chunk_params params;
        uint8_t id;
        chunk_data data;
    };
    const int chunks = std::ceil(fb->len / CHUNK_DATA_SIZE);
    fec_t *fec = fec_new(chunks, chunks + crc_count);
    uint8_t *src[chunks];
    uint8_t *crc[crc_count];
    uint8_t crc_buffer[crc_count * CHUNK_DATA_SIZE];
    for (int i = 0; i < chunks; i++) {
        src[i] = &fb->buf[i * CHUNK_DATA_SIZE];
    }
    for (int i = 0; i < crc_count; i++) {
        crc[i] = &crc_buffer[i * CHUNK_DATA_SIZE];
    }
    std::vector<u_int> num(chunks + crc_count);
    std::iota(num.begin(), num.end(), 0);
    fec_encode(fec, src, crc, num.data() + chunks, crc_count, CHUNK_DATA_SIZE);
    for (int i = 0; i < chunks + crc_count; i++) {
        std::array<uint8_t, 1000> chunk{};
        if (i < chunks) {
            std::copy_n(src[i], CHUNK_DATA_SIZE, chunk.data());
        } else {
            std::copy_n(crc[i - chunks], CHUNK_DATA_SIZE, chunk.data());
        }
        container data = {
            frame,
            chunks,
            chunks + crc_count,
            num.at(i),
            std::move(chunk)
        };
        reading(data, true);
    }
    esp_camera_fb_return(fb);
    fec_free(fec);
    frame++;
}