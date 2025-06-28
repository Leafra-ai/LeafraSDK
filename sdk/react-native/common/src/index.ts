import { NativeModules, NativeEventEmitter } from 'react-native';

const { LeafraSDK: LeafraSDKNative } = NativeModules;

// TypeScript interfaces

// Chunking configuration
export interface ChunkingConfig {
  enabled?: boolean;
  chunk_size?: number;
  overlap_percentage?: number;
  preserve_word_boundaries?: boolean;
  include_metadata?: boolean;
  size_unit?: string; // "CHARACTERS" or "TOKENS"
  token_method?: string; // "SIMPLE" or "SENTENCEPIECE"
  print_chunks_full?: boolean;
  print_chunks_brief?: boolean;
  max_lines?: number;
}

// Tokenizer configuration
export interface TokenizerConfig {
  enabled?: boolean;
  model_name?: string;
  model_path?: string;
  model_json_path?: string;
}

// Embedding model configuration
export interface EmbeddingModelConfig {
  enabled?: boolean;
  framework?: string; // "coreml", "tensorflow_lite", "tensorflow"
  model_path?: string;
  // CoreML specific
  coreml_compute_units?: string; // "all", "cpuOnly", "cpuAndGPU", "cpuAndNeuralEngine"
  // TensorFlow Lite specific
  tflite_enable_coreml_delegate?: boolean;
  tflite_enable_metal_delegate?: boolean;
  tflite_enable_xnnpack_delegate?: boolean;
  tflite_num_threads?: number;
  tflite_use_nnapi?: boolean;
}

// Vector search configuration
export interface VectorSearchConfig {
  enabled?: boolean;
  dimension?: number;
  index_type?: string; // "FLAT", "IVF_FLAT", "IVF_PQ", "HNSW", "LSH"
  metric?: string; // "L2", "INNER_PRODUCT", "COSINE"
  nlist?: number;
  nprobe?: number;
  m?: number;
  nbits?: number;
  hnsw_m?: number;
  lsh_nbits?: number;
  index_definition?: string;
  auto_save?: boolean;
  auto_load?: boolean;
}

// LLM configuration
export interface LLMConfig {
  enabled?: boolean;
  model_path?: string;
  framework?: string; // "llamacpp", "ollama", etc.
  // Context and processing parameters
  n_ctx?: number;
  n_predict?: number;
  n_batch?: number;
  n_ubatch?: number;
  n_threads?: number;
  n_threads_batch?: number;
  // Generation parameters
  temperature?: number;
  top_p?: number;
  top_k?: number;
  min_p?: number;
  repeat_penalty?: number;
  repeat_last_n?: number;
  tfs_z?: number;
  typical_p?: number;
  // Performance and hardware parameters
  n_gpu_layers?: number;
  use_mmap?: boolean;
  use_mlock?: boolean;
  numa?: boolean;
  // System configuration
  system_prompt?: string;
  seed?: number;
  debug_mode?: boolean;
  verbose_prompt?: boolean;
}

// Main configuration interface
export interface LeafraConfig {
  name?: string;
  version?: string;
  debug_mode?: boolean;
  max_threads?: number;
  buffer_size?: number;
  leafra_document_database_name?: string;
  chunking?: ChunkingConfig;
  tokenizer?: TokenizerConfig;
  embedding_inference?: EmbeddingModelConfig;
  vector_search?: VectorSearchConfig;
  llm?: LLMConfig;
}

// Search result interface for semantic search
export interface SearchResult {
  id: number;         // Vector ID (FAISS ID)
  distance: number;   // Distance/similarity score
  // Optional chunk metadata
  docId?: number;     // Document ID from database
  chunkIndex?: number; // Chunk index within document
  pageNumber?: number; // Page number where chunk appears
  content?: string;   // Chunk text content
  filename?: string;  // Source document filename
}

export interface Point2D {
  x: number;
  y: number;
}

export interface Point3D {
  x: number;
  y: number;
  z: number;
}

export interface Matrix3x3 {
  data: number[]; // 9 elements representing 3x3 matrix
}

export interface ProcessDataResult {
  result: number;
  output: number[];
}

export interface MatrixMultiplyResult {
  result: number;
  matrix: Matrix3x3;
}

