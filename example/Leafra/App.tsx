import React, { useState } from 'react';
import { StyleSheet, View, Alert } from 'react-native';
import { StatusBar } from 'expo-status-bar';

import LeafraSDKDocuchat from './components/LeafraSDKDocuchat';
import ChatInterface from './components/ChatInterface';
import TestInterface from './components/TestInterface';
import SettingsModal from './components/SettingsModal';

type Screen = 'docuchat' | 'chat' | 'test';

export default function App() {
  const [currentScreen, setCurrentScreen] = useState<Screen>('docuchat');
  const [settingsVisible, setSettingsVisible] = useState(false);

  const handleSettings = () => {
    setSettingsVisible(true);
  };

  const handleCloseSettings = () => {
    setSettingsVisible(false);
  };

  const handleTestInterface = () => {
    setCurrentScreen('test');
  };

  const handleBackToDocuchat = () => {
    setCurrentScreen('docuchat');
  };

  const renderCurrentScreen = () => {
    switch (currentScreen) {
      case 'docuchat':
        return (
          <LeafraSDKDocuchat
            onAddFiles={() => {}}
            onSettings={handleSettings}
          />
        );
      case 'chat':
        return (
          <ChatInterface
            onAddFiles={() => {}}
            onSettings={handleSettings}
          />
        );
      case 'test':
        return <TestInterface onBack={handleBackToDocuchat} />;
      default:
        return (
          <LeafraSDKDocuchat
            onAddFiles={() => {}}
            onSettings={handleSettings}
          />
        );
    }
  };

  return (
    <View style={styles.container}>
      <StatusBar style="auto" />
      
      {renderCurrentScreen()}
      
      <SettingsModal
        visible={settingsVisible}
        onClose={handleCloseSettings}
        onTestInterface={handleTestInterface}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
});
