import React, { useState, useRef, useEffect } from 'react';
import {
  StyleSheet,
  Text,
  View,
  TextInput,
  TouchableOpacity,
  ScrollView,
  KeyboardAvoidingView,
  Platform,
  Alert,
  NativeModules,
  NativeEventEmitter,
} from 'react-native';

import FileBrowserModal from './FileBrowserModal';
import SearchResultsModal from './SearchResultsModal';
import { getSDKConfig } from '../config/sdkConfig';

// SDK Integration
const { LeafraSDK: LeafraSDKNative } = NativeModules;

// Event Emitter for token streaming
const leafraEventEmitter = new NativeEventEmitter(LeafraSDKNative);

// SDK Types (copied from TestInterface for consistency)
interface LeafraConfig {
  name?: string;
  version?: string;
  debugMode?: boolean;
  maxThreads?: number;
  bufferSize?: number;
}

interface ProcessDataResult {
  result: number;
  output: number[];
  message?: string;
}

enum ResultCode {
  SUCCESS = 0,
  ERROR_INITIALIZATION_FAILED = -1,
  ERROR_INVALID_PARAMETER = -2,
  ERROR_PROCESSING_FAILED = -3,
  ERROR_NOT_IMPLEMENTED = -4,
}

// SDK Wrapper
const LeafraSDK = {
  async initialize(config: LeafraConfig): Promise<ResultCode> {
    console.log('🔄 Initializing SDK with config:');
    try {
      const sdkConfig = getSDKConfig();
      console.log('📝 SDK Config Details:');
      console.log('  Name:', sdkConfig.name);
      console.log('  Version:', sdkConfig.version);
      console.log('  Debug Mode:', sdkConfig.debug_mode);
      
      console.log('  Chunking:');
      console.log('    Enabled:', sdkConfig.chunking.enabled);
      console.log('    Chunk Size:', sdkConfig.chunking.chunk_size);
      console.log('    Overlap %:', sdkConfig.chunking.overlap_percentage);
      console.log('    Size Unit:', sdkConfig.chunking.size_unit);
      console.log('    Token Method:', sdkConfig.chunking.token_method);
      console.log('    Preserve Word Boundaries:', sdkConfig.chunking.preserve_word_boundaries);
      console.log('    Include Metadata:', sdkConfig.chunking.include_metadata);
      console.log('    Print Full Chunks:', sdkConfig.chunking.print_chunks_full);
      console.log('    Print Brief Chunks:', sdkConfig.chunking.print_chunks_brief);
      console.log('    Max Lines:', sdkConfig.chunking.max_lines);
      
      console.log('  Tokenizer:');
      console.log('    Enabled:', sdkConfig.tokenizer.enabled);
      console.log('    Model Name:', sdkConfig.tokenizer.model_name);
      console.log('    Model Path:', sdkConfig.tokenizer.model_path);
      console.log('    Model JSON Path:', sdkConfig.tokenizer.model_json_path);
      
      console.log('  Embedding Inference:');
      console.log('    Enabled:', sdkConfig.embedding_inference.enabled);
      console.log('    Framework:', sdkConfig.embedding_inference.framework);
      console.log('    Model Path:', sdkConfig.embedding_inference.model_path);
    } catch (error) {
      console.error('❌ Error printing config:', error);
    }
    return LeafraSDKNative.initialize(config);
  },
  async isInitialized(): Promise<boolean> {
    return LeafraSDKNative.isInitialized();
  },
  async processUserFiles(fileUrls: string[]): Promise<ProcessDataResult> {
    return LeafraSDKNative.processUserFiles(fileUrls);
  },
  async getVersion(): Promise<string> {
    return LeafraSDKNative.getVersion();
  },
  async semanticSearchWithLLM(query: string, maxResults: number): Promise<any> {
    return LeafraSDKNative.semanticSearchWithLLM(query, maxResults);
  },
};

interface Message {
  id: string;
  text: string;
  timestamp: Date;
  isUser: boolean;
  isClickable?: boolean;
}

interface LeafraSDKDocuchatProps {
  onAddFiles: () => void;
  onSettings: () => void;
}

interface SelectedFile {
  uri: string;
  name: string;
  size: number;
  type: string;
}

