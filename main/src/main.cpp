#include <stdio.h>
#include "driver/i2c.h"

#include "motion_sensor.h"
#include "adxl345_sensor.h"
#include "power_manager.h"
#include "esp32_power_manager.h"
#include "inference_engine.h"
#include "mnist_inference_engine.h"

extern const signed char test_image_data[];
extern const int test_image_label;

/* Pin del ESP32 donde esta conectado fisicamente INT1
 * del ADXL345 (ajustar segun tu cableado real). */
constexpr gpio_num_t kMotionInterruptPin = GPIO_NUM_27;

constexpr gpio_num_t kI2cSdaPin = GPIO_NUM_21;
constexpr gpio_num_t kI2cSclPin = GPIO_NUM_22;

static bool init_i2c_bus()
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = kI2cSdaPin;
    conf.scl_io_num = kI2cSclPin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;

    i2c_param_config(I2C_NUM_0, &conf);
    esp_err_t err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

    return err == ESP_OK;
}

extern "C" void app_main(void)
{
    printf("=== Motion-Triggered Inference (ESP32) ===\n");

    if (!init_i2c_bus()) {
        printf("I2C bus initialization failed\n");
        return;
    }

    /* Unico cambio real respecto a la version anterior:
     * ADXL345Sensor en vez de MPU6050Sensor. Todo lo demas
     * (PowerManager, InferenceEngine, la logica de app_main)
     * permanece IDENTICO -- exactamente el beneficio de
     * diseño esperado al usar interfaces (IMotionSensor). */
    IMotionSensor* motion_sensor =
        new ADXL345Sensor(I2C_NUM_0, kMotionInterruptPin);
    IPowerManager* power_manager = new ESP32PowerManager();
    IInferenceEngine* inference_engine = new MnistInferenceEngine();

    WakeReason reason = power_manager->get_wake_reason();

    motion_sensor->init();
    inference_engine->init();

    if (reason == WakeReason::POWER_ON) {
        printf("First boot -- system ready\n");

    } else if (reason == WakeReason::EXT_INTERRUPT) {
        printf("Woke up due to motion detected\n");

        InferenceResult result = inference_engine->run(
            test_image_data, 28 * 28);

        if (result.success) {
            printf("Prediction: %d (confidence %.2f%%)\n",
                   result.predicted_class, result.confidence * 100.0f);
            printf("Expected: %d -- %s\n", test_image_label,
                   result.predicted_class == test_image_label
                       ? "CORRECT" : "INCORRECT");
        }

    } else {
        printf("Unexpected wake reason: %d\n", (int)reason);
    }

    motion_sensor->arm_motion_interrupt();
    power_manager->configure_wake_source(
        (gpio_num_t)motion_sensor->get_interrupt_gpio(), /* level = */ 1);

    printf("Going to Deep Sleep, waiting for motion...\n");
    power_manager->enter_deep_sleep();
}