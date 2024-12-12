/**
 * @file main.c
 * @brief Program entrypoint
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "esp_err.h"
#include "metronome.h"
#include "nvs_flash.h"
#include "server.h"
#include "wifi.h"

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();
    server_init();

    metronome_init();
    metronome_start();
}
