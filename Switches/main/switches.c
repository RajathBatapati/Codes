/* Controller code basic */
/* ESP-IDF */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "Wifi.h"
#include "mqtt_client.h"
#include "esp_netif.h"
#include "driver/gpio.h"  
#include <stdint.h>
#include <stddef.h>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "cJSON.h"

#define MQTT_TOPIC "device/update"

// Define GPIO pins for each device
#define GPIO_PIN_1  GPIO_NUM_12
#define GPIO_PIN_2  GPIO_NUM_14
#define GPIO_PIN_3  GPIO_NUM_25
#define GPIO_PIN_4  GPIO_NUM_26


static const char *TAG1 = "wifi station";
static const char *TAG = "mqtt_event_switch";
esp_mqtt_client_handle_t client;

// Define an array to store device states (0 for OFF, 1 for ON)
int device_state[5] = {0, 0, 0, 0};

//A structure to map device names to GPIO pins
typedef struct {
    const char *name;
    gpio_num_t gpio_pin;
} DeviceMapping;

//The array of device mappings
DeviceMapping device_mappings[] = {
    {"Meter 1", GPIO_PIN_1},
    {"Meter 2", GPIO_PIN_2},
    {"Meter 3", GPIO_PIN_3},
    {"Meter 4", GPIO_PIN_4},
    
    // we can Add more mappings as needed
};

// Function to update device state based on device name and state
void update_device_state(const char *device_name, int state) {
    for (int i = 0; i < sizeof(device_mappings) / sizeof(device_mappings[0]); i++) {
        if (strcmp(device_name, device_mappings[i].name) == 0) {
            device_state[i] = state;
            break;
        }
    }
}

// Function to process MQTT message
void process_mqtt_message(const char *mqtt_message) {
    cJSON *root = cJSON_Parse(mqtt_message);
    if (root == NULL) {
        printf("Failed to parse MQTT message\n");
        return;
    }

    cJSON *device = cJSON_GetObjectItem(root, "device");
    cJSON *value = cJSON_GetObjectItem(root, "value");

    if (device == NULL || value == NULL) {
        printf("Invalid MQTT message format\n");
        cJSON_Delete(root);
        return;
    }

    int gpio_pin = -1;

    // Map device name to GPIO pin
    for (int i = 0; i < sizeof(device_mappings) / sizeof(device_mappings[0]); i++) {
        if (strcmp(device->valuestring, device_mappings[i].name) == 0) {
            gpio_pin = device_mappings[i].gpio_pin;
            break;
        }
    }

    if (gpio_pin != -1) {
        int value_int;
        if (cJSON_IsString(value)) {
            // Convert the string value to an integer only if it's a string
            value_int = atoi(value->valuestring);
        } else if (cJSON_IsNumber(value)) {
            // Use the value as is if it's already a number
            value_int = value->valueint;
        } else {
            printf("Invalid value format\n");
            cJSON_Delete(root);
            return;
        }

        gpio_set_level(gpio_pin, value_int);
        printf("Set %s to %d\n", device->valuestring, value_int);
        update_device_state(device->valuestring, value_int);

        // Publish status message for the changed device
        char status_message[50];
        snprintf(status_message, sizeof(status_message), "{\"device\":\"%s\",\"value\":%d}", device->valuestring, value_int);
        esp_mqtt_client_publish(client, "device/response", status_message, 0, 1, 0);

    } else {
        printf("Invalid device name: %s\n", device->valuestring);
    }

    cJSON_Delete(root);
}

// MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, MQTT_TOPIC, 1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        process_mqtt_message(event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

// MQTT initialization
static void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://13.233.104.64:1883",
        .credentials.username = "rajath",
        .credentials.authentication.password = "rajath3345",
        .session.keepalive = 600,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

// Main function
void app_main(void) {
    // GPIO configuration
    gpio_config_t io_config;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.pin_bit_mask = (1ULL << GPIO_PIN_1) | (1ULL << GPIO_PIN_2) |
                             (1ULL << GPIO_PIN_3) | (1ULL << GPIO_PIN_4) ;
    gpio_config(&io_config);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG1, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    mqtt_app_start();
}
