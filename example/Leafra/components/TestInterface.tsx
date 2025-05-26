import React, { useState } from 'react';
import { StyleSheet, Text, View, Button, ScrollView, NativeModules } from 'react-native';

// Direct import from NativeModules instead of package
const { LeafraSDK: LeafraSDKNative } = NativeModules;

// Define types locally
interface LeafraConfig {
  name?: string;
  version?: string;
  debugMode?: boolean;
  maxThreads?: number;
  bufferSize?: number;
}

interface Point2D {
  x: number;
  y: number;
}

interface Point3D {
  x: number;
  y: number;
  z: number;
}

interface Matrix3x3 {
  data: number[];
}

interface ProcessDataResult {
  result: number;
  output: number[];
}

enum ResultCode {
  SUCCESS = 0,
  ERROR_INITIALIZATION_FAILED = -1,
  ERROR_INVALID_PARAMETER = -2,
  ERROR_PROCESSING_FAILED = -3,
  ERROR_NOT_IMPLEMENTED = -4,
}

// Simple SDK wrapper
const LeafraSDK = {
  async initialize(config: LeafraConfig): Promise<ResultCode> {
    return LeafraSDKNative.initialize(config);
  },
  
  async getVersion(): Promise<string> {
    return LeafraSDKNative.getVersion();
  },
  
  async getPlatform(): Promise<string> {
    return LeafraSDKNative.getPlatform();
  },
  
  async isInitialized(): Promise<boolean> {
    return LeafraSDKNative.isInitialized();
  },
  
  async calculateDistance2D(p1: Point2D, p2: Point2D): Promise<number> {
    return LeafraSDKNative.calculateDistance2D(p1, p2);
  },
  
  async calculateDistance3D(p1: Point3D, p2: Point3D): Promise<number> {
    return LeafraSDKNative.calculateDistance3D(p1, p2);
  },
  
  async processData(input: number[]): Promise<ProcessDataResult> {
    return LeafraSDKNative.processData(input);
  },
  
  async matrixDeterminant(matrix: Matrix3x3): Promise<number> {
    return LeafraSDKNative.matrixDeterminant(matrix);
  }
};

interface TestInterfaceProps {
  onBack: () => void;
}

