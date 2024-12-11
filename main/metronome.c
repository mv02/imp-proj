/**
 * @file metronome.c
 * @brief Program entrypoint
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"

static const char* TAG = "http";

static esp_err_t hello_handler(httpd_req_t* req)
{
    httpd_resp_send(req, "Hello world", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri = "/hello",
    .method = HTTP_GET,
    .handler = hello_handler,
};

static void server_init()
{
    httpd_handle_t server;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &config));
    ESP_LOGI(TAG, "HTTP server started");

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &hello);
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();
    server_init();
}
