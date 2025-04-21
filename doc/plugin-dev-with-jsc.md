# 运行在 JavaScriptCore 引擎上的 Rime 插件开发

## 目录

- [运行在 JavaScriptCore 引擎上的 Rime 插件开发](#运行在-javascriptcore-引擎上的-rime-插件开发)
  - [目录](#目录)
  - [性能对比 vs QuickJS](#性能对比-vs-quickjs)
    - [内存占用](#内存占用)
    - [运行效率](#运行效率)
  - [JavaScriptCore 引擎介绍](#javascriptcore-引擎介绍)
    - [引擎特性](#引擎特性)
    - [性能优势](#性能优势)
    - [内存管理](#内存管理)
  - [系统兼容性](#系统兼容性)
  - [插件开发](#插件开发)
    - [不支持的 JavaScript 特性](#不支持的-javascript-特性)
  - [脚本打包](#脚本打包)
    - [打包工具](#打包工具)
    - [注意事项](#注意事项)
    - [部署方式](#部署方式)
  - [Rime 配置](#rime-配置)

## 性能对比 vs QuickJS

### 内存占用

macOS系统，Squirrel 前端，加载[示例输入方案](https://github.com/HuangJian/rime-frost)的所有 js 插件，正常码字十分钟后的内存占用情况：

- QuickJS: 50 MB
- JavaScriptCore: 64 MB

### 运行效率

- 计算密集场景 ① - 使用正则表达式提取 2304 个候选项注释中的拼音
  - QuickJS 耗时: 13~14 ms
  - JavaScriptCore 耗时: 3~4 ms (JIT 优化后未见差异)
- 计算密集场景 ② - 解析 116617 行的词典文本，存到 Map 中
  - QuickJS 耗时: 720 ms
  - JavaScriptCore 解析模式耗时: ~130 ms
  - JavaScriptCore JIT 模式耗时: ~105 ms
- 普通输入场景 ① - 依次输入 `xingnengceshi`，执行完毕[示例输入方案](https://github.com/HuangJian/rime-frost)的所有 filter
  - <details>
     <summary>QuickJS 总耗时: 201 ms</summary>
     <pre><blockquote>
        [benchmark] all qjs filters run for  54 ms, with input = x
        [benchmark] all qjs filters run for  15 ms, with input = xi
        [benchmark] all qjs filters run for   7 ms, with input = xin
        [benchmark] all qjs filters run for   7 ms, with input = xing
        [benchmark] all qjs filters run for  21 ms, with input = xingn
        [benchmark] all qjs filters run for   6 ms, with input = xingne
        [benchmark] all qjs filters run for   7 ms, with input = xingnen
        [benchmark] all qjs filters run for   7 ms, with input = xingneng
        [benchmark] all qjs filters run for  21 ms, with input = xingnengc
        [benchmark] all qjs filters run for   7 ms, with input = xingnengce
        [benchmark] all qjs filters run for  21 ms, with input = xingnengces
        [benchmark] all qjs filters run for  21 ms, with input = xingnengcesh
        [benchmark] all qjs filters run for   7 ms, with input = xingnengceshi
     </blockquote></pre>
  - <details>
     <summary>JavaScriptCore JIT 模式耗时: 214 ms</summary>
     <pre><blockquote>
        [benchmark] all jsc filters run for  66 ms, with input = x
        [benchmark] all jsc filters run for  13 ms, with input = xi
        [benchmark] all jsc filters run for   6 ms, with input = xin
        [benchmark] all jsc filters run for   6 ms, with input = xing
        [benchmark] all jsc filters run for  23 ms, with input = xingn
        [benchmark] all jsc filters run for   6 ms, with input = xingne
        [benchmark] all jsc filters run for   6 ms, with input = xingnen
        [benchmark] all jsc filters run for   6 ms, with input = xingneng
        [benchmark] all jsc filters run for  23 ms, with input = xingnengc
        [benchmark] all jsc filters run for   6 ms, with input = xingnengce
        [benchmark] all jsc filters run for  24 ms, with input = xingnengces
        [benchmark] all jsc filters run for  23 ms, with input = xingnengcesh
        [benchmark] all jsc filters run for   6 ms, with input = xingnengceshi
     </blockquote></pre>
- 普通输入场景 ② - 输入 `nl`，执行[农历插件](https://github.com/HuangJian/rime-frost/blob/hj/js/lunar_translator.js)生成农历日期候选项
  - QuickJS 耗时: ~2 ms
  - JavaScriptCore JIT 模式耗时: < 1 ms

## JavaScriptCore 引擎介绍

JavaScriptCore 是 Apple 公司开发的开源 JavaScript 引擎。
作为 WebKit 的一部分，它为 Safari 浏览器和 iOS/macOS 应用程序提供 JavaScript 执行环境。
在 Rime 中集成 JavaScriptCore 引擎，可以为输入法插件提供更高效、更稳定的运行环境。

### 引擎特性

- **原生集成**：作为 macOS 和 iOS 系统的原生 JavaScript 引擎，JavaScriptCore 与系统深度集成，可以直接调用系统 API。
- **现代化特性**：支持最新的 ECMAScript 标准，包括 Unicode、Regex、BigInt 等现代 JavaScript 特性。
- **JIT 编译**：采用即时编译技术，将 JavaScript 代码编译成机器码执行，显著提升运行性能。
- **类型推断**：通过静态分析和运行时监控，优化类型相关的操作，减少动态类型检查开销。

### 性能优势

- **编译优化**：相比 QuickJS 的解释执行，JavaScriptCore 的 JIT 编译可以带来数倍的性能提升。
- **并发执行**：支持多线程并发执行 JavaScript 代码，可以更好地利用多核处理器。
- **资源占用**：在处理大量数据时，内存和 CPU 使用率都保持在较低水平。

### 内存管理

- **垃圾回收**：采用分代垃圾回收机制，通过新生代和老生代的分区管理，提高回收效率。
- **内存池**：使用对象池技术，减少内存分配和释放的开销。
- **内存限制**：提供内存使用限制和监控机制，防止内存泄漏和过度占用。

## 系统兼容性

- **macOS**
  - 原生支持，作为系统内置 JavaScript 引擎
  - 支持 macOS 10.5 (Catalina) 及以上版本
  - 完整支持 JIT 编译和系统级优化

- **iOS**
  - 原生支持，与 macOS 共享相同的引擎架构
  - 支持 iOS 16.0 及以上版本
  - 受系统限制，可能不支持 JIT 编译和系统级优化

- **Linux**
  - 理论上可通过安装 WebKitGTK 开发包获得支持。未验证，欢迎贡献。

- **Windows**
  - 未尝试，欢迎贡献。

- **Android**
  - 未尝试，欢迎贡献。

## 插件开发

与[运行在 QuickJS 引擎上的 Rime 插件](https://github.com/HuangJian/librime-qjs/blob/main/doc/plugin-dev.cn.md)开发方式相同，开发者可以使用 JavaScript 语言编写插件逻辑。

### 不支持的 JavaScript 特性

除下列内容外，JavaScriptCore 几乎完整支持 ECMAScript 2022 标准特性。
- ES Modules：使用 `import` 和 `export` 语句进行模块导入和导出。
  - JavaScriptCore 仅在 Safari 浏览器内支持以 `<script type="module">` 的形式使用该特性。
  - Rime 插件基于该特性导入外部代码。为绕过这一限制，我们通过代码打包的方式，将插件代码和依赖库打包成一个单独的文件。具体请参考[脚本打包](#脚本打包)章节。
- 正则表达式 [Lookbehind assertion](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Regular_expressions/Lookbehind_assertion): `(?<=...)`, `(?<!...)`
  - 报错信息： `SyntaxError: Invalid regular expression: invalid group specifier name`
  - 解决方案参考：用 `/\bpi\b/gi` 替换 `/(?<!\w)pi(?!\w)/gi`
- 待发现

## 脚本打包

### 打包工具

为了处理 JavaScriptCore 引擎对 ES Modules 的限制，我们需要使用打包工具将插件代码及其依赖库打包成单一文件。推荐使用以下打包工具：

- **esbuild**
  - 极快的打包速度：使用 Go 语言编写，比传统打包工具快 10-100 倍
  - 简单的配置：支持零配置使用，也可通过 JavaScript/TypeScript 配置
  - 内置优化：自动进行 tree-shaking、代码压缩和 source map 生成
  - 配置示例：[bundle.js](../tests/js/bundle.js)
  - 打包命令：`npx esbuild --bundle ./plugin.js --outfile=./dist/plugin.dist.js`

- **bun**
  - 内置打包功能：作为 JavaScript 运行时，自带高性能打包器
  - 原生 TypeScript 支持：无需额外配置即可打包 TypeScript 代码
  - 零配置打包：自动识别入口文件和依赖关系
  - 配置示例：未尝试，欢迎贡献
  - 打包命令：`bun build ./plugin.ts --outfile ./dist/plugin.dist.js`

### 注意事项

- **插件实例化代码注入**
  - 为了避免不同插件加载到 JavaScriptCore 引擎后出现变量名冲突，我们需要把 js 代码打包为 [IIFE 格式](https://esbuild.github.io/api/#format-iife)。
  - 以 IIFE 格式打包后，插件定义类不再是全局变量。我们需要在打包过程中注入一行插件实例化代码。
  - 为了避免实例化代码注入失败，插件源码中请勿定义多个导出类 (`export class XXX`)。
  - 为了避免实例化代码注入失败，插件源码中请勿使用 globalThis 全局变量。

- **打包选项**
  - 代码混淆：该选项将修改变量名，使 librime-qjs 无法正确加载插件。不建议启用。
  - Tree-shaking：该选项将移除未使用的代码，减小文件体积，但可能影响插件的正常运行。建议启用。

- **依赖库**
  - 确保插件依赖的库在打包时也被包含，包括通过 npm 安装的依赖
  - 避免使用 `eval`：使用 `Function` 构造函数创建函数
  - 避免使用 `require`：使用 `import` 语句导入依赖库

- **代码美化**
  - 为便于定位问题，建议对打包后的代码进行格式化
  - 推荐工具：[prettier](https://prettier.io/)

- **配置示例**
  - [bundle.js](../tests/js/bundle.js)
  - [.prettierrc](../tests/js/.prettierrc)

### 部署方式

打包后的插件文件需要按照以下规范部署：

1. **文件位置**
   - macOS: `~/Library/Rime/js/dist/<plugin-name>.dist.js`

2. **目录结构**
    ```pre
    <Rime-user-folder>                         # Rime 前端程序的用户文件夹，如 Squirrel 输入法默认的 `~/Library/Rime` 目录
    ├─── build                                 # Rime 核心引擎的编译输出目录，可用于查看实际启用的输入法选项和插件信息
    ├─── js                                    # JavaScript 插件目录
    │    ├─── package.json                     # 插件包配置文件，用于管理依赖
    │    ├─── node_modules                     # 依赖库
    │    ├─── <your-plugin>.js                 # JavaScript 插件源代码
    │    ├─── dist                             # JavaScript 插件打包发布目录
    │    │     └─── <your-plugin>.iife.js      # 打包后的 JavaScript 插件文件
    │    ├─── bundle.js                        # 打包脚本
    │    └─── .pretierrc                       # 代码美化配置文件
    └─── ...
    ```

## Rime 配置

```yaml
patch:
  'engine/processors/@before 1':    # 添加自定义的 jsc processors。"@before 1"表示把处理器插件置于 Rime 默认处理器前，以保证插件生效。
    jsc_processor@your-plugin       # 示例处理器，@ 后面的部分为插件文件名（不是 dist 目录的打包文件名，也不要包含扩展名）。
  engine/translators/+:             # 添加自定义的 jsc translator
    - jsc_translator@your-plugin    # 示例转换器，@ 后面的部分为插件文件名（不是 dist 目录的打包文件名，也不要包含扩展名）。
  engine/filters/+:                 # 添加自定义的 jsc filter
    - jsc_filter@your-plugin        # 示例过滤器，@ 后面的部分为插件文件名（不是 dist 目录的打包文件名，也不要包含扩展名）。
```

\* 在未支持 JavaScriptCore 引擎的系统上，libRime-qjs 会将上述插件配置自动回退到 QuickJS 引擎。
