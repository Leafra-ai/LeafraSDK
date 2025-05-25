import { NativeModules, Platform } from 'react-native';

const LINKING_ERROR =
  `The package 'react-native-leafra-sdk' doesn't seem to be linked. Make sure: \n\n` +
  Platform.select({ ios: "- You have run 'cd ios && pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo managed workflow\n';

// TypeScript interfaces
export interface LeafraConfig {
  name: string;
  version: string;
  debugMode?: boolean;
  maxThreads?: number;
  bufferSize?: number;
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
  data: number[]; // 9 elements
}

export enum ResultCode {
  SUCCESS = 0,
  ERROR_INVALID_PARAMETER = -1,
  ERROR_INITIALIZATION_FAILED = -2,
  ERROR_PROCESSING_FAILED = -3,
  ERROR_NOT_IMPLEMENTED = -4,
  ERROR_OUT_OF_MEMORY = -5,
}

// Native module interface
interface LeafraSDKNativeModule {
  initialize(config: LeafraConfig): Promise<number>;
  shutdown(): Promise<number>;
  isInitialized(): Promise<boolean>;
  getVersion(): Promise<string>;
  getPlatform(): Promise<string>;
  processData(input: number[]): Promise<{ result: number; output: number[] }>;
  calculateDistance2D(p1: Point2D, p2: Point2D): Promise<number>;
  calculateDistance3D(p1: Point3D, p2: Point3D): Promise<number>;
  multiplyMatrix3x3(a: Matrix3x3, b: Matrix3x3): Promise<{ result: number; matrix: Matrix3x3 }>;
  matrixDeterminant(matrix: Matrix3x3): Promise<number>;
  setEventCallback(callback: (message: string) => void): void;
}

// Get the native module
const LeafraSDKNative = NativeModules.LeafraSDK
  ? NativeModules.LeafraSDK
  : new Proxy(
      {},
      {
        get() {
          throw new Error(LINKING_ERROR);
        },
      }
    ) as LeafraSDKNativeModule;

// Main SDK class
export class LeafraSDK {
  private static instance: LeafraSDK | null = null;
  private eventCallback: ((message: string) => void) | null = null;

  private constructor() {}

  /**
   * Get singleton instance of LeafraSDK
   */
  static getInstance(): LeafraSDK {
    if (!LeafraSDK.instance) {
      LeafraSDK.instance = new LeafraSDK();
    }
    return LeafraSDK.instance;
  }

  /**
   * Initialize the SDK with configuration
   */
  async initialize(config: LeafraConfig): Promise<ResultCode> {
    try {
      const result = await LeafraSDKNative.initialize(config);
      return result as ResultCode;
    } catch (error) {
      console.error('LeafraSDK initialization failed:', error);
      return ResultCode.ERROR_INITIALIZATION_FAILED;
    }
  }

  /**
   * Shutdown the SDK
   */
  async shutdown(): Promise<ResultCode> {
    try {
      const result = await LeafraSDKNative.shutdown();
      return result as ResultCode;
    } catch (error) {
      console.error('LeafraSDK shutdown failed:', error);
      return ResultCode.ERROR_PROCESSING_FAILED;
    }
  }

  /**
   * Check if SDK is initialized
   */
  async isInitialized(): Promise<boolean> {
    try {
      return await LeafraSDKNative.isInitialized();
    } catch (error) {
      console.error('LeafraSDK isInitialized failed:', error);
      return false;
    }
  }

  /**
   * Get SDK version
   */
  async getVersion(): Promise<string> {
    try {
      return await LeafraSDKNative.getVersion();
    } catch (error) {
      console.error('LeafraSDK getVersion failed:', error);
      return '0.0.0';
    }
  }

  /**
   * Get platform information
   */
  async getPlatform(): Promise<string> {
    try {
      return await LeafraSDKNative.getPlatform();
    } catch (error) {
      console.error('LeafraSDK getPlatform failed:', error);
      return 'Unknown';
    }
  }

  /**
   * Process data through the SDK
   */
  async processData(input: number[]): Promise<{ result: ResultCode; output: number[] }> {
    try {
      const response = await LeafraSDKNative.processData(input);
      return {
        result: response.result as ResultCode,
        output: response.output,
      };
    } catch (error) {
      console.error('LeafraSDK processData failed:', error);
      return {
        result: ResultCode.ERROR_PROCESSING_FAILED,
        output: [],
      };
    }
  }

  /**
   * Calculate distance between two 2D points
   */
  async calculateDistance2D(p1: Point2D, p2: Point2D): Promise<number> {
    try {
      return await LeafraSDKNative.calculateDistance2D(p1, p2);
    } catch (error) {
      console.error('LeafraSDK calculateDistance2D failed:', error);
      return 0;
    }
  }

  /**
   * Calculate distance between two 3D points
   */
  async calculateDistance3D(p1: Point3D, p2: Point3D): Promise<number> {
    try {
      return await LeafraSDKNative.calculateDistance3D(p1, p2);
    } catch (error) {
      console.error('LeafraSDK calculateDistance3D failed:', error);
      return 0;
    }
  }

  /**
   * Multiply two 3x3 matrices
   */
  async multiplyMatrix3x3(a: Matrix3x3, b: Matrix3x3): Promise<{ result: ResultCode; matrix: Matrix3x3 }> {
    try {
      const response = await LeafraSDKNative.multiplyMatrix3x3(a, b);
      return {
        result: response.result as ResultCode,
        matrix: response.matrix,
      };
    } catch (error) {
      console.error('LeafraSDK multiplyMatrix3x3 failed:', error);
      return {
        result: ResultCode.ERROR_PROCESSING_FAILED,
        matrix: { data: new Array(9).fill(0) },
      };
    }
  }

  /**
   * Calculate determinant of a 3x3 matrix
   */
  async matrixDeterminant(matrix: Matrix3x3): Promise<number> {
    try {
      return await LeafraSDKNative.matrixDeterminant(matrix);
    } catch (error) {
      console.error('LeafraSDK matrixDeterminant failed:', error);
      return 0;
    }
  }

  /**
   * Set event callback for SDK events
   */
  setEventCallback(callback: (message: string) => void): void {
    this.eventCallback = callback;
    try {
      LeafraSDKNative.setEventCallback(callback);
    } catch (error) {
      console.error('LeafraSDK setEventCallback failed:', error);
    }
  }

  /**
   * Remove event callback
   */
  removeEventCallback(): void {
    this.eventCallback = null;
    try {
      LeafraSDKNative.setEventCallback(() => {});
    } catch (error) {
      console.error('LeafraSDK removeEventCallback failed:', error);
    }
  }
}

// Export the singleton instance
export default LeafraSDK.getInstance();

// Export types and enums
export * from './types'; 