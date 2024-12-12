/**
 * @file mqtt.c
 * @brief MQTT Client implementation for handling registration and OTA updates.
 *
 * This file contains the implementation of the MQTT client functions, including event handlers,
 * NVS storage for credentials, and initialization functions for both standard and OTA operations.
 */

#include "mqtt.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "cJSON.h"

/** @brief Tag for logging messages. */
#define TAG "MQTT"

/** @brief NVS namespace for storing credentials. */
#define NVS_NAMESPACE "storage"

/** @brief NVS key for storing the username. */
#define NVS_USERNAME_KEY "username"

/** @brief NVS key for storing the password. */
#define NVS_PASSWORD_KEY "password"

/** @brief NVS key for storing the client ID. */
#define NVS_CLIENTID_KEY "clientid"

/** @brief Handle for the primary MQTT client. */
static esp_mqtt_client_handle_t client1;

/** @brief Handle for the secondary MQTT client used for OTA updates. */
esp_mqtt_client_handle_t client2;

/** @brief Buffer to store the new username. */
static char new_username[32];

/** @brief Buffer to store the new password. */
static char new_password[32];

/** @brief Buffer to store the new client ID. */
static char new_client_id[64];

/** @brief Buffer to store the device ID. */
static char device_id[64];

/** @brief Buffer to store the OTA topic. */
char ota_topic[64 + sizeof(MQTT_OTA_TOPIC_PREFIX)];

/** @brief Buffer to store the status topic. */
char status_topic[64 + sizeof(MQTT_STATUS_TOPIC_PREFIX)];

/** @brief Buffer to store the OTA firmware URL. */
char ota_firmware_url[256];

/** @brief Flag to indicate if an OTA message has been received. */
bool ota_message_received = false;

/**
 * @brief Logs an error message if the error code is non-zero.
 *
 * @param[in] message The error message to log.
 * @param[in] error_code The error code to check.
 */
static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/**
 * @brief Stores MQTT credentials in NVS.
 *
 * This function stores the new username, password, and client ID in NVS.
 */
static void store_credentials_in_nvs() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(nvs_handle, NVS_USERNAME_KEY, new_username);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error storing username in NVS: %s", esp_err_to_name(err));
    }

    err = nvs_set_str(nvs_handle, NVS_PASSWORD_KEY, new_password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error storing password in NVS: %s", esp_err_to_name(err));
    }

    err = nvs_set_str(nvs_handle, NVS_CLIENTID_KEY, new_client_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error storing client ID in NVS: %s", esp_err_to_name(err));
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS storage: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
}

/**
 * @brief Retrieves MQTT credentials from NVS.
 *
 * This function retrieves the stored username, password, and client ID from NVS.
 */
static void retrieve_credentials_from_nvs() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }

    size_t required_size;

    // Retrieve username
    err = nvs_get_str(nvs_handle, NVS_USERNAME_KEY, NULL, &required_size);
    if (err == ESP_OK) {
        nvs_get_str(nvs_handle, NVS_USERNAME_KEY, new_username, &required_size);
    } else {
        ESP_LOGE(TAG, "Error retrieving username from NVS: %s", esp_err_to_name(err));
    }

    // Retrieve password
    err = nvs_get_str(nvs_handle, NVS_PASSWORD_KEY, NULL, &required_size);
    if (err == ESP_OK) {
        nvs_get_str(nvs_handle, NVS_PASSWORD_KEY, new_password, &required_size);
    } else {
        ESP_LOGE(TAG, "Error retrieving password from NVS: %s", esp_err_to_name(err));
    }

    // Retrieve client ID
    err = nvs_get_str(nvs_handle, NVS_CLIENTID_KEY, NULL, &required_size);
    if (err == ESP_OK) {
        nvs_get_str(nvs_handle, NVS_CLIENTID_KEY, new_client_id, &required_size);
    } else {
        ESP_LOGE(TAG, "Error retrieving client ID from NVS: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
}

/**
 * @brief Event handler for MQTT events.
 *
 * This function handles various MQTT events such as connection, disconnection, subscription,
 * message publication, and message reception.
 *
 * @param[in] handler_args Arguments for the handler.
 * @param[in] base Event base.
 * @param[in] event_id Event ID.
 * @param[in] event_data Event data.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client1 = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(client1, MQTT_REGISTER_TOPIC, 1);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            const char* json_message = "{\n"
                                       "    \"action\": \"add\",\n"
                                       "    \"payload\": {\n"
                                       "        \"mac_address\": \"FD:D8:27:19:JD\",\n"
                                       "        \"firmwareVersion\": \"v1.0.1\"\n"
                                       "    }\n"
                                       "}";    
            esp_mqtt_client_publish(client1, MQTT_REGISTER_TOPIC, json_message, 0, 1, 0);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            esp_mqtt_client_disconnect(client1);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            cJSON *json = cJSON_Parse(event->data);
            if (json != NULL) {
                cJSON *success = cJSON_GetObjectItem(json, "success");
                if (cJSON_IsBool(success) && success->valueint == 1) {
                    cJSON *deviceID = cJSON_GetObjectItem(json, "deviceID");
                    cJSON *user = cJSON_GetObjectItem(json, "user");
                    cJSON *pass = cJSON_GetObjectItem(json, "pass");
                    cJSON *clientID = cJSON_GetObjectItem(json, "clientID");
                    if (cJSON_IsString(deviceID) && cJSON_IsString(user) &&
                        cJSON_IsString(pass) && cJSON_IsString(clientID)) {
                        strncpy(new_username, user->valuestring, sizeof(new_username) - 1);
                        strncpy(new_password, pass->valuestring, sizeof(new_password) - 1);
                        strncpy(new_client_id, clientID->valuestring, sizeof(new_client_id) - 1);
                        strncpy(device_id, deviceID->valuestring, sizeof(device_id) - 1);
                        new_username[sizeof(new_username) - 1] = '\0';
                        new_password[sizeof(new_password) - 1] = '\0';
                        new_client_id[sizeof(new_client_id) - 1] = '\0';
                        device_id[sizeof(device_id) - 1] = '\0';
                        ESP_LOGI(TAG, "User: %s", new_username);
                        ESP_LOGI(TAG, "Pass: %s", new_password);
                        ESP_LOGI(TAG, "Client ID: %s", new_client_id);
                        ESP_LOGI(TAG, "Device ID: %s", device_id);
                        store_credentials_in_nvs();
                        esp_mqtt_client_unsubscribe(client1, MQTT_REGISTER_TOPIC);
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                    }
                }
                cJSON_Delete(json);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

/**
 * @brief Initialize and start the MQTT client.
 *
 * This function initializes the MQTT client with the configured broker URI,
 * client ID, username, and password, and starts the client.
 */
void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg_1 = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
        .credentials.client_id = MQTT_CLIENT_ID,
        .network.disable_auto_reconnect = true,
    };

    client1 = esp_mqtt_client_init(&mqtt_cfg_1);
    esp_mqtt_client_register_event(client1, ESP_EVENT_ANY_ID, mqtt_event_handler, client1);
    esp_mqtt_client_start(client1);
}

