# Quick Build Instructions for Leafra

## iOS Build in Xcode

1. **Open the project:**
   ```bash
   open ios/Leafra.xcworkspace
   ```

2. **In Xcode:**
   - Select "Leafra" scheme
   - Choose your target (iOS Simulator or Device)
   - Press ⌘+R to build and run

## macOS Build Options

### Option 1: Mac Catalyst (Recommended)
1. Open `ios/Leafra.xcworkspace` in Xcode
2. Select the "Leafra" target
3. Go to "General" tab
4. Under "Supported Destinations", check "Mac"
5. Select "My Mac (Designed for iPad)" from the scheme selector
6. Press ⌘+R to build and run

### Option 2: Native macOS Target
1. In Xcode, click "+" to add a new target
2. Choose "macOS" → "App"
3. Configure with same bundle ID but add `.macos` suffix
4. Copy source files and configure build phases
5. Build and run

## Development Commands

```bash
# Start development server
npm start

# Run on iOS simulator
npm run ios

# Run on Android
npm run android

# Run on web
npm run web
```

## Troubleshooting

- If CocoaPods issues: `cd ios && pod install`
- If Metro cache issues: `npx expo start --clear`
- If Xcode issues: Clean build folder (⌘+Shift+K) 