# Leafra - React Native Expo App

A beautiful cross-platform React Native app built with Expo that supports both iOS and macOS.

## Features

- 🌿 Modern, beautiful UI design
- 📱 Cross-platform support (iOS & macOS)
- ⚡ Built with React Native & Expo SDK
- 🔧 TypeScript for type safety
- 🎨 Responsive design with platform-specific optimizations

## Prerequisites

- Node.js (v16 or later)
- npm or yarn
- Xcode (for iOS/macOS builds)
- Expo CLI
- CocoaPods

## Installation

1. Clone or navigate to the project directory:
```bash
cd example/Leafra
```

2. Install dependencies:
```bash
npm install
```

3. Install iOS dependencies:
```bash
cd ios && pod install && cd ..
```

## Running the App

### Development Mode

Start the Expo development server:
```bash
npm start
```

### iOS Simulator
```bash
npm run ios
```

### iOS Device
```bash
npm run ios --device
```

### Web
```bash
npm run web
```

## Building for Production

### iOS Build

1. Open the iOS project in Xcode:
```bash
open ios/Leafra.xcworkspace
```

2. In Xcode:
   - Select your development team
   - Choose your target device or simulator
   - Build and run (⌘+R)

### macOS Build

To add macOS support to your iOS app:

1. Open `ios/Leafra.xcworkspace` in Xcode
2. Select your project in the navigator
3. Click the "+" button to add a new target
4. Choose "macOS" → "App"
5. Configure the macOS target:
   - Product Name: Leafra macOS
   - Bundle Identifier: com.leafra.app.macos
   - Language: Objective-C
   - Use Core Data: No

6. Add the React Native libraries to the macOS target:
   - Select the macOS target
   - Go to "Build Phases" → "Link Binary With Libraries"
   - Add the same frameworks as the iOS target

7. Configure the macOS target to use the same source files as iOS
8. Update the Info.plist for macOS if needed
9. Build and run for macOS

### Alternative: Using Mac Catalyst

For easier macOS support, you can enable Mac Catalyst:

1. Open `ios/Leafra.xcworkspace` in Xcode
2. Select your project → iOS target
3. Go to "General" tab
4. Check "Mac" under "Supported Destinations"
5. Configure Mac-specific settings if needed
6. Build and run

## Project Structure

```
Leafra/
├── App.tsx                 # Main app component
├── app.json               # Expo configuration
├── package.json           # Dependencies and scripts
├── assets/                # Images and static assets
├── ios/                   # iOS native code
│   ├── Leafra.xcworkspace # Xcode workspace
│   └── Leafra/           # iOS app source
└── android/              # Android native code
```

## Configuration

The app is configured in `app.json` with:
- iOS bundle identifier: `com.leafra.app`
- Minimum iOS version: 15.1
- Support for tablets and phones
- Custom splash screen and icons

## Customization

### Changing App Name
Update the `name` field in `app.json` and rebuild.

### Updating Bundle Identifier
Update the `bundleIdentifier` in `app.json` and rebuild.

### Adding New Features
Edit `App.tsx` to add new components and functionality.

## Troubleshooting

### CocoaPods Issues
```bash
cd ios
pod deintegrate
pod install
```

### Metro Cache Issues
```bash
npx expo start --clear
```

### Xcode Build Issues
- Clean build folder (⌘+Shift+K)
- Delete derived data
- Restart Xcode

## License

This project is for demonstration purposes. 