#pragma once

/*
    define based-4GB namespace. all pointers are relative to the provided memory base
    this kind of tree is relocatable and serializable.
*/

#define     CRSPTREE_NAMESPACE  crsptree32_based


#define     CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(...)  unsigned char* memory_space, __VA_ARGS__
#define     CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(...)    memory_space, __VA_ARGS__

#define     CRSPTREE_MEMORYSPACE_PARAM                  unsigned char* memory_space
#define     CRSPTREE_PASS_MEMORYSPACE_PARAM             memory_space

#define     CRSPTREE_PACKED_POINTER_TYPE(pointee)       uint32_t
#define     CRSPTREE_PACKED_UINTPTR_TYPE()              uint32_t

#define     CRSPTREE_NULL_POINTER                       0U

static inline void* translate_crsptree_32_based(unsigned char* memory_space, uint32_t ptr) {
    if (ptr) {
        return (void*)(&memory_space[ptr]);
    }
    else {
        return nullptr;
    }
}

static inline uint32_t untranslate_crsptree_32_based(unsigned char* memory_space, void* whatever) {
    if (whatever) {
        return static_cast<uint32_t>(reinterpret_cast<unsigned char*>(whatever) - memory_space);
    }
    else {
        return 0;
    }
}

#define     CRSPTREE_TRANSLATE_POINTER(type, ...)               ((type*)translate_crsptree_32_based(memory_space, __VA_ARGS__))

#define     CRSPTREE_UNTRANSLATE_POINTER(...)                   untranslate_crsptree_32_based(memory_space, (void*)(__VA_ARGS__))

#define     CRSPTREE_TRANSLATE_NONNULL_POINTER(type, ...)       ((type*) (&memory_space[__VA_ARGS__]))
#define     CRSPTREE_UNTRANSLATE_NONNULL_POINTER(...)           static_cast<uint32_t>(((unsigned char*)(__VA_ARGS__)) - memory_space)

#define     CRSPTREE_ENABLE_ITERATORS       0

#include "crsptree_namespace.h"
