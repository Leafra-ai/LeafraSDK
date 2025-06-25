#import "LeafraSDKBridge.h"
#include "leafra/leafra_core.h"
#include "leafra/math_utils.h"
#include "leafra/data_processor.h"
#include "leafra/leafra_chunker.h"

#ifdef LEAFRA_HAS_FAISS
#include "leafra/leafra_faiss.h"
#endif

#include <memory>

@interface LeafraSDKBridge()
@property (nonatomic, assign) std::shared_ptr<leafra::LeafraCore> coreSDK;
@property (nonatomic, assign) std::shared_ptr<leafra::MathUtils> mathUtils;
@property (nonatomic, assign) std::shared_ptr<leafra::DataProcessor> dataProcessor;
@property (nonatomic, copy) void (^eventCallback)(NSString *message);
@end

@implementation LeafraSDKBridge

- (instancetype)init {
    self = [super init];
    if (self) {
        _coreSDK = leafra::LeafraCore::create();
        _mathUtils = std::make_shared<leafra::MathUtils>();
        _dataProcessor = std::make_shared<leafra::DataProcessor>();
        _eventCallback = nil;
        
        // Set up C++ event callback with proper weak reference handling
        LeafraSDKBridge* __weak weakSelf = self;
        _coreSDK->set_event_callback([weakSelf](const std::string& message) {
            LeafraSDKBridge* strongSelf = weakSelf;
            if (strongSelf && strongSelf->_eventCallback) {
                NSString *nsMessage = [NSString stringWithUTF8String:message.c_str()];
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (strongSelf->_eventCallback) {
                        strongSelf->_eventCallback(nsMessage);
                    }
                });
            }
        });
    }
    return self;
}

- (void)dealloc {
    // Clear the callback to prevent crashes
    if (_coreSDK) {
        _coreSDK->set_event_callback(nullptr);
    }
    _eventCallback = nil;
}

#pragma mark - Helper Methods

