#pragma once

/*
    templateable 16-bit indexed implementation

    index 0 in the array must be unused, only 32767 nodes are addressable because bit 15 is used for color in the parent node
*/


#define     CRSPTREE_NAMESPACE  crsptree16_indexed

#define CRSPTREE_INSTANTIATION_TEMPLATE_PARAM_LIST          template<size_t index_scale>




#define     CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(...)  unsigned char* memory_space, __VA_ARGS__
#define     CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(...)    memory_space, __VA_ARGS__

#define     CRSPTREE_MEMORYSPACE_PARAM                  unsigned char* memory_space
#define     CRSPTREE_PASS_MEMORYSPACE_PARAM             memory_space

#define     CRSPTREE_PACKED_POINTER_TYPE(pointee)       uint16_t
#define     CRSPTREE_PACKED_UINTPTR_TYPE()              uint16_t

#define     CRSPTREE_NULL_POINTER                       static_cast<uint16_t>(0U)

template<size_t index_scale>
static inline void* translate_crsptree_16_indexed(unsigned char* memory_space, uint16_t ptr) {
    if (ptr) {
        return (void*)(&memory_space[static_cast<size_t>(ptr) * index_scale]);
    }
    else {
        return nullptr;
    }
}
template<size_t index_scale>
static inline uint32_t untranslate_crsptree_16_indexed(unsigned char* memory_space, void* whatever) {
    if (whatever) {
        return static_cast<uintptr_t>(reinterpret_cast<unsigned char*>(whatever) - memory_space) / index_scale;
    }
    else {
        return 0;
    }
}

#define     CRSPTREE_TRANSLATE_POINTER(type, ...)               ((type*)translate_crsptree_16_indexed<index_scale>(memory_space, __VA_ARGS__))

#define     CRSPTREE_UNTRANSLATE_POINTER(...)                   untranslate_crsptree_16_indexed<index_scale>(memory_space, (void*)(__VA_ARGS__))

#define     CRSPTREE_TRANSLATE_NONNULL_POINTER(type, ...)       ((type*) (&memory_space[static_cast<size_t>(__VA_ARGS__) * index_scale]))
#define     CRSPTREE_UNTRANSLATE_NONNULL_POINTER(...)           static_cast<uint16_t>(static_cast<size_t>(((unsigned char*)(__VA_ARGS__)) - memory_space) / index_scale)

#define     CRSPTREE_ENABLE_ITERATORS       0

#define     CRSPTREE_COLOR_BITMASK          static_cast<uint16_t>(0x8000)

#include "crsptree_namespace.h"
