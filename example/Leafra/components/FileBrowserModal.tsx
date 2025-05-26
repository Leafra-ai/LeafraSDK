import React, { useState, useEffect } from 'react';
import {
  StyleSheet,
  Text,
  View,
  TouchableOpacity,
  Modal,
  ScrollView,
  Alert,
  ActivityIndicator,
} from 'react-native';
import * as DocumentPicker from 'expo-document-picker';
import * as FileSystem from 'expo-file-system';

interface SelectedFile {
  uri: string;
  name: string;
  size: number;
  type: string;
}

interface FileBrowserModalProps {
  visible: boolean;
  onClose: () => void;
  onFilesSelected: (files: SelectedFile[]) => void;
}

export default function FileBrowserModal({ visible, onClose, onFilesSelected }: FileBrowserModalProps) {
  const [selectedFiles, setSelectedFiles] = useState<SelectedFile[]>([]);
  const [isLoading, setIsLoading] = useState(false);

  const handleDocumentPicker = async () => {
    try {
      setIsLoading(true);
      
      const result = await DocumentPicker.getDocumentAsync({
        type: 'application/pdf',
        multiple: true,
        copyToCacheDirectory: true,
      });

      if (!result.canceled && result.assets) {
        const newFiles: SelectedFile[] = result.assets.map(asset => ({
          uri: asset.uri,
          name: asset.name,
          size: asset.size || 0,
          type: asset.mimeType || 'application/pdf',
        }));

        // Add to existing selection, avoiding duplicates
        setSelectedFiles(prev => {
          const existingUris = new Set(prev.map(f => f.uri));
          const uniqueNewFiles = newFiles.filter(f => !existingUris.has(f.uri));
          return [...prev, ...uniqueNewFiles];
        });
      }
    } catch (error) {
      Alert.alert('Error', 'Failed to select files: ' + error);
    } finally {
      setIsLoading(false);
    }
  };

  const handleFolderSelection = async () => {
    try {
      setIsLoading(true);
      
      // Note: iOS doesn't support folder selection directly through DocumentPicker
      // This is a workaround that allows multiple file selection
      Alert.alert(
        'Folder Selection',
        'iOS doesn\'t support direct folder selection. Please select multiple PDF files from your desired folder.',
        [
          { text: 'Cancel', style: 'cancel' },
          { text: 'Select Files', onPress: handleDocumentPicker },
        ]
      );
    } catch (error) {
      Alert.alert('Error', 'Failed to access folder: ' + error);
    } finally {
      setIsLoading(false);
    }
  };

  const removeFile = (uri: string) => {
    setSelectedFiles(prev => prev.filter(file => file.uri !== uri));
  };

  const clearSelection = () => {
    setSelectedFiles([]);
  };

  const confirmSelection = () => {
    if (selectedFiles.length === 0) {
      Alert.alert('No Files Selected', 'Please select at least one PDF file.');
      return;
    }

    onFilesSelected(selectedFiles);
    setSelectedFiles([]);
    onClose();
  };

  const formatFileSize = (bytes: number): string => {
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const getTotalSize = (): string => {
    const totalBytes = selectedFiles.reduce((sum, file) => sum + file.size, 0);
    return formatFileSize(totalBytes);
  };

  return (
    <Modal
      visible={visible}
      animationType="slide"
      presentationStyle="pageSheet"
      onRequestClose={onClose}
    >
      <View style={styles.container}>
        {/* Header */}
        <View style={styles.header}>
          <TouchableOpacity style={styles.cancelButton} onPress={onClose}>
            <Text style={styles.cancelButtonText}>Cancel</Text>
          </TouchableOpacity>
          <Text style={styles.headerTitle}>Select PDF Files</Text>
          <TouchableOpacity 
            style={[styles.doneButton, selectedFiles.length === 0 && styles.doneButtonDisabled]} 
            onPress={confirmSelection}
            disabled={selectedFiles.length === 0}
          >
            <Text style={[styles.doneButtonText, selectedFiles.length === 0 && styles.doneButtonTextDisabled]}>
              Done ({selectedFiles.length})
            </Text>
          </TouchableOpacity>
        </View>

        {/* Selection Actions */}
        <View style={styles.actionsContainer}>
          <TouchableOpacity 
            style={styles.actionButton} 
            onPress={handleDocumentPicker}
            disabled={isLoading}
          >
            <Text style={styles.actionButtonIcon}>📄</Text>
            <Text style={styles.actionButtonText}>Select Files</Text>
          </TouchableOpacity>
          
          <TouchableOpacity 
            style={styles.actionButton} 
            onPress={handleFolderSelection}
            disabled={isLoading}
          >
            <Text style={styles.actionButtonIcon}>📁</Text>
            <Text style={styles.actionButtonText}>Browse Folder</Text>
          </TouchableOpacity>
        </View>

        {isLoading && (
          <View style={styles.loadingContainer}>
            <ActivityIndicator size="large" color="#007AFF" />
            <Text style={styles.loadingText}>Loading files...</Text>
          </View>
        )}

        {/* Selected Files */}
        <View style={styles.selectedContainer}>
          <View style={styles.selectedHeader}>
            <Text style={styles.selectedTitle}>
              Selected Files ({selectedFiles.length})
            </Text>
            {selectedFiles.length > 0 && (
              <TouchableOpacity onPress={clearSelection}>
                <Text style={styles.clearText}>Clear All</Text>
              </TouchableOpacity>
            )}
          </View>

          {selectedFiles.length > 0 && (
            <Text style={styles.totalSize}>Total Size: {getTotalSize()}</Text>
          )}

          <ScrollView style={styles.filesList}>
            {selectedFiles.length === 0 ? (
              <View style={styles.emptyState}>
                <Text style={styles.emptyStateIcon}>📄</Text>
                <Text style={styles.emptyStateText}>No PDF files selected</Text>
                <Text style={styles.emptyStateSubtext}>
                  Tap "Select Files" to choose PDF documents
                </Text>
              </View>
            ) : (
              selectedFiles.map((file, index) => (
                <View key={file.uri} style={styles.fileItem}>
                  <View style={styles.fileIcon}>
                    <Text style={styles.fileIconText}>📄</Text>
                  </View>
                  <View style={styles.fileInfo}>
                    <Text style={styles.fileName} numberOfLines={2}>
                      {file.name}
                    </Text>
                    <Text style={styles.fileSize}>
                      {formatFileSize(file.size)}
                    </Text>
                  </View>
                  <TouchableOpacity 
                    style={styles.removeButton}
                    onPress={() => removeFile(file.uri)}
                  >
                    <Text style={styles.removeButtonText}>×</Text>
                  </TouchableOpacity>
                </View>
              ))
            )}
          </ScrollView>
        </View>

        {/* Info */}
        <View style={styles.infoContainer}>
          <Text style={styles.infoText}>
            📋 Only PDF files are supported. You can select multiple files from different locations.
          </Text>
        </View>
      </View>
    </Modal>
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
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333',
  },
  cancelButton: {
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  cancelButtonText: {
    color: '#007AFF',
    fontSize: 16,
  },
  doneButton: {
    paddingHorizontal: 15,
    paddingVertical: 8,
    backgroundColor: '#007AFF',
    borderRadius: 20,
  },
  doneButtonDisabled: {
    backgroundColor: '#ccc',
  },
  doneButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
  doneButtonTextDisabled: {
    color: '#999',
  },
  actionsContainer: {
    flexDirection: 'row',
    padding: 20,
    gap: 15,
  },
  actionButton: {
    flex: 1,
    backgroundColor: '#fff',
    padding: 20,
    borderRadius: 12,
    alignItems: 'center',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.1,
    shadowRadius: 2,
    elevation: 2,
  },
  actionButtonIcon: {
    fontSize: 32,
    marginBottom: 8,
  },
  actionButtonText: {
    fontSize: 16,
    fontWeight: '600',
    color: '#333',
  },
  loadingContainer: {
    alignItems: 'center',
    padding: 20,
  },
  loadingText: {
    marginTop: 10,
    fontSize: 16,
    color: '#666',
  },
  selectedContainer: {
    flex: 1,
    paddingHorizontal: 20,
  },
  selectedHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 10,
  },
  selectedTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333',
  },
  clearText: {
    color: '#FF3B30',
    fontSize: 16,
    fontWeight: '600',
  },
  totalSize: {
    fontSize: 14,
    color: '#666',
    marginBottom: 15,
  },
  filesList: {
    flex: 1,
  },
  emptyState: {
    alignItems: 'center',
    paddingVertical: 40,
  },
  emptyStateIcon: {
    fontSize: 48,
    marginBottom: 15,
  },
  emptyStateText: {
    fontSize: 18,
    fontWeight: '600',
    color: '#666',
    marginBottom: 5,
  },
  emptyStateSubtext: {
    fontSize: 14,
    color: '#999',
    textAlign: 'center',
  },
  fileItem: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#fff',
    padding: 15,
    borderRadius: 12,
    marginBottom: 10,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.1,
    shadowRadius: 2,
    elevation: 2,
  },
  fileIcon: {
    width: 40,
    height: 40,
    borderRadius: 20,
    backgroundColor: '#f0f0f0',
    justifyContent: 'center',
    alignItems: 'center',
    marginRight: 15,
  },
  fileIconText: {
    fontSize: 20,
  },
  fileInfo: {
    flex: 1,
  },
  fileName: {
    fontSize: 16,
    fontWeight: '600',
    color: '#333',
    marginBottom: 2,
  },
  fileSize: {
    fontSize: 14,
    color: '#666',
  },
  removeButton: {
    width: 30,
    height: 30,
    borderRadius: 15,
    backgroundColor: '#FF3B30',
    justifyContent: 'center',
    alignItems: 'center',
  },
  removeButtonText: {
    color: '#fff',
    fontSize: 18,
    fontWeight: 'bold',
  },
  infoContainer: {
    padding: 20,
    backgroundColor: '#fff',
    borderTopWidth: 1,
    borderTopColor: '#e0e0e0',
  },
  infoText: {
    fontSize: 14,
    color: '#666',
    textAlign: 'center',
    lineHeight: 20,
  },
}); 