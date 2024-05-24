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
//     - :intrinsics | cpu or compiler level builtins
//     - :arena      | memory arena implementation
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

// platform specific includes
//
#if OS_WINDOWS
    #if !defined(WIN32_LEAN_AND_MEAN)
        #define WIN32_LEAN_AND_MEAN 1
    #endif

    // :note ~windows
    //
    // we include windows.h here because we need a lot of its definitions for the winapi implementation
    // and due to conflicting macro definitions for function decorators it is best if it is included
    // as early as possible.
    //
    // currently this doesn't really serve a purpose for the public facing api, however, in the future
    // it is likely we will want to expose portions of the platform specific implementation
    // which will require us to include this anyway
    //
    #include <windows.h>
    #pragma warning(disable : 4201) // nonstandard extension used: nameless struct/union
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

//
// --------------------------------------------------------------------------------
// :intrinsics
// --------------------------------------------------------------------------------
//

// returns the operand width in bits if the input is zero
//
function U32 CountLeadingZeros_U32(U32 x);
function U64 CountLeadingZeros_U64(U64 x);

function U32 CountTrailingZeros_U32(U32 x);
function U64 CountTrailingZeros_U64(U64 x);

// Input count is masked to the bit width of the operand
//
function U32 RotateLeft_U32(U32 x, U32 count);
function U64 RotateLeft_U64(U64 x, U32 count);

function U32 RotateRight_U32(U32 x, U32 count);
function U64 RotateRight_U64(U64 x, U32 count);

function U32 PopCount_U32(U32 x);
function U64 PopCount_U64(U64 x);

// atomics
//
// all return the value stored in the 'ptr' before the operation
//
function U32 AtomicAdd_U32(volatile U32 *ptr, U32 value);
function U64 AtomicAdd_U64(volatile U64 *ptr, U64 value);

function U32   AtomicExchange_U32(volatile U32   *value, U32   exchange);
function U64   AtomicExchange_U64(volatile U64   *value, U64   exchange);
function void *AtomicExchange_Ptr(void *volatile *value, void *exchange);

// return true if operation succeeded, storing 'exchange' in 'value', otherwise returns false if
// the current value of 'value' doesn't match that of 'comparand'
//
function B32 AtomicCompareExchange_U32(volatile U32   *value, U32   exchange, U32   comparand);
function B32 AtomicCompareExchange_U64(volatile U64   *value, U64   exchange, U64   comparand);
function B32 AtomicCompareExchange_Ptr(void *volatile *value, void *exchange, void *comparand);

//
// --------------------------------------------------------------------------------
// :arena
// --------------------------------------------------------------------------------
//

// os virtual memory
//
function void *OS_ReserveMemory(U64 size);
function B32   OS_CommitMemory(void *base, U64 size);
function void  OS_DecommitMemory(void *base, U64 size);
function void  OS_ReleaseMemory(void *base, U64 size);

function U64 OS_GetPageSize();
function U64 OS_GetAllocationGranularity();

// memory size macros
//
#define KB(x) ((U64) (x) << 10)
#define MB(x) ((U64) (x) << 20)
#define GB(x) ((U64) (x) << 30)
#define TB(x) ((U64) (x) << 40)

typedef U32 M_ArenaFlags;
enum {
    // prevents the arena from growing when the limit is reached
    //
    // provided when arena is allocated
    //
    M_ARENA_DONT_GROW = (1 << 0),

    // doesn't clear push allocations to zero
    //
    // provided per push call
    //
    M_ARENA_NO_ZERO = (1 << 1)
};

typedef union M_Arena M_Arena;
union M_Arena {
    struct {
        M_Arena *current;
        M_Arena *prev;

        U64 base;
        U64 limit;
        U64 offset;
        U64 last_offset;

        U64 committed;
        U64 commit_size;

        M_ArenaFlags flags;
    };

    // make sure arena is padded to 128 bytes, gives us a lot of leeway
    //
    U8 pad[128];
};

StaticAssert(sizeof(M_Arena) == 128, "M_Arena is not 128-bytes in size");

// allocation
//
function M_Arena *M_AllocArenaArgs(U64 limit, U64 commit, M_ArenaFlags flags);
function M_Arena *M_AllocArena(U64 limit);

// reset will clear all allocations from the arena, but it will remain valid to use for further allocations
// release will free all backing memory from the arena making it invalid to use
//
function void M_ResetArena(M_Arena *arena);
function void M_ReleaseArena(M_Arena *arena);

#if defined(__cplusplus)
}
#endif

#endif  // CORE_H_

#endif  // !CORE_IMPL

#if defined(CORE_MODULE) || defined(CORE_IMPL)

#if !defined(CORE_C_)
#define CORE_C_

//
// --------------------------------------------------------------------------------
// :impl_intrinsics
// --------------------------------------------------------------------------------
//

#if COMPILER_MSVC

#include <intrin.h>

U32 CountLeadingZeros_U32(U32 x) {
    unsigned long index;

    U32 result = _BitScanReverse(&index, x) ? (31 - index) : 32;
    return result;
}

