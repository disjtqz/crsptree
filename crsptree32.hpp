#pragma once

/*
    define low 4gb namespace - may only address the low 4GB of memory.
*/

#define     CRSPTREE_NAMESPACE  crsptree32


#define     CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(...)  __VA_ARGS__
#define     CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(...)    __VA_ARGS__

#define     CRSPTREE_MEMORYSPACE_PARAM 
#define     CRSPTREE_PASS_MEMORYSPACE_PARAM 

#define     CRSPTREE_PACKED_POINTER_TYPE(pointee)       uint32_t
#define     CRSPTREE_PACKED_UINTPTR_TYPE()              uint32_t

#define     CRSPTREE_NULL_POINTER                       0U
#define     CRSPTREE_TRANSLATE_POINTER(type, ...)       ((type*)static_cast<uintptr_t>(__VA_ARGS__))

#define     CRSPTREE_UNTRANSLATE_POINTER(...)           static_cast<uint32_t>(reinterpret_cast<uintptr_t>(__VA_ARGS__))

#define     CRSPTREE_ENABLE_ITERATORS       0

#include "crsptree_namespace.h"