/**
 * @file metronome.c
 * @brief Metronome logic
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "metronome.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"

static const char* TAG = "metronome";

static int bpm = 200;
static int period_ms;
static int beat_length_ms;

void metronome_init()
{
    // Calculate period from beats per minute
    period_ms = 60 * 1000 / bpm;
    ESP_LOGI(TAG, "period: %d ms", period_ms);
    // Calculate short enough beat length
    beat_length_ms = period_ms / 4;
    ESP_LOGI(TAG, "beat length: %d ms", beat_length_ms);

    gpio_reset_pin(CONFIG_GPIO_PIN);
    gpio_set_direction(CONFIG_GPIO_PIN, GPIO_MODE_OUTPUT);
}

static void metronome_beat()
{
    // Enable and wait
    gpio_set_level(CONFIG_GPIO_PIN, 1);
    vTaskDelay(beat_length_ms / portTICK_PERIOD_MS);
    // Disable and wait for the rest of period
    gpio_set_level(CONFIG_GPIO_PIN, 0);
    vTaskDelay((period_ms - beat_length_ms) / portTICK_PERIOD_MS);
}

void metronome_start()
{
    while (1) {
        metronome_beat();
    }
}
