#ifndef INFERENCE_ENGINE_H
#define INFERENCE_ENGINE_H

#include <cstdint>
#include <cstddef>

struct InferenceResult {
    int predicted_class;
    float confidence;
    bool success;
};

class IInferenceEngine {
public:
    virtual ~IInferenceEngine() = default;
    virtual bool init() = 0;
    virtual InferenceResult run(const int8_t* input_data,
                                 size_t input_size) = 0;
};

#endif /* INFERENCE_ENGINE_H */