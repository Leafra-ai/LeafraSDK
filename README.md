📘 Leafra SDK — Overview & Documentation
❓ What’s Leafra SDK?

Leafra is a cross-platform, on-device LLM engineering SDK.
The core is written primarily in C++, using carefully selected cross-platform libraries, and is designed to run on:

iOS

Android

Linux

macOS

Windows

It supports Chat, Question Answering, and RAG workflows for text.

Leafra SDK includes a sample React Native app called Dokuchat.

Currently tested: iOS & macOS
In development: Android, Linux, Windows

🎯 Design Principles
Separation of Apps & SDK

Example apps live under example/

Core SDK lives under sdk/

Clean entry points form the official Leafra SDK API

Cross-Platform First

Uses C++ cross-platform libraries wherever possible

Only platform-specific file so far: leafra_coreml.mm (for embeddings on iOS/macOS)

Open Source Dependencies

Third-party sources stored in sdk/corecpp/thirdparty/

Must be MIT or Apache 2.0 licensed

No GPL

📱 Language Bindings & Mobile Build Environments
Goal

iOS apps (RN or Swift) build via Xcode

Android apps (RN or Kotlin) build via Android Studio

Apps link directly to SDK output libraries

Current Status
Platform	Supported?	Notes
iOS (RN)	✅	RN bindings complete
iOS (Swift)	❌	Planned
Android	🚧	SDK not yet ported
macOS/Linux/Windows	🚧	CLI partially supported
💻 Command Line Apps

Built using CMake

Used internally during SDK development

Available for experimentation

🏗️ High-Level Leafra SDK Structure

Below is the full structure with the original annotations restored, including:

✔ “→ wrapper class for llama.cpp”
✔ “→ wrapper class for faiss”
✔ “→ parsing adapter for pdf”
etc.

Folder Structure
example/                      # Example applications
  example_files/              # Sample test files (PDF, TXT)
  Leafra/                     # React Native app (Expo)
    android/
    ios/
    assets/
    components/
    config/                   # SDK configuration exposed to RN
    ...

sdkcmdapp/                    # Command-line example app
  build/
  CMakeLists.txt
  README
  sdkcmdline.cpp

sdk/                          # Core SDK
  build/
  corecpp/
    build/
    src/
      unit_tests/             # Critical tests (AI code needs coverage)
      data_processor.cpp
      leafra_chunker.cpp           → chunker for RAG
      leafra_core.cpp              → core logic & API (RAG/Chat, etc.)
      leafra_coreml.mm             → wrapper class for CoreML (embeddings)
      leafra_debug.cpp
      leafra_faiss.cpp             → wrapper class for Faiss
      leafra_filemanager.cpp       → cross-platform file management
      leafra_llamacpp.cpp          → wrapper class for llama.cpp
      leafra_parsing_adapter_docx.cpp    → not implemented yet
      leafra_parsing_adapter_excel.cpp   → not implemented yet
      leafra_parsing_adapter_pdf.cpp     → parsing adapter for PDF files
      leafra_parsing_adapter_txt.cpp     → parsing adapter for TXT files
      leafra_parsing.cpp                → top-level document parser
      leafra_sentencepiece.cpp          → wrapper for SentencePiece tokenizer
      leafra_sqlite.cpp                 → wrapper & high-level SQLite for RAG
      leafra_unicode_cacher.cpp         → unicode helper
      leafra_unicode.cpp                → unicode helper
      logger.cpp                        → cross-platform logging
      math_utils.cpp                    → used for RN bridge bring-up
      platform_utils.cpp

    third_party/                  # Git submodules for dependencies
      executorch/                 → not used yet (future support)
      executorch_builder/         → not used yet
      faiss-mobile/               → in-memory vector DB/search engine
      llamacpp/                   → llama.cpp library used for LLMs
      models/
        prebuilt/                 # Prebuilt libs copied into SDK
      sentencepiece/              # SentencePiece tokenizer
      tensorflow/                 → not used yet
      tensorflow_builder/         → not used yet

install/                         # Final framework outputs after build

native/                          # Swift/Kotlin bindings (not implemented yet)
  iOS/                            # (Should be named Swift)
  Android/                        # (Should be named Kotlin)

react-native/                    # RN bindings → C++ bridge
  iOS/
    LeafraCore.framework          # Copied by build.sh
    llama.framework               # Copied by build.sh
  Android/                        # Not implemented yet

docs_internal/                   # Auto-generated docs (may be outdated)

utility/
  build.sh                        # Main build entrypoint
  CMakeLists.txt
  .gitmodules                     # Tracks 3rd-party sources

🏗️ Building the SDK
Clone
git clone https://github.com/Leafra-ai/LeafraSDK.git
cd LeafraSDK
git submodule update --init --recursive

Build Core SDK
cd LeafraSDK/sdk
./build.sh

Target-specific builds
./build.sh --ios
./build.sh --ios --simulator
./build.sh --macos
./build.sh --android

Build specific targets
./build.sh core
./build.sh bindings
./build.sh all

Development flags
./build.sh --clean --debug
./build.sh --verbose

Example (tested)
./build.sh --clean --ios --embedding-fw=coreml
./build.sh --clean --macos --embedding-fw=coreml

🧪 Building the Command Line App
cd example/sdkcmdapp
mkdir -p build && cd build
cmake .. -DLEAFRA_EMBEDDING_FRAMEWORK=coreml
make
./sdkcmdline

📱 Building the React Native App
Full native build
cd example/Leafra
npx expo run:ios

Run on device
npx expo run:ios --device

Hot reload workflow
npm start
# or
npx expo start

Useful commands
npx expo start --clear
open -a Simulator
npx expo run:ios --no-build-cache
