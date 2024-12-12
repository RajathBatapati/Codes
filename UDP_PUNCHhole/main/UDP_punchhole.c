#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "Wifi.h"




#define BUFLEN 512
#define PORT 9595

struct client {
    int host;
    short port;
};

static const char *TAG = "UDP_Hole_Punching";

static void diep(const char *s) {
    ESP_LOGE(TAG, "%s", s);
    vTaskDelete(NULL);
}

static void udp_client_task(void *pvParameters) {
    struct sockaddr_in si_me, si_other;
    int s, i, f, j, k, slen = sizeof(si_other);
    struct client buf;
    struct client server;
    struct client peers[10]; // 10 peers. Notice that we're not doing any bound checking.
    int n = 0;

    // Create UDP socket
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        diep("socket");
    }

    // Our own endpoint data
    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // The server's endpoint data
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    // Replace "SERVER_IP" with the actual IP address of your server
    if (inet_aton("13.126.122.184", &si_other.sin_addr) == 0) {
        diep("aton");
    }

    // Store the server's endpoint data
    server.host = si_other.sin_addr.s_addr;
    server.port = si_other.sin_port;

    // Send a simple datagram to the server to let it know of our public UDP endpoint
    if (sendto(s, "hello", 2, 0, (struct sockaddr *)&si_other, slen) == -1) {
        diep("sendto");
    }

    while (1) {
        // Receive data from the socket
        if (recvfrom(s, &buf, sizeof(buf), 0, (struct sockaddr *)&si_other, &slen) == -1) {
            diep("recvfrom");
        }

        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));

        if (server.host == si_other.sin_addr.s_addr && server.port == (short)(si_other.sin_port)) {
            f = 0;

            // Add the reported peer into our peer list
            for (i = 0; i < n && f == 0; i++) {
                if (peers[i].host == buf.host && peers[i].port == buf.port) {
                    f = 1;
                }
            }

            // Only add it if we didn't have it before.
            if (f == 0) {
                peers[n].host = buf.host;
                peers[n].port = buf.port;
                n++;
            }

            si_other.sin_addr.s_addr = buf.host;
            si_other.sin_port = buf.port;

            printf("Added peer %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            printf("Now we have %d peers\n", n);

            // Actual hole punching
            for (k = 0; k < 10; k++) {
                for (i = 0; i < n; i++) {
                    si_other.sin_addr.s_addr = peers[i].host;
                    si_other.sin_port = peers[i].port;

                    if (sendto(s, "hi", 2, 0, (struct sockaddr *)&si_other, slen) == -1) {
                        diep("sendto()");
                    }
                }
            }
        } else {
            for (i = 0; i < n; i++) {
                if (peers[i].host == buf.host && peers[i].port == (short)(buf.port)) {
                    printf("Received from peer %d!\n", i);
                    break;
                }
            }
        }
    }

    close(s);
    vTaskDelete(NULL);
}

void app_main() {

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
    
    
    xTaskCreate(&udp_client_task, "udp_client_task", 4096, NULL, 5, NULL);
}
