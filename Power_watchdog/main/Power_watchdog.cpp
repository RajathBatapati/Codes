#include "power_watchdog.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <ctime>

#define TAG "GpioScheduler"

// Define GPIO pin selections
#define GPIO_INPUT_PIN_SEL  (1ULL << GPIO_NUM_7)   // Bit mask for GPIO 7
#define GPIO_OUTPUT_PIN_SEL (1ULL << GPIO_NUM_21)  // Bit mask for GPIO 21

// Global variable
bool g_CURRENT_STATUS = false;

static GpioScheduler gpioScheduler;

// Constructor: Configures GPIO and initializes variables
GpioScheduler::GpioScheduler() 
    : gpioReadTimer_(nullptr), pulseTimer_(nullptr), pulseOffTimer_(nullptr) {
    ESP_LOGI(TAG, "GpioScheduler constructor called. Initializing GPIO configuration.");
    configureGpio();
}

// Destructor: Clean up resources
GpioScheduler::~GpioScheduler() {
    ESP_LOGI(TAG, "GpioScheduler destructor called. Cleaning up timers.");
    if (gpioReadTimer_) {
        xTimerDelete(gpioReadTimer_, portMAX_DELAY);
    }
    if (pulseTimer_) {
        xTimerDelete(pulseTimer_, portMAX_DELAY);
    }
    if (pulseOffTimer_) {
        xTimerDelete(pulseOffTimer_, portMAX_DELAY);
    }
}

// Log error messages with a timestamp
void GpioScheduler::logError(const char *msg, ErrorCode code) const {
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGE(TAG, "[%s] ERROR %d: %s", timeStr, static_cast<int>(code), msg);
}

// Configure GPIO pins with proper error handling
void GpioScheduler::configureGpio() {
    ESP_LOGI(TAG, "Configuring GPIO 7 as input.");
    gpio_config_t ioConfInput = {
        .pin_bit_mask = GPIO_INPUT_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if (gpio_config(&ioConfInput) != ESP_OK) {
        logError("Failed to configure GPIO 7 as input", ErrorCode::ERR_GPIO_READ_FAILED);
        return;
    }

    ESP_LOGI(TAG, "Configuring GPIO 21 as output.");
    gpio_config_t ioConfOutput = {
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    if (gpio_config(&ioConfOutput) != ESP_OK) {
        logError("Failed to configure GPIO 21 as output", ErrorCode::ERR_GPIO_READ_FAILED);
        return;
    }
}

// Start the FreeRTOS timers with error handling
void GpioScheduler::startTimers() {
    ESP_LOGI(TAG, "Starting timers.");
    gpioReadTimer_ = xTimerCreate("gpio_read_timer", pdMS_TO_TICKS(40000), pdTRUE, this, readGpioTimerCallback);
    pulseTimer_ = xTimerCreate("pulse_timer", pdMS_TO_TICKS(40000), pdTRUE, this, pulseTimerCallback);
    pulseOffTimer_ = xTimerCreate("pulse_off_timer", pdMS_TO_TICKS(1000), pdFALSE, this, pulseOffTimerCallback);

    if (gpioReadTimer_ == nullptr || pulseTimer_ == nullptr || pulseOffTimer_ == nullptr) {
        logError("Failed to create timers", ErrorCode::ERR_TIMER_CREATE_FAILED);
        return;
    }

    ESP_LOGI(TAG, "Timers created successfully. Attempting to start timers.");
    if (xTimerStart(gpioReadTimer_, 0) != pdPASS) {
        logError("Failed to start GPIO read timer", ErrorCode::ERR_TIMER_START_FAILED);
    } else {
        ESP_LOGI(TAG, "GPIO read timer started successfully.");
    }

    if (xTimerStart(pulseTimer_, 0) != pdPASS) {
        logError("Failed to start pulse timer", ErrorCode::ERR_TIMER_START_FAILED);
    } else {
        ESP_LOGI(TAG, "Pulse timer started successfully.");
    }
}

// Callback for reading the GPIO state
void GpioScheduler::readGpioTimerCallback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "Entering readGpioTimerCallback.");
    GpioScheduler* scheduler = static_cast<GpioScheduler*>(pvTimerGetTimerID(xTimer));
    if (!scheduler) {
        ESP_LOGE(TAG, "Timer callback failed due to invalid scheduler instance");
        return;
    }

    // set current status to true if GPIO 7 is high, otherwise false
    g_CURRENT_STATUS = gpio_get_level(GPIO_NUM_7);
    ESP_LOGI(TAG, "GPIO 7 is %s", g_CURRENT_STATUS ? "HIGH" : "LOW");
}

// Callback for sending a pulse on GPIO21
void GpioScheduler::pulseTimerCallback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "Entering pulseTimerCallback.");
    GpioScheduler* scheduler = static_cast<GpioScheduler*>(pvTimerGetTimerID(xTimer));
    if (scheduler == nullptr) {
        ESP_LOGE(TAG, "Timer callback failed due to invalid scheduler instance");
        return;
    }

    static bool previousStatus = false;

    // Send a high pulse if current or previous status is true
    if (g_CURRENT_STATUS || previousStatus) {
        ESP_LOGI(TAG, "Condition met. Sending a high pulse on GPIO 21.");
        gpio_set_level(GPIO_NUM_21, 1);  // Turn on pulse
        // Starting one-shot timer to turn off the pulse after 1 second
        if (xTimerStart(scheduler->pulseOffTimer_, 0) != pdPASS) {
            scheduler->logError("Failed to start pulse off timer", ErrorCode::ERR_TIMER_START_FAILED);
        } else {
            ESP_LOGI(TAG, "Pulse off timer started successfully.");
        }
    } else {
        ESP_LOGI(TAG, "Condition not met. No pulse sent.");
    }

    // Update previousStatus to current state
    previousStatus = g_CURRENT_STATUS;
    ESP_LOGI(TAG, "Updated previousStatus to %s.", previousStatus ? "HIGH" : "LOW");
}

// One-shot timer callback to turn off the pulse
void GpioScheduler::pulseOffTimerCallback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "Entering pulseOffTimerCallback. Turning off pulse.");
    GpioScheduler* scheduler = static_cast<GpioScheduler*>(pvTimerGetTimerID(xTimer));
    if (scheduler == nullptr) {
        ESP_LOGE(TAG, "Pulse off timer callback failed due to invalid scheduler instance");
        return;
    }

    gpio_set_level(GPIO_NUM_21, 0);  // Turn off pulse
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "app_main started. Starting timers.");
    vTaskDelay(pdMS_TO_TICKS(10000));  // Optional delay before starting timers
    gpioScheduler.startTimers();
    ESP_LOGI(TAG, "Timers have been started.");
}
