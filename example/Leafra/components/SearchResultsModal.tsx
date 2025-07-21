import React from 'react';
import {
  View,
  Text,
  Modal,
  TouchableOpacity,
  ScrollView,
  StyleSheet,
  SafeAreaView,
} from 'react-native';

interface SearchResult {
  id: number;
  distance: number;
  docId?: number;
  chunkIndex?: number;
  pageNumber?: number;
  content?: string;
  filename?: string;
}

interface SearchResultsModalProps {
  visible: boolean;
  onClose: () => void;
  results: SearchResult[];
}

export default function SearchResultsModal({ visible, onClose, results }: SearchResultsModalProps) {
  const formatDistance = (distance: number): string => {
    // Convert distance to similarity percentage (assuming cosine distance)
    const similarity = Math.max(0, (distance) * 100);
    return `${similarity.toFixed(1)}%`;
  };

  const getDocumentInfo = (result: SearchResult): string => {
    let info = '';
    if (result.filename) {
      info += result.filename;
    }
    if (result.pageNumber && result.pageNumber > 0) {
      info += ` (Page ${result.pageNumber})`;
    }
    if (result.chunkIndex !== undefined && result.chunkIndex >= 0) {
      info += ` - Chunk ${result.chunkIndex + 1}`;
    }
    return info || 'Unknown Document';
  };

  return (
    <Modal
      visible={visible}
      animationType="slide"
      presentationStyle="pageSheet"
      onRequestClose={onClose}
    >
      <SafeAreaView style={styles.container}>
        {/* Header */}
        <View style={styles.header}>
          <Text style={styles.headerTitle}>Search Results</Text>
          <TouchableOpacity style={styles.closeButton} onPress={onClose}>
            <Text style={styles.closeButtonText}>✕</Text>
          </TouchableOpacity>
        </View>

        {/* Results */}
        <ScrollView style={styles.scrollContainer} contentContainerStyle={styles.scrollContent}>
          {results.length === 0 ? (
            <View style={styles.emptyState}>
              <Text style={styles.emptyStateText}>No search results found</Text>
            </View>
          ) : (
            results.map((result, index) => (
              <View key={result.id || index} style={styles.resultContainer}>
                {/* Result Header */}
                <View style={styles.resultHeader}>
                  <Text style={styles.resultNumber}>Result {index + 1}</Text>
                  <Text style={styles.similarityScore}>
                    Relevance: {formatDistance(result.distance)}
                  </Text>
                </View>

                {/* Document Info */}
                <Text style={styles.documentInfo}>
                  📄 {getDocumentInfo(result)}
                </Text>

                {/* Content */}
                {result.content && (
                  <View style={styles.contentContainer}>
                    <Text style={styles.contentLabel}>Content:</Text>
                    <Text style={styles.contentText}>{result.content}</Text>
                  </View>
                )}

                {/* Metadata */}
                <View style={styles.metadataContainer}>
                  {result.docId !== undefined && result.docId >= 0 && (
                    <Text style={styles.metadataText}>Doc ID: {result.docId}</Text>
                  )}
                  {result.id !== undefined && (
                    <Text style={styles.metadataText}>Vector ID: {result.id}</Text>
                  )}
                </View>
              </View>
            ))
          )}
        </ScrollView>

        {/* Footer */}
        <View style={styles.footer}>
          <Text style={styles.footerText}>
            Found {results.length} relevant document chunks
          </Text>
        </View>
      </SafeAreaView>
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
    paddingHorizontal: 20,
    paddingVertical: 15,
    backgroundColor: '#fff',
    borderBottomWidth: 1,
    borderBottomColor: '#e0e0e0',
  },
  headerTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#333',
  },
  closeButton: {
    width: 32,
    height: 32,
    borderRadius: 16,
    backgroundColor: '#f0f0f0',
    justifyContent: 'center',
    alignItems: 'center',
  },
  closeButtonText: {
    fontSize: 18,
    color: '#666',
    fontWeight: 'bold',
  },
  scrollContainer: {
    flex: 1,
  },
  scrollContent: {
    paddingHorizontal: 15,
    paddingVertical: 10,
  },
  emptyState: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    paddingVertical: 50,
  },
  emptyStateText: {
    fontSize: 16,
    color: '#999',
    textAlign: 'center',
  },
  resultContainer: {
    backgroundColor: '#fff',
    borderRadius: 12,
    padding: 15,
    marginBottom: 15,
    borderWidth: 1,
    borderColor: '#e0e0e0',
    shadowColor: '#000',
    shadowOffset: {
      width: 0,
      height: 1,
    },
    shadowOpacity: 0.1,
    shadowRadius: 2,
    elevation: 2,
  },
  resultHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 10,
  },
  resultNumber: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#007AFF',
  },
  similarityScore: {
    fontSize: 14,
    color: '#666',
    backgroundColor: '#f0f8ff',
    paddingHorizontal: 8,
    paddingVertical: 2,
    borderRadius: 10,
  },
  documentInfo: {
    fontSize: 14,
    color: '#333',
    marginBottom: 10,
    fontWeight: '500',
  },
  contentContainer: {
    marginBottom: 10,
  },
  contentLabel: {
    fontSize: 14,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 5,
  },
  contentText: {
    fontSize: 14,
    color: '#555',
    lineHeight: 20,
    backgroundColor: '#f9f9f9',
    padding: 10,
    borderRadius: 8,
    borderLeftWidth: 3,
    borderLeftColor: '#007AFF',
  },
  metadataContainer: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    gap: 10,
  },
  metadataText: {
    fontSize: 12,
    color: '#999',
    backgroundColor: '#f5f5f5',
    paddingHorizontal: 6,
    paddingVertical: 2,
    borderRadius: 4,
  },
  footer: {
    paddingHorizontal: 20,
    paddingVertical: 15,
    backgroundColor: '#fff',
    borderTopWidth: 1,
    borderTopColor: '#e0e0e0',
  },
  footerText: {
    fontSize: 14,
    color: '#666',
    textAlign: 'center',
  },
}); 