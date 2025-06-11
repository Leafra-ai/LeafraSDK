//
// model.m
//
// This file was automatically generated and should not be edited.
//

#if !__has_feature(objc_arc)
#error This file must be compiled with automatic reference counting enabled (-fobjc-arc)
#endif

#import "model.h"

@implementation modelInput

- (instancetype)initWithInput_ids:(MLMultiArray *)input_ids attention_mask:(MLMultiArray *)attention_mask {
    self = [super init];
    if (self) {
        _input_ids = input_ids;
        _attention_mask = attention_mask;
    }
    return self;
}

- (NSSet<NSString *> *)featureNames {
    return [NSSet setWithArray:@[@"input_ids", @"attention_mask"]];
}

- (nullable MLFeatureValue *)featureValueForName:(NSString *)featureName {
    if ([featureName isEqualToString:@"input_ids"]) {
        return [MLFeatureValue featureValueWithMultiArray:self.input_ids];
    }
    if ([featureName isEqualToString:@"attention_mask"]) {
        return [MLFeatureValue featureValueWithMultiArray:self.attention_mask];
    }
    return nil;
}

@end

@implementation modelOutput

- (instancetype)initWithSentence_embedding:(MLMultiArray *)sentence_embedding {
    self = [super init];
    if (self) {
        _sentence_embedding = sentence_embedding;
    }
    return self;
}

- (NSSet<NSString *> *)featureNames {
    return [NSSet setWithArray:@[@"sentence_embedding"]];
}

- (nullable MLFeatureValue *)featureValueForName:(NSString *)featureName {
    if ([featureName isEqualToString:@"sentence_embedding"]) {
        return [MLFeatureValue featureValueWithMultiArray:self.sentence_embedding];
    }
    return nil;
}

@end

@implementation model


/**
    URL of the underlying .mlmodelc directory.
*/
+ (nullable NSURL *)URLOfModelInThisBundle {
    NSString *assetPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"model" ofType:@"mlmodelc"];
    if (nil == assetPath) { os_log_error(OS_LOG_DEFAULT, "Could not load model.mlmodelc in the bundle resource"); return nil; }
    return [NSURL fileURLWithPath:assetPath];
}


/**
    Initialize model instance from an existing MLModel object.

    Usually the application does not use this initializer unless it makes a subclass of model.
    Such application may want to use `-[MLModel initWithContentsOfURL:configuration:error:]` and `+URLOfModelInThisBundle` to create a MLModel object to pass-in.
*/
- (instancetype)initWithMLModel:(MLModel *)model {
    if (model == nil) {
        return nil;
    }
    self = [super init];
    if (self != nil) {
        _model = model;
    }
    return self;
}


/**
    Initialize model instance with the model in this bundle.
*/
- (nullable instancetype)init {
    return [self initWithContentsOfURL:(NSURL * _Nonnull)self.class.URLOfModelInThisBundle error:nil];
}


/**
    Initialize model instance with the model in this bundle.

    @param configuration The model configuration object
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithConfiguration:(MLModelConfiguration *)configuration error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    return [self initWithContentsOfURL:(NSURL * _Nonnull)self.class.URLOfModelInThisBundle configuration:configuration error:error];
}


/**
    Initialize model instance from the model URL.

    @param modelURL URL to the .mlmodelc directory for model.
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithContentsOfURL:(NSURL *)modelURL error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    MLModel *model = [MLModel modelWithContentsOfURL:modelURL error:error];
    if (model == nil) { return nil; }
    return [self initWithMLModel:model];
}


/**
    Initialize model instance from the model URL.

    @param modelURL URL to the .mlmodelc directory for model.
    @param configuration The model configuration object
    @param error If an error occurs, upon return contains an NSError object that describes the problem. If you are not interested in possible errors, pass in NULL.
*/
- (nullable instancetype)initWithContentsOfURL:(NSURL *)modelURL configuration:(MLModelConfiguration *)configuration error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    MLModel *model = [MLModel modelWithContentsOfURL:modelURL configuration:configuration error:error];
    if (model == nil) { return nil; }
    return [self initWithMLModel:model];
}


/**
    Construct model instance asynchronously with configuration.
    Model loading may take time when the model content is not immediately available (e.g. encrypted model). Use this factory method especially when the caller is on the main thread.

    @param configuration The model configuration
    @param handler When the model load completes successfully or unsuccessfully, the completion handler is invoked with a valid model instance or NSError object.
*/
+ (void)loadWithConfiguration:(MLModelConfiguration *)configuration completionHandler:(void (^)(model * _Nullable model, NSError * _Nullable error))handler {
    [self loadContentsOfURL:(NSURL * _Nonnull)[self URLOfModelInThisBundle]
              configuration:configuration
          completionHandler:handler];
}


