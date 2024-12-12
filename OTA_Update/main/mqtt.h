/**
 * @file mqtt.h
 * @brief MQTT Client configuration and function declarations.
 *
 * This header file contains the declarations and configurations for the MQTT client,
 * including the broker URI, client credentials, and topics for registration, OTA updates,
 * and status reporting.
 */

#ifndef MQTT_H
#define MQTT_H

#include "mqtt_client.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

/** @brief URI of the MQTT broker. */
#define MQTT_BROKER_URI       "mqtt://0.tcp.in.ngrok.io:13073"

/** @brief Username for MQTT authentication. */
#define MQTT_USERNAME         "arnowa_device"

/** @brief Password for MQTT authentication. */
#define MQTT_PASSWORD         "device3309"

/** @brief Client ID for the MQTT connection. */
#define MQTT_CLIENT_ID        "clientEnd"

/** @brief Topic for device registration. */
#define MQTT_REGISTER_TOPIC   "/register"

/** @brief Prefix for OTA update topics. */
#define MQTT_OTA_TOPIC_PREFIX "/update"

/** @brief Prefix for OTA status update topics. */
#define MQTT_STATUS_TOPIC_PREFIX "/update/status"

/** @brief Buffer to store the OTA firmware URL. */
extern char ota_firmware_url[256];

/** @brief Buffer to store the full OTA topic. */
extern char ota_topic[64 + sizeof(MQTT_OTA_TOPIC_PREFIX)];

/** @brief Buffer to store the full status topic. */
extern char status_topic[64 + sizeof(MQTT_STATUS_TOPIC_PREFIX)];

/** @brief Handle for the MQTT client. */
extern esp_mqtt_client_handle_t client2;

/** @brief Flag to indicate if an OTA message has been received. */
extern bool ota_message_received;

/**
 * @brief Initialize and start the MQTT client.
 *
 * This function initializes the MQTT client with the configured broker URI,
 * client ID, username, and password, and starts the client.
 */
void mqtt_app_start(void);

/**
 * @brief Initialize and start the MQTT client for OTA updates.
 *
 * This function initializes the MQTT client specifically for handling OTA updates,
 * including subscribing to the OTA update topic.
 */
void mqtt_ota_app_start(void);

#endif // MQTT_H
