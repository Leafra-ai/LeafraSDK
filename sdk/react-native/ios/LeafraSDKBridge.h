#pragma once

#import <Foundation/Foundation.h>

/**
 * @brief Bridge class between Objective-C and C++ LeafraSDK
 * 
 * This class handles the conversion between Objective-C types (used by React Native)
 * and C++ types (used by the LeafraSDK core library).
 */
@interface LeafraSDKBridge : NSObject

typedef void (^EventCallback)(NSString *message);

/**
 * @brief Initialize the SDK with configuration
 * @param config Configuration dictionary from React Native
 * @param error Error pointer for error handling
 * @return NSNumber containing the result code
 */
- (NSNumber *)initializeWithConfig:(NSDictionary *)config error:(NSError **)error;

/**
 * @brief Shutdown the SDK
 * @param error Error pointer for error handling
 * @return NSNumber containing the result code
 */
- (NSNumber *)shutdownWithError:(NSError **)error;

/**
 * @brief Check if SDK is initialized
 * @return YES if initialized, NO otherwise
 */
- (BOOL)isInitialized;

/**
 * @brief Get SDK version
 * @return Version string
 */
- (NSString *)getVersion;

/**
 * @brief Get platform information
 * @return Platform string
 */
- (NSString *)getPlatform;

/**
 * @brief Process data through the SDK
 * @param input Array of numbers representing input data
 * @param error Error pointer for error handling
 * @return Dictionary containing result code and output data
 */
- (NSDictionary *)processData:(NSArray<NSNumber *> *)input error:(NSError **)error;

/**
 * @brief Process user files through the SDK
 * @param fileUrls Array of file URL strings to process
 * @param error Error pointer for error handling
 * @return Dictionary containing result code and processing results
 */
- (NSDictionary *)processUserFiles:(NSArray<NSString *> *)fileUrls error:(NSError **)error;

/**
 * @brief Calculate distance between two 2D points
 * @param p1 First point dictionary with x, y keys
 * @param p2 Second point dictionary with x, y keys
 * @param error Error pointer for error handling
 * @return Distance as NSNumber
 */
- (NSNumber *)calculateDistance2D:(NSDictionary *)p1 point2:(NSDictionary *)p2 error:(NSError **)error;

/**
 * @brief Calculate distance between two 3D points
 * @param p1 First point dictionary with x, y, z keys
 * @param p2 Second point dictionary with x, y, z keys
 * @param error Error pointer for error handling
 * @return Distance as NSNumber
 */
- (NSNumber *)calculateDistance3D:(NSDictionary *)p1 point2:(NSDictionary *)p2 error:(NSError **)error;

/**
 * @brief Multiply two 3x3 matrices
 * @param matrixA First matrix dictionary with data array
 * @param matrixB Second matrix dictionary with data array
 * @param error Error pointer for error handling
 * @return Dictionary containing result code and resulting matrix
 */
- (NSDictionary *)multiplyMatrix3x3:(NSDictionary *)matrixA matrixB:(NSDictionary *)matrixB error:(NSError **)error;

/**
 * @brief Calculate determinant of a 3x3 matrix
 * @param matrix Matrix dictionary with data array
 * @param error Error pointer for error handling
 * @return Determinant as NSNumber
 */
- (NSNumber *)matrixDeterminant:(NSDictionary *)matrix error:(NSError **)error;

/**
 * @brief Semantic search method
 * @param query Search query
 * @param maxResults Maximum number of results
 * @param error Error pointer for error handling
 * @return Dictionary containing result code and search results
 */
- (NSDictionary *)semanticSearch:(NSString *)query maxResults:(NSNumber *)maxResults error:(NSError **)error;

/**
 * @brief Semantic search method with LLM
 * @param query Search query
 * @param maxResults Maximum number of results
 * @param error Error pointer for error handling
 * @return Dictionary containing result code and search results
 */
- (NSDictionary *)semanticSearchWithLLM:(NSString *)query maxResults:(NSNumber *)maxResults error:(NSError **)error;

/**
 * @brief Set event callback
 * @param callback Block to be called when events occur
 */
- (void)setEventCallback:(EventCallback)callback;

@end 