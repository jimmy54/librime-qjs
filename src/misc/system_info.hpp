#include <string>

#ifdef _WIN32
#include <VersionHelpers.h>
#include <Windows.h>
#elif defined(__linux__)
#include <sys/utsname.h>
#include <fstream>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#endif

class SystemInfo {
public:
  static std::string getOSName() {
#ifdef _WIN32
    return "Windows";
#elif defined(__linux__)
    return detectLinuxDistro() ? "Linux" : "Unknown Linux";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Unknown OS";
#endif
  }

  static std::string getOSVersion() {
#ifdef _WIN32
    return getWindowsVersion();
#elif defined(__linux__)
    return getLinuxVersion();
#elif defined(__APPLE__)
    return getMacOSVersion();
#else
    return "Unknown";
#endif
  }

  static std::string getArchitecture() {
#ifdef _WIN32
    return getWindowsArchitecture();
#elif defined(__linux__) || defined(__APPLE__)
    struct utsname sysInfo{};
    uname(&sysInfo);
    return static_cast<char*>(sysInfo.machine);
#else
    return "Unknown";
#endif
  }

private:
#ifdef _WIN32
  static std::string getWindowsVersion() {
    RTL_OSVERSIONINFOW osvi = {sizeof(osvi)};
    if (RtlGetVersion(&osvi) == 0) {
      return std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion) + "." +
             std::to_string(osvi.dwBuildNumber);
    }
    return "Unknown";
  }

  static std::string getWindowsArchitecture() {
    BOOL is64Bit = FALSE;
    IsWow64Process(GetCurrentProcess(), &is64Bit);

    if (is64Bit) {
      return "x64";
    } else {
      SYSTEM_INFO si;
      GetNativeSystemInfo(&si);
      switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
          return "x64";
        case PROCESSOR_ARCHITECTURE_ARM:
          return "ARM";
        case PROCESSOR_ARCHITECTURE_IA64:
          return "IA64";
        case PROCESSOR_ARCHITECTURE_INTEL:
          return "x86";
        default:
          return "Unknown";
      }
    }
  }
#endif

#ifdef __linux__
  static bool detectLinuxDistro() {
    return checkOsRelease() || checkLsbRelease() || checkRedHatRelease() || checkDebianVersion() ||
           checkSuseRelease() || checkArchRelease();
  }

  static std::string getLinuxVersion() {
    if (checkOsRelease()) {
      return readOsRelease();
    }
    if (checkLsbRelease()) {
      return readLsbRelease();
    }
    if (checkRedHatRelease()) {
      return readRedHatRelease();
    }
    if (checkDebianVersion()) {
      return readDebianVersion();
    }
    if (checkSuseRelease()) {
      return readSuseRelease();
    }
    if (checkArchRelease()) {
      return readArchRelease();
    }
    return "Unknown";
  }

  static bool checkOsRelease() { return std::ifstream("/etc/os-release").good(); }

  static std::string readOsRelease() {
    std::ifstream file("/etc/os-release");
    std::string line;
    std::string prettyName;

    while (std::getline(file, line)) {
      constexpr const char* KEY = "PRETTY_NAME=";
      if (line.find(KEY) == 0) {
        prettyName = line.substr(strlen(KEY));
        trimQuotes(prettyName);
        return prettyName;
      }
    }
    return "Unknown";
  }

  static bool checkLsbRelease() { return std::ifstream("/etc/lsb-release").good(); }

  static std::string readLsbRelease() {
    std::ifstream file("/etc/lsb-release");
    std::string line;

    while (std::getline(file, line)) {
      constexpr const char* KEY = "DISTRIB_DESCRIPTION=";
      if (line.find(KEY) == 0) {
        std::string desc = line.substr(strlen(KEY));
        trimQuotes(desc);
        return desc;
      }
    }
    return "Unknown";
  }

  static bool checkRedHatRelease() { return std::ifstream("/etc/redhat-release").good(); }

  static std::string readRedHatRelease() {
    std::ifstream file("/etc/redhat-release");
    std::string line;
    if (std::getline(file, line)) {
      return line;
    }
    return "Unknown";
  }

  static bool checkDebianVersion() { return std::ifstream("/etc/debian_version").good(); }

  static std::string readDebianVersion() {
    std::ifstream file("/etc/debian_version");
    std::string version;
    if (std::getline(file, version)) {
      return "Debian " + version;
    }
    return "Unknown";
  }

  static bool checkSuseRelease() { return std::ifstream("/etc/SuSE-release").good(); }

  static std::string readSuseRelease() {
    std::ifstream file("/etc/SuSE-release");
    std::string line;
    while (std::getline(file, line)) {
      constexpr const char* KEY = "PRETTY_NAME=";
      if (line.find(KEY) == 0) {
        std::string name = line.substr(strlen(KEY));
        trimQuotes(name);
        return name;
      }
    }
    return "Unknown";
  }

  static bool checkArchRelease() { return std::filesystem::exists("/etc/arch-release"); }

  static std::string readArchRelease() { return "Arch Linux"; }

  static void trimQuotes(std::string& str) {
    if (!str.empty() && str.front() == '"') {
      str.erase(0, 1);
    }
    if (!str.empty() && str.back() == '"') {
      str.pop_back();
    }
  }
#endif

#ifdef __APPLE__
  static std::string getMacOSVersion() {
    constexpr const int STR_SIZE = 256;
    char str[STR_SIZE];
    size_t size = sizeof(str);
    char* ptr = static_cast<char*>(str);
    if (sysctlbyname("kern.osproductversion", ptr, &size, nullptr, 0) == 0) {
      return ptr;
    }
    return "Unknown";
  }
#endif
};
