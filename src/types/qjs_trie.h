#ifndef RIME_QJS_TRIE_H_
#define RIME_QJS_TRIE_H_

#include "qjs_macros.h"
#include "qjs_type_registry.h"
#include "trie.h"

DECLARE_JS_CLASS_WITH_SHARED_POINTER(Trie,
  NO_PROPERTY_TO_DECLARE,
  DECLARE_FUNCTIONS(loadTextFile, loadBinaryFile, saveToBinaryFile, find, prefixSearch)
)

#endif // RIME_QJS_TRIE_H_
