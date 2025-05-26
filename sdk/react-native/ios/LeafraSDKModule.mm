#import "LeafraSDKModule.h"
#import "LeafraSDKBridge.h"
#import <React/RCTLog.h>
#import <React/RCTUtils.h>

@interface LeafraSDKModule()
@property (nonatomic, assign) BOOL hasListeners;
@end

@implementation LeafraSDKModule

RCT_EXPORT_MODULE(LeafraSDK);

+ (BOOL)requiresMainQueueSetup {
    return NO;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _sdkBridge = [[LeafraSDKBridge alloc] init];
        _hasListeners = NO;
        
        // Set up event callback
        LeafraSDKModule* __weak weakSelf = self;
        [_sdkBridge setEventCallback:^(NSString *message) {
            LeafraSDKModule* strongSelf = weakSelf;
            if (strongSelf && strongSelf.hasListeners) {
                [strongSelf sendEventWithName:@"LeafraSDKEvent" body:@{@"message": message}];
            }
        }];
    }
    return self;
}

- (NSArray<NSString *> *)supportedEvents {
    return @[@"LeafraSDKEvent"];
}

- (void)startObserving {
    _hasListeners = YES;
}

- (void)stopObserving {
    _hasListeners = NO;
}

#pragma mark - SDK Methods

RCT_EXPORT_METHOD(initialize:(NSDictionary *)config
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error = nil;
        NSNumber *result = [self.sdkBridge initializeWithConfig:config error:&error];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (error) {
                reject(@"INITIALIZATION_ERROR", error.localizedDescription, error);
            } else {
                resolve(result);
            }
        });
    });
}

RCT_EXPORT_METHOD(shutdown:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error = nil;
        NSNumber *result = [self.sdkBridge shutdownWithError:&error];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (error) {
                reject(@"SHUTDOWN_ERROR", error.localizedDescription, error);
            } else {
                resolve(result);
            }
        });
    });
}

RCT_EXPORT_METHOD(isInitialized:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    BOOL initialized = [self.sdkBridge isInitialized];
    resolve(@(initialized));
}

RCT_EXPORT_METHOD(getVersion:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    NSString *version = [self.sdkBridge getVersion];
    resolve(version);
}

RCT_EXPORT_METHOD(getPlatform:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    NSString *platform = [self.sdkBridge getPlatform];
    resolve(platform);
}

RCT_EXPORT_METHOD(processData:(NSArray<NSNumber *> *)input
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error = nil;
        NSDictionary *result = [self.sdkBridge processData:input error:&error];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (error) {
                reject(@"PROCESS_DATA_ERROR", error.localizedDescription, error);
            } else {
                resolve(result);
            }
        });
    });
}

RCT_EXPORT_METHOD(processUserFiles:(NSArray<NSString *> *)fileUrls
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSError *error = nil;
        NSDictionary *result = [self.sdkBridge processUserFiles:fileUrls error:&error];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            if (error) {
                reject(@"PROCESS_FILES_ERROR", error.localizedDescription, error);
            } else {
                resolve(result);
            }
        });
    });
}

RCT_EXPORT_METHOD(calculateDistance2D:(NSDictionary *)p1
                  point2:(NSDictionary *)p2
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    NSError *error = nil;
    NSNumber *distance = [self.sdkBridge calculateDistance2D:p1 point2:p2 error:&error];
    
    if (error) {
        reject(@"DISTANCE_2D_ERROR", error.localizedDescription, error);
    } else {
        resolve(distance);
    }
}

RCT_EXPORT_METHOD(calculateDistance3D:(NSDictionary *)p1
                  point2:(NSDictionary *)p2
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    NSError *error = nil;
    NSNumber *distance = [self.sdkBridge calculateDistance3D:p1 point2:p2 error:&error];
    
    if (error) {
        reject(@"DISTANCE_3D_ERROR", error.localizedDescription, error);
    } else {
        resolve(distance);
    }
}

RCT_EXPORT_METHOD(multiplyMatrix3x3:(NSDictionary *)matrixA
                  matrixB:(NSDictionary *)matrixB
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    NSError *error = nil;
    NSDictionary *result = [self.sdkBridge multiplyMatrix3x3:matrixA matrixB:matrixB error:&error];
    
    if (error) {
        reject(@"MATRIX_MULTIPLY_ERROR", error.localizedDescription, error);
    } else {
        resolve(result);
    }
}

RCT_EXPORT_METHOD(matrixDeterminant:(NSDictionary *)matrix
                  resolver:(RCTPromiseResolveBlock)resolve
                  rejecter:(RCTPromiseRejectBlock)reject) {
    NSError *error = nil;
    NSNumber *determinant = [self.sdkBridge matrixDeterminant:matrix error:&error];
    
    if (error) {
        reject(@"MATRIX_DETERMINANT_ERROR", error.localizedDescription, error);
    } else {
        resolve(determinant);
    }
}

@end 