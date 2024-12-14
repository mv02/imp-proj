/**
 * @file server.c
 * @brief Web server
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#include "server.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "http_parser.h"
#include "mdns.h"
#include "metronome.h"
#include "sys/param.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* TAG = "http";

static void fs_init()
{
    // Iniitialize and mount the filesystem partition
    esp_vfs_spiffs_conf_t config = {
        .base_path = SERVER_BASE_PATH,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false,
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&config));
}

static void receive_data(httpd_req_t* req, uint max_len, char* dest)
{
    int received = httpd_req_recv(req, dest, MIN(req->content_len, max_len));
    dest[received] = '\0';
}

static int receive_int(httpd_req_t* req, uint max_digits)
{
    // Receive data
    char buf[max_digits + 1];
    receive_data(req, max_digits, buf);

    // Try to convert to integer
    errno = 0;
    int num = strtod(buf, NULL);
    if (errno != 0) {
        ESP_LOGE(TAG, "integer conversion failed");
        return ESP_FAIL;
    }
    return num;
}

static esp_err_t ws_send_message(httpd_req_t* req, const char* message)
{
    httpd_ws_frame_t packet;
    memset(&packet, 0, sizeof(httpd_ws_frame_t));
    packet.type = HTTPD_WS_TYPE_TEXT;
    packet.payload = (uint8_t*)message;
    packet.len = strlen((char*)packet.payload);
    return httpd_ws_send_frame(req, &packet);
}

static esp_err_t ws_handler(httpd_req_t* req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "ws handshake done");
        return ESP_OK;
    }

    httpd_ws_frame_t packet;
    memset(&packet, 0, sizeof(httpd_ws_frame_t));
    packet.type = HTTPD_WS_TYPE_TEXT;

    // Set max_len to 0 to get the frame length
    esp_err_t ret = httpd_ws_recv_frame(req, &packet, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to receive ws frame length");
        return ret;
    }

    if (packet.len > 1) {
        uint8_t* buf = calloc(1, packet.len + 1);
        packet.payload = buf;

        // Get the frame payload
        ret = httpd_ws_recv_frame(req, &packet, packet.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to receive ws frame payload");
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "received ws frame: %s", packet.payload);
        free(buf);
    }

    // TODO: remove
    ESP_ERROR_CHECK(ws_send_message(req, "Response"));

    return ESP_OK;
}

static esp_err_t get_handler(httpd_req_t* req)
{
    ESP_LOGI(TAG, "uri: %s", req->uri);

    // Combine base path with requested file path
    size_t path_len = strlen(SERVER_BASE_PATH) + strlen(req->uri) + 1;
    char path[path_len];
    strcpy(path, SERVER_BASE_PATH);
    strcat(path, req->uri);
    path[path_len] = '\0';

    // Set content type according to the file extension
    if (IS_EXTENSION(path, ".html")) {
        httpd_resp_set_type(req, "text/html");
    } else if (IS_EXTENSION(path, ".js")) {
        httpd_resp_set_type(req, "text/javascript");
    } else if (IS_EXTENSION(path, ".css")) {
        httpd_resp_set_type(req, "text/css");
    } else {
        httpd_resp_set_type(req, "text/plain");
    }

    // Try to open the file
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "failed to open file: %s", path);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Read the file and send it in chunks
    char chunk[SERVER_CHUNK_SIZE];
    size_t read_bytes;
    do {
        read_bytes = fread(chunk, 1, SERVER_CHUNK_SIZE, f);

        if (read_bytes > 0) {
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                fclose(f);
                ESP_LOGE(TAG, "failed to send file");
                httpd_resp_sendstr_chunk(req, NULL);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes != 0);

    fclose(f);
    ESP_LOGI(TAG, "file sent");
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t set_bpm_patch_handler(httpd_req_t* req)
{
    int bpm = receive_int(req, 3);
    if (bpm < 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid BPM");
        return ESP_FAIL;
    }

    metronome_set_bpm(bpm);
    httpd_resp_sendstr(req, "BPM set");
    return ESP_OK;
}

static esp_err_t set_volume_patch_handler(httpd_req_t* req)
{
    int vol = receive_int(req, 3);
    if (vol < 0 || vol > 100) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid volume");
        return ESP_FAIL;
    }

    metronome_set_volume(vol);
    httpd_resp_sendstr(req, "Volume set");
    return ESP_OK;
}

static const httpd_uri_t ws_uri = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = ws_handler,
    .is_websocket = true,
};

static const httpd_uri_t get_uri = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = get_handler,
};

static const httpd_uri_t set_bpm_patch_uri = {
    .uri = "/set-bpm",
    .method = HTTP_PATCH,
    .handler = set_bpm_patch_handler,
};

static const httpd_uri_t set_volume_patch_uri = {
    .uri = "/set-volume",
    .method = HTTP_PATCH,
    .handler = set_volume_patch_handler,
};

void server_init()
{
    fs_init();
    ESP_ERROR_CHECK(mdns_init());
    mdns_hostname_set(SERVER_HOSTNAME);
    mdns_instance_name_set(SERVER_INSTANCE_NAME);

    httpd_handle_t server;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_ERROR_CHECK(httpd_start(&server, &config));
    ESP_LOGI(TAG, "HTTP server started");

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &ws_uri);
    httpd_register_uri_handler(server, &get_uri);
    httpd_register_uri_handler(server, &set_bpm_patch_uri);
    httpd_register_uri_handler(server, &set_volume_patch_uri);
}
