# Leafra SDK Documentation

## What's Leafra SDK?

Leafra is a cross platform on device LLM engineering solution SDK. Most of core SDK is written in C++ with carefully selected cross platform C++ libraries. It's designed to run on iOS/Android/Linux/MacOS/Windows. It supports Chat, Question Answering and RAG scenarios for text. Leafra SDK comes with a sample react native app called "Dokuchat".

SDK and DokuChat is tested on iOS/MacOS. Android/Linux/Windows support is under development.


---

# Design Principles

## Principles: “Leafra SDK Package”
Consists of:
- example apps (`example/`)
- the core sdk (`sdk/`)

### Apps and SDK clearly separated
Example apps (be it command line or GUI based) are clearly separated from the core of the sdk.  
The sdk’s entry points form the Leafra SDK API—this is also the entry point for iOS/Android apps and MacOS/Linux command line apps.

### Cross platformness
We try to keep `sdk/` as cross platform as possible, using cross-platform C++ libraries when needed.  
The only exception so far is CoreML support for embeddings — which uses a single Objective-C file for iOS/macOS targets.

### Open Source
The libraries `sdk/` depends on are under `sdk/corecpp/thirdparty/`,  
where I got them from / how they were built will be indicated separately.  
All libraries must be MIT or Apache 2.0 licensed — **no GPL or derivatives**.


---

# Language bindings / build environments for actual mobile apps

## Goal
- iOS Apps (RN or Swift) can be built on MacOS with XCode using RN/Swift bindings.
- Android Apps (RN or Kotlin) can be built on MacOS/Windows/Linux with Android Studio via RN/Kotlin bindings.

## Current Status
- iOS Apps (RN) can be built on MacOS with XCode using RN bindings.
- No Swift bindings or Swift example app yet.  
- **SDK is not ported to Android yet.**

iOS apps are built on macOS via XCode (RN/Swift).  
Android apps can be built via Android Studio (RN/Kotlin).

3rd-party apps (RN, Swift, Kotlin) are expected to link to libraries output from `sdk/`.  
We will support iOS and Android (and potentially MacOS/Windows/Linux in the future — references may exist in code, but Windows/Linux are not tested).


---

# Language bindings / build environments for command line apps

## Goal
Command line apps can be built using our build system and used on Linux/MacOS.

## Current Status
Command line apps can be built using our build system and used on MacOS.

These CLI apps are used internally for development, but can be used by customers for quick experimentation.

MacOS is supported for commandline `example/` already; supporting Linux is desirable.


---

# High Level Leafra SDK Structure

```
example/ → this folder has the example apps built with sdk  
example_files/ → sample files used for testing, development (pdf, txt)  

Leafra/ → React Native app (uses Expo)  
  android → generated android project  
  assets → icons  
  components → actual typescript files  
  config → SDK configuration bindings exposed to RN apps  
  ios → generated XCode project for iOS  
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

sdkcmdapp/ → command line app  
  build/ → temp build files  
  CMakeLists.txt  
  README  
  sdkcmdline.cpp → command line app  

sdk/ → this folder contains the core sdk  
  build/ → temp build files  
  corecpp/  
    build/ → temp build files  
    src/  
      unit_tests/ → working unit tests  
      data_processor.cpp  
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
      logger.cpp → x platform logging facilities  
      math_utils.cpp → used for bring up of RN bridge code - can be removed  
      platform_utils.cpp  

    third_party → these are 3p libs checked in as git submodules  
      executorch/ → not used yet, will be useful if we’d like to support executorch  
      executorch_builder/ → not used yet  
      faiss-mobile/ → in memory vector database / search engine from Facebook  
      llamacpp/ → current LLM framework we are using for the LLMs  
      models/  
        prebuilt/ —> this is where we put libs - and this is what core sdk links  
      sentencepiece/ → tokenizer library by Google  
      tensorflow/ → not used yet  
      tensorflow_builder/ → not used yet  

install/ → final installables after a successful build  

native/ → bridge code from Swift/Kotlin to C++ core (not implemented yet)  
  iOS/ → (should be named Swift)  
  Android/ → (should be named Kotlin)  

react-native/ → RN bridge to core SDK (C++)  
  iOS/ → contains bridge code and all deliverables  
    LeafraCore.framework  
    llama.framework  
  Android/ → not implemented yet  

docs_internal/ → internal documentation (some outdated)  

utility/  
  build.sh → START HERE: entry point for building core sdk  
  CMakeLists.txt  
  .gitattributes  
  .gitignore  
  .gitmodules  
```


---

# Building SDK and Example Apps

## Prerequisites
Install git and git-lfs.

### Cloning the git repository

```bash
git clone https://github.com/Leafra-ai/LeafraSDK.git
cd LeafraSDK
git submodule update --init --recursive
```

We keep 3rd-party libs as submodules.


---

# Build System High-Level View

## Core SDK
We use CMake as the build system for core SDK.

The core SDK uses a top-level script (`sdk/build.sh`) as the entry point.  
This script and CMakeLists files should be actively maintained.

## Commandline Apps
CMake is also used.

Build on MacOS:

```bash
cd build/
cmake ../
make
```

## RN/Swift/Kotlin Apps
- RN apps on iOS/Android can be built using Expo CLI.  
- GUI-based builds possible via XCode (iOS).  
- Swift/Kotlin GUIs planned.

## Build Order
1. Build core SDK first  
2. Build CLI app **or** GUI apps (RN/Swift/Kotlin)


---

# How to Build the Core SDK

## Build the native C++ SDK:

```bash
cd LeafraSDK/sdk
./build.sh
```

## Build for specific platforms:

```bash
./build.sh --ios
./build.sh --ios --simulator
./build.sh --macos
./build.sh --android
```

## Build specific targets:

```bash
./build.sh core
./build.sh bindings
./build.sh all
```

## Development options:

```bash
./build.sh --clean --debug
./build.sh --verbose
```

## Tested lines:

Do a clean build for iOS target using CoreML for embeddings:

```bash
./build.sh --clean --ios --embedding-fw=coreml
./build.sh --clean --macos --embedding-fw=coreml
```

This builds:
- LeafraCore.framework  
- llama.framework  

And copies them to:
- `sdk/install/`
- `sdk/reactnative/ios/`


---

# Pod Install Notes (iOS RN)

`build.sh` may ask for `pod install`.  
This is usually needed once during dev unless the RN podspec changes.

Changes to `.framework` files **do not** require pod install.


---

# How to Build and Run the Command Line App

First build the SDK for macOS:

```bash
./build.sh --clean --macos --embedding-fw=coreml
```

Then:

```bash
cd example/sdkcmdapp
mkdir -p build && cd build
cmake .. -DLEAFRA_EMBEDDING_FRAMEWORK=coreml
make
./sdkcmdline
```

Get help:

```bash
sdkcmdline --help
```


---

# How to Build and Run the RN App

## Start development

Use when:
- first time running the app  
- after changing native code  
- after pod install  
- app not installed on simulator  

```bash
cd ../example/Leafra
npx expo run:ios
```

Run on device:

```bash
npx expo run:ios --device
```

After install, JS-only changes hot-reload.

Start bundler:

```bash
npm start
```

## Fast iteration (JS-only changes)

```bash
npm start
# or
npx expo start
```

Useful:

```bash
npx expo start --clear
open -a Simulator
npx expo run:ios --no-build-cache
```


---