- (leafra::Config)configFromDictionary:(NSDictionary *)dict {
    leafra::Config config;
    
    // Basic configuration
    if (dict[@"name"]) {
        config.name = [dict[@"name"] UTF8String];
    }
    if (dict[@"version"]) {
        config.version = [dict[@"version"] UTF8String];
    }
    if (dict[@"debugMode"]) {
        config.debug_mode = [dict[@"debugMode"] boolValue];
    }
    if (dict[@"maxThreads"]) {
        config.max_threads = [dict[@"maxThreads"] intValue];
    }
    if (dict[@"bufferSize"]) {
        config.buffer_size = [dict[@"bufferSize"] unsignedIntegerValue];
    }
    if (dict[@"leafraDocumentDatabaseName"]) {
        config.leafra_document_database_name = [dict[@"leafraDocumentDatabaseName"] UTF8String];
    }
    
    // Chunking configuration
    if (dict[@"chunking"]) {
        NSDictionary *chunkingDict = dict[@"chunking"];
        if (chunkingDict[@"enabled"]) {
            config.chunking.enabled = [chunkingDict[@"enabled"] boolValue];
        }
        if (chunkingDict[@"chunkSize"]) {
            config.chunking.chunk_size = [chunkingDict[@"chunkSize"] unsignedIntegerValue];
        }
        if (chunkingDict[@"overlapPercentage"]) {
            config.chunking.overlap_percentage = [chunkingDict[@"overlapPercentage"] doubleValue];
        }
        if (chunkingDict[@"preserveWordBoundaries"]) {
            config.chunking.preserve_word_boundaries = [chunkingDict[@"preserveWordBoundaries"] boolValue];
        }
        if (chunkingDict[@"includeMetadata"]) {
            config.chunking.include_metadata = [chunkingDict[@"includeMetadata"] boolValue];
        }
        if (chunkingDict[@"sizeUnit"]) {
            NSString *sizeUnit = chunkingDict[@"sizeUnit"];
            if ([sizeUnit isEqualToString:@"TOKENS"]) {
                config.chunking.size_unit = leafra::ChunkSizeUnit::TOKENS;
            } else {
                config.chunking.size_unit = leafra::ChunkSizeUnit::CHARACTERS;
            }
        }
        if (chunkingDict[@"tokenMethod"]) {
            NSString *tokenMethod = chunkingDict[@"tokenMethod"];
            if ([tokenMethod isEqualToString:@"SIMPLE"]) {
                config.chunking.token_method = leafra::TokenApproximationMethod::SIMPLE;
            } else {
                config.chunking.token_method = leafra::TokenApproximationMethod::SIMPLE; // Default to SIMPLE since it's the only available option
            }
        }
        if (chunkingDict[@"printChunksFull"]) {
            config.chunking.print_chunks_full = [chunkingDict[@"printChunksFull"] boolValue];
        }
        if (chunkingDict[@"printChunksBrief"]) {
            config.chunking.print_chunks_brief = [chunkingDict[@"printChunksBrief"] boolValue];
        }
        if (chunkingDict[@"maxLines"]) {
            config.chunking.max_lines = [chunkingDict[@"maxLines"] intValue];
        }
    }
    
    // Tokenizer configuration
    if (dict[@"tokenizer"]) {
        NSDictionary *tokenizerDict = dict[@"tokenizer"];
        if (tokenizerDict[@"enableSentencepiece"]) {
            config.tokenizer.enable_sentencepiece = [tokenizerDict[@"enableSentencepiece"] boolValue];
        }
        if (tokenizerDict[@"modelName"]) {
            config.tokenizer.model_name = [tokenizerDict[@"modelName"] UTF8String];
        }
        if (tokenizerDict[@"sentencepieceModelPath"]) {
            config.tokenizer.sentencepiece_model_path = [tokenizerDict[@"sentencepieceModelPath"] UTF8String];
        }
        if (tokenizerDict[@"sentencepieceJsonPath"]) {
            config.tokenizer.sentencepiece_json_path = [tokenizerDict[@"sentencepieceJsonPath"] UTF8String];
        }
    }
    
    // Embedding model configuration
    if (dict[@"embeddingInference"]) {
        NSDictionary *embeddingDict = dict[@"embeddingInference"];
        if (embeddingDict[@"enabled"]) {
            config.embedding_inference.enabled = [embeddingDict[@"enabled"] boolValue];
        }
        if (embeddingDict[@"framework"]) {
            config.embedding_inference.framework = [embeddingDict[@"framework"] UTF8String];
        }
        if (embeddingDict[@"modelPath"]) {
            config.embedding_inference.model_path = [embeddingDict[@"modelPath"] UTF8String];
        }
        if (embeddingDict[@"coremlComputeUnits"]) {
            config.embedding_inference.coreml_compute_units = [embeddingDict[@"coremlComputeUnits"] UTF8String];
        }
        if (embeddingDict[@"tfliteEnableCoremLDelegate"]) {
            config.embedding_inference.tflite_enable_coreml_delegate = [embeddingDict[@"tfliteEnableCoremLDelegate"] boolValue];
        }
        if (embeddingDict[@"tfliteEnableMetalDelegate"]) {
            config.embedding_inference.tflite_enable_metal_delegate = [embeddingDict[@"tfliteEnableMetalDelegate"] boolValue];
        }
        if (embeddingDict[@"tfliteEnableXnnpackDelegate"]) {
            config.embedding_inference.tflite_enable_xnnpack_delegate = [embeddingDict[@"tfliteEnableXnnpackDelegate"] boolValue];
        }
        if (embeddingDict[@"tfliteNumThreads"]) {
            config.embedding_inference.tflite_num_threads = [embeddingDict[@"tfliteNumThreads"] intValue];
        }
        if (embeddingDict[@"tfliteUseNnapi"]) {
            config.embedding_inference.tflite_use_nnapi = [embeddingDict[@"tfliteUseNnapi"] boolValue];
        }
    }
    
    // Vector search configuration
    if (dict[@"vectorSearch"]) {
        NSDictionary *vectorDict = dict[@"vectorSearch"];
        if (vectorDict[@"enabled"]) {
            config.vector_search.enabled = [vectorDict[@"enabled"] boolValue];
        }
        if (vectorDict[@"dimension"]) {
            config.vector_search.dimension = [vectorDict[@"dimension"] intValue];
        }
        if (vectorDict[@"indexType"]) {
            config.vector_search.index_type = [vectorDict[@"indexType"] UTF8String];
        }
        if (vectorDict[@"metric"]) {
            config.vector_search.metric = [vectorDict[@"metric"] UTF8String];
        }
        if (vectorDict[@"nlist"]) {
            config.vector_search.nlist = [vectorDict[@"nlist"] intValue];
        }
        if (vectorDict[@"nprobe"]) {
            config.vector_search.nprobe = [vectorDict[@"nprobe"] intValue];
        }
        if (vectorDict[@"m"]) {
            config.vector_search.m = [vectorDict[@"m"] intValue];
        }
        if (vectorDict[@"nbits"]) {
            config.vector_search.nbits = [vectorDict[@"nbits"] intValue];
        }
        if (vectorDict[@"hnswM"]) {
            config.vector_search.hnsw_m = [vectorDict[@"hnswM"] intValue];
        }
        if (vectorDict[@"lshNbits"]) {
            config.vector_search.lsh_nbits = [vectorDict[@"lshNbits"] intValue];
        }
        if (vectorDict[@"indexDefinition"]) {
            config.vector_search.index_definition = [vectorDict[@"indexDefinition"] UTF8String];
        }
        if (vectorDict[@"autoSave"]) {
            config.vector_search.auto_save = [vectorDict[@"autoSave"] boolValue];
        }
        if (vectorDict[@"autoLoad"]) {
            config.vector_search.auto_load = [vectorDict[@"autoLoad"] boolValue];
        }
    }
    
    // LLM configuration
    if (dict[@"llm"]) {
        NSDictionary *llmDict = dict[@"llm"];
        if (llmDict[@"enabled"]) {
            config.llm.enabled = [llmDict[@"enabled"] boolValue];
        }
        if (llmDict[@"modelPath"]) {
            config.llm.model_path = [llmDict[@"modelPath"] UTF8String];
        }
        if (llmDict[@"framework"]) {
            config.llm.framework = [llmDict[@"framework"] UTF8String];
        }
        if (llmDict[@"nCtx"]) {
            config.llm.n_ctx = [llmDict[@"nCtx"] intValue];
        }
        if (llmDict[@"nPredict"]) {
            config.llm.n_predict = [llmDict[@"nPredict"] intValue];
        }
        if (llmDict[@"nBatch"]) {
            config.llm.n_batch = [llmDict[@"nBatch"] intValue];
        }
        if (llmDict[@"nUbatch"]) {
            config.llm.n_ubatch = [llmDict[@"nUbatch"] intValue];
        }
        if (llmDict[@"nThreads"]) {
            config.llm.n_threads = [llmDict[@"nThreads"] intValue];
        }
        if (llmDict[@"nThreadsBatch"]) {
            config.llm.n_threads_batch = [llmDict[@"nThreadsBatch"] intValue];
        }
        if (llmDict[@"temperature"]) {
            config.llm.temperature = [llmDict[@"temperature"] floatValue];
        }
        if (llmDict[@"topP"]) {
            config.llm.top_p = [llmDict[@"topP"] floatValue];
        }
        if (llmDict[@"topK"]) {
            config.llm.top_k = [llmDict[@"topK"] intValue];
        }
        if (llmDict[@"minP"]) {
            config.llm.min_p = [llmDict[@"minP"] floatValue];
        }
        if (llmDict[@"repeatPenalty"]) {
            config.llm.repeat_penalty = [llmDict[@"repeatPenalty"] floatValue];
        }
        if (llmDict[@"repeatLastN"]) {
            config.llm.repeat_last_n = [llmDict[@"repeatLastN"] intValue];
        }
        if (llmDict[@"tfsZ"]) {
            config.llm.tfs_z = [llmDict[@"tfsZ"] floatValue];
        }
        if (llmDict[@"typicalP"]) {
            config.llm.typical_p = [llmDict[@"typicalP"] floatValue];
        }
        if (llmDict[@"nGpuLayers"]) {
            config.llm.n_gpu_layers = [llmDict[@"nGpuLayers"] intValue];
        }
        if (llmDict[@"useMmap"]) {
            config.llm.use_mmap = [llmDict[@"useMmap"] boolValue];
        }
        if (llmDict[@"useMlock"]) {
            config.llm.use_mlock = [llmDict[@"useMlock"] boolValue];
        }
        if (llmDict[@"numa"]) {
            config.llm.numa = [llmDict[@"numa"] boolValue];
        }
        if (llmDict[@"systemPrompt"]) {
            config.llm.system_prompt = [llmDict[@"systemPrompt"] UTF8String];
        }
        if (llmDict[@"seed"]) {
            config.llm.seed = [llmDict[@"seed"] intValue];
        }
        if (llmDict[@"debugMode"]) {
            config.llm.debug_mode = [llmDict[@"debugMode"] boolValue];
        }
        if (llmDict[@"verbosePrompt"]) {
            config.llm.verbose_prompt = [llmDict[@"verbosePrompt"] boolValue];
        }
    }
    
    return config;
}

