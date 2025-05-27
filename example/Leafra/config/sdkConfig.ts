// SDK Configuration
export class SDKConfigManager {
  private static instance: SDKConfigManager;
  private debugMode: boolean = __DEV__;
  
  static getInstance(): SDKConfigManager {
    if (!SDKConfigManager.instance) {
      SDKConfigManager.instance = new SDKConfigManager();
    }
    return SDKConfigManager.instance;
  }
  
  setDebugMode(enabled: boolean) {
    this.debugMode = enabled;
  }
  
  getDebugMode(): boolean {
    return this.debugMode;
  }
  
  getConfig(type: 'chat' | 'test' = 'chat') {
    const baseConfig = type === 'chat' ? {
      name: 'LeafraChatApp',
      version: '1.0.0',
      maxThreads: 2,
      bufferSize: 1024,
    } : {
      name: 'LeafraTestApp',
      version: '1.0.0',
      maxThreads: 2,
      bufferSize: 512,
    };
    
    return {
      ...baseConfig,
      debugMode: this.debugMode,
    };
  }
}

// Environment configuration
const ENV_CONFIG = {
  // Force debug mode regardless of __DEV__ (useful for testing)
  FORCE_DEBUG: false,
  
  // Force production mode regardless of __DEV__ (useful for testing release builds with debug off)
  FORCE_PRODUCTION: false,
  
  // Custom debug levels
  DEBUG_LEVEL: 'auto', // 'auto' | 'debug' | 'info' | 'warning' | 'error' | 'none'
};

// Smart debug mode calculation
const getDebugMode = (): boolean => {
  if (ENV_CONFIG.FORCE_DEBUG) return true;
  if (ENV_CONFIG.FORCE_PRODUCTION) return false;
  return __DEV__; // Default to React Native's __DEV__
};

export const SDKConfig = {
  // Smart debug mode - respects environment overrides
  DEBUG_MODE: getDebugMode(),
  
  // Alternative: Manual override (uncomment to use)
  // DEBUG_MODE: true,  // Force debug on
  // DEBUG_MODE: false, // Force debug off
  
  // Common settings
  DEFAULT_CONFIG: {
    name: 'LeafraApp',
    version: '1.0.0',
    debugMode: getDebugMode(),
    maxThreads: 2,
    bufferSize: 1024,
  },
  
  // Chat-specific config
  CHAT_CONFIG: {
    name: 'LeafraChatApp',
    version: '1.0.0',
    debugMode: getDebugMode(),
    maxThreads: 2,
    bufferSize: 1024,
  },
  
  // Test-specific config
  TEST_CONFIG: {
    name: 'LeafraTestApp',
    version: '1.0.0',
    debugMode: getDebugMode(),
    maxThreads: 2,
    bufferSize: 512,
  },
  
  // Environment info
  ENV: {
    IS_DEV: __DEV__,
    IS_DEBUG_FORCED: ENV_CONFIG.FORCE_DEBUG,
    IS_PRODUCTION_FORCED: ENV_CONFIG.FORCE_PRODUCTION,
    DEBUG_LEVEL: ENV_CONFIG.DEBUG_LEVEL,
  },
};

// Helper function to get config based on environment
export const getSDKConfig = (type: 'chat' | 'test' = 'chat') => {
  return SDKConfigManager.getInstance().getConfig(type);
};

// Helper to check current environment
export const getEnvironmentInfo = () => {
  return {
    __DEV__,
    debugMode: getDebugMode(),
    buildType: __DEV__ ? 'development' : 'production',
    forceDebug: ENV_CONFIG.FORCE_DEBUG,
    forceProduction: ENV_CONFIG.FORCE_PRODUCTION,
  };
}; 