/**
    Construct model instance asynchronously with URL of .mlmodelc directory and optional configuration.

    Model loading may take time when the model content is not immediately available (e.g. encrypted model). Use this factory method especially when the caller is on the main thread.

    @param modelURL The model URL.
    @param configuration The model configuration
    @param handler When the model load completes successfully or unsuccessfully, the completion handler is invoked with a valid model instance or NSError object.
*/
+ (void)loadContentsOfURL:(NSURL *)modelURL configuration:(MLModelConfiguration *)configuration completionHandler:(void (^)(model * _Nullable model, NSError * _Nullable error))handler {
    [MLModel loadContentsOfURL:modelURL
                 configuration:configuration
             completionHandler:^(MLModel *model, NSError *error) {
        if (model != nil) {
            model *typedModel = [[model alloc] initWithMLModel:model];
            handler(typedModel, nil);
        } else {
            handler(nil, error);
        }
    }];
}

- (nullable modelOutput *)predictionFromFeatures:(modelInput *)input error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    return [self predictionFromFeatures:input options:[[MLPredictionOptions alloc] init] error:error];
}

- (nullable modelOutput *)predictionFromFeatures:(modelInput *)input options:(MLPredictionOptions *)options error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    id<MLFeatureProvider> outFeatures = [self.model predictionFromFeatures:input options:options error:error];
    if (!outFeatures) { return nil; }
    return [[modelOutput alloc] initWithSentence_embedding:(MLMultiArray *)[outFeatures featureValueForName:@"sentence_embedding"].multiArrayValue];
}

- (void)predictionFromFeatures:(modelInput *)input completionHandler:(void (^)(modelOutput * _Nullable output, NSError * _Nullable error))completionHandler {
    [self.model predictionFromFeatures:input completionHandler:^(id<MLFeatureProvider> prediction, NSError *predictionError) {
        if (prediction != nil) {
            modelOutput *output = [[modelOutput alloc] initWithSentence_embedding:(MLMultiArray *)[prediction featureValueForName:@"sentence_embedding"].multiArrayValue];
            completionHandler(output, predictionError);
        } else {
            completionHandler(nil, predictionError);
        }
    }];
}

- (void)predictionFromFeatures:(modelInput *)input options:(MLPredictionOptions *)options completionHandler:(void (^)(modelOutput * _Nullable output, NSError * _Nullable error))completionHandler {
    [self.model predictionFromFeatures:input options:options completionHandler:^(id<MLFeatureProvider> prediction, NSError *predictionError) {
        if (prediction != nil) {
            modelOutput *output = [[modelOutput alloc] initWithSentence_embedding:(MLMultiArray *)[prediction featureValueForName:@"sentence_embedding"].multiArrayValue];
            completionHandler(output, predictionError);
        } else {
            completionHandler(nil, predictionError);
        }
    }];
}

- (nullable modelOutput *)predictionFromInput_ids:(MLMultiArray *)input_ids attention_mask:(MLMultiArray *)attention_mask error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    modelInput *input_ = [[modelInput alloc] initWithInput_ids:input_ids attention_mask:attention_mask];
    return [self predictionFromFeatures:input_ error:error];
}

- (nullable NSArray<modelOutput *> *)predictionsFromInputs:(NSArray<modelInput*> *)inputArray options:(MLPredictionOptions *)options error:(NSError * _Nullable __autoreleasing * _Nullable)error {
    id<MLBatchProvider> inBatch = [[MLArrayBatchProvider alloc] initWithFeatureProviderArray:inputArray];
    id<MLBatchProvider> outBatch = [self.model predictionsFromBatch:inBatch options:options error:error];
    if (!outBatch) { return nil; }
    NSMutableArray<modelOutput*> *results = [NSMutableArray arrayWithCapacity:(NSUInteger)outBatch.count];
    for (NSInteger i = 0; i < outBatch.count; i++) {
        id<MLFeatureProvider> resultProvider = [outBatch featuresAtIndex:i];
        modelOutput * result = [[modelOutput alloc] initWithSentence_embedding:(MLMultiArray *)[resultProvider featureValueForName:@"sentence_embedding"].multiArrayValue];
        [results addObject:result];
    }
    return results;
}

@end
