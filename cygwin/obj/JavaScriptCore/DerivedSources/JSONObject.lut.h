// Automatically generated from /cygdrive/f/chromium/WEBKIT~1/WebKit/JAVASC~1/runtime/JSONObject.cpp using /cygdrive/f/chromium/WEBKIT~1/WebKit/JAVASC~1/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {
#if ENABLE(JIT)
#define THUNK_GENERATOR(generator) , generator
#else
#define THUNK_GENERATOR(generator)
#endif

static const struct HashTableValue jsonTableValues[3] = {
   { "parse", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(JSONProtoFuncParse), (intptr_t)1 THUNK_GENERATOR(0) },
   { "stringify", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(JSONProtoFuncStringify), (intptr_t)1 THUNK_GENERATOR(0) },
   { 0, 0, 0, 0 THUNK_GENERATOR(0) }
};

#undef THUNK_GENERATOR
extern JSC_CONST_HASHTABLE HashTable jsonTable =
    { 4, 3, jsonTableValues, 0 };
} // namespace