- (leafra::Point2D)point2DFromDictionary:(NSDictionary *)dict {
    leafra::Point2D point;
    if (dict[@"x"]) {
        point.x = [dict[@"x"] doubleValue];
    }
    if (dict[@"y"]) {
        point.y = [dict[@"y"] doubleValue];
    }
    return point;
}

- (leafra::Point3D)point3DFromDictionary:(NSDictionary *)dict {
    leafra::Point3D point;
    if (dict[@"x"]) {
        point.x = [dict[@"x"] doubleValue];
    }
    if (dict[@"y"]) {
        point.y = [dict[@"y"] doubleValue];
    }
    if (dict[@"z"]) {
        point.z = [dict[@"z"] doubleValue];
    }
    return point;
}

- (leafra::Matrix3x3)matrix3x3FromDictionary:(NSDictionary *)dict {
    leafra::Matrix3x3 matrix;
    NSArray *data = dict[@"data"];
    if (data && data.count >= 9) {
        for (int i = 0; i < 9; i++) {
            matrix.data[i] = [data[i] doubleValue];
        }
    }
    return matrix;
}

- (NSDictionary *)dictionaryFromMatrix3x3:(const leafra::Matrix3x3&)matrix {
    NSMutableArray *data = [NSMutableArray arrayWithCapacity:9];
    for (int i = 0; i < 9; i++) {
        [data addObject:@(matrix.data[i])];
    }
    return @{@"data": data};
}