/**
 * @brief Event handler for MQTT OTA events.
 *
 * This function handles various MQTT events specific to OTA updates such as connection,
 * disconnection, subscription, message publication, and message reception.
 *
 * @param[in] handler_args Arguments for the handler.
 * @param[in] base Event base.
 * @param[in] event_id Event ID.
 * @param[in] event_data Event data.
 */
static void mqtt_ota_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client2 = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            snprintf(ota_topic, sizeof(ota_topic), "/%s%s", device_id, MQTT_OTA_TOPIC_PREFIX);
            snprintf(status_topic, sizeof(status_topic), "/%s%s", device_id, MQTT_STATUS_TOPIC_PREFIX);
            esp_mqtt_client_subscribe(client2, status_topic, 0);
            esp_mqtt_client_subscribe(client2, ota_topic, 0);
            ESP_LOGI(TAG, "Subscribed to OTA topic: %s", ota_topic);
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

            cJSON *json = cJSON_Parse(event->data);
            if (json != NULL) {
                cJSON *url = cJSON_GetObjectItem(json, "url");
                if (cJSON_IsString(url)) {
                    strncpy(ota_firmware_url, url->valuestring, sizeof(ota_firmware_url) - 1);
                    ota_firmware_url[sizeof(ota_firmware_url) - 1] = '\0';
                    ESP_LOGI(TAG, "Firmware URL: %s", ota_firmware_url);
                    ota_message_received = true;
                    // Send confirmation message
                    cJSON *confirmation = cJSON_CreateObject();
                    cJSON_AddStringToObject(confirmation, "action", "set");
                    cJSON *payload = cJSON_CreateObject();
                    cJSON_AddItemToObject(confirmation, "payload", payload);
                    cJSON_AddStringToObject(payload, "status", "received");
                    cJSON_AddStringToObject(payload, "url", ota_firmware_url);

                    char *confirmation_str = cJSON_PrintUnformatted(confirmation);
                    esp_mqtt_client_publish(client2, status_topic, confirmation_str, 0, 1, 0);

                    cJSON_free(confirmation_str);
                    cJSON_Delete(confirmation);
                }
                cJSON_Delete(json);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

/**
 * @brief Initialize and start the MQTT client for OTA updates.
 *
 * This function stops the primary MQTT client, retrieves credentials from NVS,
 * initializes the secondary MQTT client for OTA updates, and starts the client.
 */
void mqtt_ota_app_start(void) {
    esp_mqtt_client_stop(client1);

    retrieve_credentials_from_nvs();

    esp_mqtt_client_config_t mqtt_cfg_2 = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = new_username,
        .credentials.authentication.password = new_password,
        .credentials.client_id = new_client_id,
        .network.disable_auto_reconnect = true,
    };

    client2 = esp_mqtt_client_init(&mqtt_cfg_2);
    esp_mqtt_client_register_event(client2, ESP_EVENT_ANY_ID, mqtt_ota_event_handler, client2);
    esp_mqtt_client_start(client2);
}
