#pragma once

#include <cstdio>
#include <fstream>

#define PRINT_DURATION(color, title, statements) \
{                                                                              \
    auto start = std::chrono::high_resolution_clock::now();                    \
                                                                               \
    statements;                                                                \
                                                                               \
    auto end = std::chrono::high_resolution_clock::now();                      \
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>      \
        (end - start);                                                         \
                                                                               \
    std::cout << color                                                         \
              << title                                                         \
              << duration.count()                                              \
              << " ms"                                                         \
              << RESET                                                         \
                << std::endl;                                                  \
}

#define RESAVE_FILE(path, statements) \
{                                                                              \
    std::filesystem::remove(path);                                             \
    std::ifstream file(path);                                                  \
    if (!file.good()) {                                                        \
        statements;                                                            \
    }                                                                          \
}

// Foreground colors
constexpr const char* RESET     = "\033[0m";
constexpr const char* BLACK     = "\033[30m";
constexpr const char* RED       = "\033[31m";
constexpr const char* GREEN     = "\033[32m";
constexpr const char* YELLOW    = "\033[33m";
constexpr const char* BLUE      = "\033[34m";
constexpr const char* MAGENTA   = "\033[35m";
constexpr const char* CYAN      = "\033[36m";
constexpr const char* WHITE     = "\033[37m";
