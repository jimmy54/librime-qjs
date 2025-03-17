#include "qjs_helper.h"

#include <glog/logging.h>

#include <cstddef>
#include <fstream>
#include <sstream>

#include "jsvalue_raii.h"
#include "quickjs.h"

using namespace rime;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::string QjsHelper::baseFolder;

QjsHelper& QjsHelper::getInstance() {
  static QjsHelper instance;
  return instance;
}

std::ifstream QjsHelper::tryLoadFileToStream(const std::string& path) {
  const std::vector<std::string> possibleExtensions = {"", ".js", ".mjs", ".cjs"};
  for (const auto& ext : possibleExtensions) {
    auto stream = std::ifstream(path + ext);
    if (stream.good()) {
      return stream;
    }
  }
  return std::ifstream{nullptr};
}

std::string QjsHelper::tryFindNodeModuleEntryFileName(const std::string& folder,
                                                      const std::string& key) {
  if (auto packageJson = std::ifstream(folder + "/package.json")) {
    std::string entryFileName;

    std::string line;
    while (std::getline(packageJson, line)) {
      auto pos = line.find(key);
      if (pos == std::string::npos) {
        continue;
      }
      // format: `"module": "lib/index.js",` or `"module": "lib/index.js"`
      auto leftQuote = line.find('\"', pos + key.size());
      auto rightQuote = line.find('\"', leftQuote + 1);
      entryFileName = line.substr(leftQuote + 1, rightQuote - leftQuote - 1);

      LOG(INFO) << "Found entry file: [" << entryFileName << "] from package.json line: " << line;
      break;
    }

    auto stream = tryLoadFileToStream(folder + "/" + entryFileName);
    if (stream.good()) {
      return entryFileName;
    }
  }
  return "";
}

std::string QjsHelper::tryFindNodeModuleEntryPath(const std::string& moduleName) {
  std::string folder = QjsHelper::baseFolder + "/node_modules/" + moduleName;

  // format: `"module": "lib/index.js",` or `"module": "lib/index.js"`
  std::string entryFileName = tryFindNodeModuleEntryFileName(folder, "\"module\":");
  if (entryFileName.empty()) {
    // format: `"main": "main.js",` or `"main": "main.js"`
    entryFileName = tryFindNodeModuleEntryFileName(folder, "\"main\":");
  }
  if (entryFileName.empty()) {
    LOG(ERROR) << "Failed to find the entry file of the node module: " << moduleName;
    return "";
  }

  return entryFileName;
}

JSModuleDef* QjsHelper::jsModuleLoader(JSContext* ctx, const char* moduleName, void* opaque) {
  LOG(INFO) << "Loading js module: " << moduleName;
  std::string relativePath = moduleName;
  if (relativePath.empty()) {
    LOG(ERROR) << "fileName is empty in loading js module";
    return nullptr;
  }

  // 1. try to load the file relative to baseFolder
  auto stream = tryLoadFileToStream(QjsHelper::baseFolder + "/" + relativePath);
  if (!stream.good()) {
    // 2. try to load the file from baseFolder/node_modules/moduleName
    auto nodeModuleEntryFile = tryFindNodeModuleEntryPath(relativePath);
    if (nodeModuleEntryFile.empty()) {
      relativePath = "";
    } else {
      relativePath = "/node_modules/" + relativePath + "/" + nodeModuleEntryFile;
    }
  }

  if (!relativePath.empty()) {
    JSValue funcObj = loadJsModule(ctx, relativePath.c_str());
    return reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcObj));
  }

  LOG(ERROR) << "Failed to load the js module: " << moduleName;
  return nullptr;
}

JSValue QjsHelper::loadJsModuleToNamespace(JSContext* ctx, const char* moduleName) {
  JSValue funcObj = loadJsModule(ctx, moduleName);
  if (JS_IsException(funcObj)) {
    return funcObj;
  }

  auto* md = reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcObj));
  JSValue evalResult = JS_EvalFunction(ctx, funcObj);
  if (JS_IsException(funcObj)) {
    return evalResult;
  }

  JS_FreeValue(ctx, evalResult);
  return JS_GetModuleNamespace(ctx, md);
}

JSValue QjsHelper::loadJsModuleToGlobalThis(JSContext* ctx, const char* moduleName) {
  std::string jsCode = readJsCode(ctx, moduleName);
  return JS_Eval(ctx, jsCode.c_str(), jsCode.size(), moduleName, JS_EVAL_TYPE_MODULE);
}

void QjsHelper::exposeLogToJsConsole(JSContext* ctx) {
  JSValue globalObj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, jsLog, "log", 1));
  JS_SetPropertyStr(ctx, console, "error", JS_NewCFunction(ctx, jsError, "error", 1));
  JS_SetPropertyStr(ctx, globalObj, "console", console);
  JS_FreeValue(ctx, globalObj);
}

std::string QjsHelper::loadFile(const char* absolutePath) {
  std::ifstream stream = tryLoadFileToStream(absolutePath);
  if (!stream.is_open()) {
    LOG(ERROR) << "Failed to open file at: " << absolutePath;
    return "";
  }
  std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
  return content;
}

