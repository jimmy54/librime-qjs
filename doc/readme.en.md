English | [中文](../readme.md) | [Plugin Development Guide](./plugin-dev.en.md) | [插件开发指南](./plugin-dev.cn.md)

# librime-qjs

Experience a vast JavaScript plugin ecosystem for the Rime Input Method Engine, delivering lightning-fast speed and feather-light performance for a revolutionary input experience!

## Features

- 🔌 Powerful JavaScript plugin ecosystem for the [Rime Input Method Engine](https://github.com/rime/librime).
  - 🎮 Unleash the full potential of JavaScript with all essential Rime engine features at your fingertips.
  - ✨ Tired of writing code and debugging? The NPM repository has everything you need, all in one place.
  - 👀 See our capabilities in action! All Lua plugins from [Rime Frost](https://github.com/gaboolic/rime-frost) have been perfectly rewritten in [JavaScript](https://github.com/HuangJian/rime-frost/tree/hj/js).
  - 📝 Smooth plugin development with comprehensive [JavaScript type definitions](./contrib/rime.d.ts).
  - 🔄 Simple and flexible [type binding templates](./src/helpers/qjs_macros.h) for seamless JavaScript and Rime engine integration.
- 🚀 Lightweight JavaScript engine powered by [QuickJS-NG](https://github.com/quickjs-ng/quickjs).
  - 💪 Enjoy the latest ECMAScript features: regular expressions, Unicode, ESM, big numbers, and more!
  - 🚄 Blazing-fast performance: all plugins respond within milliseconds.
  - 🪶 Incredibly small memory footprint: <20MB!
- 📚 Custom-built Trie structure for large dictionaries.
  - 💥 Lightning-fast dictionary loading: 110,000-entry [Chinese-English dictionary](https://www.mdbg.net/chinese/dictionary?page=cc-cedict) loads in just 20ms after binary conversion.
  - 🎯 Swift exact lookups: finding English definitions for 200 Chinese words in under 5ms.
  - 🌪️ Rapid prefix search: searching English words with Chinese translations in a 60,000-entry [English-Chinese dictionary](https://github.com/skywind3000/ECDICT) takes only 1-3ms.
- 🗡️ Share JavaScript plugins across all Rime sessions for seamless transitions.
  - 🎉 No more lag when switching input methods with large plugins - solved once and for all!
  - 🚀 Ready for immersive writing across different applications!
- 🛡️ Comprehensive testing with both C++ and JavaScript.
  - ✅ Every Rime API thoroughly tested with [C++ tests](./tests/).
  - 🧪 JavaScript plugins? Test freely with qjs/nodejs/bun/deno using our [test suite](https://github.com/HuangJian/rime-frost/tree/hj/js/tests).

## TODO

- [ ] More JavaScript Engine Support (PR Welcome)
  - [ ] JavaScriptCore on macOS/iOS
    - Built-in on macOS/iOS systems
    - Most performant JavaScript engine
  - [ ] V8 engine support via nodejs/deno

## [JavaScript Plugin Development Guild](./plugin-dev.en.md)

## libRime-qjs Development

- [build on macOS](./build-macos.md)
- [build on Linux](./build-linux.md)
