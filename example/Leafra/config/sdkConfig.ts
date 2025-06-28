// SDK Configuration

// Helper function to resolve paths to LeafraCore.framework's resource bundle
const addFrameworkResourcePathPrefix = (filename: string): string => {
  // For React Native, we need to resolve to the framework's resource bundle
  // The framework structure is: LeafraCore.framework/LeafraResources.bundle/models/
  return `LeafraCore.framework/LeafraResources.bundle/models/${filename}`;
};

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
    const baseConfig = type === 'chat' ? SDKConfig.CHAT_CONFIG : SDKConfig.TEST_CONFIG;
    
    return {
      ...baseConfig,
      debug_mode: this.debugMode, // Override with current debug mode from manager
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

// Chat-specific config - matches sdkcmdline.cpp configuration
const CHAT_CONFIG = {
  name: 'LeafraSDK-Chat',
  version: '1.0.0',
  debug_mode: getDebugMode(),
  
  // Chunking configuration (matching sdkcmdline.cpp)
  chunking: {
    enabled: true,
    chunk_size: 400,  // 400 tokens per chunk (matches sdkcmdline)
    overlap_percentage: 0.2,  // 20% overlap
    size_unit: 'TOKENS',
    token_method: 'SIMPLE',
    preserve_word_boundaries: true,
    include_metadata: true,
    print_chunks_full: false,
    print_chunks_brief: true,
    max_lines: 5,
  },
  
  // Tokenizer configuration - resolved to framework bundle
  tokenizer: {
    enabled: true,
    model_name: 'multilingual-e5-small',
    model_path: addFrameworkResourcePathPrefix('sentencepiece.bpe.model'),
    model_json_path: addFrameworkResourcePathPrefix('tokenizer_config.json'),
  },
  
  // Embedding inference configuration - resolved to framework bundle
  embedding_inference: {
    enabled: true,
    framework: 'coreml',
    model_path: addFrameworkResourcePathPrefix('e5_embedding_model_i512a512_FP32.mlmodelc'),
  },
  
  // Vector search configuration
  vector_search: {
    enabled: true,
    index_type: 'FLAT',  // matches sdkcmdline.cpp
    metric: 'COSINE',
    dimension: 384,
  },
  
  // LLM configuration - resolved to framework bundle
  llm: {
    enabled: true,
    model_path: addFrameworkResourcePathPrefix('Llama-3.2-3B-Instruct-Q4_K_M.gguf'),
    n_ctx: 4096,  // max tokens for context
    n_predict: 256,  // max tokens to generate
  },
};

export const SDKConfig = {
  // Smart debug mode - respects environment overrides
  DEBUG_MODE: getDebugMode(),
  
  // Alternative: Manual override (uncomment to use)
  // DEBUG_MODE: true,  // Force debug on
  // DEBUG_MODE: false, // Force debug off
  

  // Chat-specific config - matches sdkcmdline.cpp configuration
  CHAT_CONFIG,
  
  // Test-specific config - assignment from chat config with name override
  TEST_CONFIG: (() => {
    const config = { ...CHAT_CONFIG };
    config.name = 'LeafraSDK-Test';
    return config;
  })(),
  
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