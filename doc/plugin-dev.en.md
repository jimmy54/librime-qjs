# Rime JavaScript Plugin Development Guide

## Table of Contents

- [Rime JavaScript Plugin Development Guide](#rime-javascript-plugin-development-guide)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [System Architecture](#system-architecture)
    - [Core Components](#core-components)
    - [Workflow](#workflow)
  - [Development Environment](#development-environment)
    - [Installing the Plugin](#installing-the-plugin)
      - [Download libRime-qjs](#download-librime-qjs)
      - [Install on macOS (Squirrel)](#install-on-macos-squirrel)
      - [Install on Windows (Weasel)](#install-on-windows-weasel)
      - [Install on Linux (PR Welcome)](#install-on-linux-pr-welcome)
    - [Development Tools](#development-tools)
    - [Project Structure (Example Plugin Repository)](#project-structure-example-plugin-repository)
  - [Quick Start](#quick-start)
    - [Create Plugin Directory](#create-plugin-directory)
    - [Write Plugin Main File](#write-plugin-main-file)
    - [Embrace NPM](#embrace-npm)
    - [Configure Input Method Schema](#configure-input-method-schema)
  - [API Reference](#api-reference)
    - [Basic Types](#basic-types)
      - [Environment](#environment)
      - [Candidate](#candidate)
      - [Segment](#segment)
      - [KeyEvent](#keyevent)
      - [Trie](#trie)
  - [Plugin Lifecycle](#plugin-lifecycle)
    - [1. Loading Phase](#1-loading-phase)
    - [2. Initialization Phase: Call Plugin Constructor `constructor()`](#2-initialization-phase-call-plugin-constructor-constructor)
    - [3. Running Phase: Call Plugin Main Methods `process()`, `translate()`, `filter()`](#3-running-phase-call-plugin-main-methods-process-translate-filter)
    - [4. Unloading Phase: Call Plugin Unload Function `finalizer()`](#4-unloading-phase-call-plugin-unload-function-finalizer)
  - [Example References](#example-references)
    - [processor Key Processors](#processor-key-processors)
    - [translator Translators](#translator-translators)
    - [filter Filters](#filter-filters)
  - [Debugging and Testing](#debugging-and-testing)
    - [Logging Output](#logging-output)
    - [Unit Testing](#unit-testing)
      - [Installation and Configuration](#installation-and-configuration)
      - [Test File Organization](#test-file-organization)
      - [Writing Test Cases](#writing-test-cases)
      - [Running Tests](#running-tests)
    - [Using Other JavaScript Engines](#using-other-javascript-engines)
    - [Type Definition Files](#type-definition-files)
      - [Installation](#installation)
      - [Configuration](#configuration)
      - [IDE Integration](#ide-integration)
      - [Common Issues](#common-issues)
  - [Best Practices (To Be Added)](#best-practices-to-be-added)
  - [Common Issues (To Be Added)](#common-issues-to-be-added)

## Introduction

The libRime-qjs plugin system allows developers to create various functional components for the Rime input method using JavaScript, such as key processors, translators, and filters. This document will guide you through developing, testing, and deploying these JavaScript plugins.

## System Architecture

```pre
                                             ,---,---,---,---,---,---,---,---,---,---,---,---,---,-------,
                                             |1/2| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | + | ' | <-    |
                                             |---'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-----|
                                             | ->| | Q | W | E | R | T | Y | U | I | O | P | ] | ^ |     |
                                             |-----',--',--',--',--',--',--',--',--',--',--',--',--'|    |
                                             | Caps | A | S | D | F | G | H | J | K | L | \ | [ | * |    |
                                             |----,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'-,-'---'----|
                                             |    | < | Z | X | C | V | B | N | M | , | . | - |          |
                                             |----'-,-',--'--,'---'---'---'---'---'---'-,-'---',--,------|
                                             | ctrl |  | alt |                          |altgr |  | ctrl |
                                             '------'  '-----'--------------------------'------'  '------'
                                                                                   ││    ││
                                                                               \   ││    ││   /
                                                                                 \ ││    ││ /
                                                                                   \       /
                                                                                     \   /
                                                                                       .
      ┌───────────────────────────────────────────────────────┐         ┌────────────────────────────────┐
      │                                                       │         │                                │
      │                                                       │ <=====  │                                │
      │                       libRime                         │         │    Squirrel/Weasel/Fcitx/...   │
      │                                                       │ =====>  │                                │
      │             ┌───────────────────────────┐             │         │                                │
      └─────────────│                           │─────────────┘         └────────────────────────────────┘
                    │       libRime-qjs         │                                  ││    ││
                    │                           │                              \   ││    ││   /
                    └───────────────────────────┘                                \ ││    ││ /
                                 │                                                 \       /
          ┌──────────────────────+─────────────────────┐                             \   /
          │                      │                     │                               .
     ──────────             ──────────            ──────────
   ──          ──         ──          ──        ──          ──              .---"""          """---.
  │               │      │               │     │               │           :  .------------------.  :
│                   │  │                   │ │                   │         | :                    : |
│     plugin1.js    │  │     plugin2.js    │ │     plugin3.js    │         | |                    | |
│                   │  │                   │ │                   │         | | Smart input method  | |
  │               │      │               │     │               │           | |  understands me!    | |
   ──          ──         ──          ──        ──          ──             | |                    | |
     ──────────             ──────────            ──────────               | :                    : |
                                                                           :  '------------------'  :
                                                                            '---...___________...---'
                                                                              /                   \
                                                                               '-...___________...-'
```

### Core Components

- **libRime**: The core input method engine responsible for overall scheduling and coordination. It handles basic input method functions such as text input, candidate generation, dictionary management, and provides a unified interface and runtime environment for various components.

- **libRime-qjs**: A JavaScript runtime plugin that bridges libRime and JavaScript plugins. Based on the QuickJS engine, it provides the loading and execution environment for JavaScript plugins and necessary API interfaces, enabling developers to develop input method components using JavaScript.

- **Squirrel/Weasel/Fcitx/..**: The front-end programs of the input method, responsible for interacting with the operating system and providing the user interface. They receive user keystrokes, pass them to libRime for processing, and display the results (such as candidates) to users. Different front-end programs are adapted for different operating system platforms.

- **plugin.js**: User-developed specific plugin implementations that can include various types of input method components:
  - Processor: Handles user keystroke events
  - Translator: Converts input into candidates
  - Filter: Filters and sorts candidates

### Workflow

1. User inputs text in the front-end program
2. Front-end program passes the input to libRime
3. libRime calls corresponding JavaScript plugins based on configuration
4. libRime-qjs executes these plugins and returns results to libRime
5. libRime processes and sends results back to the front-end program for display

## Development Environment

### Installing the Plugin

#### Download libRime-qjs

Download the latest libRime-qjs package from the project repository: https://github.com/HuangJian/librime-qjs/releases

#### Install on macOS (Squirrel)

1. Backup all dynamic library files of Squirrel input method: `/Library/Input Methods/Squirrel.app/Contents/Frameworks`
2. Copy `librime-qjs.dylib` to `/Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins`
3. If `/Library/Input Methods/Squirrel.app/Contents/Frameworks/librime.x.xx.x.dylib` version differs from the package, replace it with `librime.y.yy.y.dylib`, `librime.dylib`, and `librime.1.dylib` from the package
4. Verify installation:
    - Use <kbd>⌘</kbd> + <kbd>Space</kbd> to enable Squirrel input method
    - Check logs: `cat $TMPDIR/rime.squirrel.INFO | grep qjs`
    - If you see similar output, libRime-qjs is working:
        > loading plugin 'qjs' from /Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins/librime-qjs.dylib
        > registering component: qjs_processor
        > registering component: qjs_filter
        > loaded plugin: qjs
5. If issues occur, please report with detailed reproduction steps; or delete `rime-plugins/librime-qjs.dylib` and restore from backup.

#### Install on Windows (Weasel)

1. Close Weasel:
    - Right-click the Weasel icon in system tray
    - Select "Exit(Q)"

2. Backup program library: `C:\Program Files\Rime\weasel-x.xx.x\rime.dll`

3. Install plugin:
    - Copy `rime.dll` from the package to `C:\Program Files\Rime\weasel-x.xx.x`, replacing the backed-up `rime.dll`
    - Note: This DLL includes components like libRime, libRime-qjs, libRime-lua, librime-octagram, librime-predict

4. Restart Weasel:
    - Right-click the Weasel icon in system tray
    - Select "Deploy(E)"

5. Verify installation:
    - Right-click the Weasel icon in system tray
    - Select "View Log(L)"
    - In the latest log file (`rime.weasel.xxxxx.INFO.xxxxxx.log`), you should see:
      ```log
      module.cc:17] [qjs] registering components from module 'qjs'.
      qjs_types.cc:22] [qjs] registering rime types to the quickjs engine...
      registry.cc:14] registering component: qjs_processor
      registry.cc:14] registering component: qjs_filter
      registry.cc:14] registering component: qjs_translator
     ```
6. If issues occur, please report with detailed reproduction steps; or delete `C:\Program Files\Rime\weasel-x.xx.x\rime.dll` and restore from backup.

#### Install on Linux (PR Welcome)

### Development Tools

- qjs: QuickJS command-line program for testing JavaScript plugins, included in the release package
    - Program name is qjs for macOS and qjs.exe for Windows
    - Run `./qjs --help` in command line for version info and usage instructions
    - Copy it to `<Rime-user-folder>/js` directory to run `./qjs ./tests/<your-plugin>.test.js` for plugin testing
    - This program has been patched to load node modules and cannot be replaced with the official QuickJS repository's program
- npm/yarn/pnpm: For dependency management
- typescript: Optional, for type hints and smart error correction
- Node.js/bun/deno: Optional, for development and testing assistance

### Project Structure ([Example Plugin Repository](https://github.com/HuangJian/rime-frost/tree/hj))

```pre
<Rime-user-folder>                         # Rime front-end program's user folder, e.g., Squirrel's default `~/Library/Rime` directory
├─── build                                 # Rime core engine's compilation output directory, useful for checking enabled input method options and plugin information
├─── js                                    # JavaScript plugin directory
│    ├─── qjs                              # QuickJS command-line program for running JavaScript plugin unit tests, copy from release package
│    ├─── package.json                     # Plugin package configuration file for managing dependencies
│    ├─── node_modules                     # Dependency libraries
│    ├─── <your-plugin>.js                 # JavaScript plugin source code
│    ├─── tests                            # Unit test directory
│    │     └─── <your-plugin>.test.js      # JavaScript unit test file
│    ├─── jsconfig.json                    # TypeScript configuration file for loading rime type definition files, providing type hints and smart error correction
│    └─── type
│          └─── rime.d.ts                  # Rime type definition file for IDE type hints and smart error correction, copy from release package
├─── squirrel.custom.yaml                  # Squirrel personalization configuration file
├─── xxx.schema.yaml                       # Input method schema configuration file
├─── xxx.custom.yaml                       # Input method schema personalization configuration file
├─── default.yaml                          # Rime configuration file
└─── ...
```

## Quick Start

### Create Plugin Directory
- Create `js` directory in Rime user folder:
  ```bash
  cd ~/Library/Rime  # macOS
  # or cd %APPDATA%\Rime  # Windows
  mkdir js
  ```
- Create `package.json` file for dependency management:
  ```json
  {
    "name": "my-rime-plugin",
    "version": "1.0.0",
    "type": "module",
    "private": true
  }
  ```

### Write Plugin Main File
- Create processor plugin file in `js` directory, e.g., `processor_example.js`:
  ```javascript
  // processor_example.js
  export class ExampleProcessor {
      process(keyEvent, env) {
          // Implement key processing logic
          console.log('Processing key event:', keyEvent.repr)
          if (condition1) {
              return 'kAccepted' // Accept key event, stop passing to next processor
          }
          if (condition2) {
              return 'kRejected' // Leave key event to OS, stop passing to next processor
          }
          return 'kNoop' // Don't process key event, continue passing to next processor
      }
  }
  ```

- Create translator plugin file in `js` directory, e.g., `translator_example.js`:
  ```javascript
  // translator_example.js
  export class ExampleTranslator {
      translate(input, segment, env) {
          // Implement translation logic
          console.log(`translating input: ${input}`)
          return [
              new Candidate('example', segment.start, segment.end, 'Candidate1', 'Note1', 999),
              new Candidate('example', segment.start, segment.end, 'Candidate2', 'Note2', 999),
              new Candidate('example', segment.start, segment.end, 'Candidate3', 'Note3', 999),
          ]
      }
  }
  ```

- Create filter plugin file in `js` directory, e.g., `filter_example.js`:
  ```javascript
  // filter_example.js
  export class ExampleFilter {
      filter(candidates, env) {
          // Implement filtering logic
          console.log(`filtering #${candidates.length} candidates`)
          return candidates.filter(candidate => candidate.text.includes('example'))
      }
  }
  ```

### Embrace NPM
- libRime-qjs supports using NPM modules in plugins, but note:
  1. Only supports ESM format modules, not CommonJS format
  2. Doesn't support Node.js built-in modules (like fs, path), and hasn't enabled quickjs's `qjs:std`, `qjs:os`, and `qjs:bjson` modules
  3. Doesn't support modules depending on binary extensions

- Install dependencies:
  ```bash
  cd ~/Library/Rime/js  # macOS
  npm install lunar-typescript  # Install lunar calendar module
  ```

- Use third-party modules in plugins:
  ```javascript
  // lunar_translator.js
  import { Lunar } from 'lunar-typescript'

  export class LunarTranslator {
      translate(input, segment, env) {
          const lunar = Lunar.fromDate(new Date())
          return [
              new Candidate('lunar', segment.start, segment.end, lunar.getYearInGanZhi(), 'Year', 1), // Year in Chinese calendar
              new Candidate('lunar', segment.start, segment.end, lunar.getMonthInGanZhi(), 'Month', 2), // Month in Chinese calendar
              new Candidate('lunar', segment.start, segment.end, lunar.getDayInGanZhi(), 'Day', 3) // Day in Chinese calendar
          ]
      }
  }
  ```

### Configure Input Method Schema
- Edit input method schema personalization configuration file in Rime user folder, e.g., `rime_frost.custom.yaml`:
  ```yaml
  patch:
    'engine/processors/@before 1':         # Add custom qjs processors. "@before 1" places processor plugin before Rime default processors to ensure plugin effectiveness.
      qjs_processor@processor_example      # Example processor, part after @ is plugin filename (without extension). Don't add - symbol when using "@before" syntax.
    engine/translators/+:                  # Add custom qjs translator
      - qjs_translator@translator_example  # Example translator, part after @ is plugin filename (without extension). Add - symbol when using "+" syntax.
      - qjs_translator@lunar_translator    # Example lunar calendar translator
    engine/filters/+:                      # Add custom qjs filter
      - qjs_filter@filter_example          # Example filter, part after @ is plugin filename (without extension). Add - symbol when using "+" syntax.
  ```
- Redeploy input method to apply configuration:
    - macOS Squirrel: Press <kbd>⌃</kbd> + <kbd>⌥</kbd> + <kbd>`</kbd>, or right-click Squirrel icon in system tray and select "Deploy"
    - Windows Weasel: Right-click Weasel icon in system tray and select "Deploy(E)"

## API Reference

For detailed API reference, please check the [JavaScript Type Definition File](https://github.com/HuangJian/librime-qjs/blob/main/contrib/rime.d.ts).
This file is also included in every release package.

### Basic Types

#### Environment

Input method environment object, provides access to input method context.

**Properties and Methods**
- `engine`: Input method engine object, for accessing input method configuration and state
- `engine.schema`: Input method schema object, contains configuration information of current schema
- `engine.schema.config`: Configuration object, for reading input method configuration items
- `engine.context`: Context object, contains current input state information

**Usage Example**
```javascript
// Access environment object in plugin
export class MyProcessor {
    process(keyEvent, env) {
        const option = env.engine.schema.config.getString('some_option') // Read configuration item
        env.engine.commitText('text') // Commit text
        env.engine.context.clear() // Clear input state
    }
}
```

**Notes**
- Environment object is passed by input method engine, no need to create manually
- Each plugin has its own Environment object, modifying its configuration won't affect other plugins
- Environment objects are different in different input method sessions

#### Candidate

Candidate object, represents a translation candidate.

**Properties**
- `type`: Candidate type, like 'simple', 'phrase', etc.
- `start`: Start position of candidate in input string
- `end`: End position of candidate in input string
- `text`: Display text of candidate
- `comment`: Comment text of candidate
- `quality`: Weight of candidate, used for sorting

**Usage Example**
```javascript
// Create candidates in translator
export class MyTranslator {
    translate(input, segment, env) {
        return [
            new Candidate('type', 0, input.length, 'Candidate', 'Comment', 100)
        ]
    }
}
```

**Notes**
- Must provide all required parameters when creating Candidate object
- Higher quality value means higher position in sorting
- text and comment should have appropriate length to avoid display overflow
- Don't include newlines in text
- comment supports newline `\n` and tab `\t` characters

#### Segment

Segment object, represents a segmentation interval of input string. For example, input `shuxuguan` will be segmented into multiple segments like `shuxuguan`, `shuru`, and `shu`.

**Properties and Methods**
- `start`: Segmentation start position
- `end`: Segmentation end position
- `prompt`: Segmentation prompt text
- `selectedIndex`: Currently selected candidate index in candidate menu associated with segmentation. Commonly used in processor plugins
- `candidateSize`: Number of candidates in candidate menu associated with segmentation. Commonly used in processor plugins
- `hasTag(tag)`: Check if segmentation contains specified tag
- `getCandidateAt(index)`: Get candidate at specified index in candidate menu associated with segmentation. Commonly used in processor plugins

**Usage Example**
```javascript
// Use segmentation information in translator
class MyTranslator {
    translate(input, segment, env) {
        // Get input text of segmentation interval
        const text = input.substring(segment.start, segment.end)
        // Check segmentation tag
        if (segment.hasTag('abc')) {
            // Handle specific type of segmentation
        }
    }
}
```

**Notes**
- Segment object is created and managed by input method engine
- In processor plugins, if no candidate menu is expanded currently, Segment object obtained through `env.engine.context.lastSegment` is null

#### KeyEvent

Key event object, represents user's key input.

**Properties**
- `repr`: String representation of key, like 'Return', 'space', etc. See detailed list in [rime.js](https://github.com/HuangJian/rime-frost/blob/hj/js/lib/rime.js)
- `shift`: Whether Shift key is pressed
- `ctrl`: Whether Ctrl key is pressed
- `alt`: Whether Alt key is pressed
- `release`: Whether it's a key release event

**Usage Example**
```javascript
// Handle key events in processor
export class MyProcessor {
    process(keyEvent, env) {
        // Check specific key
        if (keyEvent.repr === 'space') { // Space key
            return 'kAccepted'
        }
        // Check key combination
        if (keyEvent.ctrl && keyEvent.repr.toLowerCase() === 'b') { // Ctrl+B
            return 'kAccepted'
        }
        return 'kNoop'
    }
}
```

**Notes**
- Use KeyRepr constants defined in [rime.js](https://github.com/HuangJian/rime-frost/blob/hj/js/lib/rime.js) instead of custom strings
- Pay attention to modifier key combinations

#### Trie

Trie data structure, used for efficient storage and retrieval of dictionary data (key-value pairs).

**Constructor**
```javascript
const trie = new Trie()
```

**Methods**
- `loadTextFile(path: string, entrySize?: number)`: Load dictionary data from text file. Slow, takes >100ms to load and parse 60k lines of English-Chinese dictionary
  - `path`: Text file path
  - `entrySize`: Number of entries in file
  - One entry per line, format: `<key>\t<value>`
  - Lines starting with `#` are treated as comments and ignored
  - Throws exception: When file doesn't exist or format is invalid

- `saveToBinaryFile(path: string)`: Save dictionary data to binary file for fast loading in future use
  - `path`: Target file path
  - Throws exception: When file can't be written

- `loadBinaryFile(path: string)`: Load dictionary data from binary file. Fast, takes about 10ms to load and parse 60k lines of English-Chinese dictionary
  - `path`: Binary file path
  - Throws exception: When file doesn't exist or format is invalid

- `find(key: string)`: Exact key-value pair lookup
  - `key`: Key to look up
  - Returns: Corresponding value if found, null otherwise

- `prefixSearch(prefix: string)`: Prefix search
  - `prefix`: Prefix to search
  - Returns: Array of key-value pairs matching prefix, each element contains `text` (key) and `info` (value)

**Usage Example**
```javascript
// Load dictionary from text file
const trie = new Trie()
trie.loadTextFile('/path/to/dict.txt', 60000) // Load 60k-line dictionary

// Lookup and search
const value = trie.find('hello')  // Exact lookup
const matches = trie.prefixSearch('he')  // Prefix search

// Save to binary format (for faster future loading)
trie.saveToBinaryFile('/path/to/dict.bin')
```

**Notes**
- Convert text dictionary to binary format first to significantly improve loading speed
- Ensure text dictionary format is correct and entry count matches `entrySize` parameter
- Exception handling: Use try-catch to catch possible exceptions when calling methods

## Plugin Lifecycle

Plugin lifecycle is divided into four phases, each with specific tasks and considerations:

### 1. Loading Phase

In this phase, the libRime-qjs engine will:
- Read plugin JavaScript source file
- Perform syntax check and pre-compilation
- Verify if plugin exported interfaces comply with specifications

Notes:
- Ensure plugin file path is correct and has read permission
- Avoid time-consuming operations in global scope
- Handle file encoding properly (UTF-8 recommended)

### 2. Initialization Phase: Call Plugin Constructor `constructor()`

This phase mainly completes:
- Create plugin class instance
- Load configuration information
- Initialize resources needed by plugin

Notes:
- Handle missing configuration cases properly
- Avoid time-consuming initialization operations
- Handle initialization failures properly

### 3. Running Phase: Call Plugin Main Methods `process()`, `translate()`, `filter()`

Running phase is plugin's main working phase:
- Respond to input method engine calls
- Handle user input
- Manage plugin state and context

Notes:
- Maintain good performance, avoid time-consuming operations
  - For intensive string splitting operations, prefer `string.indexOf()` and `string.substring()` over regular expressions.
- Handle exceptions properly
- Use logging reasonably for debugging
  - `filter()` function usually handles thousands of candidates, avoid `console.log()` for each object to prevent log flooding and difficulty in locating issues.

### 4. Unloading Phase: Call Plugin Unload Function `finalizer()`

This phase handles cleanup:
- Release occupied resources
- Save data that needs persistence
- Clean up temporary files created by plugin

Notes:
- Ensure all resources are properly released
- Handle exceptions when saving user data
- Avoid time-consuming operations in this phase

## [Example References](https://github.com/HuangJian/rime-frost)

### processor Key Processors
   - [select_character.js](https://github.com/HuangJian/rime-frost/blob/hj/js/select_character.js) - Select character from word
   - [pairs.js](https://github.com/HuangJian/rime-frost/blob/hj/js/pairs.js)  - Symbol pairing: Auto-complete paired symbols and move cursor inside. Cursor movement currently only supports macOS platform.
   - [shortcut.js](https://github.com/HuangJian/rime-frost/blob/hj/js/shortcut.js) - Shortcut commands: /deploy, /screenshot, ... Deployed as processor to execute commands
   - [slash.js](https://github.com/HuangJian/rime-frost/blob/hj/js/slash.js) - Consecutive slashes: Toggle between candidates

### translator Translators
   - [lunar_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/lunar_translator.js) - Lunar calendar, depends on node module `lunar-typescript`
   - [calculator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/calculator.js) - Calculator, implements high-precision arithmetic based on BigInt, supports `Math` library functions
   - [date_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/date_translator.js) - Time, date, weekday
   - [custom_phrase.js](https://github.com/HuangJian/rime-frost/blob/hj/js/custom_phrase.js) - Custom phrases custom_phrase.txt
   - [unicode_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/unicode_translator.js) - Unicode, input U62fc to get '拼'
   - [number_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/number_translator.js) - Numbers, uppercase amount
   - [help_menu.js](https://github.com/HuangJian/rime-frost/blob/hj/js/help_menu.js) - Help menu, triggered by /help
   - [shortcut.js](https://github.com/HuangJian/rime-frost/blob/hj/js/shortcut.js) - Shortcut commands: /deploy, /screenshot, ... Deployed as translator to provide candidates

### filter Filters
   - [en2cn](https://github.com/HuangJian/rime-frost/blob/hj/js/en2cn.js) - Simple English word definitions. Memory usage ~12MB.
   - [cn2en_pinyin](https://github.com/HuangJian/rime-frost/blob/hj/js/cn2en_pinyin.js) - Chinese to English translation with pinyin. Memory usage ~16MB.
   - [sort_by_pinyin](https://github.com/HuangJian/rime-frost/blob/hj/js/sort_by_pinyin.js) - Reorder candidates in place based on pinyin matching degree with input code. Eliminates fuzzy pronunciation effects.
   - [charset_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/charset_filter.js) - Filter out candidates containing CJK extended characters to avoid invisible characters in candidate box.
   - [is_in_user_dict](https://github.com/HuangJian/rime-frost/blob/hj/js/is_in_user_dict.js) - Add * to user dictionary words and ∞ to long sentences
   - [pin_cand_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/pin_cand_filter.js) - Pin candidates to top (Order requirement: Pinned candidates > Emoji > Traditional/Simplified switch)
   - [long_word_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/long_word_filter.js) - Long word priority (Order requirement: Long word priority > Emoji)
   - [reduce_english_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/reduce_english_filter.js) - Lower the position of certain English words in candidates
   - [autocap_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/autocap_filter.js) - Auto-capitalize English. Place at the end to handle English candidates added by en2cn.

## Debugging and Testing

### Logging Output

- Use `console.log()` for debug output
- Use `console.error()` for exception logging
- macOS Squirrel logs output to `$TMPDIR/rime.squirrel.INFO` and `$TMPDIR/rime.squirrel.ERROR`
- Windows Weasel logs output to `%AppData%\Local\Temp\rime.weasel\rime.weasel.xxxxx.INFO.xxxxxx.log` and `%AppData%\Local\Temp\rime.weasel\rime.weasel.xxxxx.ERROR.xxxxxx.log`
- JavaScript plugin logs are prefixed with `$qjs$` for easy identification

### Unit Testing

#### Installation and Configuration
1. Copy `qjs` command-line program from release package to `<Rime-user-folder>/js` directory
2. Create test file in `tests` directory, e.g., `<your-plugin>.test.js`
3. Run test: `./qjs ./tests/<your-plugin>.test.js`

#### Test File Organization
```javascript
// tests/example.test.js
import { ExampleProcessor } from '../example.js'

const processor = new ExampleProcessor()
const env = { /* mock environment object */ }

// Test case 1: Process space key
const spaceKey = { repr: 'space', shift: false, ctrl: false, alt: false, release: false }
const result1 = processor.process(spaceKey, env)
console.log('Test case 1:', result1 === 'kAccepted' ? 'PASS' : 'FAIL')

// Test case 2: Process Ctrl+B
const ctrlB = { repr: 'b', shift: false, ctrl: true, alt: false, release: false }
const result2 = processor.process(ctrlB, env)
console.log('Test case 2:', result2 === 'kAccepted' ? 'PASS' : 'FAIL')
```

#### Writing Test Cases
- Test each component type (processor/translator/filter) separately
- Mock necessary environment objects and input data
- Test both normal and edge cases
- Add descriptive test names and error messages

#### Running Tests
```bash
cd ~/Library/Rime/js  # macOS
./qjs ./tests/<your-plugin>.test.js
```

### Using Other JavaScript Engines

- Besides QuickJS, you can use other JavaScript engines like Node.js/Bun/Deno to run unit tests, helping debug issues and verify ECMAScript feature support.
- Most Node.js-specific modules can be used in Bun and Deno without additional adaptation, but none are supported in QuickJS.
- The `node:fs` module can be replaced with QuickJS's `std` module. For usage examples, please refer to [en2cn.benchmark.js](https://github.com/HuangJian/rime-frost/blob/hj/js/tests/benchmark/en2cn.benchmark.js).
- 
### Type Definition Files

#### Installation
1. Copy `rime.d.ts` from release package to `<Rime-user-folder>/js/type` directory
2. Create `jsconfig.json` in `js` directory:
```json
{
  "compilerOptions": {
    "target": "es2022",
    "module": "es2022",
    "moduleResolution": "node",
    "checkJs": true,
    "allowJs": true,
    "noEmit": true,
    "typeRoots": ["./type"],
    "types": ["rime"]
  },
  "include": ["*.js", "tests/**/*.js"],
  "exclude": ["node_modules"]
}
```

#### Configuration
The type definition file provides TypeScript type information for:
- Basic types (Environment, Candidate, Segment, KeyEvent)
- Plugin interfaces (Processor, Translator, Filter)
- Utility classes (Trie)

#### IDE Integration
- Visual Studio Code: Automatically recognizes type definitions
- Other IDEs: Configure TypeScript support according to IDE documentation

#### Common Issues
1. Type definitions not working
   - Check if `rime.d.ts` is in the correct location
   - Verify `jsconfig.json` configuration
   - Ensure IDE TypeScript support is enabled

2. Incorrect type hints
   - Update to latest `rime.d.ts` from release package
   - Check if using correct import syntax
   - Verify plugin class implements correct interface

## Best Practices (To Be Added)

## Common Issues (To Be Added)
