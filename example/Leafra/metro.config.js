const { getDefaultConfig } = require('expo/metro-config');
const path = require('path');

const config = getDefaultConfig(__dirname);

// Add SDK paths to Metro resolver
config.resolver.platforms = ['ios', 'android', 'native', 'web'];

// Include SDK directory in watchFolders for hot reloading
config.watchFolders = [
  path.resolve(__dirname, '../../sdk'),
];

// Add support for additional file extensions
config.resolver.sourceExts.push('cpp', 'cxx', 'cc', 'c', 'h', 'hpp', 'mm');

module.exports = config; 