std::string QjsHelper::readJsCode(JSContext* ctx, const char* relativePath) {
  if (QjsHelper::baseFolder.empty()) {
    LOG(ERROR) << "basePath is empty in loading js file: " << relativePath;
    JS_ThrowReferenceError(ctx, "basePath is empty in loading js file: %s", relativePath);
    return "";
  }
  std::string fullPath = QjsHelper::baseFolder + "/" + relativePath;
  return loadFile(fullPath.c_str());
}

JSValue QjsHelper::loadJsModule(JSContext* ctx, const char* fileName) {
  std::string code = readJsCode(ctx, fileName);
  if (code.empty()) {
    return JS_ThrowReferenceError(ctx, "Could not open %s", fileName);
  }

  int flags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
  JSValue funcObj = JS_Eval(ctx, code.c_str(), code.size(), fileName, flags);

  if (JS_IsException(funcObj)) {
    JSValue exception = JS_GetException(ctx);
    JSValue message = JS_GetPropertyStr(ctx, exception, "message");
    const char* messageStr = JS_ToCString(ctx, message);
    LOG(ERROR) << "Module evaluation failed: " << messageStr;

    JS_FreeCString(ctx, messageStr);
    JS_FreeValue(ctx, message);
    JS_FreeValue(ctx, exception);
  }
  return funcObj;
}

static std::string logToStringStream(JSContext* ctx, int argc, JSValueConst* argv) {
  std::ostringstream oss;
  for (int i = 0; i < argc; i++) {
    const char* str = JS_ToCString(ctx, argv[i]);
    if (str != nullptr) {
      oss << (i != 0 ? " " : "") << str;
      JS_FreeCString(ctx, str);
    }
  }
  return oss.str();
}

JSValue QjsHelper::jsLog(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  LOG(INFO) << "$qjs$ " << logToStringStream(ctx, argc, argv);
  return JS_UNDEFINED;
}

JSValue QjsHelper::jsError(JSContext* ctx, JSValueConst thisVal, int argc, JSValueConst* argv) {
  LOG(ERROR) << "$qjs$ " << logToStringStream(ctx, argc, argv);
  return JS_UNDEFINED;
}

JSValue QjsHelper::getMethodByNameInClass(JSContext* ctx,
                                          JSValue classObj,
                                          const char* methodName) {
  JSValueRAII proto = JS_GetPropertyStr(ctx, classObj, "prototype");
  if (JS_IsException(proto)) {
    return JS_UNDEFINED;
  }

  JSValue method = JS_GetPropertyStr(ctx, proto, methodName);
  if (JS_IsException(method) || !JS_IsFunction(ctx, method)) {
    JS_FreeValue(ctx, method);
    return JS_UNDEFINED;
  }

  return method;
}

JSValue QjsHelper::getExportedClassByNameInModule(JSContext* ctx,
                                                  JSValue moduleObj,
                                                  const char* className) {
  JSPropertyEnum* props = nullptr;
  uint32_t propCount = 0;  // Get all enumerable properties from namespace
  int flags = JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY;
  if (JS_GetOwnPropertyNames(ctx, &props, &propCount, moduleObj, flags) == 0) {
    size_t n = strlen(className);
    for (uint32_t i = 0; i < propCount; i++) {
      JSValue propVal = JS_GetProperty(ctx, moduleObj, props[i].atom);
      const char* propName = JS_AtomToCString(ctx, props[i].atom);
      bool isMatched = !JS_IsException(propVal) && strncmp(propName, className, n) == 0;

      JS_FreeCString(ctx, propName);
      JS_FreeAtom(ctx, props[i].atom);

      if (isMatched) {
        js_free(ctx, props);
        return propVal;
      }

      JS_FreeValue(ctx, propVal);
    }
  }

  js_free(ctx, props);
  return JS_UNDEFINED;
}

JSValue QjsHelper::getExportedClassHavingMethodNameInModule(JSContext* ctx,
                                                            JSValue moduleObj,
                                                            const char* methodName) {
  JSPropertyEnum* props = nullptr;
  uint32_t propCount = 0;  // Get all enumerable properties from namespace
  int flags = JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY;
  if (JS_GetOwnPropertyNames(ctx, &props, &propCount, moduleObj, flags) == 0) {
    for (uint32_t i = 0; i < propCount; i++) {
      JSValue propVal = JS_GetProperty(ctx, moduleObj, props[i].atom);
      const char* propName = JS_AtomToCString(ctx, props[i].atom);

      bool found = false;
      if (JS_IsObject(propVal)) {
        JSValueRAII method = getMethodByNameInClass(ctx, propVal, methodName);
        found = !JS_IsException(method) && !JS_IsUndefined(method) && JS_IsFunction(ctx, method);
      }

      JS_FreeCString(ctx, propName);
      JS_FreeAtom(ctx, props[i].atom);

      if (found) {
        js_free(ctx, props);
        return propVal;
      }

      JS_FreeValue(ctx, propVal);
    }
  }

  js_free(ctx, props);
  return JS_UNDEFINED;
}
