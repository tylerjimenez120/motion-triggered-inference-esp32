#include "adxl345_sensor.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "ADXL345";

ADXL345Sensor::ADXL345Sensor(i2c_port_t i2c_port, int interrupt_gpio)
    : i2c_port_(i2c_port), interrupt_gpio_(interrupt_gpio)
{
}

bool ADXL345Sensor::write_register(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
    esp_err_t err = i2c_master_write_to_device(
        i2c_port_, ADXL345_ADDR, buf, sizeof(buf),
        pdMS_TO_TICKS(100));
    return err == ESP_OK;
}

bool ADXL345Sensor::read_register(uint8_t reg, uint8_t* out_value)
{
    esp_err_t err = i2c_master_write_read_device(
        i2c_port_, ADXL345_ADDR, &reg, 1, out_value, 1,
        pdMS_TO_TICKS(100));
    return err == ESP_OK;
}

bool ADXL345Sensor::init()
{
    /* Verificar identidad del chip (DEVID debe ser 0xE5,
     * fijo por el fabricante -- a diferencia del MPU6050,
     * este valor SI esta consistentemente documentado y
     * confirmado en el datasheet oficial de Analog Devices). */
    uint8_t devid = 0;
    if (!read_register(REG_DEVID, &devid)) {
        ESP_LOGE(TAG, "I2C read failed");
        return false;
    }
    if (devid != EXPECTED_DEVID) {
        ESP_LOGE(TAG, "Unexpected DEVID: 0x%02X (expected 0x%02X)",
                 devid, EXPECTED_DEVID);
        return false;
    }

    /* Secuencia de configuracion confirmada por multiples
     * fuentes de referencia (comunidad STM32, i2cdevlib,
     * datasheet oficial): */

    /* Rango de medicion +-2g (suficiente para deteccion de
     * movimiento general, no necesita mayor rango). */
    write_register(REG_DATA_FORMAT, 0x00);

    /* Tasa de muestreo -- 100Hz es un valor estandar
     * razonable para deteccion de actividad. */
    write_register(REG_BW_RATE, 0x0A);

    /* Umbral de actividad. Escala: 62.5 mg/LSB (documentado).
     * Valor 20 = 20 * 62.5mg = 1250mg (1.25g) de umbral --
     * sensible a movimientos claros, sin disparar por ruido
     * minimo. Ajustable segun sensibilidad deseada. */
    write_register(REG_THRESH_ACT, 20);

    /* Habilita deteccion de actividad en los 3 ejes (X,Y,Z),
     * usando datos DC-coupled (bit 7 = 0) segun la
     * configuracion mas simple del datasheet. */
    write_register(REG_ACT_INACT_CTL, 0x70);

    /* Mapea el evento de ACTIVITY al pin INT1 (0 = INT1 en
     * cada bit correspondiente del registro INT_MAP). */
    write_register(REG_INT_MAP, 0x00);

    /* Habilita especificamente la interrupcion de ACTIVITY. */
    write_register(REG_INT_ENABLE, BIT_ACTIVITY);

    /* POWER_CTL: bit "Measure" (bit 3) en 1 -- saca el chip
     * de standby y lo pone en modo de medicion activa. Sin
     * esto, el ADXL345 no mide nada (viene en standby de
     * fabrica, analogo al modo sleep del MPU6050). */
    write_register(REG_POWER_CTL, 0x08);

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << interrupt_gpio_);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "ADXL345 initialized correctly");
    return true;
}

void ADXL345Sensor::arm_motion_interrupt()
{
    /* Leer INT_SOURCE limpia cualquier interrupcion pendiente
     * anterior -- deja el sensor listo para la proxima
     * deteccion (mismo patron que REG_INT_STATUS en el
     * MPU6050, pero aqui SI funciona de forma confiable). */
    uint8_t source;
    read_register(REG_INT_SOURCE, &source);
}

int ADXL345Sensor::get_interrupt_gpio() const
{
    return interrupt_gpio_;
}