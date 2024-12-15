/**
 * @file metronome.c
 * @brief Metronome logic
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "metronome.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/ledc_types.h"
#include "portmacro.h"
#include "server.h"

static const char* TAG = "metronome";

static bool running = true;

static int current_bpm;
static int period_ms;
static int beat_length_ms;
static int accent_beat_length_ms;
static int current_volume;
static int current_beats;

void pwm_init()
{
    ledc_timer_config_t timer_config = {
        .speed_mode = PWM_SPEED_MODE,
        .duty_resolution = PWM_DUTY_RES,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = CONFIG_PWM_PIN,
        .speed_mode = PWM_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

void metronome_init()
{
    pwm_init();
    gpio_reset_pin(CONFIG_GPIO_PIN);
    gpio_set_direction(CONFIG_GPIO_PIN, GPIO_MODE_OUTPUT);
    metronome_set_bpm(DEFAULT_BPM);
    metronome_set_volume(DEFAULT_VOLUME);
    metronome_set_beats(DEFAULT_BEATS);
}

void metronome_set_bpm(unsigned int bpm)
{
    ESP_LOGI(TAG, "setting bpm to %d", bpm);
    current_bpm = bpm;
    // Calculate period from beats per minute
    period_ms = 60 * 1000 / bpm;
    ESP_LOGI(TAG, "period: %d ms", period_ms);
    // Calculate short enough beat length
    beat_length_ms = period_ms / 4;
    accent_beat_length_ms = beat_length_ms * 2;
    ESP_LOGI(TAG, "beat lengths: %d ms, %d ms", beat_length_ms, accent_beat_length_ms);
}

void metronome_set_volume(unsigned int volume)
{
    ESP_LOGI(TAG, "setting volume to %d", volume);
    current_volume = volume;
    // Quadratic scaling
    int duty = PWM_MAX_DUTY * (volume / 100.0) * (volume / 100.0);
    ESP_LOGI(TAG, "duty: %d", duty);
    ledc_set_duty(PWM_SPEED_MODE, PWM_CHANNEL, duty);
    ledc_update_duty(PWM_SPEED_MODE, PWM_CHANNEL);
}

void metronome_set_beats(unsigned int beats)
{
    ESP_LOGI(TAG, "setting beats to %d", beats);
    current_beats = beats;
}

static void metronome_beat(bool accent)
{
    int beat_len = accent ? accent_beat_length_ms : beat_length_ms;
    // Enable and wait
    gpio_set_level(CONFIG_GPIO_PIN, 1);
    vTaskDelay(beat_len / portTICK_PERIOD_MS);
    // Disable and wait for the rest of period
    gpio_set_level(CONFIG_GPIO_PIN, 0);
    vTaskDelay((period_ms - beat_len) / portTICK_PERIOD_MS);
}

void metronome_loop()
{
    while (1) {
        if (!running) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        metronome_beat(current_beats > 1);
        for (int i = 1; i < current_beats; i++) {
            if (!running) {
                break;
            }
            metronome_beat(false);
        }
    }
}

void metronome_start()
{
    ESP_LOGI(TAG, "starting");
    running = true;
}

void metronome_stop()
{
    ESP_LOGI(TAG, "stopping");
    running = false;
}

char* metronome_status_json()
{
    cJSON* json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "running", running);
    cJSON_AddNumberToObject(json, "bpm", current_bpm);
    cJSON_AddNumberToObject(json, "volume", current_volume);
    cJSON_AddNumberToObject(json, "beats", current_beats);
    char* jsonString = cJSON_Print(json);
    return jsonString;
}
