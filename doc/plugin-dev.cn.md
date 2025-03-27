# Rime JavaScript 插件开发指南

## 目录

- [Rime JavaScript 插件开发指南](#rime-javascript-插件开发指南)
  - [目录](#目录)
  - [简介](#简介)
  - [系统架构](#系统架构)
    - [核心组件](#核心组件)
    - [工作流程](#工作流程)
  - [开发环境](#开发环境)
    - [安装插件](#安装插件)
      - [下载 libRime-qjs](#下载-librime-qjs)
      - [安装到 macOS (Squirrel)](#安装到-macos-squirrel)
      - [安装到 Windows (小狼毫)](#安装到-windows-小狼毫)
      - [安装到 Linux (PR Welcome)](#安装到-linux-pr-welcome)
    - [开发工具](#开发工具)
    - [项目结构 （示例插件仓库）](#项目结构-示例插件仓库)
  - [快速开始](#快速开始)
    - [创建插件目录](#创建插件目录)
    - [编写插件主文件](#编写插件主文件)
    - [拥抱 NPM](#拥抱-npm)
    - [配置输入法方案](#配置输入法方案)
  - [API 参考](#api-参考)
    - [基础类型](#基础类型)
      - [Environment](#environment)
      - [Candidate](#candidate)
      - [Segment](#segment)
      - [KeyEvent](#keyevent)
      - [Trie](#trie)
  - [插件生命周期](#插件生命周期)
    - [1. 加载阶段](#1-加载阶段)
    - [2. 初始化阶段：调用插件的构造函数 `constructor()`](#2-初始化阶段调用插件的构造函数-constructor)
    - [3. 运行阶段：调用插件的主方法 `process()`、`translate()`、`filter()`](#3-运行阶段调用插件的主方法-processtranslatefilter)
    - [4. 卸载阶段：调用插件的卸载函数 `finalizer()`](#4-卸载阶段调用插件的卸载函数-finalizer)
  - [实例参考](#实例参考)
    - [processor 按键处理器](#processor-按键处理器)
    - [translator 翻译器](#translator-翻译器)
    - [filter 过滤器](#filter-过滤器)
  - [调试与测试](#调试与测试)
    - [日志输出](#日志输出)
    - [单元测试](#单元测试)
      - [安装和配置](#安装和配置)
      - [测试文件组织](#测试文件组织)
      - [编写测试用例](#编写测试用例)
      - [执行测试](#执行测试)
    - [使用其他 JavaScript 引擎](#使用其他-javascript-引擎)
    - [类型定义文件](#类型定义文件)
      - [安装](#安装)
      - [配置](#配置)
      - [IDE 集成](#ide-集成)
      - [常见问题](#常见问题)
  - [最佳实践 （待补充）](#最佳实践-待补充)
  - [常见问题 （待补充）](#常见问题-待补充)

## 简介

libRime-qjs 插件系统允许开发者使用 JavaScript 语言为 Rime 输入法开发各类功能组件，如按键处理器（Processor）、翻译器（Translator）、过滤器（Filter）等。
本文档将引导您开发、测试和部署这些 JavaScript 插件。

## 系统架构

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
│                   │  │                   │ │                   │         | | 聪明的输入法懂我心意█  | |
  │               │      │               │     │               │           | |                    | |
   ──          ──         ──          ──        ──          ──             | |                    | |
     ──────────             ──────────            ──────────               | :                    : |
                                                                           :  '------------------'  :
                                                                            '---...___________...---'
                                                                              /                   \
Edit/view: https://cascii.app/0ca1e                                           '-...___________...-'
```

### 核心组件

- **libRime**：输入法核心引擎，负责整体调度和协调。它处理输入法的基本功能，如文字输入、候选词生成、词典管理等，并为各类组件提供统一的接口和运行环境。

- **libRime-qjs**：JavaScript 运行时插件，是连接 libRime 和 JavaScript 插件的桥梁。它基于 QuickJS 引擎，提供了 JavaScript 插件的加载、执行环境和必要的 API 接口，使开发者能够用 JavaScript 语言开发输入法组件。

- **Squirrel/Weasel/Fcitx/..**：输入法的前端程序，负责与操作系统进行交互并提供用户界面。它们接收用户的按键输入，将其传递给 libRime 处理，并将处理结果（如候选词）展示给用户。不同的前端程序适配不同的操作系统平台。

- **plugin.js**：用户开发的具体插件实现，可以包含多种类型的输入法组件：
  - Processor（处理器）：处理用户的按键事件
  - Translator（翻译器）：将输入转换为候选词
  - Filter（过滤器）：对候选词进行过滤和排序

### 工作流程

1. 用户在前端程序中输入文字
2. 前端程序将输入传递给 libRime
3. libRime 根据配置调用相应的 JavaScript 插件
4. libRime-qjs 负责执行这些插件，并将结果返回给 libRime
5. libRime 处理完成后，将结果传回前端程序显示

## 开发环境

### 安装插件

#### 下载 libRime-qjs

从项目仓库下载最新的 libRime-qjs 压缩包：https://github.com/HuangJian/librime-qjs/releases

#### 安装到 macOS (Squirrel)

1. 备份 Squirrel 输入法的所有动态链接库文件：`/Library/Input Methods/Squirrel.app/Contents/Frameworks`
2. 把 `librime-qjs.dylib` 复制到 `/Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins`
3. 如果 `/Library/Input Methods/Squirrel.app/Contents/Frameworks/librime.x.xx.x.dylib` 文件与压缩包的版本不一致，可替换为压缩包里的 `librime.y.yy.y.dylib`、`librime.dylib` 和 `librime.1.dylib`
4. 验证安装：
    - 使用 <kbd>⌘</kbd> + <kbd>Space</kbd> 启用 Squirrel 输入法
    - 检查日志：`cat $TMPDIR/rime.squirrel.INFO | grep qjs`
    - 如看到类似输出，表示 lirRime-qjs 已生效：
        > loading plugin 'qjs' from /Library/Input Methods/Squirrel.app/Contents/Frameworks/rime-plugins/librime-qjs.dylib
        > registering component: qjs_processor
        > registering component: qjs_filter
        > loaded plugin: qjs
5. 如果遇到问题，请报 issue 并提供详细重现步骤；或删除`rime-plugins/librime-qjs.dylib` 文件，并使用备份版本恢复输入法。

#### 安装到 Windows (小狼毫)

1. 关闭小狼毫：
    - 右键点击系统托盘中的小狼毫图标
    - 选择「退出(Q)」

2. 备份程序库：`C:\Program Files\Rime\weasel-x.xx.x\rime.dll`

3. 安装插件：
    - 把压缩包里的 `rime.dll` 复制到 `C:\Program Files\Rime\weasel-x.xx.x`，替换上一步备份的 `rime.dll`
    - 注：该动态链接库已包含 libRime、libRime-qjs、libRime-lua、librime-octagram、librime-predict 等组件

4. 重启小狼毫：
    - 右键点击系统托盘中的小狼毫图标
    - 选择「重新部署(E)」

5. 验证安装：
    - 右键点击系统托盘中的小狼毫图标
    - 选择「查看日志(L)」
    - 在最新的日志文件（`rime.weasel.xxxxx.INFO.xxxxxx.log`）中应该看到：
      ```log
      module.cc:17] [qjs] registering components from module 'qjs'.
      qjs_types.cc:22] [qjs] registering rime types to the quickjs engine...
      registry.cc:14] registering component: qjs_processor
      registry.cc:14] registering component: qjs_filter
      registry.cc:14] registering component: qjs_translator
      ```
6. 如果遇到问题，请报 issue 并提供详细重现步骤；或删除`C:\Program Files\Rime\weasel-x.xx.x\rime.dll` 文件，并使用备份版本恢复输入法。

#### 安装到 Linux (PR Welcome)

### 开发工具

- qjs: quickjs 命令行程序，用于测试 JavaScript 插件，已附在 release 包内
    - 适配 macOS 的程序名为 qjs， 适配 Windows 的程序名为 qjs.exe
    - 命令行执行 `./qjs --help` 查看版本信息和使用说明
    - 将其复制到 `<Rime-user-folder>/js` 目录下，可执行 `./qjs ./tests/<your-plugin>.test.js` 测试插件代码
    - 该程序已打上加载 node module 的补丁，不能用 quickjs 官方仓库发布的同名程序替代
- npm/yarn/pnpm: 用于管理依赖
- typescript: 可选，用于类型提示、智能纠错
- Node.js/bun/deno: 可选，用于辅助开发测试

### 项目结构 （[示例插件仓库](https://github.com/HuangJian/rime-frost/tree/hj)）

```pre
<Rime-user-folder>                         # Rime 前端程序的用户文件夹，如 Squirrel 输入法默认的 `~/Library/Rime` 目录
├─── build                                 # Rime 核心引擎的编译输出目录，可用于查看实际启用的输入法选项和插件信息
├─── js                                    # JavaScript 插件目录
│    ├─── qjs                              # quickjs 的命令行程序，用于执行 JavaScript 插件的单元测试，请从 release 包内复制过来
│    ├─── package.json                     # 插件包配置文件，用于管理依赖
│    ├─── node_modules                     # 依赖库
│    ├─── <your-plugin>.js                 # JavaScript 插件源代码
│    ├─── tests                            # 单元测试目录
│    │     └─── <your-plugin>.test.js      # JavaScript 单元测试文件
│    ├─── jsconfig.json                    # TypeScript 配置文件，用于加载 rime 类型定义文件，为 IDE 提供类型提示和智能纠错
│    └─── type
│          └─── rime.d.ts                  # Rime 类型定义文件，为 IDE 提供类型提示和智能纠错，请从 release 包内复制过来
├─── squirrel.custom.yaml                  # Squirrel 个性化配置文件
├─── xxx.schema.yaml                       # 输入法方案配置文件
├─── xxx.custom.yaml                       # 输入法方案个性化配置文件
├─── default.yaml                          # Rime 配置文件
└─── ...
```

## 快速开始

### 创建插件目录
- 在 Rime 用户文件夹下创建 `js` 目录：
  ```bash
  cd ~/Library/Rime  # macOS
  # 或 cd %APPDATA%\Rime  # Windows
  mkdir js
  ```
- 创建 `package.json` 文件，用于管理依赖：
  ```json
  {
    "name": "my-rime-plugin",
    "version": "1.0.0",
     "type": "module",
    "private": true
  }
  ```

### 编写插件主文件
- 在 `js` 目录下创建处理器插件文件，如 `processor_example.js`：
  ```javascript
  // processor_example.js
  export class ExampleProcessor {
      process(keyEvent, env) {
          // 实现按键处理逻辑
          console.log('Processing key event:', keyEvent.repr)
          if (condition1) {
              return 'kAccepted' // 接收按键事件，不再传递给下一个 processor
          }
          if (condition2) {
              return 'kRejected' // 将按键事件留给操作系统处理，不再传递给下一个 processor
          }
          return 'kNoop' // 不处理按键事件，继续传递给下一个 processor
      }
  }

- 在 `js` 目录下创建翻译器插件文件，如 `translator_example.js`：
  ```javascript
  // translator_example.js
  export class ExampleTranslator {
      translate(input, segment, env) {
          // 实现转换逻辑
          console.log(`translating input: ${input}`)
          return [
              new Candidate('example', segment.start, segment.end, '候选词1', '注解1', 999),
              new Candidate('example', segment.start, segment.end, '候选词2', '注解2', 999),
              new Candidate('example', segment.start, segment.end, '候选词3', '注解3', 999),
          ]
      }
  }
  ```

- 在 `js` 目录下创建过滤器插件文件，如 `filter_example.js`：
  ```javascript
  // filter_example.js
  export class ExampleFilter {
      filter(candidates, env) {
          // 实现过滤逻辑
          console.log(`filtering #${candidates.length} candidates`)
          return candidates.filter(candidate => candidate.text.includes('example'))
      }
  }
  ```

### 拥抱 NPM
- libRime-qjs 支持在插件中使用 NPM 模块，但需要注意以下几点：
  1. 只支持 ESM 格式的模块，不支持 CommonJS 格式
  2. 不支持 Node.js 内置模块（如 fs、path 等），也未启用 quickjs 的 `qjs:std`、 `qjs:os` 和 `qjs:bjson` 模块
  3. 不支持依赖二进制扩展的模块

- 安装依赖：
  ```bash
  cd ~/Library/Rime/js  # macOS
  npm install lunar-typescript  # 安装农历模块
  ```

- 在插件中使用第三方模块：
  ```javascript
  // lunar_translator.js
  import { Lunar } from 'lunar-typescript'

  export class LunarTranslator {
      translate(input, segment, env) {
          const lunar = Lunar.fromDate(new Date())
          return [
              new Candidate('lunar', segment.start, segment.end, lunar.getYearInGanZhi(), '年', 1), // 乙巳年
              new Candidate('lunar', segment.start, segment.end, lunar.getMonthInGanZhi(), '月', 2), // 甲申月
              new Candidate('lunar', segment.start, segment.end, lunar.getDayInGanZhi(), '日', 3) // 丙辰日
          ]
      }
  }
  ```

### 配置输入法方案
- 在 Rime 用户文件夹下，编辑输入法方案个性化配置文件，如 `rime_frost.custome.yaml`：
  ```yaml
  patch:
    'engine/processors/@before 1':         # 添加自定义的 qjs processors。"@before 1"表示把处理器插件置于 Rime 默认处理器前，以保证插件生效。
      qjs_processor@processor_example      # 示例处理器，@ 后面的部分为插件文件名（不包含扩展名）。因为使用"@before"语法，前面不要加 - 符号。
    engine/translators/+:                  # 添加自定义的 qjs translator
      - qjs_translator@translator_example  # 示例转换器，@ 后面的部分为插件文件名（不包含扩展名）。因为使用"+"语法，前面需要加 - 符号。
      - qjs_translator@lunar_translator    # 示例农历转换器
    engine/filters/+:                      # 添加自定义的 qjs filter
      - qjs_filter@filter_example          # 示例过滤器，@ 后面的部分为插件文件名（不包含扩展名）。因为使用"+"语法，前面需要加 - 符号。
  ```
- 重新部署输入法使配置生效：
    - macOS Squirrel: 按下 <kbd>⌃</kbd> + <kbd>⌥</kbd> + <kbd>`</kbd> 组合键，或者右键点击系统托盘中的 Squirrel 图标，选择「重新部署」
    - Windows Weasel: 右键点击系统托盘中的小狼毫图标，选择「重新部署(E)」

## API 参考

详细的 API 参考请查看 [JavaScript 类型定义文件](https://github.com/HuangJian/librime-qjs/blob/main/contrib/rime.d.ts)。
该文件也已经打包到每一个版本的 release 包中。

### 基础类型

#### Environment

输入法环境对象，提供了访问输入法上下文的能力。

**属性和方法**
- `engine`: 输入法引擎对象，可用于访问输入法的配置和状态
- `engine.schema`: 输入法方案对象，包含当前使用的输入方案的配置信息
- `engine.schema.config`: 配置对象，用于读取输入法的配置项
- `engine.context`: 上下文对象，包含当前输入状态的信息

**使用示例**
```javascript
// 在插件中访问环境对象
export class MyProcessor {
    process(keyEvent, env) {
        const option = env.engine.schema.config.getString('some_option') // 读取配置项
        env.engine.commitText('text') // 上屏文字: text
        env.engine.context.clear() // 清空输入状态
    }
}
```

**注意事项**
- Environment 对象由输入法引擎传入，不需要手动创建
- 每个插件独享一个 Environment 对象，修改其配置不会影响其他插件
- 在输入法的不同会话中，Environment 对象是不同的

#### Candidate

候选项对象，表示一个转换候选词。

**属性**
- `type`: 候选项类型，如 'simple'、'phrase' 等
- `start`: 候选项在输入串中的起始位置
- `end`: 候选项在输入串中的结束位置
- `text`: 候选项的显示文本
- `comment`: 候选项的注释文本
- `quality`: 候选项的权重，用于排序

**使用示例**
```javascript
// 在翻译器中创建候选项
export class MyTranslator {
    translate(input, segment, env) {
        return [
            new Candidate('type', 0, input.length, '候选词', '注释', 100)
        ]
    }
}
```

**注意事项**
- 创建 Candidate 对象时必须提供所有必需的参数
- quality 值越大，候选项排序越靠前
- text 和 comment 应该使用合适的长度，避免显示溢出
- 不要在 text 中包含换行符
- comment 支持换行符 `\n` 和制表符 `\t`

#### Segment

切分片段对象，表示输入串的一个切分区间。 如输入 `shuxuguan`, 会切分为 `shuxuguan`、`shuru`和`shu` 等多种片段。

**属性方法**
- `start`: 切分起始位置
- `end`: 切分结束位置
- `prompt`: 切分提示文本
- `selectedIndex`: 切分关联的候选项菜单上，当前选中的候选项索引。常用于 processor 插件
- `candidateSize`: 与切分关联的候选项菜单上，候选项的数量。常用于 processor 插件
- `hasTag(tag)`: 检查切分是否包含指定标签
- `getCandidateAt(index)`: 获取切分关联的候选项菜单上，指定索引处的候选项。常用于 processor 插件

**使用示例**
```javascript
// 在翻译器中使用切分信息
class MyTranslator {
    translate(input, segment, env) {
        // 获取切分区间的输入文本
        const text = input.substring(segment.start, segment.end)
        // 检查切分标签
        if (segment.hasTag('abc')) {
            // 处理特定类型的切分
        }
    }
}
```

**注意事项**
- Segment 对象由输入法引擎创建和管理
- 在 processor 插件中，如果当前没有候选项菜单展开，通过 `env.engine.context.lastSegment` 获得的 Segment 对象为 null

#### KeyEvent

按键事件对象，表示用户的按键输入。

**属性**
- `repr`: 按键的字符串表示，如 'Return'、'space' 等。详细列表请查看 [rime.js](https://github.com/HuangJian/rime-frost/blob/hj/js/lib/rime.js)
- `shift`: 是否按下 Shift 键
- `ctrl`: 是否按下 Ctrl 键
- `alt`: 是否按下 Alt 键
- `release`: 是否按键释放事件

**使用示例**
```javascript
// 在处理器中处理按键事件
export class MyProcessor {
    process(keyEvent, env) {
        // 检查特定按键
        if (keyEvent.repr === 'space') { // 空格键
            return 'kAccepted'
        }
        // 检查组合键
        if (keyEvent.ctrl && keyEvent.repr.toLowerCase() === 'b') { // Ctrl+B
            return 'kAccepted'
        }
        return 'kNoop'
    }
}
```

**注意事项**
- 使用 [rime.js](https://github.com/HuangJian/rime-frost/blob/hj/js/lib/rime.js) 中定义的 KeyRepr 常量，而不是自定义字符串
- 注意处理修饰键的组合情况

#### Trie

词典树数据结构，用于高效存储和检索词典数据（键值对）。

**构造函数**
```javascript
const trie = new Trie()
```

**方法**
- `loadTextFile(path: string, entrySize?: number)`: 从文本文件加载词典数据。速度慢，加载解析 6 万行的英译汉词典耗时 > 100ms
  - `path`: 文本文件路径
  - `entrySize`: 文件中条目行数
  - 每行一个条目，格式为 `<key>\t<value>`
  - `#` 开头的行被视为注释，会被忽略
  - 抛出异常：文件不存在或格式无效时

- `saveToBinaryFile(path: string)`: 将字典数据保存为二进制文件，以便后继使用时快速加载
  - `path`: 目标文件路径
  - 抛出异常：文件无法写入时

- `loadBinaryFile(path: string)`: 从二进制文件加载字典数据。速度快，加载解析 6 万行的英译汉词典耗时约 10ms
  - `path`: 二进制文件路径
  - 抛出异常：文件不存在或格式无效时

- `find(key: string)`: 精确查找键值对
  - `key`: 要查找的键
  - 返回：找到时返回对应的值，否则返回 null

- `prefixSearch(prefix: string)`: 前缀搜索
  - `prefix`: 要搜索的前缀
  - 返回：匹配前缀的键值对数组，每个元素包含 `text`（键）和 `info`（值）

**使用示例**
```javascript
// 从文本文件加载字典
const trie = new Trie()
trie.loadTextFile('/path/to/dict.txt', 60000) // 加载 6 万行的词典

// 查找和搜索
const value = trie.find('hello')  // 精确查找
const matches = trie.prefixSearch('he')  // 前缀搜索

// 保存为二进制格式（加快后续加载）
trie.saveToBinaryFile('/path/to/dict.bin')
```

**注意事项**
- 先将文本格式的字典转换为二进制格式，可显著提升加载速度
- 确保文本字典格式正确，条目数量与 `entrySize` 参数匹配
- 异常处理：调用方法时注意使用 try-catch 捕获可能的异常

## 插件生命周期

插件的生命周期分为四个阶段，每个阶段都有其特定的任务和注意事项：

### 1. 加载阶段

在这个阶段，libRime-qjs 引擎会：
- 读取插件的 JavaScript 源文件
- 进行语法检查和预编译
- 验证插件导出的接口是否符合规范

注意事项：
- 确保插件文件路径正确且有读取权限
- 避免在全局作用域进行耗时操作
- 处理好文件编码（推荐使用 UTF-8）

### 2. 初始化阶段：调用插件的构造函数 `constructor()`

这个阶段主要完成：
- 创建插件类的实例
- 加载配置信息
- 初始化插件所需的资源

注意事项：
- 合理处理配置缺失的情况
- 避免耗时过长的初始化操作
- 妥善处理初始化失败的情况

### 3. 运行阶段：调用插件的主方法 `process()`、`translate()`、`filter()`

运行阶段是插件的主要工作阶段：
- 响应输入法引擎的调用
- 处理用户输入
- 管理插件的状态和上下文

注意事项：
- 保持良好的性能，避免耗时操作
  - 密集的字符串分割操作，优先使用 `string.indexOf()` 和 `string.substring()` 代替正则表达式。
- 正确处理异常情况
- 合理使用日志，便于调试
  - `filter()` 函数通常会处理几千个候选项，避免 `console.log()` 每一个对象，造成日志刷屏难以定位问题。

### 4. 卸载阶段：调用插件的卸载函数 `finalizer()`

这个阶段负责清理工作：
- 释放占用的资源
- 保存需要持久化的数据
- 清理插件创建的临时文件

注意事项：
- 确保所有资源都被正确释放
- 保存用户数据时要处理好异常情况
- 不要在这个阶段进行耗时操作

## [实例参考](https://github.com/HuangJian/rime-frost)

### processor 按键处理器
   - [select_character.js](https://github.com/HuangJian/rime-frost/blob/hj/js/select_character.js) - 以词定字
   - [pairs.js](https://github.com/HuangJian/rime-frost/blob/hj/js/pairs.js)  - 符号配对：自动补全配对的符号，并把光标左移到符号对内部。光标移动功能目前仅支持 macOS 平台。
   - [shortcut.js](https://github.com/HuangJian/rime-frost/blob/hj/js/shortcut.js) - 快捷指令：/deploy 布署、/screenshot 截屏、…… 部署为 processor 以执行指令
   - [slash.js](https://github.com/HuangJian/rime-frost/blob/hj/js/slash.js) - 连续斜杠：在候选项中交替切换

### translator 翻译器
   - [lunar_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/lunar_translator.js) - 农历，依赖 node module `lunar-typescript`
   - [calculator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/calculator.js) - 计算器，基于 BigInt 实现高精度四则运算，支持 `Math` 库函数
   - [date_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/date_translator.js) - 时间、日期、星期
   - [custom_phrase.js](https://github.com/HuangJian/rime-frost/blob/hj/js/custom_phrase.js) - 自定义短语 custom_phrase.txt
   - [unicode_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/unicode_translator.js) - Unicode，输入 U62fc 得到 '拼'
   - [number_translator.js](https://github.com/HuangJian/rime-frost/blob/hj/js/number_translator.js) - 数字、金额大写
   - [help_menu.js](https://github.com/HuangJian/rime-frost/blob/hj/js/help_menu.js) - 帮助菜单，/help 触发显示
   - [shortcut.js](https://github.com/HuangJian/rime-frost/blob/hj/js/shortcut.js) - 快捷指令：/deploy 布署、/screenshot 截屏、…… 部署为 translator 以提供候选项

### filter 过滤器
   - [en2cn](https://github.com/HuangJian/rime-frost/blob/hj/js/en2cn.js) - 英文单词简单释义。内存占用~12MB。
   - [cn2en_pinyin](https://github.com/HuangJian/rime-frost/blob/hj/js/cn2en_pinyin.js) - 汉译英，并显示拼音。内存占用~16MB。
   - [sort_by_pinyin](https://github.com/HuangJian/rime-frost/blob/hj/js/sort_by_pinyin.js) - 根据拼音与输入码的匹配程度，就地重排候选项。消除模糊音影响。
   - [charset_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/charset_filter.js) - 滤除含 CJK 扩展汉字的候选项，避免候选框中出现不可见的字符集。
   - [is_in_user_dict](https://github.com/HuangJian/rime-frost/blob/hj/js/is_in_user_dict.js) - 用户词典的词加上一个*，长句加上一个 ∞
   - [pin_cand_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/pin_cand_filter.js) - 置顶候选项（顺序要求：置顶候选项 > Emoji > 简繁切换）
   - [long_word_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/long_word_filter.js) - 长词优先（顺序要求：长词优先 > Emoji）
   - [reduce_english_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/reduce_english_filter.js) - 降低部分英语单词在候选项的位置
   - [autocap_filter](https://github.com/HuangJian/rime-frost/blob/hj/js/autocap_filter.js) - 英文自动大写。放在最后面，以处理 en2cn 增加的英文候选项。

## 调试与测试

### 日志输出

- 使用 `console.log()` 进行调试输出
- 使用 `console.error()` 记录异常情况
- macOS Squirrel 的日志输出到 `$TMPDIR/rime.squirrel.INFO` 和 `$TMPDIR/rime.squirrel.ERROR`
- Windows Weasel 的日志输出到 `%AppData%\Local\Temp\rime.weasel\rime.weasel.xxxxx.INFO.xxxxxx.log` 和 `$%AppData%\Local\Temp\rime.weasel\rime.weasel.xxxxx.ERROR.xxxxxx.log`
- JavaScript 插件输出的日志，均带有 `$qjs$` 前缀，便于区分

### 单元测试

- 如果在输入法运行过程中，进行调试插件将会异常繁琐。建议为每一个插件编写单元测试，便于随时运行测试，快速发现并定位问题，也避免后继修改破坏插件逻辑。
- [实例参考](https://github.com/HuangJian/rime-frost/tree/hj/js/tests)

#### 安装和配置

1. 从 release 包中获取 qjs 程序（macOS 下为 qjs，Windows 下为 qjs.exe）
2. 将程序复制到 `<Rime-user-folder>/js` 目录下

#### 测试文件组织

测试文件应遵循以下规范：

1. 测试文件命名：`<plugin-name>.test.js`
2. 测试文件位置：放置在 `<Rime-user-folder>/js/tests` 目录下
3. 测试用例组织：每个测试文件对应一个插件组件

#### 编写测试用例

1. 导入测试工具：
```javascript
import { assert, assertEquals, passedTests, totalTests } from './testutils.js'
```
2. 导入插件模块：
```javascript
import { MyFilter } from '../my_filter.js'
```

3. 编写测试代码：
```javascript
const mockEnv = { namespace: 'my_filter', /* 其他环境参数 */  }
const filter = new MyFilter(mockEnv)
const input = 'hello'
const candidates = [ { text: 'hello' }, { text: 'world' } ]
const result = filter.filter(candidates, input, mockEnv)
assertEquals(result.length, 1, 'should filter out "world"')
```

4. 使用断言函数：
- `assert(condition, message)`: 验证条件是否为真
- `assertEquals(actual, expected, message)`: 验证实际值是否等于期望值

5. 打印测试结果：
```javascript

console.log(`\nTest Summary: ${passedTests}/${totalTests} tests passed`)
```
#### 执行测试

1. 基本测试命令：
```bash
./qjs ./tests/<plugin-name>.test.js
```

2. 测试结果解读：
- ✓ 表示测试通过
- ✗ 表示测试失败，会显示期望值和实际值的差异
- 最后显示测试统计信息：总测试数和通过测试数

### 使用其他 JavaScript 引擎

- 除了 quickjs，您还可以使用 nodejs/bun/deno 等 JavaScript 引擎来执行单元测试，帮助定位问题和检查 ECMAScript 语法特性支持情况。
- 绝大多数 nodejs 特有模块都可以在 bun 和 deno 中使用，无需额外的适配，但 quickjs 均未支持。
- `node:fs` 模块可以使用 quickjs 的 `std` 模块替代，具体使用请参考 [en2cn.benchmark.js](https://github.com/HuangJian/rime-frost/blob/hj/js/tests/benchmark/en2cn.benchmark.js)。

### 类型定义文件

#### 安装

1. 从 release 包中获取 `rime.d.ts` 文件
2. 将文件复制到 `<Rime-user-folder>/js/type` 目录下

#### 配置

1. 在 `<Rime-user-folder>/js` 目录下创建 `jsconfig.json` 或 `tsconfig.json` 文件：
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

#### IDE 集成

1. Visual Studio Code
   - 安装 JavaScript/TypeScript 相关扩展
   - 打开 `<Rime-user-folder>/js` 目录
   - VSCode 会自动识别 `jsconfig.json` 或 `tsconfig.json` 配置
   - 编写代码时会获得类型提示和智能补全

2. WebStorm/IntelliJ IDEA
   - 打开 `<Rime-user-folder>/js` 目录
   - IDE 会自动识别 `jsconfig.json` 或 `tsconfig.json` 配置
   - 编写代码时会获得类型提示和智能补全

#### 常见问题

1. 类型定义文件版本不匹配
   - 确保使用的 `rime.d.ts` 文件来自同一版本的 release 包
   - 如遇到类型错误，请更新 `rime.d.ts` 文件到最新版本

2. IDE 无法识别类型定义
   - 检查 `jsconfig.json` 或 `tsconfig.json` 配置是否正确
   - 确保 `typeRoots` 路径指向正确的类型定义文件目录
   - 重启 IDE 或重新加载项目

## 最佳实践 （待补充）

- 代码组织建议
- 性能优化建议
- 错误处理建议

## 常见问题 （待补充）

- 问题1：...
- 问题2：...
- 问题3：...
