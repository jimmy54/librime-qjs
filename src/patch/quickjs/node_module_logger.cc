#include <glog/logging.h>

extern "C" {
void logInfoImpl(const char* message) {
  LOG(INFO) << message << "\n";
}

void logErrorImpl(const char* message) {
  LOG(ERROR) << message << "\n";
}
}
