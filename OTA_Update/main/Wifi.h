/**
 * @file wifi.h
 * @brief Function declarations for Wi-Fi initialization and event handling.
 */

#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

/** @brief Event bit for Wi-Fi connected status */
#define WIFI_CONNECTED_BIT BIT0

/** @brief Event bit for Wi-Fi failure status */
#define WIFI_FAIL_BIT      BIT1

/** @brief Wi-Fi event group handle */
extern EventGroupHandle_t s_wifi_event_group;

/**
 * @brief Event handler for Wi-Fi events.
 *
 * This function handles Wi-Fi events and sets the appropriate bits in the event group.
 *
 * @param arg User-defined argument.
 * @param event_base Event base for the Wi-Fi event.
 * @param event_id Event ID.
 * @param event_data Event data.
 */
void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/**
 * @brief Initialize Wi-Fi in station mode.
 *
 * This function initializes the Wi-Fi in station mode and connects to the configured AP.
 */
void wifi_init_sta(void);

#endif /* WIFI_H */