export default function LeafraSDKDocuchat({ onAddFiles, onSettings }: LeafraSDKDocuchatProps) {
  const [messages, setMessages] = useState<Message[]>([
    {
      id: '1',
      text: 'Hello! I\'m your LeafraSDK Docuchat assistant. How can I help you today?',
      timestamp: new Date(),
      isUser: false,
    },
  ]);
  const [inputText, setInputText] = useState('');
  const [fileBrowserVisible, setFileBrowserVisible] = useState(false);
  const [sdkInitialized, setSdkInitialized] = useState(false);
  const [streamingResponse, setStreamingResponse] = useState('');
  const [isStreaming, setIsStreaming] = useState(false);
  const [searchResults, setSearchResults] = useState<any[]>([]);
  const [searchResultsModalVisible, setSearchResultsModalVisible] = useState(false);
  const scrollViewRef = useRef<ScrollView>(null);
  const streamingResponseRef = useRef('');
  const autoScrollEnabled = useRef(true);

  // Initialize SDK when component mounts
  useEffect(() => {
    initializeSDK();
    
    // Set up token event listener
    const tokenSubscription = leafraEventEmitter.addListener('LeafraSDKTokenEvent', (event) => {
      console.log('🔤 Received token:', event.token);
      const newResponse = streamingResponseRef.current + event.token;
      streamingResponseRef.current = newResponse;
      setStreamingResponse(newResponse);
      
      // Auto-scroll to bottom when receiving tokens (only if auto-scroll is enabled)
      if (autoScrollEnabled.current) {
        setTimeout(() => {
          scrollViewRef.current?.scrollToEnd({ animated: false });
        }, 10);
      }
    });
    
    // Cleanup subscription on unmount
    return () => {
      tokenSubscription.remove();
    };
  }, []);

  const initializeSDK = async () => {
    try {
      if (!LeafraSDKNative) {
        console.log('❌ LeafraSDK native module not found');
        return;
      }

      // Check if already initialized
      const isInitialized = await LeafraSDK.isInitialized();
      if (isInitialized) {
        setSdkInitialized(true);
        console.log('✅ SDK already initialized');
        return;
      }

      // Initialize SDK
      const config: LeafraConfig = getSDKConfig();

      const result = await LeafraSDK.initialize(config);
      
      if (result === ResultCode.SUCCESS) {
        setSdkInitialized(true);
        console.log('✅ SDK initialized successfully in LeafraSDKDocuchat');
        
        // Get SDK version for logging
        const version = await LeafraSDK.getVersion();
        console.log(`📱 LeafraSDK Version: ${version}`);
      } else {
        console.log(`❌ SDK initialization failed: ${result}`);
      }
      
    } catch (error) {
      console.log(`💥 SDK initialization error: ${error}`);
    }
  };

  const addMessage = (text: string, isUser: boolean = false, isClickable: boolean = false) => {
    const message: Message = {
      id: Date.now().toString() + Math.random(),
      text,
      timestamp: new Date(),
      isUser,
      isClickable,
    };
    setMessages(prev => [...prev, message]);
    
    // Enable auto-scroll and scroll to bottom for new messages
    autoScrollEnabled.current = true;
    setTimeout(() => {
      scrollViewRef.current?.scrollToEnd({ animated: true });
    }, 100);
  };

  const handleScroll = (event: any) => {
    const { layoutMeasurement, contentOffset, contentSize } = event.nativeEvent;
    const isAtBottom = layoutMeasurement.height + contentOffset.y >= contentSize.height - 20;
    
    // Enable auto-scroll only if user is at the bottom
    autoScrollEnabled.current = isAtBottom;
  };

  const handleMessagePress = (message: Message) => {
    if (message.isClickable && message.text.includes('document chunks')) {
      setSearchResultsModalVisible(true);
    }
  };

  const sendMessage = async () => {
    if (inputText.trim() === '') return;

    const userQuery = inputText.trim();
    const userMessage: Message = {
      id: Date.now().toString(),
      text: userQuery,
      timestamp: new Date(),
      isUser: true,
    };

    setMessages(prev => [...prev, userMessage]);
    setInputText('');

    // Check if SDK is initialized
    if (!sdkInitialized) {
      addMessage('⚠️ SDK not initialized. Attempting to initialize...');
      await initializeSDK();
      
      if (!sdkInitialized) {
        addMessage('❌ Failed to initialize SDK. Please try again.');
        return;
      }
    }

    // Show processing message
    addMessage('🔄 Searching through your documents...');

    try {
      // Reset streaming state and start streaming
      setStreamingResponse('');
      streamingResponseRef.current = '';
      setIsStreaming(true);
      
      // Call semanticSearchWithLLM with the user's query
      const result = await LeafraSDK.semanticSearchWithLLM(userQuery, 3);
      
      console.log('🔍 Semantic search result:', result);
      
      // Stop streaming and add the final response as a message
      setIsStreaming(false);
      
      // Re-enable auto-scroll for the final message
      autoScrollEnabled.current = true;
      
      // Add the final streamed response as a proper message
      const finalResponse = streamingResponseRef.current.trim();
      if (finalResponse) {
        addMessage(finalResponse);
      } else {
        addMessage('No response generated.');
      }
      
      // Store search results and add clickable link if we have results
      if (result && result.results && result.results.length > 0) {
        setSearchResults(result.results);
        addMessage(`📚 Found ${result.results.length} relevant document chunks. Tap to view details →`, false, true);
      }
      
      // Clear streaming response for next use
      setStreamingResponse('');
      streamingResponseRef.current = '';
      
    } catch (error) {
      console.error('💥 Semantic search error:', error);
      setIsStreaming(false);
      setStreamingResponse('');
      addMessage(`❌ Search error: ${error}`);
    }

    // Scroll to bottom
    setTimeout(() => {
      scrollViewRef.current?.scrollToEnd({ animated: true });
    }, 100);
  };

  const handleAddFiles = () => {
    setFileBrowserVisible(true);
  };

  const handleFilesSelected = async (files: SelectedFile[]) => {
    try {
      // Add a message showing the selected files
      addMessage(`📎 Selected ${files.length} PDF file(s):\n${files.map(f => `• ${f.name}`).join('\n')}`, true);

      // Check if SDK is initialized
      if (!sdkInitialized) {
        addMessage('⚠️ SDK not initialized. Attempting to initialize...');
        await initializeSDK();
        
        if (!sdkInitialized) {
          addMessage('❌ Failed to initialize SDK. Please try again or use the test interface.');
          return;
        }
      }

      // Show processing message
      addMessage('🔄 Processing your files through LeafraSDK...');

      // Extract file URLs for SDK processing
      const fileUrls = files.map(file => file.uri);
      console.log('📄 Processing files:', fileUrls);

      try {
        // Actually call the SDK to process files
        const result = await LeafraSDK.processUserFiles(fileUrls);
        
        if (result.result === ResultCode.SUCCESS) {
          addMessage(`✅ Successfully processed ${files.length} PDF files through LeafraSDK!`);

          
          // Log additional details if available
          if (result.output && result.output.length > 0) {
            console.log('📊 SDK Output:', result.output);
            addMessage(`📈 Generated ${result.output.length} data points from the analysis.`);
          }
          
          if (result.message) {
            console.log('📝 SDK Message:', result.message);
          }
          
        } else {
          addMessage(`❌ File processing failed with error code: ${result.result}`);
          console.log('❌ SDK processing failed:', result);
        }
        
      } catch (sdkError) {
        console.log('💥 SDK processing error:', sdkError);
        addMessage(`❌ SDK processing error: ${sdkError}`);
      }

    } catch (error) {
      console.log('💥 File selection error:', error);
      addMessage(`❌ Failed to process files: ${error}`);
    }
  };

  const formatTime = (date: Date): string => {
    return date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
  };

  return (
    <KeyboardAvoidingView 
      style={styles.container} 
      behavior={Platform.OS === 'ios' ? 'padding' : 'height'}
    >
      {/* Header */}
      <View style={styles.header}>
        <Text style={styles.headerTitle}>LeafraSDK Docuchat</Text>
        <View style={styles.headerButtons}>
          <TouchableOpacity style={styles.headerButton} onPress={handleAddFiles}>
            <Text style={styles.headerButtonText}>+</Text>
          </TouchableOpacity>
          <TouchableOpacity style={styles.headerButton} onPress={onSettings}>
            <Text style={styles.headerButtonText}>⚙️</Text>
          </TouchableOpacity>
        </View>
      </View>

      {/* Messages */}
      <ScrollView 
        ref={scrollViewRef}
        style={styles.messagesContainer}
        contentContainerStyle={styles.messagesContent}
        onScroll={handleScroll}
        scrollEventThrottle={100}
      >
        {messages.map((message) => (
          <TouchableOpacity
            key={message.id}
            style={[
              styles.messageContainer,
              message.isUser ? styles.userMessage : styles.aiMessage,
              message.isClickable && styles.clickableMessage,
            ]}
            onPress={() => handleMessagePress(message)}
            disabled={!message.isClickable}
            activeOpacity={message.isClickable ? 0.7 : 1}
          >
            <Text style={[
              styles.messageText,
              message.isUser ? styles.userMessageText : styles.aiMessageText,
              message.isClickable && styles.clickableMessageText,
            ]}>
              {message.text}
            </Text>
            <Text style={[
              styles.messageTime,
              message.isUser ? styles.userMessageTime : styles.aiMessageTime,
            ]}>
              {formatTime(message.timestamp)}
            </Text>
          </TouchableOpacity>
        ))}
        
        {/* Show streaming response in real-time */}
        {isStreaming && (
          <View style={[styles.messageContainer, styles.aiMessage]}>
            <Text style={[styles.messageText, styles.aiMessageText]}>
              {streamingResponse || '🤖 Thinking...'}
            </Text>
            <Text style={[styles.messageTime, styles.aiMessageTime]}>
              {formatTime(new Date())}
            </Text>
          </View>
        )}
      </ScrollView>

      {/* Input */}
      <View style={styles.inputContainer}>
        <TextInput
          style={styles.textInput}
          value={inputText}
          onChangeText={setInputText}
          placeholder="Type your message..."
          placeholderTextColor="#999"
          multiline
          maxLength={500}
          onSubmitEditing={sendMessage}
          blurOnSubmit={false}
        />
        <TouchableOpacity 
          style={[styles.sendButton, inputText.trim() === '' && styles.sendButtonDisabled]} 
          onPress={sendMessage}
          disabled={inputText.trim() === ''}
        >
          <Text style={styles.sendButtonText}>Send</Text>
        </TouchableOpacity>
      </View>

      {/* File Browser Modal */}
      <FileBrowserModal
        visible={fileBrowserVisible}
        onClose={() => setFileBrowserVisible(false)}
        onFilesSelected={handleFilesSelected}
      />

      {/* Search Results Modal */}
      <SearchResultsModal
        visible={searchResultsModalVisible}
        onClose={() => setSearchResultsModalVisible(false)}
        results={searchResults}
      />
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f5f5',
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingTop: 50,
    paddingHorizontal: 20,
    paddingBottom: 15,
    backgroundColor: '#fff',
    borderBottomWidth: 1,
    borderBottomColor: '#e0e0e0',
  },
  headerTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#333',
  },
  headerButtons: {
    flexDirection: 'row',
    gap: 10,
  },
  headerButton: {
    width: 40,
    height: 40,
    borderRadius: 20,
    backgroundColor: '#007AFF',
    justifyContent: 'center',
    alignItems: 'center',
  },
  headerButtonText: {
    color: '#fff',
    fontSize: 18,
    fontWeight: 'bold',
  },
  messagesContainer: {
    flex: 1,
    paddingHorizontal: 15,
  },
  messagesContent: {
    paddingVertical: 15,
  },
  messageContainer: {
    marginVertical: 5,
    maxWidth: '80%',
    padding: 12,
    borderRadius: 18,
  },
  userMessage: {
    alignSelf: 'flex-end',
    backgroundColor: '#007AFF',
  },
  aiMessage: {
    alignSelf: 'flex-start',
    backgroundColor: '#fff',
    borderWidth: 1,
    borderColor: '#e0e0e0',
  },
  clickableMessage: {
    borderColor: '#007AFF',
    borderWidth: 2,
    backgroundColor: '#f0f8ff',
  },
  messageText: {
    fontSize: 16,
    lineHeight: 20,
  },
  userMessageText: {
    color: '#fff',
  },
  aiMessageText: {
    color: '#333',
  },
  clickableMessageText: {
    color: '#007AFF',
    fontWeight: '500',
  },
  messageTime: {
    fontSize: 12,
    marginTop: 5,
  },
  userMessageTime: {
    color: 'rgba(255, 255, 255, 0.7)',
    textAlign: 'right',
  },
  aiMessageTime: {
    color: '#999',
  },
  inputContainer: {
    flexDirection: 'row',
    padding: 15,
    backgroundColor: '#fff',
    borderTopWidth: 1,
    borderTopColor: '#e0e0e0',
    alignItems: 'flex-end',
  },
  textInput: {
    flex: 1,
    borderWidth: 1,
    borderColor: '#e0e0e0',
    borderRadius: 20,
    paddingHorizontal: 15,
    paddingVertical: 10,
    fontSize: 16,
    maxHeight: 100,
    marginRight: 10,
    backgroundColor: '#f9f9f9',
  },
  sendButton: {
    backgroundColor: '#007AFF',
    paddingHorizontal: 20,
    paddingVertical: 10,
    borderRadius: 20,
    justifyContent: 'center',
    alignItems: 'center',
  },
  sendButtonDisabled: {
    backgroundColor: '#ccc',
  },
  sendButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
}); 