#ifndef ESP32_POWER_MANAGER_H
#define ESP32_POWER_MANAGER_H

#include "power_manager.h"

class ESP32PowerManager : public IPowerManager {
public:
    void configure_wake_source(gpio_num_t pin, int level) override;
    void enter_deep_sleep() override;
    WakeReason get_wake_reason() override;
};

#endif /* ESP32_POWER_MANAGER_H */