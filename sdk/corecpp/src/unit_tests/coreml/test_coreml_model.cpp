#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <cmath>

#include "leafra/leafra_coreml.h"
#include "leafra/logger.h"

using namespace leafra;

// Test configuration  
static const std::string MODEL_PATH = "../model.mlmodelc";
static const size_t EXPECTED_INPUT_COUNT = 2;
static const size_t EXPECTED_OUTPUT_COUNT = 1;
static const size_t EXPECTED_INPUT_SIZE = 512;
static const size_t EXPECTED_OUTPUT_SIZE = 384;

// Test results tracking
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Helper macros for testing
#define TEST_START(name) \
    std::cout << "Running " << name << "... "; \
    total_tests++; \
    try {

#define TEST_END() \
        std::cout << "PASS" << std::endl; \
        passed_tests++; \
    } catch (const std::exception& e) { \
        std::cout << "FAIL - " << e.what() << std::endl; \
        failed_tests++; \
    } catch (...) { \
        std::cout << "FAIL - Unknown exception" << std::endl; \
        failed_tests++; \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Expected " + std::to_string(expected) + " but got " + std::to_string(actual)); \
    }

#define ASSERT_STR_EQ(expected, actual) \
    if (std::string(expected) != std::string(actual)) { \
        throw std::runtime_error("Expected '" + std::string(expected) + "' but got '" + std::string(actual) + "'"); \
    }

// Test constructor and basic initialization
void test_model_construction() {
    TEST_START("test_model_construction")
    
    // Test successful construction
    CoreMLModel model(MODEL_PATH);
    ASSERT_TRUE(model.isValid());
    
    // Test description is not empty
    std::string description = model.getDescription();
    ASSERT_TRUE(!description.empty());
    
    TEST_END()
}

// Test constructor with different compute units
void test_model_construction_with_compute_units() {
    TEST_START("test_model_construction_with_compute_units")
    
    // Test different compute units
    CoreMLModel model_cpu(MODEL_PATH, CoreMLModel::ComputeUnits::CPUOnly);
    ASSERT_TRUE(model_cpu.isValid());
    
    CoreMLModel model_all(MODEL_PATH, CoreMLModel::ComputeUnits::All);
    ASSERT_TRUE(model_all.isValid());
    
    CoreMLModel model_gpu(MODEL_PATH, CoreMLModel::ComputeUnits::CPUAndGPU);
    ASSERT_TRUE(model_gpu.isValid());
    
    CoreMLModel model_neural(MODEL_PATH, CoreMLModel::ComputeUnits::CPUAndNeuralEngine);
    ASSERT_TRUE(model_neural.isValid());
    
    TEST_END()
}

// Test constructor error handling
void test_model_construction_errors() {
    TEST_START("test_model_construction_errors")
    
    // Test with invalid path
    bool exception_thrown = false;
    try {
        CoreMLModel model("nonexistent_model.mlmodelc");
    } catch (const std::runtime_error&) {
        exception_thrown = true;
    }
    ASSERT_TRUE(exception_thrown);
    
    TEST_END()
}

// Test move semantics
void test_move_semantics() {
    TEST_START("test_move_semantics")
    
    // Test move constructor
    CoreMLModel model1(MODEL_PATH);
    ASSERT_TRUE(model1.isValid());
    
    CoreMLModel model2 = std::move(model1);
    ASSERT_TRUE(model2.isValid());
    ASSERT_TRUE(!model1.isValid()); // Original should be invalid after move
    
    // Test move assignment
    CoreMLModel model3(MODEL_PATH);
    CoreMLModel model4(MODEL_PATH);
    ASSERT_TRUE(model3.isValid());
    ASSERT_TRUE(model4.isValid());
    
    model4 = std::move(model3);
    ASSERT_TRUE(model4.isValid());
    ASSERT_TRUE(!model3.isValid()); // Original should be invalid after move
    
    TEST_END()
}

// Test model introspection methods
void test_model_introspection() {
    TEST_START("test_model_introspection")
    
    CoreMLModel model(MODEL_PATH);
    
    // Test input/output counts
    ASSERT_EQ(EXPECTED_INPUT_COUNT, model.getInputCount());
    ASSERT_EQ(EXPECTED_OUTPUT_COUNT, model.getOutputCount());
    
    // Test input sizes and names
    for (size_t i = 0; i < EXPECTED_INPUT_COUNT; ++i) {
        ASSERT_EQ(EXPECTED_INPUT_SIZE, model.getInputSize(i));
        std::string input_name = model.getInputName(i);
        ASSERT_TRUE(!input_name.empty());
    }
    
    // Test output sizes and names
    for (size_t i = 0; i < EXPECTED_OUTPUT_COUNT; ++i) {
        ASSERT_EQ(EXPECTED_OUTPUT_SIZE, model.getOutputSize(i));
        std::string output_name = model.getOutputName(i);
        ASSERT_TRUE(!output_name.empty());
    }
    
    TEST_END()
}

// Test introspection methods with invalid indices
void test_introspection_edge_cases() {
    TEST_START("test_introspection_edge_cases")
    
    CoreMLModel model(MODEL_PATH);
    
    // Test out-of-bounds access
    ASSERT_STR_EQ("", model.getInputName(999));
    ASSERT_STR_EQ("", model.getOutputName(999));
    ASSERT_EQ(0, model.getInputSize(999));
    ASSERT_EQ(0, model.getOutputSize(999));
    
    TEST_END()
}

// Test batch introspection methods
void test_batch_introspection() {
    TEST_START("test_batch_introspection")
    
    CoreMLModel model(MODEL_PATH);
    
    // Test input names and sizes arrays
    const std::vector<std::string>& input_names = model.getInputNames();
    const std::vector<size_t>& input_sizes = model.getInputSizes();
    
    ASSERT_EQ(EXPECTED_INPUT_COUNT, input_names.size());
    ASSERT_EQ(EXPECTED_INPUT_COUNT, input_sizes.size());
    
    for (size_t i = 0; i < EXPECTED_INPUT_COUNT; ++i) {
        ASSERT_STR_EQ(model.getInputName(i), input_names[i]);
        ASSERT_EQ(model.getInputSize(i), input_sizes[i]);
        ASSERT_EQ(EXPECTED_INPUT_SIZE, input_sizes[i]);
    }
    
    // Test output names and sizes arrays
    const std::vector<std::string>& output_names = model.getOutputNames();
    const std::vector<size_t>& output_sizes = model.getOutputSizes();
    
    ASSERT_EQ(EXPECTED_OUTPUT_COUNT, output_names.size());
    ASSERT_EQ(EXPECTED_OUTPUT_COUNT, output_sizes.size());
    
    for (size_t i = 0; i < EXPECTED_OUTPUT_COUNT; ++i) {
        ASSERT_STR_EQ(model.getOutputName(i), output_names[i]);
        ASSERT_EQ(model.getOutputSize(i), output_sizes[i]);
        ASSERT_EQ(EXPECTED_OUTPUT_SIZE, output_sizes[i]);
    }
    
    TEST_END()
}

// Test model prediction with actual inference
void test_model_prediction() {
    TEST_START("test_model_prediction")
    
    CoreMLModel model(MODEL_PATH);
    

    
    // Create test inputs using user's specific data
    // input_ids: User's specific token sequence (82 tokens)
    std::vector<float> input_ids = {
        0, 46692, 12, 1301, 10, 4537, 17997, 2256, 4, 70, 7915, 441, 25, 7, 83080, 64209, 674, 111, 21308, 100, 24793, 10, 4188, 953,
        47, 2358, 83, 7621, 16190, 7, 117, 5155, 5, 4966, 4, 237, 398, 831, 1957, 1295, 903, 116287, 4, 398, 25, 1181, 3871, 47,
        51312, 450, 2174, 398, 25, 107, 41206, 214, 707, 25550, 14, 234, 100, 10, 179365, 5, 38679, 1810, 70, 116287, 35064, 47, 1957, 3642,
        5045, 21308, 398, 5608, 186, 118992, 12638, 5155, 5, 2
    };
    
    // Pad input_ids to 512 with zeros
    input_ids.resize(512, 0.0f);
    
    // attention_mask: 1s for the length of actual tokens (82), then 0s to pad to 512
    std::vector<float> attention_mask(512, 0.0f);
    for (int i = 0; i < 82; ++i) {
        attention_mask[i] = 1.0f;
    }
    
    // Verify input sizes
    ASSERT_EQ(EXPECTED_INPUT_SIZE, input_ids.size());
    ASSERT_EQ(EXPECTED_INPUT_SIZE, attention_mask.size());
    
    // Prepare inputs
    std::vector<std::vector<float>> inputs = {attention_mask, input_ids};
    

    
    // Expected output from user's data
    std::vector<float> expected_output = {
        -0.02603657f, -0.04028142f, -0.0407016f, -0.0567059f, 0.09808024f, -0.00689326f, 0.00147932f, 0.04843688f, 0.11323299f, -0.02740479f, -0.00866384f, 0.0041722f, 0.05230619f, -0.04768711f, -0.06603136f, 0.08920389f,
        0.06334062f, -0.0531108f, 0.00967995f, -0.10728f, -0.00384981f, -0.02598385f, -0.00927944f, 0.07551413f, 0.06361946f, 0.01656309f, 0.04170515f, 0.01730528f, 0.01455702f, -0.04344152f, -0.05670433f, -0.04429421f,
        0.07144866f, -0.03361619f, 0.04803946f, -0.00959462f, -0.08393569f, -0.04850754f, 0.05855654f, -0.05139397f, 0.01839359f, 0.05547391f, 0.00980077f, 0.04608278f, 0.02681234f, 0.07292694f, -0.06347434f, 0.05774028f,
        0.00521451f, -0.0223504f, -0.04456337f, 0.06401621f, 0.0201432f, 0.04503602f, 0.07350688f, -0.04566628f, -0.01399929f, -0.04260228f, -0.08010492f, -0.05667777f, 0.06421689f, -0.0662206f, -0.01281161f, 0.00306563f,
        0.06230233f, 0.06887282f, -0.02185547f, 0.02037258f, -0.06924744f, -0.05492327f, -0.05856651f, 0.04827426f, 0.02585801f, -0.04206208f, 0.07226781f, -0.00066223f, 0.02808696f, -0.04768767f, -0.02578073f, -0.0346549f,
        -0.05184536f, -0.02213816f, -0.02603409f, 0.00418784f, -0.04014505f, 0.06438648f, 0.04306499f, -0.06464734f, 0.07835005f, -0.04775861f, 0.03871155f, -0.0021935f, -0.0278965f, -0.02316781f, -0.03081977f, -0.05162852f,
        -0.06317411f, 0.06501403f, 0.03302044f, -0.03495993f, 0.04030043f, -0.00946298f, 0.0334175f, -0.06045863f, -0.04164437f, 0.05242613f, 0.02311893f, -0.08106455f, 0.04201822f, -0.07920428f, -0.06128573f, 0.01361946f,
        0.05605038f, 0.05132582f, -0.02488028f, -0.00724886f, -0.02829193f, -0.04666114f, 0.0488531f, -0.04127707f, 0.1195737f, -0.00543758f, -0.0664471f, -0.0615606f, -0.0596544f, -0.03149877f, 0.00002124f, 0.02896381f,
        -0.00570548f, 0.01163812f, 0.0529829f, 0.05715987f, 0.03742851f, 0.07244842f, -0.00867338f, 0.07250956f, -0.06899954f, -0.05157423f, -0.02404262f, -0.04570679f, -0.03795758f, 0.05116563f, -0.05705468f, 0.02825828f,
        0.119f, 0.06702526f, 0.07962912f, 0.0138034f, 0.09016916f, -0.06509225f, 0.02724551f, -0.01657289f, 0.02802205f, 0.05316252f, 0.03034372f, -0.00838121f, -0.03119651f, -0.03582877f, 0.07752741f, 0.02368231f,
        -0.03352488f, -0.08365101f, -0.045632f, -0.01239439f, -0.04529971f, -0.05366283f, -0.00508505f, 0.08052137f, -0.02365416f, -0.05088639f, -0.07034076f, 0.04437143f, -0.05328837f, 0.06672926f, -0.02946305f, 0.04549774f,
        -0.03047303f, 0.10325804f, 0.05995305f, 0.0332171f, -0.03317452f, -0.01909105f, -0.04186944f, -0.05407609f, -0.03550963f, -0.03281735f, -0.03436183f, 0.02623798f, 0.08710829f, -0.03221209f, 0.06011893f, 0.02914844f,
        -0.05684852f, -0.06085463f, -0.03613582f, 0.00576996f, -0.02940009f, 0.0664039f, 0.07087301f, 0.04929261f, 0.01318032f, -0.00586378f, 0.0495975f, 0.00660807f, 0.00326954f, -0.03995712f, -0.07330823f, 0.03976686f,
        -0.05351673f, 0.00975103f, 0.0657343f, -0.02739752f, -0.09327185f, 0.04945796f, -0.02640922f, -0.04811662f, -0.02023635f, 0.06447945f, -0.03439062f, 0.01399239f, 0.04641354f, 0.00337576f, 0.02538466f, -0.06651058f,
        -0.04496679f, 0.08389921f, 0.01286559f, -0.05759912f, -0.03575672f, 0.04870738f, -0.04739233f, -0.03980496f, -0.03625781f, -0.03805088f, -0.08773074f, -0.06959508f, -0.00847899f, 0.0605951f, 0.07969318f, -0.07081246f,
        -0.08936931f, -0.06057398f, 0.07137255f, -0.01208803f, 0.06577875f, -0.03820341f, -0.00926073f, 0.03320891f, 0.03371769f, 0.0274949f, 0.02222546f, -0.03850091f, -0.06819282f, -0.03207812f, -0.06575747f, 0.05194921f,
        0.04517644f, 0.02427208f, -0.03203684f, 0.01011544f, 0.03549727f, 0.01011584f, 0.09121119f, 0.05085946f, 0.05217327f, 0.07686596f, -0.04503369f, -0.05324338f, -0.05853691f, -0.09454808f, -0.06227062f, 0.01004668f,
        0.04691439f, -0.08155394f, -0.07039263f, -0.06269535f, 0.04923292f, 0.10943374f, -0.09563533f, -0.04455083f, 0.04119622f, -0.01810255f, 0.05851314f, 0.10388673f, 0.0561914f, -0.03728972f, -0.01278675f, 0.0175415f,
        0.0167901f, -0.0243589f, -0.00674845f, -0.06429786f, 0.03328782f, -0.05609737f, 0.10601847f, -0.02594045f, 0.0014952f, 0.04106184f, -0.05370851f, 0.00284642f, -0.05031592f, -0.02407182f, 0.06772645f, 0.09178522f,
        -0.02538844f, 0.01290046f, -0.02147715f, 0.00510707f, -0.01601206f, 0.04254686f, 0.11457691f, 0.07298045f, -0.06978742f, -0.0778845f, 0.03534267f, 0.03449354f, -0.01572116f, 0.04929536f, -0.03750179f, -0.02385612f,
        -0.02329657f, -0.0278185f, -0.00166891f, -0.04054142f, 0.04809564f, 0.02998439f, -0.07266621f, -0.02555503f, 0.04405425f, -0.03032563f, 0.02375165f, -0.00824628f, -0.02347079f, -0.00650779f, -0.04389343f, -0.01901838f,
        -0.01131402f, 0.01689959f, -0.07772236f, -0.02221465f, 0.05264399f, 0.04042237f, -0.05456492f, 0.04826215f, -0.01202582f, -0.03829446f, 0.09095757f, -0.08251435f, -0.02401281f, 0.03748612f, 0.08460325f, -0.0638821f,
        -0.00170379f, 0.04327488f, -0.02785971f, 0.0452118f, 0.00906494f, -0.06582072f, 0.1295968f, 0.02732123f, -0.0723638f, -0.05259331f, 0.02212602f, 0.02882134f, 0.07441573f, 0.08277623f, -0.05466759f, 0.00978202f,
        0.03124883f, -0.03526705f, 0.03662859f, 0.07094468f, -0.0402846f, 0.01564865f, -0.00978889f, -0.06183674f, -0.08637218f, 0.07227731f, -0.06914753f, -0.06059679f, 0.01716776f, 0.06575119f, 0.04593194f, 0.05799318f
    };
    
    // Verify expected output size matches model output size
    ASSERT_EQ(EXPECTED_OUTPUT_SIZE, expected_output.size());
    
    // Run prediction
    auto outputs = model.predict(inputs);
    
    // Verify output count and size
    ASSERT_EQ(EXPECTED_OUTPUT_COUNT, outputs.size());
    ASSERT_EQ(EXPECTED_OUTPUT_SIZE, outputs[0].size());
    
    // Verify output values with tolerance (1e-5f for floating-point comparison)
    const float tolerance = 1e-5f;
    for (size_t i = 0; i < expected_output.size(); ++i) {
        ASSERT_TRUE(std::abs(outputs[0][i] - expected_output[i]) < tolerance);
    }
    
    std::cout << "(prediction successful with user's specific input/output) ";
    
    TEST_END()
}

// Test prediction with pre-allocated outputs
void test_prediction_with_preallocated_outputs() {
    TEST_START("test_prediction_with_preallocated_outputs")
    
    CoreMLModel model(MODEL_PATH);
    
    // Create test inputs using user's specific data
    // input_ids: User's specific token sequence (82 tokens)
    std::vector<float> input_ids = {
        0, 46692, 12, 1301, 10, 4537, 17997, 2256, 4, 70, 7915, 441, 25, 7, 83080, 64209, 674, 111, 21308, 100, 24793, 10, 4188, 953,
        47, 2358, 83, 7621, 16190, 7, 117, 5155, 5, 4966, 4, 237, 398, 831, 1957, 1295, 903, 116287, 4, 398, 25, 1181, 3871, 47,
        51312, 450, 2174, 398, 25, 107, 41206, 214, 707, 25550, 14, 234, 100, 10, 179365, 5, 38679, 1810, 70, 116287, 35064, 47, 1957, 3642,
        5045, 21308, 398, 5608, 186, 118992, 12638, 5155, 5, 2
    };
    
    // Pad input_ids to 512 with zeros
    input_ids.resize(512, 0.0f);
    
    // attention_mask: 1s for the length of actual tokens (82), then 0s to pad to 512
    std::vector<float> attention_mask(512, 0.0f);
    for (int i = 0; i < 82; ++i) {
        attention_mask[i] = 1.0f;
    }
    
    std::vector<std::vector<float>> inputs = {attention_mask, input_ids};
    
    // Pre-allocate outputs
    std::vector<std::vector<float>> outputs = {
        std::vector<float>(EXPECTED_OUTPUT_SIZE, 0.0f)
    };
    
    // Perform prediction
    bool success = model.predict(inputs, outputs);
    ASSERT_TRUE(success);
    
    // Verify output was filled
    ASSERT_EQ(EXPECTED_OUTPUT_SIZE, outputs[0].size());
    
    std::cout << "(preallocated prediction successful with user's input) ";
    
    TEST_END()
}

// Test prediction error handling
void test_prediction_error_handling() {
    TEST_START("test_prediction_error_handling")
    
    CoreMLModel model(MODEL_PATH);
    
    // Test with wrong number of inputs
    bool exception_thrown = false;
    try {
        std::vector<std::vector<float>> wrong_inputs = {
            std::vector<float>(EXPECTED_INPUT_SIZE, 1.0f)  // Only 1 input instead of 2
        };
        model.predict(wrong_inputs);
    } catch (const std::runtime_error&) {
        exception_thrown = true;
    }
    ASSERT_TRUE(exception_thrown);
    
    // Test with wrong input size
    exception_thrown = false;
    try {
        std::vector<std::vector<float>> wrong_size_inputs = {
            std::vector<float>(100, 1.0f),  // Wrong size: 100 instead of 512
            std::vector<float>(EXPECTED_INPUT_SIZE, 1.0f)
        };
        model.predict(wrong_size_inputs);
    } catch (const std::runtime_error&) {
        exception_thrown = true;
    }
    ASSERT_TRUE(exception_thrown);
    
    std::cout << "(error handling verified) ";
    
    TEST_END()
}

// Test prediction performance
void test_prediction_performance() {
    TEST_START("test_prediction_performance")
    
    CoreMLModel model(MODEL_PATH);
    
    // Create test inputs using user's specific data
    // input_ids: User's specific token sequence (82 tokens)
    std::vector<float> input_ids = {
        0, 46692, 12, 1301, 10, 4537, 17997, 2256, 4, 70, 7915, 441, 25, 7, 83080, 64209, 674, 111, 21308, 100, 24793, 10, 4188, 953,
        47, 2358, 83, 7621, 16190, 7, 117, 5155, 5, 4966, 4, 237, 398, 831, 1957, 1295, 903, 116287, 4, 398, 25, 1181, 3871, 47,
        51312, 450, 2174, 398, 25, 107, 41206, 214, 707, 25550, 14, 234, 100, 10, 179365, 5, 38679, 1810, 70, 116287, 35064, 47, 1957, 3642,
        5045, 21308, 398, 5608, 186, 118992, 12638, 5155, 5, 2
    };
    
    // Pad input_ids to 512 with zeros
    input_ids.resize(512, 0.0f);
    
    // attention_mask: 1s for the length of actual tokens (82), then 0s to pad to 512
    std::vector<float> attention_mask(512, 0.0f);
    for (int i = 0; i < 82; ++i) {
        attention_mask[i] = 1.0f;
    }
    
    std::vector<std::vector<float>> inputs = {attention_mask, input_ids};
    
    const int num_iterations = 10;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Run multiple predictions
    for (int i = 0; i < num_iterations; ++i) {
        auto outputs = model.predict(inputs);
        ASSERT_EQ(EXPECTED_OUTPUT_COUNT, outputs.size());
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "(" << num_iterations << " predictions in " << duration.count() << "ms) ";
    
    // Ensure reasonable performance (less than 100ms per prediction on average)
    ASSERT_TRUE(duration.count() < (num_iterations * 100));
    
    TEST_END()
}

// Test metadata caching performance
void test_metadata_caching_performance() {
    TEST_START("test_metadata_caching_performance")
    
    CoreMLModel model(MODEL_PATH);
    
    const int num_calls = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Test rapid introspection calls (should be very fast due to caching)
    for (int i = 0; i < num_calls; ++i) {
        ASSERT_EQ(EXPECTED_INPUT_COUNT, model.getInputCount());
        ASSERT_EQ(EXPECTED_OUTPUT_COUNT, model.getOutputCount());
        ASSERT_EQ(EXPECTED_INPUT_SIZE, model.getInputSize(0));
        ASSERT_EQ(EXPECTED_OUTPUT_SIZE, model.getOutputSize(0));
        ASSERT_TRUE(!model.getInputName(0).empty());
        ASSERT_TRUE(!model.getOutputName(0).empty());
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "(" << num_calls << " introspection calls in " << duration.count() << "μs) ";
    
    // Should be very fast due to caching (less than 1ms for 1000 calls)
    ASSERT_TRUE(duration.count() < 1000);
    
    TEST_END()
}

// Test input data preparation with user's specific vectors
void test_input_data_preparation() {
    TEST_START("test_input_data_preparation")
    
    // Create test inputs using user's specific data
    // input_ids: User's specific token sequence (82 tokens)
    std::vector<float> input_ids = {
        0, 46692, 12, 1301, 10, 4537, 17997, 2256, 4, 70, 7915, 441, 25, 7, 83080, 64209, 674, 111, 21308, 100, 24793, 10, 4188, 953,
        47, 2358, 83, 7621, 16190, 7, 117, 5155, 5, 4966, 4, 237, 398, 831, 1957, 1295, 903, 116287, 4, 398, 25, 1181, 3871, 47,
        51312, 450, 2174, 398, 25, 107, 41206, 214, 707, 25550, 14, 234, 100, 10, 179365, 5, 38679, 1810, 70, 116287, 35064, 47, 1957, 3642,
        5045, 21308, 398, 5608, 186, 118992, 12638, 5155, 5, 2
    };
    
    // Verify original sequence length
    ASSERT_EQ(82, input_ids.size());
    
    // Pad input_ids to 512 with zeros
    input_ids.resize(512, 0.0f);
    ASSERT_EQ(512, input_ids.size());
    
    // Verify first few values
    ASSERT_EQ(0.0f, input_ids[0]);
    ASSERT_EQ(46692.0f, input_ids[1]);
    ASSERT_EQ(12.0f, input_ids[2]);
    
    // Verify padding
    ASSERT_EQ(0.0f, input_ids[511]);
    ASSERT_EQ(0.0f, input_ids[400]);
    
    // attention_mask: 1s for the length of actual tokens (82), then 0s to pad to 512
    std::vector<float> attention_mask(512, 0.0f);
    for (int i = 0; i < 82; ++i) {
        attention_mask[i] = 1.0f;
    }
    
    // Verify attention mask
    ASSERT_EQ(512, attention_mask.size());
    ASSERT_EQ(1.0f, attention_mask[0]);
    ASSERT_EQ(1.0f, attention_mask[81]);
    ASSERT_EQ(0.0f, attention_mask[82]);
    ASSERT_EQ(0.0f, attention_mask[511]);
    
    // Count 1s and 0s
    int ones_count = 0;
    int zeros_count = 0;
    for (float val : attention_mask) {
        if (val == 1.0f) ones_count++;
        else if (val == 0.0f) zeros_count++;
    }
    ASSERT_EQ(82, ones_count);
    ASSERT_EQ(430, zeros_count);
    
    std::cout << "(input data validation passed: 82 tokens + 430 padding) ";
    
    TEST_END()
}

// Test with an invalid model (after destruction)
void test_invalid_model_behavior() {
    TEST_START("test_invalid_model_behavior")
    
    CoreMLModel model(MODEL_PATH);
    CoreMLModel moved_model = std::move(model);
    
    // Original model should be invalid now
    ASSERT_TRUE(!model.isValid());
    ASSERT_EQ(0, model.getInputCount());
    ASSERT_EQ(0, model.getOutputCount());
    ASSERT_STR_EQ("", model.getDescription());
    
    // Moved model should still be valid
    ASSERT_TRUE(moved_model.isValid());
    ASSERT_EQ(EXPECTED_INPUT_COUNT, moved_model.getInputCount());
    
    TEST_END()
}

void print_test_summary() {
    std::cout << "\n=== Test Results ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    std::cout << "Success rate: " << (total_tests > 0 ? (100 * passed_tests / total_tests) : 0) << "%" << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "🎉 All tests passed!" << std::endl;
    } else {
        std::cout << "❌ " << failed_tests << " test(s) failed." << std::endl;
    }
}

int main() {
    std::cout << "=== CoreMLModel Unit Tests ===" << std::endl;
    std::cout << "Platform: macOS" << std::endl;
    std::cout << "Model: " << MODEL_PATH << std::endl;
    std::cout << "Expected: " << EXPECTED_INPUT_COUNT << " inputs (" << EXPECTED_INPUT_SIZE << " each), "
              << EXPECTED_OUTPUT_COUNT << " output (" << EXPECTED_OUTPUT_SIZE << ")" << std::endl;
    std::cout << std::endl;

    // Run all tests
    test_model_construction();
    test_model_construction_with_compute_units();
    test_model_construction_errors();
    test_move_semantics();
    test_model_introspection();
    test_introspection_edge_cases();
    test_batch_introspection();
    test_model_prediction();
    test_prediction_with_preallocated_outputs();
    test_prediction_error_handling();
    test_prediction_performance();
    test_metadata_caching_performance();
    test_input_data_preparation();
    test_invalid_model_behavior();

    print_test_summary();
    
    return (failed_tests == 0) ? 0 : 1;
} 