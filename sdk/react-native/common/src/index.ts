import { NativeModules, NativeEventEmitter } from 'react-native';

const { LeafraSDK: LeafraSDKNative } = NativeModules;

// TypeScript interfaces
export interface LeafraConfig {
  name?: string;
  version?: string;
  debugMode?: boolean;
  maxThreads?: number;
  bufferSize?: number;
  leafraDocumentDatabaseName?: string;
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