#include <executorch/runtime/platform/runtime.h>
#include <executorch/runtime/executor/method.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/runtime/core/exec_aten/exec_aten.h>
#include <executorch/runtime/core/data_loader.h>
#include <executorch/runtime/core/memory_allocator.h>
#include <executorch/runtime/core/evalue.h>
#include <executorch/extension/tensor/tensor.h>

#include <iostream>
#include <vector>
#include <memory>
#include <fstream>

using namespace torch::executor;
using namespace executorch::extension;
using torch::executor::Error;
using torch::executor::Result;

// Simple file data loader implementation
class FileDataLoader : public DataLoader {
public:
    explicit FileDataLoader(const char* file_path) {
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
        
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        data_.resize(file_size);
        file.read(reinterpret_cast<char*>(data_.data()), file_size);
        file.close();
    }

    Result<FreeableBuffer> load(size_t offset, size_t size, const DataLoader::SegmentInfo& /* segment_info */) const override {
        if (offset + size > data_.size()) {
            return Error::InvalidArgument;
        }
        
        // Return a view into our data
        return FreeableBuffer(data_.data() + offset, size, /*free_fn=*/nullptr);
    }

    Result<size_t> size() const override {
        return data_.size();
    }

private:
    mutable std::vector<uint8_t> data_;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./execu_runner <model.pte>\n";
        return 1;
    }

    const char* model_path = argv[1];

    // Initialize runtime
    runtime_init();

    // Load the model file manually
    std::unique_ptr<FileDataLoader> loader;
    try {
        loader = std::make_unique<FileDataLoader>(model_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load file: " << e.what() << "\n";
        return 1;
    }

    std::cout << "File loaded successfully, size: " << loader->size().get() << " bytes\n";

    // Check if this looks like a valid ExecuTorch file
    Result<size_t> file_size = loader->size();
    if (!file_size.ok() || file_size.get() < Program::kMinHeadBytes) {
        std::cerr << "File too small or invalid\n";
        return 1;
    }

    // Create program
    Result<Program> program = Program::load(loader.get());
    if (!program.ok()) {
        std::cerr << "Failed to load program: " << static_cast<int>(program.error()) << "\n";
        return 1;
    }

    std::cout << "Model loaded: " << model_path << "\n";
    std::cout << "Number of methods: " << program.get().num_methods() << "\n";

    // List available methods
    for (size_t i = 0; i < program.get().num_methods(); ++i) {
        Result<const char*> method_name_result = program.get().get_method_name(i);
        if (method_name_result.ok()) {
            std::cout << "Method " << i << ": " << method_name_result.get() << "\n";
        }
    }

    // Get the method (typically "forward") - need to provide memory manager
    const char* method_name = "forward";
    
    // Check if method exists by getting metadata first
    Result<MethodMeta> method_meta_result = program.get().method_meta(method_name);
    if (!method_meta_result.ok()) {
        std::cerr << "Method '" << method_name << "' not found in program\n";
        return 1;
    }
    
    MethodMeta method_meta = method_meta_result.get();
    std::cout << "Method metadata loaded successfully\n";
    std::cout << "Number of inputs: " << method_meta.num_inputs() << "\n";
    std::cout << "Number of outputs: " << method_meta.num_outputs() << "\n";
    std::cout << "Number of memory planned buffers: " << method_meta.num_memory_planned_buffers() << "\n";

    // Create memory allocators with more generous sizes
    std::vector<uint8_t> method_allocator_pool(4 * 1024 * 1024); // 4MB pool
    MemoryAllocator method_allocator(
        method_allocator_pool.size(),
        method_allocator_pool.data()
    );
    
    // Check if we need planned memory buffers
    HierarchicalAllocator* planned_memory = nullptr;
    std::vector<std::vector<uint8_t>> planned_buffers;
    
    if (method_meta.num_memory_planned_buffers() > 0) {
        std::cout << "Setting up planned memory buffers:\n";
        planned_buffers.reserve(method_meta.num_memory_planned_buffers());
        
        for (size_t i = 0; i < method_meta.num_memory_planned_buffers(); ++i) {
            Result<int64_t> buffer_size = method_meta.memory_planned_buffer_size(i);
            if (!buffer_size.ok()) {
                std::cerr << "Failed to get buffer size for index " << i << "\n";
                return 1;
            }
            std::cout << "Buffer " << i << " size: " << buffer_size.get() << " bytes\n";
            planned_buffers.emplace_back(buffer_size.get());
        }
        
        // Create spans for the hierarchical allocator
        std::vector<Span<uint8_t>> buffer_spans;
        for (auto& buffer : planned_buffers) {
            buffer_spans.emplace_back(buffer.data(), buffer.size());
        }
        
        planned_memory = new HierarchicalAllocator(
            Span<Span<uint8_t>>(buffer_spans.data(), buffer_spans.size())
        );
    }
    
    // Create memory manager
    MemoryManager memory_manager(&method_allocator, planned_memory);
    
    std::cout << "Loading method...\n";
    Result<Method> method = program.get().load_method(method_name, &memory_manager);
    if (!method.ok()) {
        std::cerr << "Failed to load method '" << method_name << "': " << static_cast<int>(method.error()) << "\n";
        if (planned_memory) delete planned_memory;
        return 1;
    }

    std::cout << "Method '" << method_name << "' loaded successfully\n";

    // Get method metadata (reuse the existing method_meta from earlier)
    
    // Print input information
    size_t num_inputs = method_meta.num_inputs();
    std::cout << "Number of inputs: " << num_inputs << "\n";

    std::vector<std::vector<float>> input_data_storage; // Keep float data alive
    std::vector<std::vector<int64_t>> input_int_data_storage; // Keep int64 data alive
    std::vector<TensorPtr> input_tensors; // Keep TensorPtr alive
    
    for (size_t i = 0; i < num_inputs; ++i) {
        Result<TensorInfo> input_info_result = method_meta.input_tensor_meta(i);
        if (!input_info_result.ok()) {
            std::cerr << "Failed to get input tensor info for index " << i << "\n";
            return 1;
        }
        
        TensorInfo input_info = input_info_result.get();
        
        std::cout << "Input " << i << ": ";
        std::cout << "dtype=" << static_cast<int>(input_info.scalar_type()) << " ";
        std::cout << "dims=[";
        for (int32_t dim : input_info.sizes()) {
            std::cout << dim << " ";
        }
        std::cout << "]\n";

        // Calculate total number of elements
        size_t numel = 1;
        for (int32_t dim : input_info.sizes()) {
            numel *= dim;
        }

        // Create dummy input data - use appropriate data types
        if (input_info.scalar_type() == exec_aten::ScalarType::Long) {
            // For Long/Int64 inputs (like token IDs)
            input_int_data_storage.emplace_back(numel, 1);  // Fill with 1s as dummy tokens
            
            // Use from_blob for non-owning reference to int64 data
            std::vector<int32_t> shape_vec(input_info.sizes().begin(), input_info.sizes().end());
            auto input_tensor_ptr = from_blob(
                input_int_data_storage.back().data(),
                shape_vec,
                input_info.scalar_type()
            );
            
            input_tensors.push_back(input_tensor_ptr);
            
            // Set the input using the Method API
            Error set_status = method.get().set_input(EValue(*input_tensor_ptr), i);
            if (set_status != Error::Ok) {
                std::cerr << "Failed to set input " << i << ": " << static_cast<int>(set_status) << "\n";
                return 1;
            }
        } else {
            // For Float inputs (like attention masks converted to float)
            input_data_storage.emplace_back(numel, 1.0f);
            
            // Create tensor using from_blob for non-owning reference
            std::vector<int32_t> shape_vec(input_info.sizes().begin(), input_info.sizes().end());
            auto input_tensor_ptr = from_blob(
                input_data_storage.back().data(),
                shape_vec,
                input_info.scalar_type()
            );
            
            input_tensors.push_back(input_tensor_ptr);
            
            // Set the input using the Method API
            Error set_status = method.get().set_input(EValue(*input_tensor_ptr), i);
            if (set_status != Error::Ok) {
                std::cerr << "Failed to set input " << i << ": " << static_cast<int>(set_status) << "\n";
                return 1;
            }
        }
    }

    std::cout << "Number of outputs: " << method_meta.num_outputs() << "\n";

    std::cout << "Executing method...\n";
    // Execute the method
    Error status = method.get().execute();

    if (status != Error::Ok) {
        std::cerr << "Failed to execute method: " << static_cast<int>(status) << "\n";
        
        // Print more detailed error information
        if (status == Error::DelegateInvalidHandle) {
            std::cerr << "Error: DelegateInvalidHandle - This suggests the model uses unsupported operators or delegates\n";
        }
        
        if (planned_memory) delete planned_memory;
        return 1;
    }

    std::cout << "Inference succeeded!\n";

    // Get outputs using the Method API
    std::vector<EValue> outputs(method_meta.num_outputs());
    Error get_status = method.get().get_outputs(outputs.data(), outputs.size());
    if (get_status != Error::Ok) {
        std::cerr << "Failed to get outputs: " << static_cast<int>(get_status) << "\n";
        return 1;
    }

    // Print output information
    for (size_t i = 0; i < outputs.size(); ++i) {
        if (outputs[i].isTensor()) {
            exec_aten::Tensor output_tensor = outputs[i].toTensor();
            std::cout << "Output " << i << " dims=[";
            for (int j = 0; j < output_tensor.dim(); ++j) {
                std::cout << output_tensor.size(j) << " ";
            }
            std::cout << "]\n";
            
            // Print first few values (assuming float32)
            if (output_tensor.scalar_type() == exec_aten::ScalarType::Float) {
                const float* output_data = output_tensor.const_data_ptr<float>();
                std::cout << "First few values: ";
                size_t print_count = std::min(static_cast<size_t>(5), static_cast<size_t>(output_tensor.numel()));
                for (size_t j = 0; j < print_count; ++j) {
                    std::cout << output_data[j] << " ";
                }
                std::cout << "\n";
            }
        }
    }

    // Clean up
    if (planned_memory) delete planned_memory;

    return 0;
}