U64 CountLeadingZeros_U64(U64 x) {
    unsigned long index;

    U64 result = _BitScanReverse64(&index, x) ? (63 - index) : 64;
    return result;
}

U32 CountTrailingZeros_U32(U32 x) {
    unsigned long index;

    U32 result = _BitScanForward(&index, x) ? index : 32;
    return result;
}

U64 CountTrailingZeros_U64(U64 x) {
    unsigned long index;

    U64 result = _BitScanForward64(&index, x) ? index : 64;
    return result;
}

#if ARCH_AMD64

U32 PopCount_U32(U32 x) {
    U32 result = __popcnt(x);
    return result;
}

U64 PopCount_U64(U64 x) {
    U64 result = __popcnt64(x);
    return result;
}

#elif ARCH_AARCH64

U32 PopCount_U32(U32 x) {
    U32 result = _CountOneBits(x);
    return result;
}

U64 PopCount_U64(U64 x) {
    U64 result = _CountOneBits64(x);
    return result;
}

#endif

// atomics
//
U32 AtomicAdd_U32(volatile U32 *ptr, U32 value) {
    U32 result = _InterlockedExchangeAdd((volatile long *) ptr, value);
    return result;
}

U64 AtomicAdd_U64(volatile U64 *value, U64 addend) {
    U64 result = _InterlockedExchangeAdd64((volatile __int64 *) value, addend);
    return result;
}

U32 AtomicExchange_U32(volatile U32 *value, U32 exchange) {
    U32 result = _InterlockedExchange((volatile long *) value, exchange);
    return result;
}

U64 AtomicExchange_U64(volatile U64 *value, U64 exchange) {
    U64 result = _InterlockedExchange64((volatile __int64 *) value, exchange);
    return result;
}

void *AtomicExchange_Ptr(void *volatile *value, void *exchange) {
    void *result = _InterlockedExchangePointer(value, exchange);
    return result;
}

B32 AtomicCompareExchange_U32(volatile U32 *value, U32 exchange, U32 comparand) {
    B32 result = _InterlockedCompareExchange((volatile long *) value, exchange, comparand) == (long) comparand;
    return result;
}

B32 AtomicCompareExchange_U64(volatile U64 *value, U64 exchange, U64 comparand) {
    B32 result = _InterlockedCompareExchange64((volatile __int64 *) value, exchange, comparand) == (__int64) comparand;
    return result;
}

B32 AtomicCompareExchange_Ptr(void *volatile *value, void *exchange, void *comparand) {
    B32 result = _InterlockedCompareExchangePointer(value, exchange, comparand) == comparand;
    return result;
}

#elif (COMPILER_CLANG || COMPILER_GCC)

U32 CountLeadingZeros_U32(U32 x) {
    U32 result = x ? __builtin_clz(x) : 32;
    return result;
}

U64 CountLeadingZeros_U64(U64 x) {
    U64 result = x ? __builtin_clzll(x) : 64;
    return result;
}

U32 CountTrailingZeros_U32(U32 x) {
    U32 result = x ? __builtin_ctz(x) : 32;
    return result;
}

U64 CountTrailingZeros_U64(U64 x) {
    U64 result = x ? __builtin_ctzll(x) : 64;
    return result;
}

U32 PopCount_U32(U32 x) {
    U32 result = __builtin_popcount(x);
    return result;
}

U64 PopCount_U64(U64 x) {
    U64 result = __builtin_popcountll(x);
    return result;
}

// atomics
//
// @todo: handle memory ordering semantics more correctly
//
U32 AtomicAdd_U32(volatile U32 *ptr, U32 value) {
    U32 result = __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST);
    return result;
}

U64 AtomicAdd_U64(volatile U64 *ptr, U64 value) {
    U64 result = __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST);
    return result;
}

U32 AtomicExchange_U32(volatile U32 *value, U32 exchange) {
    U32 result;

    __atomic_exchange(value, &exchange, &result, __ATOMIC_SEQ_CST);
    return result;
}

U64 AtomicExchange_U64(volatile U64 *value, U64 exchange) {
    U64 result;

    __atomic_exchange(value, &exchange, &result, __ATOMIC_SEQ_CST);
    return result;
}

void *AtomicExchange_Ptr(void *volatile *value, void *exchange) {
    void *result;

    __atomic_exchange(value, &exchange, &result, __ATOMIC_SEQ_CST);
    return result;
}

