# Building librime-qjs on macOS

## Prerequisites

### System Requirements
- macOS 12.7.6 (Intel), the other versions are not tested.
- macOS on Apple Silicon is not tested.

### Required Tools
- Xcode Command Line Tools
- cmake 3.12 or later
- clang/llvm 20 or later

## Environment Setup

### Installing Build Tools
- Xcode Command Line Tools:
  - Install Xcode Command Line Tools: `xcode-select --install`
  - Check Xcode version: `xcodebuild -version`

- cmake:
  - Install cmake: `sudo port install cmake`
  - Check cmake version: `cmake --version`

- clang/llvm:
  - Install clang/llvm: `sudo port install clang-20`
  - Add llvm to PATH: `export PATH="/opt/local/libexec/llvm-20/bin:$PATH"`
  - config llvm: `sudo port select --set llvm mp-llvm-20`
  - config clang: `sudo port select --set clang mp-clang-20`
  - Update the shell configuration file (.zshrc or .bash_profile):
    ```shell
    # Add MacPorts LLVM to your PATH
    export PATH="/opt/local/libexec/llvm-20/bin:$PATH"

    # llvm/clang installed by MacPorts
    alias clang="$(llvm-config --bindir)/clang"
    alias clang++="$(llvm-config --bindir)/clang++"

    # Set the compiler environment variables before running cmake
    export CC=$(llvm-config --bindir)/clang
    export CXX=$(llvm-config --bindir)/clang++
    # Set up compiler flags if needed
    export LDFLAGS="-L/opt/local/libexec/llvm-20/lib"
    export CPPFLAGS="-I/opt/local/libexec/llvm-20/include"
    ```
  - Check LLVM version: `llvm-config --version` and `clang --version`

## Build Steps

### Getting the Source Code

- Clone the [librime](https://github.com/rime/librime) repository:
  ```shell
  cd <your/folder/to/develop/librime>
  git clone --recursive https://github.com/rime/librime.git
  ```

- Clone the [librime-qjs](https://github.com/HuangJian/librime-qjs) repository:
  ```shell
  cd plugins
  mkdir qjs
  git clone --recursive https://github.com/HuangJian/librime-qjs.git ./qjs
  ```

### Building the Project
- Install Boost C++ libraries: `bash ./install-boost.sh`
- Build 3rd-party libraries: `make deps`
- Build librime and librime-qjs: `make` or `make debug`

### Running Tests
- Run all unit tests: `make test` or `make test-debug`
- Run only the librime-qjs tests: `(cd plugins/qjs; ctest)`

## Installation

### Installing the Plugin
```shell
export qjs_dylib="<your/librime/folder>/build/lib/rime-plugins/librime-qjs.dylib"
export rime_plugin_folder="/Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins"

sudo cp ${qjs_dylib} ${rime_plugin_folder} && \
    /Library/Input\ Methods/Squirrel.app/Contents/MacOS/Squirrel --quit
```

### Verifying Installation
- Enable the Squirrel input method with <kbd>⌘</kbd> + <kbd>Space</kbd>
- `cat $TMPDIR/rime.squirrel.INFO | grep qjs` there should be something like:
    > loading plugin 'qjs' from /Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins/librime-qjs.dylib

## Troubleshooting

### Get comprehensive logs
- Merge all the logs into one file:
    ```shell
    export qjs_dylib="<your/librime/folder>/build/lib/rime-plugins/librime-qjs.dylib"
    export rime_plugin_folder="/Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins"

    sudo cp ${qjs_dylib} ${rime_plugin_folder} && \
        /Library/Input\ Methods/Squirrel.app/Contents/MacOS/Squirrel --quit && \
        /Library/Input\ Methods/Squirrel.app/Contents/MacOS/Squirrel \
        > $TMPDIR/rime.debug-$(date +%Y%m%d).log 2>&1
    ```
- check the logs in another terminal: `tail -f $TMPDIR/rime.debug-$(date +%Y%m%d).log`
  - The librime-qjs engine's logs will be printed with the prefix `[qjs]`:
    - `module.cc:16] [qjs] registering components from module 'qjs'`
    - `qjs_component.h:111] [qjs] creating component 'select_character'.`
  - The javascript plugins' `console.log` will be printed with the prefix `$qjs$`:
    - `qjs_helper.cc:100] $qjs$ select_character.js init`
    - `qjs_helper.cc:100] $qjs$ charset_filter finit`
  - libRime and other plugins' logs will be printed as well

## Development

### VSCode C/C++ Intellisense with clangd

- Generate the compilation database: `(cd ./plugins/qjs; cmake .)`
- Move it to the build folder: `mv ./plugins/qjs/compile_commands.json ./plugins/qjs/build/`
- Install VSCode's [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) extension

### Lint with clang-tidy

- clang-tidy is a part of the clang/llvm toolchain, check it's version: `clang-tidy --version`
- The issues should be present in the VSCode Editor window after clangd setup.
- Fix the issues as best as you can.
- [ ] TODO: add clang-tidy to the CI pipeline, and ignore the issues outside our codebase

### Verify the javascript code with qjs in command line

- build the qjs executable: `(cd ./plugins/qjs/thirdparty/quickjs; cmake .; make)`
- move the built executable to your folder: `mv ./plugins/qjs/thirdparty/quickjs/qjs ./plugins/qjs/tests/qjs/js`
- run the javascript code: `./plugins/qjs/tests/qjs/js/qjs ./plugins/qjs/tests/qjs/js/main.js`

### (inaccurate) Benchmark

- uncomment the benchmark building section in CMakeLists.txt:
  ```cmake
    set(benchmark_files "tests/benchmark/dict/load_trie_benchmark.cc")
    add_executable(load-dict-benchmark ${benchmark_files})
    target_link_libraries(load-dict-benchmark
      ${rime_library}
      ${rime_dict_library}
      ${rime_gears_library}
      ${GTEST_LIBRARIES}
    )
  ```
- build it: `make`
- run it: `./plugins/qjs/build/load-dict-benchmark`
- feel free to add more benchmarks

### Debugging memory issues

1. remove the previous built files: `make clean`
2. Rebuild the project with ASan: `make debug CFLAGS="-g -fsanitize=address" LDFLAGS="-fsanitize=address"`
3. run the tests: `ctest`
4. fix the issues shown in the output.
    1. > `Assertion failed: (list_empty(&rt->gc_obj_list)), function JS_FreeRuntime, file quickjs.c, line 2145.`
        - This is a memory leak issue, which usually happens when a JSValue is returned from a function but is not used in the caller.
        - For example: `JS_EvalFunction` and `QjsHelper::loadJsModuleToGlobalThis`
        - To fix it, the JSValue should be freed by `JS_FreeValue`,
            or be wrapped into a `JSValueRAII` to free automatically.
    2. > ` ==14658==ERROR: AddressSanitizer: heap-use-after-free on address 0x6060000453e0 at pc 0x00010037f462 bp 0x7ff7bfc07f90 sp 0x7ff7bfc07f88`
        - This is a heap-use-after-free issue, which usually happens when a
            JSValue is wrapped into `JSValueRAII`, and then set as a property of another JSValue.
        - For example: `JSValueRAII xxx(); JS_SetPropertyStr(ctx, var, "xxx", xxx);`
        - To fix it, just define it as a `JSValue` rather than `JSValueRAII`.
5. a step further: `ASAN_OPTIONS=detect_leaks=1 ctest > leaks.log 2>&1`


## References

### Related Documentation

- [Rime with Mac](https://github.com/rime/librime/blob/master/README-mac.md)
