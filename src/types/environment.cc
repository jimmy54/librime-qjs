#include "environment.h"

#include <glog/logging.h>
#include "process_memory.hpp"

#ifdef _WIN32
static FILE* popenx(const std::string& command) {
  return _popen(command.c_str(), "r");
}

static int pclosex(FILE* pipe) {
  return _pclose(pipe);
}
#else
static FILE* popenx(const std::string& command) {
  return ::popen(command.c_str(), "r");
}

static int pclosex(FILE* pipe) {
  return ::pclose(pipe);
}
#endif

std::string Environment::formatMemoryUsage(size_t usage) {
  constexpr size_t KILOBYTE = 1024;
  return usage > KILOBYTE * KILOBYTE ? std::to_string(usage / KILOBYTE / KILOBYTE) + "M"
                                     : std::to_string(usage / KILOBYTE) + "K";
}

std::string Environment::loadFile(const std::string& path) {
  if (path.empty()) {
    return "";
  }

  FILE* file = fopen(path.c_str(), "rb");
  if (file == nullptr) {
    return "";
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  std::string content;
  content.resize(size);
  fread(content.data(), 1, size, file);
  fclose(file);

  return content;
}

bool Environment::fileExists(const std::string& path) {
  return std::filesystem::exists(path);
}

std::string Environment::getRimeInfo() {
  size_t vmUsage = 0;
  size_t residentSet = 0;  // memory usage in bytes
  getMemoryUsage(vmUsage, residentSet);

  std::stringstream ss{};
  ss << "libRime v" << rime_get_api()->get_version() << " | "
     << "libRime-qjs v" << RIME_QJS_VERSION << " | "
     << "Process RSS Mem: " << formatMemoryUsage(residentSet);

  return ss.str();
}

std::string Environment::popen(const std::string& command) {
  if (command.empty()) {
    return "";
  }

  FILE* pipe = popenx(command);
  if (pipe == nullptr) {
    LOG(ERROR) << "Failed to run command: " << command;
    return "";
  }

  // Read the output
  constexpr size_t READ_BUFFER_SIZE = 128;
  char buffer[READ_BUFFER_SIZE];
  char* ptrBuffer = static_cast<char*>(buffer);
  std::string result;
  while (fgets(ptrBuffer, sizeof(buffer), pipe) != nullptr) {
    result += ptrBuffer;
  }

  int status = pclosex(pipe);
  if (status != 0) {
    LOG(ERROR) << "[qjs] Command failed with status: " << status;
    return "";
  }

  return result;
}
