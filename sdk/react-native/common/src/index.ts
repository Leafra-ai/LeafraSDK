import { NativeModules, NativeEventEmitter } from 'react-native';

const { LeafraSDK: LeafraSDKNative } = NativeModules;

// TypeScript interfaces

// Chunking configuration
export interface ChunkingConfig {
  enabled?: boolean;
  chunkSize?: number;
  overlapPercentage?: number;
  preserveWordBoundaries?: boolean;
  includeMetadata?: boolean;
  sizeUnit?: string; // "CHARACTERS" or "TOKENS"
  tokenMethod?: string; // "SIMPLE" or "SENTENCEPIECE"
  printChunksFull?: boolean;
  printChunksBrief?: boolean;
  maxLines?: number;
}

// Tokenizer configuration
export interface TokenizerConfig {
  enableSentencepiece?: boolean;
  modelName?: string;
  sentencepieceModelPath?: string;
  sentencepieceJsonPath?: string;
}

// Embedding model configuration
export interface EmbeddingModelConfig {
  enabled?: boolean;
  framework?: string; // "coreml", "tensorflow_lite", "tensorflow"
  modelPath?: string;
  // CoreML specific
  coremlComputeUnits?: string; // "all", "cpuOnly", "cpuAndGPU", "cpuAndNeuralEngine"
  // TensorFlow Lite specific
  tfliteEnableCoremLDelegate?: boolean;
  tfliteEnableMetalDelegate?: boolean;
  tfliteEnableXnnpackDelegate?: boolean;
  tfliteNumThreads?: number;
  tfliteUseNnapi?: boolean;
}

// Vector search configuration
export interface VectorSearchConfig {
  enabled?: boolean;
  dimension?: number;
  indexType?: string; // "FLAT", "IVF_FLAT", "IVF_PQ", "HNSW", "LSH"
  metric?: string; // "L2", "INNER_PRODUCT", "COSINE"
  nlist?: number;
  nprobe?: number;
  m?: number;
  nbits?: number;
  hnswM?: number;
  lshNbits?: number;
  indexDefinition?: string;
  autoSave?: boolean;
  autoLoad?: boolean;
}

// LLM configuration
export interface LLMConfig {
  enabled?: boolean;
  modelPath?: string;
  framework?: string; // "llamacpp", "ollama", etc.
  // Context and processing parameters
  nCtx?: number;
  nPredict?: number;
  nBatch?: number;
  nUbatch?: number;
  nThreads?: number;
  nThreadsBatch?: number;
  // Generation parameters
  temperature?: number;
  topP?: number;
  topK?: number;
  minP?: number;
  repeatPenalty?: number;
  repeatLastN?: number;
  tfsZ?: number;
  typicalP?: number;
  // Performance and hardware parameters
  nGpuLayers?: number;
  useMmap?: boolean;
  useMlock?: boolean;
  numa?: boolean;
  // System configuration
  systemPrompt?: string;
  seed?: number;
  debugMode?: boolean;
  verbosePrompt?: boolean;
}

// Main configuration interface
export interface LeafraConfig {
  name?: string;
  version?: string;
  debugMode?: boolean;
  maxThreads?: number;
  bufferSize?: number;
  leafraDocumentDatabaseName?: string;
  chunking?: ChunkingConfig;
  tokenizer?: TokenizerConfig;
  embeddingInference?: EmbeddingModelConfig;
  vectorSearch?: VectorSearchConfig;
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