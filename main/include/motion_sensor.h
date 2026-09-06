#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

/*
 * IMotionSensor -- interfaz para cualquier sensor capaz de
 * detectar movimiento y generar una interrupcion de hardware
 * cuando lo hace. La capa de aplicacion (main.cpp) no necesita
 * saber si el sensor real es un MPU6050, otro IMU, o incluso
 * un sensor simulado para pruebas -- solo conoce este contrato.
 */
class IMotionSensor {
public:
    virtual ~IMotionSensor() = default;

    /* Inicializa el sensor (bus I2C, registros de configuracion,
     * umbral de deteccion de movimiento). Retorna false si el
     * sensor no responde o la configuracion falla. */
    virtual bool init() = 0;

    /* Arma la deteccion de movimiento: configura el sensor para
     * que genere una señal de interrupcion (pin INT) la proxima
     * vez que detecte movimiento por encima del umbral. */
    virtual void arm_motion_interrupt() = 0;

    /* En que pin GPIO del ESP32 esta conectado el pin de
     * interrupcion de este sensor -- lo necesita PowerManager
     * para configurar el wake source correcto. */
    virtual int get_interrupt_gpio() const = 0;
};

#endif /* MOTION_SENSOR_H */