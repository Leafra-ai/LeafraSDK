#import "LeafraSDKBridge.h"
#include "leafra/leafra_core.h"
#include "leafra/math_utils.h"
#include "leafra/data_processor.h"
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
    //TODO AD: Add other SDK config options here if needed later to expose to the app
    
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

- (void)setEventCallback:(void (^)(NSString *message))callback {
    _eventCallback = [callback copy];
}

@end 