import React, { useState } from 'react';
import { StyleSheet, View, Alert } from 'react-native';
import { StatusBar } from 'expo-status-bar';

import ChatInterface from './components/ChatInterface';
import TestInterface from './components/TestInterface';
import SettingsModal from './components/SettingsModal';

type Screen = 'chat' | 'test';

export default function App() {
  const [currentScreen, setCurrentScreen] = useState<Screen>('chat');
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

  const handleBackToChat = () => {
    setCurrentScreen('chat');
  };

  const renderCurrentScreen = () => {
    switch (currentScreen) {
      case 'chat':
        return (
          <ChatInterface
            onAddFiles={() => {}}
            onSettings={handleSettings}
          />
        );
      case 'test':
        return <TestInterface onBack={handleBackToChat} />;
      default:
        return (
          <ChatInterface
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