B32 AtomicCompareExchange_U32(volatile U32 *value, U32 exchange, U32 comparand) {
    B32 result = __atomic_compare_exchange(value, &comparand, &exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return result;
}

B32 AtomicCompareExchange_U64(volatile U64 *value, U64 exchange, U64 comparand) {
    B32 result = __atomic_compare_exchange(value, &comparand, &exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return result;
}

B32 AtomicCompareExchange_Ptr(void *volatile *value, void *exchange, void *comparand) {
    B32 result = __atomic_compare_exchange(value, &comparand, &exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return result;
}

#endif

// agnostic across all compilers, msvc has specific intrinsics for this but under optimisations correctly
// detects these as the right instructions
//
U32 RotateLeft_U32(U32 x, U32 count) {
    count &= 31;

    U32 result = (x << count) | (x >> (32 - count));
    return result;
}

U64 RotateLeft_U64(U64 x, U32 count) {
    count &= 63;

    U64 result = (x << count) | (x >> (64 - count));
    return result;
}

U32 RotateRight_U32(U32 x, U32 count) {
    count &= 31;

    U32 result = (x >> count) | (x << (32 - count));
    return result;
}

U64 RotateRight_U64(U64 x, U32 count) {
    count &= 63;

    U64 result = (x >> count) | (x << (64 - count));
    return result;
}

//
// --------------------------------------------------------------------------------
// :impl_arena
// --------------------------------------------------------------------------------
//

// os virtual memory
//
#if OS_WINDOWS

void *OS_ReserveMemory(U64 size) {
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    return result;
}

B32 OS_CommitMemory(void *base, U64 size) {
    B32 result = VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE) != 0;
    return result;
}

void OS_DecommitMemory(void *base, U64 size) {
    VirtualFree(base, size, MEM_DECOMMIT);
}

void OS_ReleaseMemory(void *base, U64 size) {
    (void) size;
    VirtualFree(base, 0, MEM_RELEASE);
}

U64 OS_GetPageSize() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    U64 result = info.dwPageSize;
    return result;
}

U64 OS_GetAllocationGranularity() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    U64 result = info.dwAllocationGranularity;
    return result;
}

#elif OS_MACOS
#error "macOS memory subsystem not implemented"
#elif OS_LINUX
#error "linux memory subsystem not implemented"
#elif OS_SWITCH
#error "switch memory subsystem not implemented"
#endif

#define M_ARENA_MIN_OFFSET sizeof(M_Arena)

#if !defined(M_ARENA_DEFAULT_COMMIT_SIZE)
    #define M_ARENA_DEFAULT_COMMIT_SIZE KB(64)
#endif

#if !defined(M_ARENA_MAX_RESERVE_SWITCH)
    #define M_ARENA_MAX_RESERVE_SWITCH MB(8)
#endif

internal M_Arena *__M_AllocSizedArena(U64 limit, U64 commit, M_ArenaFlags flags) {
    M_Arena *result = 0;

    U64 page_size   = OS_GetPageSize();
    U64 granularity = OS_GetAllocationGranularity();

    // have at least the allocation granularity to reserve and at least the page size to commit
    //
    U64 to_reserve = Max(AlignUp(limit,  granularity), granularity);
    U64 to_commit  = Max(AlignUp(commit, page_size), page_size);

    void *base = OS_ReserveMemory(to_reserve);
    if (base != 0) {
        if (OS_CommitMemory(base, to_commit)) {
            result = cast(M_Arena *) base;

            result->current = result;
            result->prev    = 0;

            result->base        = 0;
            result->limit       = to_reserve;
            result->offset      = M_ARENA_MIN_OFFSET;
            result->last_offset = M_ARENA_MIN_OFFSET;

            result->committed   = to_commit;
            result->commit_size = to_commit;

            result->flags = flags;
        }
    }

    Assert(result != 0);

    return result;
}

M_Arena *M_AllocArenaArgs(U64 limit, U64 commit, M_ArenaFlags flags) {
#if OS_SWITCH
    // :note ~switchbrew
    //
    // swtichbrew doesn't support virtual memory semantics, this means we can't just ask
    // for 64GiB of virtual address space to commit later and get away with it like on
    // other operating systems.
    //
    // to work around this we force growable arenas on and clamp the initial arena size to a
    // more reasonable size
    //
    flags &= ~M_ARENA_DONT_GROW;
    limit  = Min(limit, M_ARENA_MAX_RESERVE_SWITCHBREW);
#endif

    M_Arena *result = __M_AllocSizedArena(limit, commit, flags);
    return result;
}

M_Arena *M_AllocArena(U64 limit) {
    M_Arena *result = M_AllocArenaArgs(limit, M_ARENA_DEFAULT_COMMIT_SIZE, 0);
    return result;
}

void M_ResetArena(M_Arena *arena) {
    M_Arena *current = arena->current;
    while (current->prev != 0) {
        void *base = cast(void *) current;
        U64   size = current->limit;

        current = current->prev;

        OS_ReleaseMemory(base, size);
    }

    Assert(current == arena);
    Assert(current->committed >= current->commit_size);

    void *decommit_base = cast(U8 *) current + current->commit_size;
    U64   decommit_size = current->committed - current->commit_size;

    if (decommit_size != 0) { OS_DecommitMemory(decommit_base, decommit_size); }

    current->offset      = M_ARENA_MIN_OFFSET;
    current->last_offset = M_ARENA_MIN_OFFSET;
    current->committed   = arena->commit_size;

    arena->current = current;
}

void M_ReleaseArena(M_Arena *arena) {
    M_Arena *current = arena->current;
    while (current != 0) {
        void *base = cast(void *) current;
        U64   size = current->limit;

        current = current->prev;

        OS_ReleaseMemory(base, size);
    }
}



#endif  // CORE_C_

#endif  // CORE_MODULE || CORE_IMPL
