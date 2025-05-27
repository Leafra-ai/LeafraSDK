import React from 'react';
import {
  StyleSheet,
  Text,
  View,
  TouchableOpacity,
  Modal,
  ScrollView,
  Switch,
} from 'react-native';
import { SDKConfigManager, getEnvironmentInfo } from '../config/sdkConfig';

interface SettingsModalProps {
  visible: boolean;
  onClose: () => void;
  onTestInterface: () => void;
}

export default function SettingsModal({ visible, onClose, onTestInterface }: SettingsModalProps) {
  const [debugMode, setDebugMode] = React.useState(
    SDKConfigManager.getInstance().getDebugMode()
  );
  
  const envInfo = getEnvironmentInfo();

  const handleDebugToggle = (value: boolean) => {
    setDebugMode(value);
    SDKConfigManager.getInstance().setDebugMode(value);
  };

  const settingsOptions = [
    {
      id: 'test',
      title: 'Test Interface',
      description: 'Access SDK testing and debugging tools',
      icon: '🧪',
      onPress: () => {
        onClose();
        onTestInterface();
      },
    },
    {
      id: 'about',
      title: 'About LeafraSDK',
      description: 'Version information and SDK details',
      icon: 'ℹ️',
      onPress: () => {
        // TODO: Implement about screen
        console.log('About pressed');
      },
    },
    {
      id: 'docs',
      title: 'Documentation',
      description: 'View SDK documentation and guides',
      icon: '📚',
      onPress: () => {
        // TODO: Implement documentation viewer
        console.log('Documentation pressed');
      },
    },
    {
      id: 'support',
      title: 'Support',
      description: 'Get help and report issues',
      icon: '🆘',
      onPress: () => {
        // TODO: Implement support screen
        console.log('Support pressed');
      },
    },
  ];

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
          <Text style={styles.headerTitle}>Settings</Text>
          <TouchableOpacity style={styles.closeButton} onPress={onClose}>
            <Text style={styles.closeButtonText}>Done</Text>
          </TouchableOpacity>
        </View>

        {/* Debug Configuration */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Debug Configuration</Text>
          
          <View style={styles.settingRow}>
            <View style={styles.settingInfo}>
              <Text style={styles.settingLabel}>Debug Mode</Text>
              <Text style={styles.settingDescription}>
                Enable detailed logging and debug output
              </Text>
            </View>
            <Switch
              value={debugMode}
              onValueChange={handleDebugToggle}
              trackColor={{ false: '#767577', true: '#81b0ff' }}
              thumbColor={debugMode ? '#007AFF' : '#f4f3f4'}
            />
          </View>
        </View>

        {/* Environment Info */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Environment Info</Text>
          
          <View style={styles.infoRow}>
            <Text style={styles.infoLabel}>Build Type:</Text>
            <Text style={styles.infoValue}>{envInfo.buildType}</Text>
          </View>
          
          <View style={styles.infoRow}>
            <Text style={styles.infoLabel}>React Native __DEV__:</Text>
            <Text style={styles.infoValue}>{envInfo.__DEV__ ? 'true' : 'false'}</Text>
          </View>
          
          <View style={styles.infoRow}>
            <Text style={styles.infoLabel}>Current Debug Mode:</Text>
            <Text style={styles.infoValue}>{envInfo.debugMode ? 'enabled' : 'disabled'}</Text>
          </View>
          
          {envInfo.forceDebug && (
            <View style={styles.infoRow}>
              <Text style={styles.infoLabel}>Force Debug:</Text>
              <Text style={[styles.infoValue, styles.warningText]}>ENABLED</Text>
            </View>
          )}
          
          {envInfo.forceProduction && (
            <View style={styles.infoRow}>
              <Text style={styles.infoLabel}>Force Production:</Text>
              <Text style={[styles.infoValue, styles.warningText]}>ENABLED</Text>
            </View>
          )}
        </View>

        {/* Developer Tools */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Developer Tools</Text>
          
          <TouchableOpacity style={styles.menuItem} onPress={onTestInterface}>
            <Text style={styles.menuItemText}>🧪 Test Interface</Text>
            <Text style={styles.menuItemDescription}>
              Access SDK testing and debugging tools
            </Text>
          </TouchableOpacity>
        </View>

        {/* App Info */}
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>App Information</Text>
          
          <View style={styles.infoRow}>
            <Text style={styles.infoLabel}>App Version:</Text>
            <Text style={styles.infoValue}>1.0.0</Text>
          </View>
          
          <View style={styles.infoRow}>
            <Text style={styles.infoLabel}>LeafraSDK:</Text>
            <Text style={styles.infoValue}>Integrated</Text>
          </View>
        </View>

        {/* Settings Options */}
        <ScrollView style={styles.content}>
          <View style={styles.section}>
            <Text style={styles.sectionTitle}>SDK Tools</Text>
            {settingsOptions.map((option) => (
              <TouchableOpacity
                key={option.id}
                style={styles.optionContainer}
                onPress={option.onPress}
              >
                <View style={styles.optionIcon}>
                  <Text style={styles.optionIconText}>{option.icon}</Text>
                </View>
                <View style={styles.optionContent}>
                  <Text style={styles.optionTitle}>{option.title}</Text>
                  <Text style={styles.optionDescription}>{option.description}</Text>
                </View>
                <Text style={styles.optionArrow}>›</Text>
              </TouchableOpacity>
            ))}
          </View>
        </ScrollView>
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
    fontSize: 20,
    fontWeight: 'bold',
    color: '#333',
  },
  closeButton: {
    paddingHorizontal: 15,
    paddingVertical: 8,
    backgroundColor: '#007AFF',
    borderRadius: 20,
  },
  closeButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
  content: {
    flex: 1,
    paddingHorizontal: 20,
  },
  section: {
    marginTop: 30,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 15,
  },
  optionContainer: {
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
  optionIcon: {
    width: 40,
    height: 40,
    borderRadius: 20,
    backgroundColor: '#f0f0f0',
    justifyContent: 'center',
    alignItems: 'center',
    marginRight: 15,
  },
  optionIconText: {
    fontSize: 20,
  },
  optionContent: {
    flex: 1,
  },
  optionTitle: {
    fontSize: 16,
    fontWeight: '600',
    color: '#333',
    marginBottom: 2,
  },
  optionDescription: {
    fontSize: 14,
    color: '#666',
  },
  optionArrow: {
    fontSize: 20,
    color: '#ccc',
    fontWeight: 'bold',
  },
  infoContainer: {
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
  infoText: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 5,
  },
  infoSubtext: {
    fontSize: 14,
    color: '#666',
    marginBottom: 2,
  },
  settingRow: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 10,
  },
  settingInfo: {
    flex: 1,
  },
  settingLabel: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 2,
  },
  settingDescription: {
    fontSize: 14,
    color: '#666',
  },
  infoRow: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 5,
  },
  infoLabel: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333',
    marginRight: 10,
  },
  infoValue: {
    fontSize: 16,
    color: '#666',
  },
  warningText: {
    color: '#FF0000',
  },
  menuItem: {
    flexDirection: 'row',
    alignItems: 'center',
    padding: 15,
    borderRadius: 12,
    backgroundColor: '#fff',
    marginBottom: 10,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.1,
    shadowRadius: 2,
    elevation: 2,
  },
  menuItemText: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333',
    marginRight: 10,
  },
  menuItemDescription: {
    fontSize: 14,
    color: '#666',
  },
}); 