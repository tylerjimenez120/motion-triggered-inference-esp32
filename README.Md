# Motion-Triggered Inference on ESP32

An ESP32 that spends nearly all its time in Deep Sleep (~10µA), wakes up only when an accelerometer detects motion, runs a real TFLite Micro inference, and goes back to sleep — repeating indefinitely, entirely on hardware interrupt, with zero polling.

---

## What this is

Deep Sleep is the core subject of this project: the ~1000x current gap between Active mode (tens of mA) and Deep Sleep (~10µA) is what makes battery-powered edge devices viable. The mechanism that makes it useful is external wake-up — a sensor drives a GPIO pin, a small RTC-domain comparator circuit watches that pin while the rest of the chip is powered down, and only then does the CPU turn back on.

The inference workload (a quantized MNIST classifier, built in an earlier project) is the payload that runs once awake — a stand-in for whatever real task a battery-powered sensor node needs to do on a trigger.

```
Deep Sleep (~10µA)
      │  motion detected → GPIO interrupt (EXT0 wake source)
      ▼
Reboot from scratch (RAM cleared, only ~8KB RTC memory survives)
      │
      ▼
Re-init sensor + model → run inference → report result
      │
      ▼
Re-arm interrupt → back to Deep Sleep
```

---

## Architecture

Three abstract interfaces, each with one concrete implementation, so a hardware swap touches the fewest files possible:

```
IMotionSensor     → ADXL345Sensor      (I2C, activity interrupt)
IPowerManager     → ESP32PowerManager  (Deep Sleep, EXT0 wake source)
IInferenceEngine  → MnistInferenceEngine (TFLite Micro)
```

`main.cpp` only ever talks to the interfaces — it has no idea whether the concrete motion sensor is an MPU6050 or an ADXL345.

---

## The sensor swap

The first accelerometer wired in was an MPU6050. Its motion-detection interrupt never fired, even with confirmed clean I2C communication and correct raw acceleration readings on movement. Multiple independent technical sources describe the exact same failure with InvenSense's motion-detect silicon (MPU6050, MPU9250, MPU9150) — a documented, long-standing issue, not a defect specific to one module.

Switched to an ADXL345 (different manufacturer, different design). Its Activity/Inactivity Detection fired immediately and reliably. Because the motion sensor sits behind an interface, the swap was:

1. Write `adxl345_sensor.h` / `.cpp` (new `IMotionSensor` implementation)
2. Change one line in `main.cpp`
3. Delete `mpu6050_sensor.h` / `.cpp`

`PowerManager` and `InferenceEngine` never moved.

---

## Wiring (ADXL345, GY-291 module)

```
VCC   → 3.3V
GND   → GND
CS    → 3.3V     (must be HIGH for I2C mode — do not leave floating)
SDO   → GND      (sets I2C address to 0x53)
SDA   → GPIO21
SCL   → GPIO22
INT1  → GPIO27   (wake source)
INT2  → not connected
```

---

## Repository structure

```
main/
├── include/
│   ├── motion_sensor.h            IMotionSensor interface
│   ├── adxl345_sensor.h           ADXL345 implementation
│   ├── power_manager.h            IPowerManager interface
│   ├── esp32_power_manager.h      ESP32 Deep Sleep implementation
│   ├── inference_engine.h         IInferenceEngine interface
│   └── mnist_inference_engine.h   TFLite Micro implementation
├── src/
│   ├── adxl345_sensor.cpp
│   ├── esp32_power_manager.cpp
│   ├── mnist_inference_engine.cpp
│   └── main.cpp                   Application orchestration
├── model_data.cc                  Quantized MNIST model (from an earlier project)
├── test_image_data.cc             One pre-quantized test digit
└── CMakeLists.txt

MODULE_3_1_6_MOTION_TRIGGERED.md   Full technical write-up (theory, debugging, results)
```

---

## Result

```
=== Motion-Triggered Inference (ESP32) ===
ADXL345 initialized correctly
Model initialized correctly
Tensor arena used: 7860 bytes
Woke up due to motion detected
Prediction: 7 (confidence 99.61%)
Expected: 7 -- CORRECT
Going to Deep Sleep, waiting for motion...
[DEEPSLEEP_RESET]
--- cycle repeats indefinitely ---
```

---

## Building

```bash
idf.py create-project motion_triggered_inference
cd motion_triggered_inference
idf.py add-dependency "espressif/esp-tflite-micro"
```

Copy `main/include/*` and `main/src/*` from this repo into your project's `main/` directory, matching the `CMakeLists.txt` source list, then:

```bash
idf.py build flash monitor
```

---

## Related work

The TFLite Micro model used here (training, quantization, and standalone ESP32 deployment) is covered in a separate project: **tinyml-mnist-esp32**.