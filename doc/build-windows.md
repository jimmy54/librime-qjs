# Building librime-qjs on Windows

This guide will walk you through the process of building librime-qjs on Windows, from setting up your development environment to testing the final installation.

## System Requirements

While this guide is optimized for Windows 11 (version 24H2), you can likely build librime-qjs on other Windows versions with the appropriate development tools installed.

## Required Development Tools

- Visual Studio 2022 Build Tools
- CMake 3.12 or newer
- Ninja 1.12.1 or newer
- Clang/LLVM 20 or newer
- Node.js 22 or newer

## Setting Up Your Development Environment

### Installing Visual Studio 2022 Build Tools

1. Download and run the installer from: https://aka.ms/vs/17/release/vs_BuildTools.exe
2. In the installer:
   - Select "Desktop development with C++" under the "Workloads" tab
   - Ensure these components are selected:
     - MSVC v143 build tools (latest version)
     - Windows 11 SDK
     - C++ CMake tools for Windows
3. After installation, set up the Visual Studio environment by running:
   ```shell
   call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
   ```

### Installing CMake

1. Download the Windows x64 Installer from https://cmake.org/download/
2. During installation, select "Add CMake to the system PATH for all users"
3. Verify the installation by running: `cmake --version`

### Installing Ninja

1. Download the Windows package from https://github.com/ninja-build/ninja/releases
2. Extract the ninja executable to a folder (e.g., `D:\tools`)
3. Add this folder to your system's PATH environment variable
4. Verify the installation: `ninja --version`

### Installing Clang/LLVM

1. Download the Windows x64 package from the [LLVM releases page](https://github.com/llvm/llvm-project/releases/)
2. Extract it to your preferred location (e.g., `D:\tools\LLVM`)
3. Add the `bin` folder to your PATH environment variable
4. Verify the installation: `clang --version`

### Installing Node.js

1. Download and install Node.js from https://nodejs.org/en/download/
2. Verify the installation:
   ```shell
   node --version
   npm --version
   ```

## Building the Project

### Getting the Source Code

1. Clone the librime repository:
   ```shell
   cd <your-development-folder>
   git clone --recursive https://github.com/rime/librime.git
   ```

2. Clone the librime-qjs repository into the plugins folder:
   ```shell
   cd librime/plugins
   git clone --recursive https://github.com/HuangJian/librime-qjs.git qjs
   ```

### Building Dependencies

1. Set up Boost:
   - Download [boost-1.84.0](https://www.boost.org/users/history/version_1_84_0.html)
   - Extract it to `librime/deps/boost-1.84.0`
   - Build Boost with these powershell commands:
     ```powershell
     cd librime/deps/boost-1.84.0

     :: Set up Visual Studio environment
     call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

     :: Create user-config.jam
     echo using clang : : clang++ : > user-config.jam
     echo     ^<cxxflags^>"-std=c++14" >> user-config.jam
     echo     ^<cxxflags^>"-fms-compatibility-version=19.29" >> user-config.jam
     echo     ^<cxxflags^>"-D_CRT_SECURE_NO_WARNINGS" >> user-config.jam
     echo     ; >> user-config.jam

     :: Bootstrap if needed
     if not exist b2.exe (
         call bootstrap.bat
     )

     :: Build Boost
     b2.exe ^
         toolset=clang ^
         address-model=64 ^
         variant=release,debug ^
         link=static,shared ^
         threading=multi ^
         runtime-link=shared ^
         --build-type=complete ^
         -j%NUMBER_OF_PROCESSORS% ^
         stage
     ```

2. Configure the build environment:
   Create `librime/env.bat` with these settings:
   ```batch
   rem Set up build environment variables
   set RIME_ROOT=%CD%

   rem Set Boost root directory
   if not defined BOOST_ROOT set BOOST_ROOT=%RIME_ROOT%\deps\boost-1.84.0

   rem Configure build tools
   set BJAM_TOOLSET=clang-win
   set CMAKE_GENERATOR=Ninja
   set CMAKE_C_COMPILER=clang-cl
   set CMAKE_CXX_COMPILER=clang-cl
   set CC=clang-cl
   set CMAKE_LINKER=clang
   set CMAKE_CXX_LINKER=clang++
   set CXX=clang-cl
   set CMAKE_C_FLAGS="/std:c11"
   set CMAKE_CXX_FLAGS="/std:c++17"

   rem Add build tools to PATH
   set DEVTOOLS_PATH=D:\tools;D:\tools\llvm\bin;C:\Program Files\CMake\bin;
   ```

3. Build the project:
   ```shell
   build.bat deps    # Build dependencies
   build.bat         # Build librime and librime-qjs
   ```

### Running Tests

1. Install Node.js dependencies:
   ```shell
   cd plugins/qjs/tests/js
   npm install
   ```

2. Build and run the tests:
   ```shell
   build test
   plugins/qjs/build/librime-qjs-tests.exe
   ```

## Installing and Testing

### Installing in Weasel

1. Close Weasel:
   - Right-click the Weasel icon in the system tray
   - Select "Exit(Q)"

2. Install the plugin:
   - Copy `librime/dist/lib/rime.dll` to `C:\Program Files\Rime\weasel-x.xx.x`

3. Restart Weasel:
   - Right-click the Weasel icon in the system tray
   - Select "Restart(E)"

### Verifying the Installation

1. Open the log folder:
   - Right-click the Weasel icon in the system tray
   - Select "Log folder(L)"

2. Check the latest log file (`rime.weasel.xxxxx.INFO.xxxxxx.log`) for these entries:
   ```log
   module.cc:17] [qjs] registering components from module 'qjs'.
   qjs_types.cc:22] [qjs] registering rime types to the quickjs engine...
   registry.cc:14] registering component: qjs_processor
   registry.cc:14] registering component: qjs_filter
   registry.cc:14] registering component: qjs_translator
   ```

### Testing JavaScript Plugins

1. Set up the test environment:
   - Copy `librime/plugins/qjs/build/qjs.exe` (patched to load node modules) to your Weasel user folder
   - Ensure your folder structure looks like this:
     ```
     <Rime-user-folder>
     ├─── build
     ├─── js
     │    ├─── qjs.exe
     │    ├─── package.json
     │    ├─── node_modules
     │    ├─── <your-plugin>.js
     │    └─── tests
     │          └─── <your-plugin>.test.js
     ├─── ...
     ├─── weasel.yaml
     └─── default.yaml
     ```

2. Run your plugin tests:
   ```shell
   qjs.exe tests/<your-plugin>.test.js
   ```

## Additional Resources

For more information about Rime on Windows, check out the [official Windows documentation](https://github.com/rime/librime/blob/master/README-windows.md).
