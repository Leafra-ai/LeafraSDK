const path = require('path');

module.exports = {
  project: {
    ios: {},
    android: {},
  },
  assets: ['./assets/fonts/'],
  dependencies: {
    'react-native-leafra-sdk': {
      root: path.join(__dirname, '../../sdk/react-native'),
      platforms: {
        ios: {
          sourceDir: path.join(__dirname, '../../sdk/ios'),
          podspecPath: path.join(__dirname, '../../sdk/LeafraSDK.podspec'),
        },
        android: {
          sourceDir: path.join(__dirname, '../../sdk/android'),
          packageImportPath: 'import io.leafra.sdk.LeafraSDKPackage;',
        },
      },
    },
  },
}; 