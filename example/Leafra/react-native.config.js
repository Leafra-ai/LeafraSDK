module.exports = {
  project: {
    ios: {},
    android: {},
  },
  assets: ['./assets/'],
  dependencies: {
    'react-native-leafra-sdk': {
      root: '../../sdk/react-native',
      platforms: {
        android: {
          sourceDir: '../../sdk/android',
          packageImportPath: 'import com.leafra.sdk.LeafraSDKPackage;',
        },
        ios: {
          project: '../../sdk/ios/LeafraSDK.xcodeproj',
        },
      },
    },
  },
}; 