#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "driver/gpio.h"

enum class WakeReason {
    POWER_ON,
    EXT_INTERRUPT,
    TIMER,
    UNKNOWN
};

class IPowerManager {
public:
    virtual ~IPowerManager() = default;
    virtual void configure_wake_source(gpio_num_t pin, int level) = 0;
    virtual void enter_deep_sleep() = 0;
    virtual WakeReason get_wake_reason() = 0;
};

#endif /* POWER_MANAGER_H */