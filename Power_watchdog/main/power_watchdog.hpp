#ifndef GPIO_SCHEDULER_H
#define GPIO_SCHEDULER_H

#include <ctime>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"

// GPIO Definitions
#define GPIO_INPUT_PIN_SEL   (1ULL << GPIO_NUM_7)   // GPIO7 as input pin
#define GPIO_OUTPUT_PIN_SEL  (1ULL << GPIO_NUM_21)  // GPIO21 as output pin

// Error Codes
enum class ErrorCode : uint8_t {
    ERR_GPIO_READ_FAILED = 1,
    ERR_TIMER_START_FAILED,
    ERR_TIMER_CREATE_FAILED,
};

// Global variable
extern bool g_CURRENT_STATUS;

// Class to manage GPIO and Timer callbacks
class GpioScheduler {
public:
    GpioScheduler();
    ~GpioScheduler();
    void logError(const char *msg, ErrorCode code) const;
    void startTimers();

private:
    TimerHandle_t gpioReadTimer_;
    TimerHandle_t pulseTimer_;
    TimerHandle_t pulseOffTimer_;

    void configureGpio();
    static void readGpioTimerCallback(TimerHandle_t xTimer);
    static void pulseTimerCallback(TimerHandle_t xTimer);
    static void pulseOffTimerCallback(TimerHandle_t xTimer);
};

#endif // GPIO_SCHEDULER_H