- (NSError *)errorFromResultCode:(leafra::ResultCode)code message:(NSString *)message {
    NSInteger errorCode = static_cast<NSInteger>(code);
    NSString *description = message ?: @"Unknown error";
    
    return [NSError errorWithDomain:@"LeafraSDKErrorDomain"
                               code:errorCode
                           userInfo:@{NSLocalizedDescriptionKey: description}];
}

#pragma mark - Public Methods

- (NSNumber *)initializeWithConfig:(NSDictionary *)config error:(NSError **)error {
    if (!_coreSDK) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"Core SDK not available"];
        }
        return @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED);
    }
    
    leafra::Config cppConfig = [self configFromDictionary:config];
    leafra::ResultCode result = _coreSDK->initialize(cppConfig);
    
    if (result != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:result message:@"Failed to initialize SDK"];
    }
    
    return @((int)result);
}

- (NSNumber *)shutdownWithError:(NSError **)error {
    if (!_coreSDK) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"Core SDK not available"];
        }
        return @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED);
    }
    
    leafra::ResultCode result = _coreSDK->shutdown();
    
    if (result != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:result message:@"Failed to shutdown SDK"];
    }
    
    return @((int)result);
}

- (BOOL)isInitialized {
    return _coreSDK ? _coreSDK->is_initialized() : NO;
}

- (NSString *)getVersion {
    return [NSString stringWithUTF8String:leafra::LeafraCore::get_version().c_str()];
}

- (NSString *)getPlatform {
    return [NSString stringWithUTF8String:leafra::LeafraCore::get_platform().c_str()];
}

- (NSDictionary *)processData:(NSArray<NSNumber *> *)input error:(NSError **)error {
    if (!_coreSDK || !_dataProcessor) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"SDK not initialized"];
        }
        return @{@"result": @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED), @"output": @[]};
    }
    
    // Convert input array to C++ data buffer
    leafra::data_buffer_t inputBuffer;
    for (NSNumber *num in input) {
        inputBuffer.push_back([num unsignedCharValue]);
    }
    
    leafra::data_buffer_t outputBuffer;
    leafra::ResultCode result = _dataProcessor->process(inputBuffer, outputBuffer);
    
    if (result != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:result message:@"Failed to process data"];
    }
    
    // Convert output buffer to NSArray
    NSMutableArray *outputArray = [NSMutableArray arrayWithCapacity:outputBuffer.size()];
    for (auto byte : outputBuffer) {
        [outputArray addObject:@(byte)];
    }
    
    return @{
        @"result": @((int)result),
        @"output": outputArray
    };
}

