#include "mnist_inference_engine.h"
#include <cstring>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/schema/schema_generated.h"

extern const unsigned char mnist_int8_tflite[];
extern const unsigned int mnist_int8_tflite_len;

namespace {

constexpr int kTensorArenaSize = 20 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input_tensor = nullptr;
TfLiteTensor* output_tensor = nullptr;

constexpr float kOutputScale = 0.00390625f;
constexpr int kOutputZeroPoint = -128;

}  // namespace

bool MnistInferenceEngine::init()
{
    model = tflite::GetModel(mnist_int8_tflite);

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Model version (%d) does not match "
                     "supported schema version (%d)",
                     model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }

    static tflite::MicroMutableOpResolver<5> resolver;
    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddReshape();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();

    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        MicroPrintf("AllocateTensors() failed");
        return false;
    }

    input_tensor = interpreter->input(0);
    output_tensor = interpreter->output(0);

    MicroPrintf("Model initialized correctly");
    MicroPrintf("Tensor arena used: %d bytes",
                interpreter->arena_used_bytes());

    return true;
}

InferenceResult MnistInferenceEngine::run(const int8_t* input_data,
                                            size_t input_size)
{
    InferenceResult result = {};
    result.success = false;

    memcpy(input_tensor->data.int8, input_data, input_size);

    if (interpreter->Invoke() != kTfLiteOk) {
        MicroPrintf("Invoke() failed");
        return result;
    }

    int predicted_digit = 0;
    float best_score = -1.0f;

    for (int i = 0; i < 10; i++) {
        int8_t raw = output_tensor->data.int8[i];
        float real_value = (raw - kOutputZeroPoint) * kOutputScale;

        if (real_value > best_score) {
            best_score = real_value;
            predicted_digit = i;
        }
    }

    result.predicted_class = predicted_digit;
    result.confidence = best_score;
    result.success = true;

    return result;
}