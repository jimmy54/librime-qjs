#pragma once

#define PRINT_DURATION(color, title, statements)                                        \
  {                                                                                     \
    auto start = std::chrono::high_resolution_clock::now();                             \
                                                                                        \
    statements;                                                                         \
                                                                                        \
    auto end = std::chrono::high_resolution_clock::now();                               \
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); \
                                                                                        \
    std::cout << (color) << (title) << duration.count() << " ms" << RESET << '\n';      \
  }

#define RESAVE_FILE(path, statements) \
  {                                   \
    std::filesystem::remove(path);    \
    std::ifstream file(path);         \
    if (!file.good()) {               \
      statements;                     \
    }                                 \
  }

// Foreground colors
static const char* const RESET = "\033[0m";
static const char* const BLACK = "\033[30m";
static const char* const RED = "\033[31m";
static const char* const GREEN = "\033[32m";
static const char* const YELLOW = "\033[33m";
static const char* const BLUE = "\033[34m";
static const char* const MAGENTA = "\033[35m";
static const char* const CYAN = "\033[36m";
static const char* const WHITE = "\033[37m";
