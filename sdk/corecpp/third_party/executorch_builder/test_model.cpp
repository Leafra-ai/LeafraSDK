#include <executorch/runtime/executor/method.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/extension/data_loader/file_data_loader.h>
#include <executorch/extension/runner_util/inputs.h>
#include <executorch/runtime/platform/log.h>

#include <iostream>
#include <memory>

using namespace torch::executor;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <model.pte>" << std::endl;
        return 1;
    }

    const char* model_path = argv[1];
    
    ET_LOG(Info, "Loading model from: %s", model_path);
    
    // Load the model
    auto loader = std::make_unique<util::FileDataLoader>(model_path);
    auto program = Program::load(loader.get());
    if (!program.ok()) {
        ET_LOG(Error, "Failed to load program: %d", (int)program.error());
        return 1;
    }
    
    ET_LOG(Info, "Model loaded successfully");
    
    // Get method information
    auto method_meta = program->get_method_meta("forward");
    if (!method_meta.ok()) {
        ET_LOG(Error, "Failed to get method meta: %d", (int)method_meta.error());
        return 1;
    }
    
    ET_LOG(Info, "Method 'forward' found");
    ET_LOG(Info, "Number of inputs: %zu", method_meta->num_inputs());
    ET_LOG(Info, "Number of outputs: %zu", method_meta->num_outputs());
    
    // Print input tensor information
    for (size_t i = 0; i < method_meta->num_inputs(); i++) {
        auto input_meta = method_meta->input_tensor_meta(i);
        if (input_meta.ok()) {
            ET_LOG(Info, "Input %zu: dtype=%d, numel=%zu", 
                   i, (int)input_meta->scalar_type(), input_meta->numel());
        }
    }
    
    // Print output tensor information  
    for (size_t i = 0; i < method_meta->num_outputs(); i++) {
        auto output_meta = method_meta->output_tensor_meta(i);
        if (output_meta.ok()) {
            ET_LOG(Info, "Output %zu: dtype=%d, numel=%zu", 
                   i, (int)output_meta->scalar_type(), output_meta->numel());
        }
    }
    
    ET_LOG(Info, "Model inspection completed successfully");
    ET_LOG(Info, "✅ Executorch prebuilt libraries are working correctly!");
    
    return 0;
} 