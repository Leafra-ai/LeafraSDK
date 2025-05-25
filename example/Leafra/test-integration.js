// Simple test to verify LeafraSDK integration
// This can be run in the React Native debugger console

console.log('🧪 Testing LeafraSDK Integration...');

// Test if the native module is available
try {
  const { NativeModules } = require('react-native');
  const LeafraSDKNative = NativeModules.LeafraSDK;
  
  if (LeafraSDKNative) {
    console.log('✅ LeafraSDK native module found!');
    console.log('📋 Available methods:', Object.keys(LeafraSDKNative));
    
    // Test basic method calls
    LeafraSDKNative.getVersion()
      .then(version => console.log('📱 SDK Version:', version))
      .catch(error => console.log('❌ Version error:', error));
      
    LeafraSDKNative.getPlatform()
      .then(platform => console.log('🖥️ Platform:', platform))
      .catch(error => console.log('❌ Platform error:', error));
      
    LeafraSDKNative.isInitialized()
      .then(initialized => console.log('🔧 Initialized:', initialized))
      .catch(error => console.log('❌ Initialization check error:', error));
      
  } else {
    console.log('❌ LeafraSDK native module not found');
    console.log('📋 Available modules:', Object.keys(NativeModules));
  }
} catch (error) {
  console.log('💥 Test error:', error);
}

console.log('🏁 Integration test complete'); 