export enum ResultCode {
  SUCCESS = 0,
  ERROR_INITIALIZATION_FAILED = -1,
  ERROR_INVALID_PARAMETER = -2,
  ERROR_PROCESSING_FAILED = -3,
  ERROR_NOT_IMPLEMENTED = -4,
}

export type EventCallback = (message: string) => void;
export type TokenCallback = (token: string) => void;

class LeafraSDKManager {
  private eventEmitter: NativeEventEmitter;
  private eventCallback?: EventCallback;

  constructor() {
    this.eventEmitter = new NativeEventEmitter(LeafraSDKNative);
  }

  /**
   * Initialize the LeafraSDK with configuration
   */
  async initialize(config: LeafraConfig): Promise<ResultCode> {
    return LeafraSDKNative.initialize(config);
  }

  /**
   * Shutdown the LeafraSDK
   */
  async shutdown(): Promise<ResultCode> {
    return LeafraSDKNative.shutdown();
  }

  /**
   * Check if SDK is initialized
   */
  async isInitialized(): Promise<boolean> {
    return LeafraSDKNative.isInitialized();
  }

  /**
   * Get SDK version
   */
  async getVersion(): Promise<string> {
    return LeafraSDKNative.getVersion();
  }

  /**
   * Get platform information
   */
  async getPlatform(): Promise<string> {
    return LeafraSDKNative.getPlatform();
  }

  /**
   * Process data through the SDK
   */
  async processData(input: number[]): Promise<ProcessDataResult> {
    return LeafraSDKNative.processData(input);
  }

  /**
   * Calculate distance between two 2D points
   */
  async calculateDistance2D(p1: Point2D, p2: Point2D): Promise<number> {
    return LeafraSDKNative.calculateDistance2D(p1, p2);
  }

  /**
   * Calculate distance between two 3D points
   */
  async calculateDistance3D(p1: Point3D, p2: Point3D): Promise<number> {
    return LeafraSDKNative.calculateDistance3D(p1, p2);
  }

  /**
   * Multiply two 3x3 matrices
   */
  async multiplyMatrix3x3(matrixA: Matrix3x3, matrixB: Matrix3x3): Promise<MatrixMultiplyResult> {
    return LeafraSDKNative.multiplyMatrix3x3(matrixA, matrixB);
  }

  /**
   * Calculate determinant of a 3x3 matrix
   */
  async matrixDeterminant(matrix: Matrix3x3): Promise<number> {
    return LeafraSDKNative.matrixDeterminant(matrix);
  }

  /**
   * Perform semantic search using vector embeddings
   */
  async semanticSearch(query: string, maxResults: number): Promise<{ result: ResultCode; results: SearchResult[] }> {
    return LeafraSDKNative.semanticSearch(query, maxResults);
  }

  /**
   * Perform semantic search with LLM processing
   */
  async semanticSearchWithLLM(
    query: string, 
    maxResults: number, 
    tokenCallback?: TokenCallback
  ): Promise<{ result: ResultCode; results: SearchResult[] }> {
    if (tokenCallback) {
      // Set up token callback listener
      const subscription = this.eventEmitter.addListener('LeafraSDKTokenEvent', (event: { token: string }) => {
        if (event.token) {
          tokenCallback(event.token);
        }
      });
      
      try {
        const result = await LeafraSDKNative.semanticSearchWithLLM(query, maxResults);
        subscription.remove();
        return result;
      } catch (error) {
        subscription.remove();
        throw error;
      }
    } else {
      return LeafraSDKNative.semanticSearchWithLLM(query, maxResults);
    }
  }

  /**
   * Set event callback for SDK events
   */
  setEventCallback(callback: EventCallback): void {
    this.eventCallback = callback;
    this.eventEmitter.addListener('LeafraSDKEvent', (event: { message: string }) => {
      if (this.eventCallback && event.message) {
        this.eventCallback(event.message);
      }
    });
  }

  /**
   * Remove event callback
   */
  removeEventCallback(): void {
    this.eventCallback = undefined;
    this.eventEmitter.removeAllListeners('LeafraSDKEvent');
  }
}

// Singleton instance
let instance: LeafraSDKManager | null = null;

export const LeafraSDK = {
  getInstance(): LeafraSDKManager {
    if (!instance) {
      instance = new LeafraSDKManager();
    }
    return instance;
  }
};

// Export the manager class and native module
export { LeafraSDKManager, LeafraSDKNative };

// Default export
export default LeafraSDK;

// Export types and enums
export * from './types'; 