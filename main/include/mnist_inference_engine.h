#ifndef MNIST_INFERENCE_ENGINE_H
#define MNIST_INFERENCE_ENGINE_H

#include "inference_engine.h"

class MnistInferenceEngine : public IInferenceEngine {
public:
    bool init() override;
    InferenceResult run(const int8_t* input_data,
                          size_t input_size) override;
};

#endif /* MNIST_INFERENCE_ENGINE_H */