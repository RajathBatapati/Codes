#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "Wifi.h"

static const char *TAG = "Public_IP";



static void get_public_ip() {
    esp_netif_ip_info_t ip_info = {0};
    // Assuming your Wi-Fi setup has already been done and you have a valid IP
    // If not, you should wait for a valid IP before proceeding.

    // Use an external service to get the public IP address
    struct addrinfo *res = NULL;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    if (getaddrinfo("api64.ipify.org", "80", &hints, &res) == 0) {
        int s = socket(res->ai_family, res->ai_socktype, 0);
        connect(s, res->ai_addr, res->ai_addrlen);

        char *request = "GET /?format=json HTTP/1.1\r\nHost: api64.ipify.org\r\nConnection: close\r\n\r\n";
        send(s, request, strlen(request), 0);

        char buffer[256];
        int recv_len = recv(s, buffer, sizeof(buffer) - 1, 0);
        buffer[recv_len] = '\0';
        ESP_LOGI(TAG, "Public IP address: %s", buffer);

        close(s);
        freeaddrinfo(res);
    }

    vTaskDelay(portMAX_DELAY);
}


void app_main() {
    // Other initialization code

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    esp_netif_init();

    // Get and print the public IP
    xTaskCreate(get_public_ip, "get_public_ip", 4096, NULL, 5, NULL);

    // Your other code
}
