#include "qjs_filter.h"
#include "qjs_translation.h"

#include <fstream>
#include <rime/translation.h>
#include <rime/gear/filter_commons.h>

namespace rime {

QuickJSFilter::QuickJSFilter(const Ticket& ticket,
                             JSContext* ctx,
                             const std::string& jsDirectory)
    : Filter(ticket), TagMatching(ticket), ctx_(ctx) {

  // filters:
  //   - qjs_filter@abc#doFilter
  // => klass = qjs_filter, name_space = abc#doFilter
  if (ticket.name_space == "filter") {
    name_space_ = "filter";
  }

  string filename = ticket.name_space;
  size_t pos = ticket.name_space.find('#');
  if (pos != string::npos) {
    filename = ticket.name_space.substr(0, pos);
    jsFunctionName_ = ticket.name_space.substr(pos + 1);
  } else {
    jsFunctionName_ = "doFilter";
  }

  string scriptPath = jsDirectory + "/" + filename + ".js";
  isLoaded_ = LoadScript(scriptPath);
}

QuickJSFilter::~QuickJSFilter() {
  LOG(INFO) << "[qjs] QuickJSFilter::~QuickJSFilter";
}

bool QuickJSFilter::LoadScript(const string& scriptPath) {
  std::ifstream jsFile(scriptPath);
  if (!jsFile.is_open()) {
    LOG(ERROR) << "[qjs] QuickJSFilter::LoadScript Could not open " << scriptPath;
    return false;
  }

  std::string jsCode((std::istreambuf_iterator<char>(jsFile)),
                      std::istreambuf_iterator<char>());
  JSValueRAII result(JS_Eval(ctx_, jsCode.data(), jsCode.size(), "<input>", JS_EVAL_TYPE_GLOBAL));
  if (JS_IsException(result)) {
    LOG(ERROR) << "[qjs] QuickJSFilter::LoadScript Failed to eval the js code of " << scriptPath;
    return false;
  }
  return true;
}


bool QuickJSFilter::AppliesToSegment(Segment* segment) {
  return TagsMatch(segment);
}

an<Translation> QuickJSFilter::Apply(an<Translation> translation,
                                     CandidateList* candidates) {
  if (!isLoaded_) {
    return translation;
  }

  return New<QuickJSTranslation>(translation, ctx_, jsFunctionName_);
}

}  // namespace rime
