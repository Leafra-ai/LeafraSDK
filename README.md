What's Leafra SDK?

Leafra is a cross platform on device LLM engineering solution SDK. 
Most of core SDK is written in C++ with carefully selected cross platform C++ libraries. 
It's designed to run on iOS/Android/Linux/MacOS/Windows.
It supports Chat, Question Answering and RAG scenarios for text. 
Leafra SDK comes with a sample react native app called "Dokuchat". 

SDK and DokuChat is tested on iOS/MacOS. Android/Linux/Windows support is 
under development. 


>> Design Principles

Principles:
“Leafra SDK Package” consists of example apps (example/) and the core of the sdk aka Core SDK (sdk/). 

Apps and SDK clearly separated:
Example apps (be it command line or GUI based) are clearly separated from the core of the sdk. The sdk’s entry points form the Leafra SDK API - and this will be the entry point for iOS/Android apps as well as MacOS/Linux command line apps. 

Cross platformness: 
We try to keep sdk/ as cross platform as possible, try to use C++ cross platform libraries when library support is needed. The only exception so far has been CoreML support for the embeddings - which uses a single objective C file - which is linked for iOS/MacOS targets.

Open Source:
The libraries sdk/ depends on are under sdk/corecpp/thirdparty/ , where I got them from / how they were built will be indicated in a separate section explicitly. All libraries we depend on should be MIT or Apache 2.0 licensed due to obvious reasons, no GPL or derivatives. 



>> Language bindings / build environments for actual mobile apps: 

Goal: 
iOS Apps (RN or Swift) can be built on MacOS w. XCode using RN/Swift bindings for iOS 
          Android Apps (RN or Kotlin) can be built on MacOS/Windows/Linux 
          with Android Studio via SDK RN/Kotlin language bindings.
Current Status: 
           iOS Apps (RN) can be built on MacOS with XCode 
using RN language bindings for iOS. No swift language bindings or example swift app yet.
           ** SDK is not ported to Android yet. 

iOS apps will be built on MacOS typically using XCode. They can be RN/Swift apps.

Android apps can be built on MacOS/Windows/Linux using Android Studio. They can be RN/Kotlin apps. 

