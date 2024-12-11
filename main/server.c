/**
 * @file server.c
 * @brief Web server
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "server.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"

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

void server_init()
{
    httpd_handle_t server;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &config));
    ESP_LOGI(TAG, "HTTP server started");

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &hello);
}
