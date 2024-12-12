/**
 * @file metronome.c
 * @brief Metronome logic
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "metronome.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/ledc_types.h"
#include "portmacro.h"

static const char* TAG = "metronome";

static int period_ms;
static int beat_length_ms;

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
}

void metronome_set_bpm(unsigned int bpm)
{
    ESP_LOGI(TAG, "setting bpm to %d", bpm);
    // Calculate period from beats per minute
    period_ms = 60 * 1000 / bpm;
    ESP_LOGI(TAG, "period: %d ms", period_ms);
    // Calculate short enough beat length
    beat_length_ms = period_ms / 4;
    ESP_LOGI(TAG, "beat length: %d ms", beat_length_ms);
}

void metronome_set_volume(unsigned int volume)
{
    ESP_LOGI(TAG, "setting volume to %d", volume);
    // Quadratic scaling
    int duty = PWM_MAX_DUTY * (volume / 100.0) * (volume / 100.0);
    ESP_LOGI(TAG, "duty: %d", duty);
    ledc_set_duty(PWM_SPEED_MODE, PWM_CHANNEL, duty);
    ledc_update_duty(PWM_SPEED_MODE, PWM_CHANNEL);
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