Normally we expect 3ps to link their apps (RN, Swift, Kotlin) to sdk/’s output libraries. 
We’ll support the iOS and Android (and in the future MacOS, Windows, Linux possibly - you might see references to these in the code - but Windows and Linux is not tested at all. 


>> Language bindings / build environments for command line apps: 
Goal: 
Command line apps can be built using our build system and used on Linux/MacOS 
Current Status: 
	Command line apps can be built using our build system and be used on MacOS

Command line apps are used by us for SDK development, but app developers can also use them for quick experimentation. 

MacOS is supported for commandline example/ already and likely our customers will use Linux for development as well so it would be good to support MacOS/Linux for commandline app for development purposes. 

High Level Leafra SDK Structure 

example/  → this folder has the example apps built with sdk 
example_files/ → sample files used for testing, development (pdf, txt)
Leafra/  → React Native app (uses Expo) 
	android → generated android project 
	assets → icons 
components → actual typescript files 
	config → SDK configuration bindings exposed to RN apps 
	ios → generated xcode project for iOS
	node_modules → react native node modules 
	app.json
	App.tsx 
	BUILD_INSTRUCTIONS
	index.ts
	metro.config.js
	package-lock.json 
	package.json 
	react-native.conf
	README.md 
	
	sdkcmdapp/ →  command line app 
	build/ → temp build files 
	CMakeLists.txt
	README
	sdkcmdline.cpp → command line app 

sdk/ → this folder contains the core sdk 
build/ → temp build files  
	corecpp/ 
		build / → temp build files
		src/
			unit_tests/ → working unit tests , very important to create tests for all dev. as AI generated code frequently misses corner cases etc. 
		data_processor.cpp → 
		leafra_chunker.cpp → chunker for RAG
		leafra_core.cpp → core logic and API of the SDK resides here (RAG/Chat etc.)
		leafra_coreml.mm → wrapper class for coreml (mainly used for embeddings) 
		leafra_debug.cpp
		leafra_faiss.cpp → wrapper class for faiss
		leafra_filemanager.cpp → x platform file management facilities
		leafra_llamacpp.cpp → wrapper class for llama.cpp 
		leafra_parsing_adapter_docx.cpp → not implemented yet 
		leafra_parsing_adapter_excel.cpp → not implemented yet
		leafra_parsing_adapter_pdf.cpp → parsing adapter for pdf files
		leafra_parsing_adapter_txt.cpp → parsing adapter for txt files
		leafra_parsing.cpp → top level parsing class for documents
		leafra_sentencepiece.cpp → wrapper for sentencepiece tokenizer 
		leafra_sqlite.cpp → wrapper and high level functions for sqlite for RAG
		leafra_unicode_cacher.cpp → helper class for unicode, used by chunker 
		leafra_unicode.cpp → helper class for unicode, used by chunker
		logger.cpp → x platfrom logging facilities
		math_utils.cpp → these were used for bring up of RN bridge code - can be removed
		platform_utils.cpp
	third_party → these are 3p libs checked in as git submodules to keep track of source of 3p libs we are using (but not everything I am using is built from these sources - read on the separate section) 
		executorch/ → not used yet, will be useful if we’d like to support executorch
		executorch_builder/ → not used yet 
		faiss-mobile/ → in memory vector database / search engine from Facebook
		llamacpp/ → current LLM framework we are using for the LLMs
		models/ → 
		prebuilt/ —> this is where we put libs - and this is what core sdk links, read below to understand where they are coming from 
		sentencepiece/ → tokenizer library by Google, used on chunks. 
		tensorflow/ → not used yet, will be useful if we’d like to support tensorflowlite 
		tensorflow_builder/ → not used yet
	
docs_internal/ → these are a set up documentation I asked Cursor to generate during development; the target audience was SDK developers. Having said that, some are outdated or inaccurate as they were not updated in time. So be careful about relying on these. 
install/ → final installables after a successful build are copied here 
native/ → bridge code (bindings) from Swift/Kotlin to core SDK (C++) (not implemented yet)
	iOS/ → better to name this Swift
	Android/ → better to name this Kotlin
react-native/ → bridge code (bindings) from RN to core SDK (C++)
	iOS/ → contains the bridge code - and all the SDK deliverables (LeafraCore.framework + llama.framework is copied here by sdk’s build.sh script after each build) to be consumed by RN iOS app. 
	Android/ → not implemented yet 

utility/ 
build.sh → START HERE: Entry point for building the core sdk - I am documenting below. But PTAL. 
CMakeLists.txt → used by build.sh 
.gitattributes
.gitignore
.gitmodules → these are 3p git submodules clones from source they can also be models. 





Building SDK and Example apps

Prerequisites

Install git and git-lfs 

Cloning the git repository 

git clone https://github.com/Leafra-ai/LeafraSDK.git 
cd LeafraSDK 
git submodule update --init --recursive 



we keep 3p libs’ source as submodules (even if we don’t build from the source - it’s good practice to keep track) - we don’t make modifications to 3p libs.
Build System High Level View 
Core SDK 
We use CMake as the build system for core SDK
Core SDK (corecpp) uses a top level script (sdk/build.sh) as the entry point. 
This script and the CMakeLists.txt files should be actively maintained. 

Commandline Apps 
We use CMake as the build system for core SDK.

We build the commandline app on MacOS by simply going to the folder, creating a build/ subfolder
cd build/ , cmake ../ make with some parameters - see below. 

RN/Swift/Kotlin Apps 
For RN apps on iOS/Android, expo offers a command line interface. 

RN apps can also be built via GUI by opening RN generated XCode iOS app project (and Android Studio projects not supported yet). 

For Swift/Kotlin apps, I expect GUI to be the flow via XCode/Android Studio. (not supported yet) 

Build Order 
You should always build the core sdk first (for command line app need to use macos target, for GUI iOS/Android target, Android is not there obviously)

Followed by 

Building the command line app 

Or 

Building the GUI apps (RN/Swift/Kotlin) 
How to build the Core SDK
Build the native C++ SDK (one-time or when SDK changes)


cd LeafraSDK/sdk
# Build everything for current platform
./build.sh
# Build for specific platforms
./build.sh --ios          # Build for iOS device
./build.sh --ios --simulator  # Build for iOS simulator
./build.sh --macos        # Build for macOS
./build.sh --android      # Build for Android
# Build specific targets
./build.sh core           # Build only C++ core
./build.sh bindings       # Build only React Native bindings
./build.sh all            # Build everything (default)
# Development options
./build.sh --clean --debug    # Clean build in debug mode
./build.sh --verbose          # Verbose output


Build.sh creates and copies LeafraSDK.framework (which has the bundled models in it as well) and copies llama.framework into sdk/install for iOS/MacOS and also to sdk/reactnative/ios (for iOS only)

Tested lines: 

Do a clean build for iOS target, using coreml as the embedding inference framework 

./build.sh --clean --ios --embedding-fw=coreml 
./build.sh --clean --macos --embedding-fw=coreml 

** Note that embedding-fw is mandatory 


This builds the sdk including all libs for the iOS/MacOS target platform (2 .framework files) and copies LeafraCore.framework and llama.framework to reactnative/ios folder. 

Note on pod install step:
build.sh will ask if you’d like to do a “pod install” at the end, this should normally required only one time during dev flow for iOS React Native development in XCode, and only once more if any of the contents in the reactnative/ios/react-native-leafra-sdk.podspec 
changes (new files added / files removed etc.). Changes to the sdk framework does not require this step, latest version of the LeafraCore.framework and llama.framework will be linked to the app automatically. See Podfile.lock and find (react-native-leafra-sdk (1.0.0) line in there). Leafra/ios/Pods/Local Podspecs/react-native-leafra-sdk.podspec is what’s copied 

See below in terms of how linking of core sdk to apps work.


How To Build and Run The Command Line App

Tested lines: 
First make sure you built the sdk core for the macos target - sdk is a macos command line app. 
(./build.sh --clean --macos --embedding-fw=coreml)


cd example/sdkcmdapp
mkdir -p build && cd build
cmake .. -DLEAFRA_EMBEDDING_FRAMEWORK=coreml
make 
./sdkcmdline 

*for a clean build you can just rm -rf * of build folder you created. 

now get help and try the examples. 

sdkcmdline --help 




How To Build and Run the RN App 
# 3. Start development
A) 
Use when:
First time running the app
After changing native code (C++, Objective-C++)
After running pod install
After adding new native dependencies
When the app isn't installed on simulator
cd ../example/Leafra
# Build and run 
npx expo run:ios  —> runs simulator 
npx expo run:ios —device  —> runs on device 
# After this point make changes to App.tsx - hot reload works! 
npm start  // start the bundler if it’s not running already…
B) 
Use when:
   #if app is already installed on the device - no native C++ changes
Making JavaScript/TypeScript changes
Working on UI/React components
You already have the app installed on simulator
You want fast development iteration
 
 - just starting the metro bundler is sufficient - hot reload will take care of the rest
npm start  OR  npx expo start  
# ✅ Metro bundler serves JavaScript
** npx expo start --clear. 
** open -a Simulator opens the iOS simulator
** npx expo run:ios --no-build-cache ** rebuild everything 


