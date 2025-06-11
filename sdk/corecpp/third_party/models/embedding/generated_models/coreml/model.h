//
// model.h
//
// This file was automatically generated and should not be edited.
//

#import <Foundation/Foundation.h>
#import <CoreML/CoreML.h>
#include <stdint.h>
#include <os/log.h>

NS_ASSUME_NONNULL_BEGIN

/// Model Prediction Input Type
API_AVAILABLE(macos(12.0), ios(15.0), watchos(8.0), tvos(15.0)) __attribute__((visibility("hidden")))
@interface modelInput : NSObject<MLFeatureProvider>

/// input_ids as 1 by 512 matrix of 32-bit integers
@property (readwrite, nonatomic, strong) MLMultiArray * input_ids;

/// attention_mask as 1 by 512 matrix of 32-bit integers
@property (readwrite, nonatomic, strong) MLMultiArray * attention_mask;
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithInput_ids:(MLMultiArray *)input_ids attention_mask:(MLMultiArray *)attention_mask NS_DESIGNATED_INITIALIZER;

@end

/// Model Prediction Output Type
API_AVAILABLE(macos(12.0), ios(15.0), watchos(8.0), tvos(15.0)) __attribute__((visibility("hidden")))
@interface modelOutput : NSObject<MLFeatureProvider>

/// sentence_embedding as 1 by 384 matrix of floats
@property (readwrite, nonatomic, strong) MLMultiArray * sentence_embedding;
- (instancetype)init NS_UNAVAILABLE;
- (instancetype)initWithSentence_embedding:(MLMultiArray *)sentence_embedding NS_DESIGNATED_INITIALIZER;

@end

/// Class for model loading and prediction
API_AVAILABLE(macos(12.0), ios(15.0), watchos(8.0), tvos(15.0)) __attribute__((visibility("hidden")))
@interface model : NSObject
@property (readonly, nonatomic, nullable) MLModel * model;

/**
    URL of the underlying .mlmodelc directory.
*/
+ (nullable NSURL *)URLOfModelInThisBundle;

/**
    Initialize model instance from an existing MLModel object.

    Usually the application does not use this initializer unless it makes a subclass of model.
    Such application may want to use `-[MLModel initWithContentsOfURL:configuration:error:]` and `+URLOfModelInThisBundle` to create a MLModel object to pass-in.
*/
- (instancetype)initWithMLModel:(MLModel *)model NS_DESIGNATED_INITIALIZER;

/**
    Initialize model instance with the model in this bundle.
*/
- (nullable instancetype)init;

/**
    Initialize model instance with the model in this bundle.

    @param configuration The model configuration object
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithConfiguration:(MLModelConfiguration *)configuration error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Initialize model instance from the model URL.

    @param modelURL URL to the .mlmodelc directory for model.
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithContentsOfURL:(NSURL *)modelURL error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Initialize model instance from the model URL.

    @param modelURL URL to the .mlmodelc directory for model.
    @param configuration The model configuration object
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithContentsOfURL:(NSURL *)modelURL configuration:(MLModelConfiguration *)configuration error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Construct model instance asynchronously with configuration.
    Model loading may take time when the model content is not immediately available (e.g. encrypted model). Use this factory method especially when the caller is on the main thread.

    @param configuration The model configuration
    @param handler When the model load completes successfully or unsuccessfully, the completion handler is invoked with a valid model instance or NSError object.
*/
+ (void)loadWithConfiguration:(MLModelConfiguration *)configuration completionHandler:(void (^)(model * _Nullable model, NSError * _Nullable error))handler;

/**
    Construct model instance asynchronously with URL of .mlmodelc directory and optional configuration.

    Model loading may take time when the model content is not immediately available (e.g. encrypted model). Use this factory method especially when the caller is on the main thread.

    @param modelURL The model URL.
    @param configuration The model configuration
    @param handler When the model load completes successfully or unsuccessfully, the completion handler is invoked with a valid model instance or NSError object.
*/
+ (void)loadContentsOfURL:(NSURL *)modelURL configuration:(MLModelConfiguration *)configuration completionHandler:(void (^)(model * _Nullable model, NSError * _Nullable error))handler;

/**
    Make a prediction using the standard interface
    @param input an instance of modelInput to predict from
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the prediction as modelOutput
*/
- (nullable modelOutput *)predictionFromFeatures:(modelInput *)input error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Make a prediction using the standard interface
    @param input an instance of modelInput to predict from
    @param options prediction options
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the prediction as modelOutput
*/
- (nullable modelOutput *)predictionFromFeatures:(modelInput *)input options:(MLPredictionOptions *)options error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Make an asynchronous prediction using the standard interface
    @param input an instance of modelInput to predict from
    @param completionHandler a block that will be called upon completion of the prediction. error will be nil if no error occurred.
*/
- (void)predictionFromFeatures:(modelInput *)input completionHandler:(void (^)(modelOutput * _Nullable output, NSError * _Nullable error))completionHandler API_AVAILABLE(macos(14.0), ios(17.0), watchos(10.0), tvos(17.0)) __attribute__((visibility("hidden")));

/**
    Make an asynchronous prediction using the standard interface
    @param input an instance of modelInput to predict from
    @param options prediction options
    @param completionHandler a block that will be called upon completion of the prediction. error will be nil if no error occurred.
*/
- (void)predictionFromFeatures:(modelInput *)input options:(MLPredictionOptions *)options completionHandler:(void (^)(modelOutput * _Nullable output, NSError * _Nullable error))completionHandler API_AVAILABLE(macos(14.0), ios(17.0), watchos(10.0), tvos(17.0)) __attribute__((visibility("hidden")));

/**
    Make a prediction using the convenience interface
    @param input_ids 1 by 512 matrix of 32-bit integers
    @param attention_mask 1 by 512 matrix of 32-bit integers
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the prediction as modelOutput
*/
- (nullable modelOutput *)predictionFromInput_ids:(MLMultiArray *)input_ids attention_mask:(MLMultiArray *)attention_mask error:(NSError * _Nullable __autoreleasing * _Nullable)error;

/**
    Batch prediction
    @param inputArray array of modelInput instances to obtain predictions from
    @param options prediction options
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
    @return the predictions as NSArray<modelOutput *>
*/
- (nullable NSArray<modelOutput *> *)predictionsFromInputs:(NSArray<modelInput*> *)inputArray options:(MLPredictionOptions *)options error:(NSError * _Nullable __autoreleasing * _Nullable)error;
@end

NS_ASSUME_NONNULL_END