export default function TestInterface({ onBack }: TestInterfaceProps) {
  const [sdkStatus, setSdkStatus] = useState('Not initialized');
  const [results, setResults] = useState<string[]>([]);

  const addResult = (message: string) => {
    setResults(prev => [...prev.slice(-9), `${new Date().toLocaleTimeString()}: ${message}`]);
  };

  const testSDKIntegration = async () => {
    try {
      addResult('✅ Testing LeafraSDK integration...');
      
      // Check if native module exists
      if (!LeafraSDKNative) {
        addResult('❌ LeafraSDK native module not found');
        addResult('📋 Available modules: ' + Object.keys(NativeModules).join(', '));
        setSdkStatus('Module not found');
        return;
      }
      
      addResult('✅ LeafraSDK native module found!');
      addResult('📋 Available methods: ' + Object.keys(LeafraSDKNative).join(', '));
      
      const config: LeafraConfig = {
        name: 'LeafraTestApp',
        version: '1.0.0',
        debugMode: true,
        maxThreads: 2,
        bufferSize: 512
      };

      const result = await LeafraSDK.initialize(config);
      
      if (result === ResultCode.SUCCESS) {
        setSdkStatus('Initialized');
        addResult('✅ SDK initialized successfully');
        
        // Get SDK info
        const version = await LeafraSDK.getVersion();
        const platform = await LeafraSDK.getPlatform();
        addResult(`📱 Platform: ${platform}, Version: ${version}`);
        
        // Test distance calculation
        const distance = await LeafraSDK.calculateDistance2D(
          { x: 0, y: 0 }, 
          { x: 3, y: 4 }
        );
        addResult(`📐 Distance (0,0) to (3,4): ${distance.toFixed(2)}`);
        
      } else {
        setSdkStatus('Failed');
        addResult(`❌ Initialization failed: ${result}`);
      }
      
    } catch (error) {
      setSdkStatus('Error');
      addResult(`💥 Error: ${error}`);
    }
  };

  const testMathOperations = async () => {
    try {
      addResult('🔢 Testing math operations...');
      
      if (!LeafraSDKNative) {
        addResult('❌ Native module not available');
        return;
      }
      
      // Test 2D distance
      const distance2D = await LeafraSDK.calculateDistance2D(
        { x: 0, y: 0 }, 
        { x: 3, y: 4 }
      );
      addResult(`📐 2D Distance: ${distance2D.toFixed(2)}`);
      
      // Test 3D distance
      const distance3D = await LeafraSDK.calculateDistance3D(
        { x: 0, y: 0, z: 0 }, 
        { x: 1, y: 1, z: 1 }
      );
      addResult(`📐 3D Distance: ${distance3D.toFixed(2)}`);
      
      // Test matrix determinant
      const identityMatrix = { 
        data: [1, 0, 0, 0, 1, 0, 0, 0, 1] 
      };
      const det = await LeafraSDK.matrixDeterminant(identityMatrix);
      addResult(`🔢 Matrix determinant: ${det}`);
      
    } catch (error) {
      addResult(`💥 Math error: ${error}`);
    }
  };

  const testDataProcessing = async () => {
    try {
      addResult('📊 Testing data processing...');
      
      if (!LeafraSDKNative) {
        addResult('❌ Native module not available');
        return;
      }
      
      const testData = [10, 20, 30, 40, 50];
      
      const result = await LeafraSDK.processData(testData);
      if (result.result === ResultCode.SUCCESS) {
        addResult(`📊 Input: [${testData.join(', ')}]`);
        addResult(`📊 Output: [${result.output.join(', ')}]`);
      } else {
        addResult(`❌ Processing failed: ${result.result}`);
      }
      
    } catch (error) {
      addResult(`💥 Processing error: ${error}`);
    }
  };

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Button title="← Back" onPress={onBack} />
        <Text style={styles.title}>LeafraSDK Test Interface</Text>
      </View>
      
      <Text style={styles.status}>Status: {sdkStatus}</Text>
      
      <View style={styles.buttonContainer}>
        <Button title="Test SDK" onPress={testSDKIntegration} />
        <Button title="Test Math" onPress={testMathOperations} />
        <Button title="Test Data" onPress={testDataProcessing} />
      </View>
      
      <View style={styles.resultsContainer}>
        <Text style={styles.resultsTitle}>Results:</Text>
        <ScrollView style={styles.scrollView}>
          {results.map((result, index) => (
            <Text key={index} style={styles.resultText}>
              {result}
            </Text>
          ))}
        </ScrollView>
      </View>
      
      <Text style={styles.note}>
        🎉 LeafraSDK React Native bridge testing!
        Using direct NativeModules import.
      </Text>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f5f5',
    paddingTop: 50,
    paddingHorizontal: 20,
  },
  header: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 20,
    gap: 15,
  },
  title: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#333',
    flex: 1,
  },
  status: {
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 20,
    color: '#666',
    fontWeight: '500',
  },
  buttonContainer: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    marginBottom: 20,
    gap: 10,
  },
  resultsContainer: {
    flex: 1,
    backgroundColor: '#fff',
    padding: 15,
    borderRadius: 8,
    marginBottom: 10,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  resultsTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 10,
    color: '#333',
  },
  scrollView: {
    flex: 1,
  },
  resultText: {
    fontSize: 14,
    marginBottom: 5,
    fontFamily: 'monospace',
    color: '#555',
  },
  note: {
    fontSize: 12,
    color: '#4CAF50',
    textAlign: 'center',
    fontStyle: 'italic',
    marginBottom: 10,
    fontWeight: '600',
  },
}); 