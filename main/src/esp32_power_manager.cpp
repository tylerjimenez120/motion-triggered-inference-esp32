#include "esp32_power_manager.h"
#include "esp_sleep.h"
#include "esp_log.h"

static const char* TAG = "PowerManager";

void ESP32PowerManager::configure_wake_source(gpio_num_t pin, int level)
{
    esp_sleep_enable_ext0_wakeup(pin, level);
    ESP_LOGI(TAG, "Wake source configured: GPIO %d, level %d", pin, level);
}

void ESP32PowerManager::enter_deep_sleep()
{
    ESP_LOGI(TAG, "Entering Deep Sleep now");
    esp_deep_sleep_start();
}

WakeReason ESP32PowerManager::get_wake_reason()
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    switch (cause) {
        case ESP_SLEEP_WAKEUP_EXT0:
            return WakeReason::EXT_INTERRUPT;
        case ESP_SLEEP_WAKEUP_TIMER:
            return WakeReason::TIMER;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            return WakeReason::POWER_ON;
        default:
            return WakeReason::UNKNOWN;
    }
}