/**
 * @file server.c
 * @brief Web server
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "server.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "http_parser.h"
#include "metronome.h"
#include "sys/param.h"
#include <errno.h>
#include <stdlib.h>

static const char* TAG = "http";

static esp_err_t hello_handler(httpd_req_t* req)
{
    httpd_resp_send(req, "Hello world", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t set_bpm_patch_handler(httpd_req_t* req)
{
    // Receive data
    char buf[4];
    int received = httpd_req_recv(req, buf, MIN(req->content_len, sizeof(buf)));
    buf[received] = '\0';

    // Try to convert to integer
    errno = 0;
    int bpm = strtod(buf, NULL);
    if (errno != 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid BPM");
        return ESP_FAIL;
    }

    metronome_set_bpm(bpm);
    httpd_resp_sendstr(req, "BPM set");
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri = "/hello",
    .method = HTTP_GET,
    .handler = hello_handler,
};

static const httpd_uri_t set_bpm_patch_uri = {
    .uri = "/set-bpm",
    .method = HTTP_PATCH,
    .handler = set_bpm_patch_handler,
};

void server_init()
{
    httpd_handle_t server;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &config));
    ESP_LOGI(TAG, "HTTP server started");

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &hello);
    httpd_register_uri_handler(server, &set_bpm_patch_uri);
}