- (NSDictionary *)processUserFiles:(NSArray<NSString *> *)fileUrls error:(NSError **)error {
    if (!_coreSDK) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"SDK not initialized"];
        }
        return @{@"result": @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED), @"processedFiles": @[], @"message": @"SDK not initialized"};
    }
    
    // Convert NSArray of NSString to std::vector<std::string>
    std::vector<std::string> filePaths;
    for (NSString *urlString in fileUrls) {
        // Convert file URL to local path
        NSURL *url = [NSURL URLWithString:urlString];
        if (url && url.isFileURL) {
            std::string path = [url.path UTF8String];
            filePaths.push_back(path);
        } else {
            // If it's not a file URL, try to use the string directly
            std::string path = [urlString UTF8String];
            filePaths.push_back(path);
        }
    }
    
    // Call the C++ SDK method
    leafra::ResultCode result = _coreSDK->process_user_files(filePaths);
    
    if (result != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:result message:@"Failed to process user files"];
    }
    
    // Create response with processed file information
    NSMutableArray *processedFiles = [NSMutableArray arrayWithCapacity:filePaths.size()];
    for (const auto& path : filePaths) {
        [processedFiles addObject:[NSString stringWithUTF8String:path.c_str()]];
    }
    
    NSString *message = (result == leafra::ResultCode::SUCCESS) ? 
        [NSString stringWithFormat:@"Successfully processed %lu files", (unsigned long)filePaths.size()] :
        @"Failed to process some files";
    
    return @{
        @"result": @((int)result),
        @"processedFiles": processedFiles,
        @"message": message
    };
}

- (NSNumber *)calculateDistance2D:(NSDictionary *)p1 point2:(NSDictionary *)p2 error:(NSError **)error {
    if (!_mathUtils) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"Math utils not available"];
        }
        return @(0.0);
    }
    
    leafra::Point2D point1 = [self point2DFromDictionary:p1];
    leafra::Point2D point2 = [self point2DFromDictionary:p2];
    
    double distance = _mathUtils->calculate_distance_2d(point1, point2);
    return @(distance);
}

- (NSNumber *)calculateDistance3D:(NSDictionary *)p1 point2:(NSDictionary *)p2 error:(NSError **)error {
    if (!_mathUtils) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"Math utils not available"];
        }
        return @(0.0);
    }
    
    leafra::Point3D point1 = [self point3DFromDictionary:p1];
    leafra::Point3D point2 = [self point3DFromDictionary:p2];
    
    double distance = _mathUtils->calculate_distance_3d(point1, point2);
    return @(distance);
}

- (NSDictionary *)multiplyMatrix3x3:(NSDictionary *)matrixA matrixB:(NSDictionary *)matrixB error:(NSError **)error {
    if (!_mathUtils) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"Math utils not available"];
        }
        return @{@"result": @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED), @"matrix": @{@"data": @[]}};
    }
    
    leafra::Matrix3x3 a = [self matrix3x3FromDictionary:matrixA];
    leafra::Matrix3x3 b = [self matrix3x3FromDictionary:matrixB];
    leafra::Matrix3x3 result;
    
    leafra::ResultCode code = _mathUtils->multiply_matrix_3x3(a, b, result);
    
    if (code != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:code message:@"Failed to multiply matrices"];
    }
    
    return @{
        @"result": @((int)code),
        @"matrix": [self dictionaryFromMatrix3x3:result]
    };
}

- (NSNumber *)matrixDeterminant:(NSDictionary *)matrix error:(NSError **)error {
    if (!_mathUtils) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"Math utils not available"];
        }
        return @(0.0);
    }
    
    leafra::Matrix3x3 mat = [self matrix3x3FromDictionary:matrix];
    double determinant = _mathUtils->matrix_determinant(mat);
    
    return @(determinant);
}

#pragma mark - Semantic Search Methods

#ifdef LEAFRA_HAS_FAISS

- (NSDictionary *)dictionaryFromSearchResult:(const leafra::FaissIndex::SearchResult&)result {
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    
    dict[@"id"] = @(result.id);
    dict[@"distance"] = @(result.distance);
    
    // Optional chunk metadata
    if (result.doc_id != -1) {
        dict[@"docId"] = @(result.doc_id);
    }
    if (result.chunk_index != -1) {
        dict[@"chunkIndex"] = @(result.chunk_index);
    }
    if (result.page_number != -1) {
        dict[@"pageNumber"] = @(result.page_number);
    }
    if (!result.content.empty()) {
        dict[@"content"] = [NSString stringWithUTF8String:result.content.c_str()];
    }
    if (!result.filename.empty()) {
        dict[@"filename"] = [NSString stringWithUTF8String:result.filename.c_str()];
    }
    
    return dict;
}

