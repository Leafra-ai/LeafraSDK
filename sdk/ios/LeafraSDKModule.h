#pragma once

#import <React/RCTBridgeModule.h>
#import <React/RCTEventEmitter.h>

@class LeafraSDKBridge;

/**
 * @brief React Native module for LeafraSDK
 * 
 * This module provides the React Native interface to the LeafraSDK C++ library.
 * It handles initialization, data processing, and mathematical operations.
 */
@interface LeafraSDKModule : RCTEventEmitter <RCTBridgeModule>

@property (nonatomic, strong) LeafraSDKBridge *sdkBridge;

@end 