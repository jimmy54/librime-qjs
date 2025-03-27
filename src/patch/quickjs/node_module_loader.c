#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node_module_loader.h"

enum { PATH_MAX = 1024 };

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static char* qjsBaseFolder = NULL;

void setQjsBaseFolder(const char* path) {
  if (qjsBaseFolder) {
    free(qjsBaseFolder);
  }
  qjsBaseFolder = path ? strdup(path) : NULL;
}

#ifdef _WIN32
#include <windows.h>
__attribute__((constructor)) void initBaseFolder() {
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  char* last_slash = strrchr(path, '\\');
  if (last_slash) {
    *last_slash = '\0';
    setQjsBaseFolder(path);
  }
}
#endif

#ifdef __APPLE__
#include <unistd.h>
#include <limits.h>
#include <mach-o/dyld.h>  // For _NSGetExecutablePath on macOS

__attribute__((constructor)) void initBaseFolder() {
  char path[PATH_MAX];
  uint32_t size = PATH_MAX;
  // macOS specific path retrieval
  if (_NSGetExecutablePath(path, &size) == 0) {
    char* lastSlash = strrchr(path, '/');
    if (lastSlash) {
      *lastSlash = '\0';
      setQjsBaseFolder(path);
    }
  }
}
#endif

#ifdef __linux__
#include <unistd.h>
#include <limits.h>

__attribute__((constructor)) void initBaseFolder() {
  char path[PATH_MAX];
  uint32_t size = PATH_MAX;
  // Linux path retrieval
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  if (count != -1) {
    path[count] = '\0';  // Ensure null termination
    char* lastSlash = strrchr(path, '/');
    if (lastSlash) {
      *lastSlash = '\0';
      setQjsBaseFolder(path);
    }
  }
}
#endif

#ifdef BUILD_FOR_QJS_EXE
void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
}

void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fwrite("\n", 1, 1, stderr);
}
#else

// Use extern "C" to properly interface with C++ glog library
#ifdef __cplusplus
#include <glog/logging.h>
#else
// For C code, declare external logging functions that will be implemented in C++
extern void logInfoImpl(const char* message);
extern void logErrorImpl(const char* message);
#endif

void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[PATH_MAX];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

#ifdef __cplusplus
  LOG(INFO) << buffer << "\n";
#else
  logInfoImpl(buffer);
#endif
}

void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[PATH_MAX];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

#ifdef __cplusplus
  LOG(ERROR) << buffer << "\n";
#else
  logErrorImpl(buffer);
#endif
}

#endif

FILE* tryLoadFile(const char* path) {
  const char* possibleExtensions[] = {"", ".js", ".mjs", ".cjs"};
  const int numExtensions =
      sizeof(possibleExtensions) / sizeof(possibleExtensions[0]);

  char fullPath[PATH_MAX];
  FILE* file = NULL;

  for (int i = 0; i < numExtensions; i++) {
    snprintf(fullPath, sizeof(fullPath), "%s%s", path, possibleExtensions[i]);
    file = fopen(fullPath, "rb");
    if (file != NULL) {
      return file;
    }
  }

  return NULL;
}

char* tryFindNodeModuleEntryFileName(const char* folder, const char* key) {
  char packageJsonPath[PATH_MAX];
  snprintf(packageJsonPath, sizeof(packageJsonPath), "%s/package.json", folder);

  FILE* packageJson = fopen(packageJsonPath, "r");
  if (!packageJson) {
    return NULL;
  }

  char* entryFileName = NULL;
  char line[PATH_MAX];

  while (fgets(line, sizeof(line), packageJson)) {
    char* pos = strstr(line, key);
    if (!pos) {
      continue;
    }

    char* leftQuote = strchr(pos + strlen(key), '\"');
    if (!leftQuote) {
      continue;
    }

    char* rightQuote = strchr(leftQuote + 1, '\"');
    if (!rightQuote) {
      continue;
    }

    size_t len = rightQuote - (leftQuote + 1);
    entryFileName = (char*)malloc(len + 1);
    strncpy(entryFileName, leftQuote + 1, len);
    entryFileName[len] = '\0';

    logInfo("Found entry file: [%s] from package.json line: %s", entryFileName,
            line);
    break;
  }

  fclose(packageJson);

  char entryFilePath[PATH_MAX];
  snprintf(entryFilePath, sizeof(entryFilePath), "%s/%s", folder,
           entryFileName);
  FILE* file = tryLoadFile(entryFilePath);
  if (file) {
    fclose(file);
    return entryFileName;
  }

  return NULL;
}