- (NSDictionary *)semanticSearch:(NSString *)query maxResults:(NSNumber *)maxResults error:(NSError **)error {
    if (!_coreSDK) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"SDK not initialized"];
        }
        return @{@"result": @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED), @"results": @[]};
    }
    
    std::string queryString = [query UTF8String];
    int maxResultsInt = [maxResults intValue];
    std::vector<leafra::FaissIndex::SearchResult> searchResults;
    
    leafra::ResultCode result = _coreSDK->semantic_search(queryString, maxResultsInt, searchResults);
    
    if (result != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:result message:@"Semantic search failed"];
    }
    
    // Convert search results to NSArray
    NSMutableArray *resultsArray = [NSMutableArray arrayWithCapacity:searchResults.size()];
    for (const auto& searchResult : searchResults) {
        [resultsArray addObject:[self dictionaryFromSearchResult:searchResult]];
    }
    
    return @{
        @"result": @((int)result),
        @"results": resultsArray
    };
}

- (NSDictionary *)semanticSearchWithLLM:(NSString *)query maxResults:(NSNumber *)maxResults error:(NSError **)error {
    if (!_coreSDK) {
        if (error) {
            *error = [self errorFromResultCode:leafra::ResultCode::ERROR_INITIALIZATION_FAILED
                                       message:@"SDK not initialized"];
        }
        return @{@"result": @((int)leafra::ResultCode::ERROR_INITIALIZATION_FAILED), @"results": @[]};
    }
    
    std::string queryString = [query UTF8String];
    int maxResultsInt = [maxResults intValue];
    std::vector<leafra::FaissIndex::SearchResult> searchResults;
    
    // Create token callback that sends tokens via React Native event emitter
    auto tokenCallback = [self](const std::string& token, bool isEnd) -> bool {
        if (_eventCallback) {
            NSString *tokenString = [NSString stringWithUTF8String:token.c_str()];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                // Send token event to React Native
                [[NSNotificationCenter defaultCenter] postNotificationName:@"LeafraSDKTokenEvent"
                                                                    object:nil
                                                                  userInfo:@{@"token": tokenString}];
            });
        }
        return true; // Continue generation
    };
    
    leafra::ResultCode result = _coreSDK->semantic_search_with_llm(queryString, maxResultsInt, searchResults, tokenCallback);
    
    if (result != leafra::ResultCode::SUCCESS && error) {
        *error = [self errorFromResultCode:result message:@"Semantic search with LLM failed"];
    }
    
    // Convert search results to NSArray
    NSMutableArray *resultsArray = [NSMutableArray arrayWithCapacity:searchResults.size()];
    for (const auto& searchResult : searchResults) {
        [resultsArray addObject:[self dictionaryFromSearchResult:searchResult]];
    }
    
    return @{
        @"result": @((int)result),
        @"results": resultsArray
    };
}

#else

// Fallback implementations when FAISS is not available
- (NSDictionary *)semanticSearch:(NSString *)query maxResults:(NSNumber *)maxResults error:(NSError **)error {
    if (error) {
        *error = [self errorFromResultCode:leafra::ResultCode::ERROR_NOT_IMPLEMENTED
                                   message:@"FAISS support not compiled"];
    }
    return @{@"result": @((int)leafra::ResultCode::ERROR_NOT_IMPLEMENTED), @"results": @[]};
}

- (NSDictionary *)semanticSearchWithLLM:(NSString *)query maxResults:(NSNumber *)maxResults error:(NSError **)error {
    if (error) {
        *error = [self errorFromResultCode:leafra::ResultCode::ERROR_NOT_IMPLEMENTED
                                   message:@"FAISS support not compiled"];
    }
    return @{@"result": @((int)leafra::ResultCode::ERROR_NOT_IMPLEMENTED), @"results": @[]};
}

#endif

- (void)setEventCallback:(void (^)(NSString *message))callback {
    _eventCallback = [callback copy];
}

@end 