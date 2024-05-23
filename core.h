#if !defined(CORE_IMPL)

#if !defined(CORE_H_)
#define CORE_H_

#if defined(__cplusplus)
extern "C" {
#endif

// :header This is the header porition of this file, it provides all of the necessary defines for the
// code to be used.
//
// The beginning of the implementation can be found here :impl
//
// This header provides the following 'core' implementaitons for quick project startup, other header
// files will rely on this file when providing further functionality:
//
//     - :core_types | core types used throughout
//     - :macros     | utility macros and helpers
//
// This header can be included in one of three ways:
//     1. As a standalone header by simply including the header with no specific pre-defines. This is
//        to be used with a linked library version of the implementation via CORE_LIB or '3.' below
//
//         ...
//         #include "core.h"
//         ...
//
//     2. As a fully implemented module at include time by defining CORE_MODULE before including the header
//         ...
//         #define CORE_MODULE
//         #include "core.h"
//         ...
//
//     3. As a standalone 'c' file that contains the implementation code only, by defining CORE_IMPL
//        before including the header. This is used in conjunction with '1.' above
//
//        ...
//        #include "core.h"
//        ...
//        ...
//        ...
//        #define CORE_IMPL
//        #include "core.h"
//        ...
//
// Optionally CORE_LIB can be defined to correctly export/import api functions to allow the header to be built
// as a shared library.
//      library.c
//          ...
//          #define CORE_LIB
//          #define CORE_MODULE
//          #include "core.h"
//          ...
//
//     usage.c
//          ...
//          #define CORE_LIB
//          #include "core.h"
//          ...
//

//
// --------------------------------------------------------------------------------
// :core_types
// --------------------------------------------------------------------------------
//

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t  S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;

typedef int8_t  B8;
typedef int16_t B16;
typedef int32_t B32;
typedef int64_t B64;

typedef float  F32;
typedef double F64;

typedef struct Str8 Str8;
struct Str8 {
    S64 count;
    U8 *data;
};

#define U8_MAX  ((U8)  -1)
#define U16_MAX ((U16) -1)
#define U32_MAX ((U32) -1)
#define U64_MAX ((U64) -1)

#define S8_MIN  ((S8)  0x80)
#define S16_MIN ((S16) 0x8000)
#define S32_MIN ((S32) 0x80000000)
#define S64_MIN ((S64) 0x8000000000000000)

#define S8_MAX  ((U8)  0x7F)
#define S16_MAX ((U16) 0x7FFF)
#define S32_MAX ((U32) 0x7FFFFFFF)
#define S64_MAX ((U64) 0x7FFFFFFFFFFFFFFF)

#define F32_MAX ((F32) 3.40282346638528859811704183484516925e+038F)
#define F64_MAX ((F64) 1.79769313486231570814527423731704357e+308 )

//
// --------------------------------------------------------------------------------
// :macros
// --------------------------------------------------------------------------------
//

// operating system
//
#define OS_WINDOWS 0
#define OS_MACOS   0
#define OS_LINUX   0
#define OS_SWITCH  0

#if defined(_WIN32)
    #undef  OS_WINDOWS
    #define OS_WINDOWS 1
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_OSX
        #undef  OS_MACOS
        #define OS_MACOS 1

        #error "macos support is not yet implemented."
    #else
        #error "macos is the only supported apple platform."
    #endif
#elif defined(__linux__)
    #undef  OS_LINUX
    #define OS_LINUX 1
#elif defined(__SWITCH__)
    #undef  OS_SWITCH
    #define OS_SWITCH 1
#else
    #error "unsupported operating system."
#endif

// cpu architecture
//
#define ARCH_AMD64   0
#define ARCH_AARCH64 0

#if defined(__amd64__) || defined(_M_AMD64)
    #undef  ARCH_AMD64
    #define ARCH_AMD64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #undef  ARCH_AARCH64
    #define ARCH_AARCH64 1
#else
    #error "unsupported cpu architecture."
#endif

// compiler
//
#define COMPILER_CLANG 0
#define COMPILER_MSVC  0
#define COMPILER_GCC   0

#if defined(__clang__)
    #undef  COMPILER_CLANG
    #define COMPILER_CLANG 1
#elif defined(_MSC_VER)
    #undef  COMPILER_MSVC
    #define COMPILER_MSVC 1
#elif defined(__GNUC__)
    #undef  COMPILER_GCC
    #define COMPILER_GCC 1
#else
    #error "unsupported compiler"
#endif

// language
//
#define LANG_C    0
#define LANG_CPP  0
#define LANG_OBJC 0

#if defined(__OBJC__)
    #undef  LANG_OBJC
    #define LANG_OBJC 1

    #error "objective-c compilation mode is currently unsupported."
#elif defined(__cplusplus)
    #undef  LANG_CPP
    #define LANG_CPP 1
#else
    #undef  LANG_C
    #define LANG_C 1
#endif

// utilities
//

#include <assert.h>

#define cast(x) (x)

#define Assert(exp) assert(exp)
#define StaticAssert(exp, msg) static_assert(exp, msg)

#define ArraySize(x) (sizeof(x) / sizeof((x)[0]))
#define OffsetTo(T, m) ((U64) &(((T *) 0)->m))

#if LANG_CPP
    #define AlignOf(x) alignof(x)
#else
    #define AlignOf(x) _Alignof(x)
#endif

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(min, x, max) (Min(Max(x, min), max))
#define Clamp01(x) Clamp(0, x, 1)
#define Abs(x) ((x) < 0 ? -(x) : (x))

#define AlignUp(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define AlignDown(x, a) ((x) & ~((a) - 1))

#define _Glue(a, b) a##b
#define Glue(a, b) _Glue(a, b)

#define _Stringify(x) #x
#define Stringify(x) _Stringify(x)

#define FourCC(a, b, c, d) (((U32) (d) << 24) | ((U32) (b) << 16) | ((U32) (c) << 8) | ((U32) (a) << 0))

// linked list handling
//
#define LL_CheckNil(v) ((v) == 0)
#define LL_SetNil(v)   ((v) =  0)

// queue enqueue macros
//
#define SLL_Enqueue(h, t, n) SLL_EnqueueN(h, t, n, next)
#define SLL_EnqueueN(h, t, n, next) \
    (LL_CheckNil(h) ? ((h) = (t) = (n), LL_SetNil((n)->next)) : \
     ((t)->next = (n), (t) = (n), LL_SetNil((n)->next)))

// queue enqueue front macros
//
#define SLL_EnqueueFront(h, t, n) SLL_EnqueueFrontN(h, t, n, next)
#define SLL_EnqueueFrontN(h, t, n, next) \
    (LL_CheckNil(h) ? ((h) = (t) = (n), LL_SetNil((n)->next)) : ((n)->next = (h), (h) = (n)))

// queue dequeue macros
//
#define SLL_Dequeue(h, t) SLL_DequeueN(h, t, next)
#define SLL_DequeueN(h, t, next) \
    ((h) == (t) ? (LL_SetNil(h), LL_SetNil(t)) : ((h) = (h)->next))

// stack push macros
//
#define SLL_Push(h, n) SLL_PushN(h, n, next)
#define SLL_PushN(h, n, next) ((n)->next = (h), (h) = (n))

// stack pop macros
//
#define SLL_Pop(h) SLL_PopN(h, next)
#define SLL_PopN(h, next) ((h) = (h)->next)

// double-link list insert macros
//
// thanks ryan & allen! this is a crazy macro which leaves a lot to be desired when it comes
// to macro syntax in c/c++
//
#define DLL_Insert(h, t, p, n) DLL_InsertNP(h, t, p, n, next, prev)
#define DLL_InsertNP(h, t, p, n, next, prev) \
    (LL_CheckNil(h) ? ((h) = (t) = (n), LL_SetNil((n)->next), LL_SetNil((n)->prev)) : \
     LL_CheckNil(p) ? ((n)->next = (h), (h)->prev = (n), (h) = (n), LL_SetNil((n)->prev)) : \
     ((p) == (t)) ? ((t)->next = (n), (n)->prev = (t), (t) = (n), LL_SetNil((n)->next)) : \
     (((!LL_CheckNil(p) && LL_CheckNil((p)->next)) ? (0) : ((p)->next->prev = (n))), \
      ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))

// insert at front of d-list
//
#define DLL_InsertFront(h, t, n) DLL_InsertFrontNP(h, t, n, next, prev)
#define DLL_InsertFrontNP(h, t, n, next, prev) DLL_InsertNP(t, h, h, n, prev, next)

// insert at back of d-list
//
#define DLL_InsertBack(h, t, n) DLL_InsertBackNP(h, t, n, next, prev)
#define DLL_InsertBackNP(h, t, n, next, prev) DLL_InsertNP(h, t, t, n, next, prev)

// double-link list remove macros
//
#define DLL_Remove(h, t, n) DLL_RemoveNP(h, t, n, next, prev)
#define DLL_RemoveNP(h, t, n, next, prev) \
    (((n) == (h) ? (h) = (h)->next : (0)), \
     ((n) == (t) ? (t) = (t)->prev : (0)), \
     (LL_CheckNil((n)->prev) ? (0) : ((n)->prev->next = (n)->next)), \
     (LL_CheckNil((n)->next) ? (0) : ((n)->next->prev = (n)->prev)))

// function decorators
//
#if LANG_CPP
    #define c_linkage extern "C"
    #define c_linkage_begin extern "C" {
    #define c_linkage_end   }
#else
    #define c_linkage extern
    #define c_linkage_begin
    #define c_linkage_end
#endif

#define global_var    static
#define internal      static
#define local_persist static

#if COMPILER_MSVC
    #define export_function c_linkage __declspec(dllexport)
    #define thread_var __declspec(thread)
#else
    #define export_function c_linkage
    #define thread_var __thread
#endif

#if defined(CORE_LIB)
    #if COMPILER_MSVC
        #if defined(CORE_MODULE)
            #define function c_linkage __declspec(dllexport)
        #else
            #define function c_linkage __declspec(dllimport)
        #endif
    #else
        #define function c_linkage
    #endif
#else
    #define function static
#endif

#if defined(__cplusplus)
}
#endif

#endif  // CORE_H_

#endif  // !CORE_IMPL
