#ifndef ADXL345_SENSOR_H
#define ADXL345_SENSOR_H

#include "motion_sensor.h"
#include "driver/i2c.h"
#include <cstdint>

/*
 * ADXL345Sensor -- implementacion concreta de IMotionSensor
 * para el chip ADXL345 (acelerometro de 3 ejes, Analog Devices).
 *
 * Se eligio sobre el MPU6050 despues de confirmar (con
 * evidencia real: lectura de aceleracion correcta, pero
 * pin INT nunca disparandose pese a movimiento fisico
 * confirmado) que el subsistema de motion-detection de la
 * familia InvenSense (MPU6050/9250/9150) es notoriamente
 * poco confiable y mal documentado -- problema reportado
 * de forma independiente por multiples fuentes tecnicas,
 * no exclusivo de un modulo/clon especifico.
 *
 * El ADXL345 (fabricante distinto, diseño de silicio
 * distinto) tiene su mecanismo de Activity/Inactivity
 * Detection bien documentado y con reputacion de
 * funcionar de forma confiable para wake-on-motion.
 */
class ADXL345Sensor : public IMotionSensor {
public:
    explicit ADXL345Sensor(i2c_port_t i2c_port, int interrupt_gpio);

    bool init() override;
    void arm_motion_interrupt() override;
    int get_interrupt_gpio() const override;

private:
    i2c_port_t i2c_port_;
    int interrupt_gpio_;

    /* Direccion I2C: 0x53 con SDO en HIGH (configuracion
     * por defecto en la mayoria de breakout boards). Si tu
     * modulo especifico tiene SDO a GND, seria 0x1D. */
    static constexpr uint8_t ADXL345_ADDR = 0x53;

    static constexpr uint8_t EXPECTED_DEVID = 0xE5;

    static constexpr uint8_t REG_DEVID          = 0x00;
    static constexpr uint8_t REG_THRESH_ACT     = 0x24;
    static constexpr uint8_t REG_ACT_INACT_CTL  = 0x27;
    static constexpr uint8_t REG_BW_RATE        = 0x2C;
    static constexpr uint8_t REG_POWER_CTL      = 0x2D;
    static constexpr uint8_t REG_INT_ENABLE     = 0x2E;
    static constexpr uint8_t REG_INT_MAP        = 0x2F;
    static constexpr uint8_t REG_INT_SOURCE     = 0x30;
    static constexpr uint8_t REG_DATA_FORMAT    = 0x31;
    static constexpr uint8_t REG_DATAX0         = 0x32;

    /* Bit "ACTIVITY" dentro de INT_ENABLE/INT_MAP/INT_SOURCE
     * (bit 4, ver Tabla de mapeo de interrupciones del
     * datasheet). */
    static constexpr uint8_t BIT_ACTIVITY = 0x10;

    bool write_register(uint8_t reg, uint8_t value);
    bool read_register(uint8_t reg, uint8_t* out_value);
};

#endif /* ADXL345_SENSOR_H */