char* tryFindNodeModuleEntryPath(const char* baseFolder,
                                 const char* moduleName) {
  char folder[PATH_MAX];
  snprintf(folder, sizeof(folder), "%s/node_modules/%s", baseFolder,
           moduleName);

  char* entryFileName = tryFindNodeModuleEntryFileName(folder, "\"module\":");
  if (!entryFileName) {
    entryFileName = tryFindNodeModuleEntryFileName(folder, "\"main\":");
  }

  if (!entryFileName) {
    logError("Failed to find the entry file of the node module: %s",
             moduleName);
    return NULL;
  }

  return entryFileName;
}

char* loadFile(const char* absolutePath) {
  FILE* file = tryLoadFile(absolutePath);
  if (!file) {
    logError("Failed to open file at: %s", absolutePath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (length <= 0) {
    logError("Invalid file length: %ld for file: %s", length, absolutePath);
    fclose(file);
    return NULL;
  }

  char* content = (char*)malloc(length + 1);
  if (!content) {
    logError("Failed to allocate memory for file: %s", absolutePath);
    fclose(file);
    return NULL;
  }

  size_t read = fread(content, 1, length, file);
  if (read != (size_t)length) {
    logError("Failed to read file: %s, expected %ld bytes but got %zu",
             absolutePath, length, read);
    free(content);
    fclose(file);
    return NULL;
  }

  content[length] = '\0';
  fclose(file);
  return content;
}

char* readJsCode(JSContext* ctx, const char* relativePath) {
  if (!qjsBaseFolder || strlen(qjsBaseFolder) == 0) {
    logError("basePath is empty in loading js file: %s", relativePath);
    JS_ThrowReferenceError(ctx, "basePath is empty in loading js file: %s",
                           relativePath);
    return NULL;
  }

  char fullPath[PATH_MAX];
  snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolder, relativePath);

  return loadFile(fullPath);
}

JSValue loadJsModule(JSContext* ctx, const char* fileName) {
  char* code = readJsCode(ctx, fileName);
  if (!code) {
    return JS_ThrowReferenceError(ctx, "Could not open %s", fileName);
  }

  int flags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
  JSValue funcObj = JS_Eval(ctx, code, strlen(code), fileName, flags);
  free(code);

  if (JS_IsException(funcObj)) {
    JSValue exception = JS_GetException(ctx);
    JSValue message = JS_GetPropertyStr(ctx, exception, "message");
    const char* messageStr = JS_ToCString(ctx, message);
    logError("Module evaluation failed: %s", messageStr);

    JS_FreeCString(ctx, messageStr);
    JS_FreeValue(ctx, message);
    JS_FreeValue(ctx, exception);
  }
  return funcObj;
}

// NOLINTNEXTLINE(readability-identifier-naming)
JSModuleDef* js_module_loader(JSContext* ctx,
                              const char* moduleName,
                              void* opaque) {
  logInfo("Loading js module: %s", moduleName);
  char fullPath[PATH_MAX];
  snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolder, moduleName);

  FILE* file = tryLoadFile(fullPath);
  if (!file) {
    char* nodeModuleEntryFile =
        tryFindNodeModuleEntryPath(qjsBaseFolder, moduleName);
    if (!nodeModuleEntryFile) {
      logError("Failed to load the js module: %s", moduleName);
      return NULL;
    }

    char modulePath[PATH_MAX];
    snprintf(modulePath, sizeof(modulePath), "/node_modules/%s/%s", moduleName,
             nodeModuleEntryFile);
    free(nodeModuleEntryFile);

    JSValue funcObj = loadJsModule(ctx, modulePath);
    return (JSModuleDef*)JS_VALUE_GET_PTR(funcObj);
  }
  fclose(file);

  JSValue funcObj = loadJsModule(ctx, moduleName);
  return (JSModuleDef*)JS_VALUE_GET_PTR(funcObj);
}
