#pragma once
/*
    This header requires that you include cstddef cstdint and climits
*/



//compiler-specific features for better codegen


#if defined(_MSC_VER)
#define		CRSPTREE_NOINLINE		__declspec(noinline)
#define		CRSPTREE_FORCEINLINE	__forceinline
#define		CRSPTREE_RESTRICT		__restrict
#define		CRSPTREE_PUREISH		__declspec(noalias)
#elif defined(__GNUG__)
#define		CRSPTREE_NOINLINE		__attribute__((noinline))
#define		CRSPTREE_FORCEINLINE	__attribute__((always_inline))
#define		CRSPTREE_RESTRICT		__restrict
#define		CRSPTREE_PUREISH		__attribute__((pure))
#else
#define		CRSPTREE_NOINLINE		
#define		CRSPTREE_FORCEINLINE	inline
#define		CRSPTREE_RESTRICT		
#define		CRSPTREE_PUREISH
#endif
#define		crsptree_offsetof_m		__builtin_offsetof
#define		crsptree_containing_record_m(p, container, memb)	reinterpret_cast<container *>( reinterpret_cast<char *>(p) - crsptree_offsetof_m(container, memb))
//end compiler-specific features

#define		CRSPTREE_PARENT_FIRST		


/*
    define default namespace - fully addressable 64-bit pointers
*/

#define     CRSPTREE_NAMESPACE  crsptree


#define     CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(...)  __VA_ARGS__
#define     CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(...)    __VA_ARGS__

#define     CRSPTREE_MEMORYSPACE_PARAM 
#define     CRSPTREE_PASS_MEMORYSPACE_PARAM 

#define     CRSPTREE_PACKED_POINTER_TYPE(pointee)       pointee*
#define     CRSPTREE_PACKED_UINTPTR_TYPE()              uintptr_t

#define     CRSPTREE_NULL_POINTER                       nullptr
#define     CRSPTREE_TRANSLATE_POINTER(type, ...)       ((type*)(__VA_ARGS__))

#define     CRSPTREE_UNTRANSLATE_POINTER(...)           (__VA_ARGS__)

#define     CRSPTREE_ENABLE_ITERATORS       1

#include "crsptree_namespace.h"
