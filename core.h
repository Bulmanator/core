#if !defined(CORE_IMPL)

#if !defined(CORE_H_)
#define CORE_H_

#if defined(__cplusplus)
extern "C" {
#endif

// :header This is the header porition of this file, it provides all of the necessary
// defines for the code to be used.
//
// The beginning of the implementation can be found by searching :impl. The
// implementation sections can be found by searching :impl_<section_name> where the
// section names are defined just below
//
// This header provides the following 'core' implementaitons for quick project
// startup, other header files will rely on this file when providing further
// functionality:
//
//     - :core_types | core types used throughout
//     - :macros     | utility macros and helpers
//     - :intrinsics | cpu or compiler level builtins
//     - :utilities  | some basic utility functions
//     - :arena      | memory arena implementation
//     - :strings    | counted string helpers
//     - :logging    | logging interface
//     - :filesystem | filesystem + file io interface
//
// This header can be included in one of three ways:
//
//     1. As a standalone header by simply including the header with no specific
//     pre-defines. This is to be used with a linked library version of the
//     implementation via CORE_LIB or '3.' below
//
//         ...
//         #include "core.h"
//         ...
//
//     2. As a fully implemented module at include time by defining CORE_MODULE
//     before including the header
//         ...
//         #define CORE_MODULE
//         #include "core.h"
//         ...
//
//     3. As a standalone 'c' file that contains the implementation code only, by
//     defining CORE_IMPL before including the header. This is used in conjunction
//     with '1.' above
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
// Optionally CORE_LIB can be defined to correctly export/import api functions to
// allow the header to be built as a shared library.
//
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

// generic handle for representing primitives implemented by the operating system
//
typedef struct OS_Handle OS_Handle;
struct OS_Handle {
    U64 v[1];
};

typedef void VoidProc(void);

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
    // we include windows.h here because we need a lot of its definitions for the winapi
    // implementation and due to conflicting macro definitions for function decorators it
    // is best if it is included as early as possible.
    //
    // currently this doesn't really serve a purpose for the public facing api, however, in
    // the future it is likely we will want to expose portions of the platform specific
    // implementation which will require us to include this anyway
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

#if OS_WINDOWS
    // taken from the windows kits header, allows us to have a nice graphical message box
    // for assert when compiling for windows subsystem
    //
    // clang correctly uses the windows kits headers as well, however, msvc has an internal
    // function called '_assert' that is the same as '_wassert' but takes regular ansi strings
    // and is available without extra includes
    //
    // clang does not provide this '_assert' function so we're better off just using the '_wassert'
    // call regardless
    //
    #define AssertAlways(exp) (void) ((!!(exp)) || (_wassert(L#exp, Glue(L, __FILE__), __LINE__), 0))
#elif OS_SWITCH
    // devkitpro has the parameters in a wack order for some reason
    //
    #define AssertAlways(exp) (void) ((!!(exp)) || (__assert_func(__FILE__, __LINE__, __PRETTY_FUNCTION__, #exp), 0))
#else
    // default fallback, both normal clang and gcc use this
    //
    #define AssertAlways(exp) (void) ((!!(exp)) || (__assert_fail(#exp, __FILE__, __LINE__, __PRETTY_FUNCTION__), 0))
#endif

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

#define FourCC(a, b, c, d) (((U32) (d) << 24) | ((U32) (c) << 16) | ((U32) (b) << 8) | ((U32) (a) << 0))

#define Compose_U64(hi, lo) (((U64) (hi) << 32) | (lo))

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

#if COMPILER_MSVC
    #define export_function c_linkage __declspec(dllexport)
    #define thread_var __declspec(thread)
    #define THIS_FUNCTION_NAME      S(__FUNCTION__)
    #define THIS_FUNCTION_SIGNATURE S(__FUNCSIG__)
#else
    #define export_function c_linkage
    #define thread_var __thread
    #define THIS_FUNCTION_NAME      S(__FUNCTION__)
    #define THIS_FUNCTION_SIGNATURE S(__PRETTY_FUNCTION__)
#endif

#if !defined(THIS_FUNCTION)
    // this function expands to just the name, can define THIS_FUNCTION as THIS_FUNCTION_SIGNATURE
    // to get the full signature before including this header, or just use _NAME and _SIGNATURE
    // independently where required
    //
    #define THIS_FUNCTION THIS_FUNCTION_NAME
#endif

#define THIS_FILE S(__FILE__)
#define THIS_LINE  (__LINE__)

#define global_var    static
#define internal      static
#define local_persist static
#define thread_static static thread_var

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

// constants
//
#if !defined(EPSILON_F32)
    // clang default value for FLT_EPSILON
    //
    #define EPSILON_F32 (F32) 1.19209290e-7f
#endif

#if !defined(EPSILON_F64)
    // clang default value for DBL_EPSILON
    //
    #define EPSILON_F64 (F64) 2.2204460492503131e-16
#endif

#define E_F32   (2.718281828459045235360287f)
#define PI_F32  (3.141592653589793238462643f)
#define TAU_F32 (2.0f * PI_F32)

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
// :utilities
// --------------------------------------------------------------------------------
//

// os handle utilities
//
function OS_Handle OS_NilHandle();

function B32 OS_HandleEqual(OS_Handle a, OS_Handle b);
function B32 OS_HandleValid(OS_Handle a); // only vaild if != OS_NilHandle()

// number utilities
//
function U32 SaturateCast_U32(U64 value);
function U16 SaturateCast_U16(U32 value);
function U8  SaturateCast_U8 (U16 value);

function U32 SafeTruncate_U32(U64 value);
function U16 SafeTruncate_U16(U32 value);
function U8  SafeTruncate_U8 (U16 value);

function F32 SafeRatio0_F32(F32 num, F32 denom);
function F64 SafeRatio0_F64(F64 num, F64 denom);

function F32 SafeRatio1_F32(F32 num, F32 denom);
function F64 SafeRatio1_F64(F64 num, F64 denom);

function U32 NextPow2_U32(U32 value);
function U64 NextPow2_U64(U64 value);

function U32 PrevPow2_U32(U32 value);
function U64 PrevPow2_U64(U64 value);

function U32 NearestPow2_U32(U32 value);
function U64 NearestPow2_U64(U64 value);

function U16 SwapBytes_U16(U16 x);
function U32 SwapBytes_U32(U32 x);
function U64 SwapBytes_U64(U64 x);

// These macros are for little-endian machines, they are however macros in
// the event a popular big endian machine comes along we can have defines
// to override them (inverting their definitions) so code that expects to
// read BE values to host values still work correctly.
//
// Mainly for reading files where integer values are setup in a specific byte
// order (normally network = BE)
//
#define ReadBE_U16(x) SwapBytes_U16(x)
#define ReadBE_U32(x) SwapBytes_U32(x)
#define ReadBE_U64(x) SwapBytes_U64(x)

#define ReadLE_U16(x) (x)
#define ReadLE_U32(x) (x)
#define ReadLE_U64(x) (x)

function U8  ReverseBits_U8(U8 x);
function U16 ReverseBits_U16(U16 x);
function U32 ReverseBits_U32(U32 x);
function U64 ReverseBits_U64(U64 x);

// all return a pointer to the beginning of dst
//
function void *M_CopySize(void *dst, void *src, U64 size);
function void *M_FillSize(void *dst, U8 value, U64 size);
function void *M_ZeroSize(void *dst, U64 size);

function B32 M_CompareSize(void *a, void *b, U64 size);

// return -1 for a < b, 0 for a == b, 1 for a > b
//
#define COMPARE_FUNC(name) S32 name(void *a, void *b)
typedef COMPARE_FUNC(CompareFunc);

// quick sort is typically faster but merge sort is stable so have both depending on use-case
//
function void _MergeSort(void *array, S64 count, CompareFunc *Compare, U64 element_size);
function void _QuickSort(void *array, S64 count, CompareFunc *Compare, U64 element_size);

#define MergeSort(array, count, Compare) _MergeSort((void *) (array), (count), Compare, sizeof(*(array)))
#define QuickSort(array, count, Compare) _QuickSort((void *) (array), (count), Compare, sizeof(*(array)))

//
// --------------------------------------------------------------------------------
// :arena
// --------------------------------------------------------------------------------
//

// virtual memory
//
function void *M_Reserve(U64 size);
function B32   M_Commit(void *base, U64 size);
function void  M_Decommit(void *base, U64 size);
function void  M_Release(void *base, U64 size);

function U64 M_GetPageSize();
function U64 M_GetAllocationGranularity();

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

        M_ArenaFlags flags;
    };

    // make sure arena is padded to 64 bytes, can increase to 128 bytes if needed
    // this currently fits into a single cache line which is nice
    //
    U8 pad[64];
};

StaticAssert(sizeof(M_Arena) == 64, "M_Arena is not 64-bytes in size");

// allocation
//
function M_Arena *M_AllocArenaArgs(U64 limit, U64 initial_commit, M_ArenaFlags flags);
function M_Arena *M_AllocArena(U64 limit);

// reset will clear all allocations from the arena, but it will remain valid to use for
// further allocations
//
// release will free all backing memory from the arena making it invalid to use
//
function void M_ResetArena(M_Arena *arena);
function void M_ReleaseArena(M_Arena *arena);

// push/push copy calls to allocate from arenas
//
function void *M_ArenaPushFrom(M_Arena *arena, U64 size, M_ArenaFlags flags, U64 alignment);
function void *M_ArenaPushCopyFrom(M_Arena *arena, void *src, U64 size, M_ArenaFlags flags, U64 alignment);

// helper macros for allocating types and arrays from arenas
//
#define M_ArenaPush(...)     M_ArenaPushExpand((__VA_ARGS__, M_ArenaPushTNFA, M_ArenaPushTNF, M_ArenaPushTN, M_ArenaPushT))(__VA_ARGS__)
#define M_ArenaPushCopy(...) M_ArenaPushCopyExpand((__VA_ARGS__, M_ArenaPushCopyTNFA, M_ArenaPushCopyTNF, M_ArenaPushCopyTN, M_ArenaPushCopyT))(__VA_ARGS__)

function U64 M_GetArenaOffset(M_Arena *arena);

// pop calls to remove allocations from the end of arenas
//
function void M_ArenaPopTo(M_Arena *arena, U64 offset);
function void M_ArenaPopSize(M_Arena *arena, U64 size);
function void M_ArenaPopLast(M_Arena *arena);

// helper macro for popping types and arrays
//
#define M_ArenaPop(...) M_ArenaPopExpand((__VA_ARGS__, M_ArenaPopTN, M_ArenaPopT))(__VA_ARGS__)

// thread local temporary arenas
//
typedef struct M_Temp M_Temp;
struct M_Temp {
    M_Arena *arena;
    U64 offset;
};

// get a thread local temporary arena, any arenas supplied in the 'conflicts' array
// will not be re-acquired by this call
//
// releasing a temp arena will relinquish control of the memory allocated from
// it, signalling to the system it is no longer needed
//
function M_Temp M_AcquireTemp(U32 count, M_Arena **conflicts);
function void   M_ReleaseTemp(M_Temp temp);

// supporting macros for push/pop default argument selection
//
// :note these are just implementation details and can be mostly ignored
//
#define M_ArenaPushExpand(args) M_ArenaPushSelect args
#define M_ArenaPushCopyExpand(args) M_ArenaPushCopySelect args
#define M_ArenaPopExpand(args) M_ArenaPopSelect args

#define M_ArenaPushSelect(a, b, c, d, e, f, ...) f
#define M_ArenaPushCopySelect(a, b, c, d, e, f, g, ...) g
#define M_ArenaPopSelect(a, b, c, d, ...) d

#define M_ArenaPushT(arena, T)             (T *) M_ArenaPushFrom((arena),       sizeof(T), 0, AlignOf(T))
#define M_ArenaPushTN(arena, T, n)         (T *) M_ArenaPushFrom((arena), (n) * sizeof(T), 0, AlignOf(T))
#define M_ArenaPushTNF(arena, T, n, f)     (T *) M_ArenaPushFrom((arena), (n) * sizeof(T), f, AlignOf(T))
#define M_ArenaPushTNFA(arena, T, n, f, a) (T *) M_ArenaPushFrom((arena), (n) * sizeof(T), f, a)

#define M_ArenaPushCopyT(arena, src, T)             (T *) M_ArenaPushCopyFrom((arena), (src),       sizeof(T), 0, AlignOf(T))
#define M_ArenaPushCopyTN(arena, src, T, n)         (T *) M_ArenaPushCopyFrom((arena), (src), (n) * sizeof(T), 0, AlignOf(T))
#define M_ArenaPushCopyTNF(arena, src, T, n, f)     (T *) M_ArenaPushCopyFrom((arena), (src), (n) * sizeof(T), f, AlignOf(T))
#define M_ArenaPushCopyTNFA(arena, src, T, n, f, a) (T *) M_ArenaPushCopyFrom((arena), (src), (n) * sizeof(T), f, a)

#define M_ArenaPopT(arena, T)     M_ArenaPopSize((arena),       sizeof(T))
#define M_ArenaPopTN(arena, T, n) M_ArenaPopSize((arena), (n) * sizeof(T))

//
// --------------------------------------------------------------------------------
// :strings
// --------------------------------------------------------------------------------
//
#include <stdarg.h>

// utility macros for wrapping string literals, struct pointers and expanding as arguments
//
#define S(x) Str8_Wrap(sizeof(x) - sizeof(*(x)), (U8 *) (x))
#define Sz(x) Str8_WrapZ((U8 *) (x))
#define Sl(x) { sizeof(x) - sizeof(*(x)), (U8 *) (x) }
#define Sv(x) (int) (x).count, (x).data
#define Sf(arena, format, ...) Str8_Format((arena), (format), ##__VA_ARGS__)

function Str8 Str8_Wrap(S64 count, U8 *data);
function Str8 Str8_WrapZ(U8 *zstr); // null-terminated
function Str8 Str8_WrapRange(U8 *start, U8 *end);

// will be null-terminated
//
function Str8 Str8_Copy(M_Arena *arena, Str8 str);
function Str8 Str8_Concat(M_Arena *arena, Str8 a, Str8 b);

// string equality
//
typedef U32 Str8_EqualFlags;
enum {
    STR8_EQUAL_FLAG_IGNORE_CASE = (1 << 0),
    STR8_EQUAL_FLAG_INEXACT_RHS = (1 << 1)
};

function B32 Str8_Equal(Str8 a, Str8 b, Str8_EqualFlags flags);

// string formatting
//
function Str8 Str8_FormatArgs(M_Arena *arena, const char *format, va_list args);
function Str8 Str8_Format(M_Arena *arena, const char *format, ...);

// string slicing, all counts are in bytes
//
function Str8 Str8_Prefix(Str8 str, S64 count);
function Str8 Str8_Suffix(Str8 str, S64 count);
function Str8 Str8_Advance(Str8 str, S64 count);
function Str8 Str8_Remove(Str8 str, S64 count);
function Str8 Str8_Slice(Str8 str, S64 start, S64 end);

// these are ascii only and are meant to be used with char literals
//
// including the found character, returns 'str' if 'chr' is not found
//
function Str8 Str8_RemoveAfterFirst(Str8 str, U8 chr);
function Str8 Str8_RemoveAfterLast(Str8 str, U8 chr);

function Str8 Str8_RemoveBeforeFirst(Str8 str, U8 chr);
function Str8 Str8_RemoveBeforeLast(Str8 str, U8 chr);

// these work with both forward and backslash separators on windows, forward slash only on other platforms.
//
function Str8 Str8_GetBasename(Str8 path);  // includes extension
function Str8 Str8_GetDirname(Str8 path);   // excludes trailing separator
function Str8 Str8_GetExtension(Str8 path); // gives shortest extension excluding separating '.'

function Str8 Str8_StripExtension(Str8 path); // removes separating '.'

// utf-8 unicode handling
//
typedef struct Codepoint Codepoint;
struct Codepoint {
    U32 count; // number of bytes used to encode/decode
    U32 value;
};

function Codepoint UTF8_Decode(Str8 str);
function U32 UTF8_Encode(U8 *output, U32 codepoint); // output expects enough space for the encoded codepoint

// character utilities
//
function B32 Chr_IsWhitespace(U8 c);
function B32 Chr_IsAlpha(U8 c);
function B32 Chr_IsUppercase(U8 c);
function B32 Chr_IsLowercase(U8 c);
function B32 Chr_IsNumber(U8 c);
function B32 Chr_IsHex(U8 c);
function B32 Chr_IsSlash(U8 c); // checks for forward and backward slashes
function B32 Chr_IsPathSeparator(U8 c); // checks for fwd/bkwd on windows, only fwd on other platforms

function U8 Chr_ToUppercase(U8 c);
function U8 Chr_ToLowercase(U8 c);

//
// --------------------------------------------------------------------------------
// :logging
// --------------------------------------------------------------------------------
//
enum {
    // These are negative to be expandable, users can define their own custom
    // log codes which are >= 0 to have more control
    //
    // Any future built-in codes will also be negative
    //
    LOG_ERROR = -1,
    LOG_WARN  = -2,
    LOG_INFO  = -3,
    LOG_DEBUG = -4
};

typedef struct Log_Message Log_Message;
struct Log_Message {
    Log_Message *next;

    // Can be anything you want, the default usage is "log level" with the above
    // enum
    //
    S32 code;

    Str8 file;
    Str8 func;
    U32  line;

    Str8 message;
};

typedef struct Log_MessageList Log_MessageList;
struct Log_MessageList {
    Log_Message *first;
    Log_Message *last;

    U32 num_messages;
};

typedef struct Log_MessageArray Log_MessageArray;
struct Log_MessageArray {
    Log_Message *items;
    U32 count;
};

typedef struct Log_Scope Log_Scope;
struct Log_Scope {
    Log_Scope *next;

    U64 offset;
    Log_MessageList messages;
};

typedef struct Log_Context Log_Context;
struct Log_Context {
    M_Arena   *arena;
    Log_Scope *scopes;
};

function Str8 Log_StrFromLevel(S32 level);

// @todo: once we have a more substantial init call for core generally this will
// go away
//
function void Log_Init();

// Push/pop logging scopes
//
// When popping an array of all of the messages that have been push onto the scope
//
function void Log_PushScope();
function Log_MessageArray Log_PopScope(M_Arena *arena);

function void Log_PushMessageArgs(S32 code, Str8 file, U32 line, Str8 func, const char *format, va_list args);
function void Log_PushMessage(S32 code, Str8 file, U32 line, Str8 func, const char *format, ...);

#if !defined(NDEBUG)
    #define Log_Debug(f, ...) Log_PushMessage(LOG_DEBUG, THIS_FILE, THIS_LINE, THIS_FUNCTION, f, ##__VA_ARGS__)
#else
    #define Log_Debug(...)
#endif

#define Log_Info(f, ...)  Log_PushMessage(LOG_INFO,  THIS_FILE, THIS_LINE, THIS_FUNCTION, f, ##__VA_ARGS__)
#define Log_Warn(f, ...)  Log_PushMessage(LOG_WARN,  THIS_FILE, THIS_LINE, THIS_FUNCTION, f, ##__VA_ARGS__)
#define Log_Error(f, ...) Log_PushMessage(LOG_ERROR, THIS_FILE, THIS_LINE, THIS_FUNCTION, f, ##__VA_ARGS__)

//
// --------------------------------------------------------------------------------
// :stream
// --------------------------------------------------------------------------------
//
// A very simple, expandable buffered stream interface. Working at a byte
// granularity by default with some bit-oriented helper functions
//
// The bit functions can read a max of 32-bits ahead at the moment
//
typedef struct Stream_Context Stream_Context;

typedef S32 Stream_Error;
enum {
    // This is made to be externally expandable, common error-codes defined
    // here will always be increasingly negative and zero will always mean
    // "no error".
    //
    // If it is desired to add custom stream errors as long as you manage
    // conflicts between your subsystems and they are > 0 you will never
    // conflict with the common error-codes
    //
    STREAM_ERROR_READ_PAST_END = -1,
    STREAM_ERROR_NONE = 0,
};

#define STREAM_REFILL(name) Stream_Error name(Stream_Context *stream)
typedef STREAM_REFILL(Stream_Refill);

struct Stream_Context {
    U8 *start;
    U8 *end;

    U8 *pos;

    U32 bit_buffer;
    U32 bit_count;

    Stream_Refill *RefillFunc;

    Stream_Error error;
};

// Utility macros
//
#define Stream_Total(s)     (S64) ((s)->end - (s)->start)
#define Stream_Remaining(s) (S64) ((s)->end - (s)->pos)
#define Stream_Read(s, T) (T *) (s)->pos; (s)->pos += sizeof(T)

// Construction
//
function void Stream_Zero(Stream_Context *stream); // virtual zero stream
function void Stream_FromMemory(Stream_Context *stream, Str8 memory);

function Stream_Error Stream_Fail(Stream_Context *stream, Stream_Error err);

// Bit functions
//
function void Stream_RefillBits(Stream_Context *stream);
function void Stream_ConsumeBits(Stream_Context *stream, U64 count);

function U64 Stream_PeekBits(Stream_Context *stream, U64 count);

// This will peek and then consume 'count' bits in one go
//
function U64 Stream_ReadBits(Stream_Context *stream, U64 count);

//
// --------------------------------------------------------------------------------
// :filesystem
// --------------------------------------------------------------------------------
//

typedef U32 FS_Properties;
enum {
    FS_PROPERTY_IS_DIRECTORY = (1 << 0),
    FS_PROPERTY_IS_HIDDEN    = (1 << 1)
};

typedef struct FS_Time FS_Time;
struct FS_Time {
    U64 written;
    U64 accessed;
    U64 created;
};

typedef struct FS_Entry FS_Entry;
struct FS_Entry {
    FS_Entry *next;

    Str8 path;

    FS_Properties props;
    U64 size;

    FS_Time times;
};

typedef struct FS_List FS_List;
struct FS_List {
    FS_Entry *first;
    FS_Entry *last;

    U32 num_entries;
};

typedef U32 FS_ListFlags;
enum {
    FS_LIST_RECURSIVE      = (1 << 0),
    FS_LIST_INCLUDE_HIDDEN = (1 << 1) // relative '.' and '..' directories are never included in the list
};

function FS_List FS_ListPath(M_Arena *arena, Str8 path, FS_ListFlags flags);

typedef U32 FS_Access;
enum {
    FS_ACCESS_READ      = (1 << 0),
    FS_ACCESS_WRITE     = (1 << 1),
    FS_ACCESS_READWRITE = (FS_ACCESS_READ | FS_ACCESS_WRITE)
};

function OS_Handle FS_OpenFile(Str8 path, FS_Access access);
function void      FS_CloseFile(OS_Handle file);
function B32       FS_RemoveFile(Str8 path);

// returns number of bytes read/written
//
function S64 FS_ReadFile(OS_Handle file, Str8 data, U64 offset);
function S64 FS_WriteFile(OS_Handle file, Str8 data, U64 offset);
function S64 FS_AppendFile(OS_Handle file, Str8 data);

// info from handle
//
function FS_Properties FS_PropertiesFromHandle(OS_Handle file);
function FS_Time       FS_TimeFromHandle(OS_Handle file);
function Str8          FS_PathFromHandle(M_Arena *arena, OS_Handle file);
function U64           FS_SizeFromHandle(OS_Handle file);

// info from path
//
function FS_Properties FS_PropertiesFromPath(Str8 path);
function FS_Time       FS_TimeFromPath(Str8 path);
function U64           FS_SizeFromPath(Str8 path);

function B32 FS_CreateDirectory(Str8 path);
function B32 FS_RemoveDirectory(Str8 path);

function Str8 FS_ReadEntireFile(M_Arena *arena, Str8 path);

typedef U32 FS_PathType;
enum {
    FS_PATH_EXE,
    FS_PATH_USER,
    FS_PATH_TEMP,
    FS_PATH_WORKING
};

function Str8 FS_GetPath(M_Arena *arena, FS_PathType type);

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

//
// :msvc_intrinsics
//

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

//
// :clang_intrinsics
// :gcc_intrinsics
//

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

// agnostic across all compilers, msvc has specific intrinsics for this but under
// optimisations correctly detects these as the right instructions
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
// :impl_utilities
// --------------------------------------------------------------------------------
//

OS_Handle OS_NilHandle() {
    OS_Handle result = { 0 };
    return result;
}

B32 OS_HandleEqual(OS_Handle a, OS_Handle b) {
    B32 result = (a.v[0] == b.v[0]);
    return result;
}

B32 OS_HandleValid(OS_Handle a) {
    B32 result = !OS_HandleEqual(a, OS_NilHandle());
    return result;
}

U32 SaturateCast_U32(U64 value) {
    U32 result = (value > U32_MAX) ? U32_MAX : cast(U32) value;
    return result;
}

U16 SaturateCast_U16(U32 value) {
    U16 result = (value > U16_MAX) ? U16_MAX : cast(U16) value;
    return result;
}

U8 SaturateCast_U8(U16 value) {
    U8 result = (value > U8_MAX) ? U8_MAX : cast(U8) value;
    return result;
}

U32 SafeTruncate_U32(U64 value) {
    AssertAlways(value <= U32_MAX);

    U32 result = cast(U32) value;
    return result;
}

U16 SafeTruncate_U16(U32 value) {
    AssertAlways(value <= U16_MAX);

    U16 result = cast(U16) value;
    return result;
}

U8 SafeTruncate_U8(U16 value) {
    AssertAlways(value <= U8_MAX);

    U8 result = cast(U8) value;
    return result;
}

F32 SafeRatio0_F32(F32 num, F32 denom) {
    F32 result = 0;

    if (denom < -EPSILON_F32 || denom > EPSILON_F32) {
        result = num / denom;
    }

    return result;
}

F64 SafeRatio0_F64(F64 num, F64 denom) {
    F64 result = 0;

    if (denom < -EPSILON_F64 || denom > EPSILON_F64) {
        result = num / denom;
    }

    return result;
}

F32 SafeRatio1_F32(F32 num, F32 denom) {
    F32 result = 1;

    if (denom < -EPSILON_F32 || denom > EPSILON_F32) {
        result = num / denom;
    }

    return result;
}

F64 SafeRatio1_F64(F64 num, F64 denom) {
    F64 result = 1;

    if (denom < -EPSILON_F64 || denom > EPSILON_F64) {
        result = num / denom;
    }

    return result;
}

U32 NextPow2_U32(U32 value) {
    U32 result = value;

    result -= 1;
    result |= (result >> 1);
    result |= (result >> 2);
    result |= (result >> 4);
    result |= (result >> 8);
    result |= (result >> 16);
    result += 1;

    return result;
}

U64 NextPow2_U64(U64 value) {
    U64 result = value;

    result -= 1;
    result |= (result >> 1);
    result |= (result >> 2);
    result |= (result >> 4);
    result |= (result >> 8);
    result |= (result >> 16);
    result |= (result >> 32);
    result += 1;

    return result;
}

U32 PrevPow2_U32(U32 value) {
    U32 result = NextPow2_U32(value) >> 1;
    return result;
}

U64 PrevPow2_U64(U64 value) {
    U64 result = NextPow2_U64(value) >> 1;
    return result;
}

U32 NearestPow2_U32(U32 value) {
    U32 next = NextPow2_U32(value);
    U32 prev = next >> 1;

    U32 result = ((next - value) < (value - prev)) ? next : prev;
    return result;
}

U64 NearestPow2_U64(U64 value) {
    U64 next = NextPow2_U64(value);
    U64 prev = next >> 1;

    U64 result = ((next - value) < (value - prev)) ? next : prev;
    return result;
}

// These are correctly detected as bswap/rev instructions under all
// major compilers with optimisations turned on.... at least on an
// up to date version
//

U16 SwapBytes_U16(U16 x) {
    U16 result = (x >> 8) | (x << 8);
    return result;
}

U32 SwapBytes_U32(U32 x) {
    U32 result =
        (x & 0xFF000000) >> 24 |
        (x & 0x00FF0000) >>  8 |
        (x & 0x0000FF00) <<  8 |
        (x & 0x000000FF) << 24;

    return result;
}

U64 SwapBytes_U64(U64 x) {
    U64 result =
        ((x & 0xFF00000000000000) >> 56) |
        ((x & 0x00FF000000000000) >> 40) |
        ((x & 0x0000FF0000000000) >> 24) |
        ((x & 0x000000FF00000000) >>  8) |
        ((x & 0x00000000FF000000) <<  8) |
        ((x & 0x0000000000FF0000) << 24) |
        ((x & 0x000000000000FF00) << 40) |
        ((x & 0x00000000000000FF) << 56);

    return result;
}

U8 ReverseBits_U8(U8 x) {
    U8 result = x;

    result = ((result & 0xAA) >> 1) | ((result & 0x55) << 1);
    result = ((result & 0xCC) >> 2) | ((result & 0x33) << 2);
    result = ((result & 0xF0) >> 4) | ((result & 0x0F) << 4);

    return result;
}

U16 ReverseBits_U16(U16 x) {
    U16 result = x;

    result = ((result & 0xAAAA) >> 1) | ((result & 0x5555) << 1);
    result = ((result & 0xCCCC) >> 2) | ((result & 0x3333) << 2);
    result = ((result & 0xF0F0) >> 4) | ((result & 0x0F0F) << 4);
    result = ((result & 0xFF00) >> 8) | ((result & 0x00FF) << 8);

    return result;
}

U32 ReverseBits_U32(U32 x) {
    U32 result = x;

    result = ((result & 0xAAAAAAAA) >>  1) | ((result & 0x55555555) <<  1);
    result = ((result & 0xCCCCCCCC) >>  2) | ((result & 0x33333333) <<  2);
    result = ((result & 0xF0F0F0F0) >>  4) | ((result & 0x0F0F0F0F) <<  4);
    result = ((result & 0xFF00FF00) >>  8) | ((result & 0x00FF00FF) <<  8);
    result = ((result & 0xFFFF0000) >> 16) | ((result & 0x0000FFFF) << 16);

    return result;
}

U64 ReverseBits_U64(U64 x) {
    U64 result = x;

    result = ((result & 0xAAAAAAAAAAAAAAAA) >>  1) | ((result & 0x5555555555555555) <<  1);
    result = ((result & 0xCCCCCCCCCCCCCCCC) >>  2) | ((result & 0x3333333333333333) <<  2);
    result = ((result & 0xF0F0F0F0F0F0F0F0) >>  4) | ((result & 0x0F0F0F0F0F0F0F0F) <<  4);
    result = ((result & 0xFF00FF00FF00FF00) >>  8) | ((result & 0x00FF00FF00FF00FF) <<  8);
    result = ((result & 0xFFFF0000FFFF0000) >> 16) | ((result & 0x0000FFFF0000FFFF) << 16);
    result = ((result & 0xFFFFFFFF00000000) >> 32) | ((result & 0x00000000FFFFFFFF) << 32);

    return result;
}

void *M_CopySize(void *dst, void *src, U64 size) {
    void *result = dst;

    U8 *dst8 = cast(U8 *) dst;
    U8 *src8 = cast(U8 *) src;

    while (size--) {
        *dst8++ = *src8++;
    }

    return result;
}

void *M_FillSize(void *dst, U8 value, U64 size) {
    void *result = dst;

    U8 *dst8 = cast(U8 *) dst;

    while (size--) {
        *dst8++ = value;
    }

    return result;
}

void *M_ZeroSize(void *dst, U64 size) {
    void *result = dst;

    U8 *dst8 = cast(U8 *) dst;

    while (size--) {
        *dst8++ = 0;
    }

    return result;
}

B32 M_CompareSize(void *a, void *b, U64 size) {
    B32 result = true;

    U8 *a8 = cast(U8 *) a;
    U8 *b8 = cast(U8 *) b;

    while (size--) {
        if (*a8++ != *b8++) {
            result = false;
            break;
        }
    }

    return result;
}

// merge sort implementation
//
internal void _MergeSortMerge(U8 *array, CompareFunc *Compare, U64 element_size, S64 start, S64 middle, S64 end) {
    S64 lcount = middle - start + 1;
    S64 rcount = end - middle;

    M_Temp temp = M_AcquireTemp(0, 0);

    U8 *lstart = array + ((start)      * element_size);
    U8 *rstart = array + ((middle + 1) * element_size);

    U8 *l = M_ArenaPushCopy(temp.arena, lstart, U8, lcount * element_size);
    U8 *r = M_ArenaPushCopy(temp.arena, rstart, U8, rcount * element_size);

    U8 *lend = l + (lcount * element_size);
    U8 *rend = r + (rcount * element_size);

    U8 *element = array + (start * element_size);

    while (l < lend && r < rend) {
        if (Compare(l, r) <= 0) {
            M_CopySize(element, l, element_size);
            l += element_size;
        }
        else {
            M_CopySize(element, r, element_size);
            r += element_size;
        }

        element += element_size;
    }

    U64 l_remainder = cast(U64) (lend - l);
    U64 r_remainder = cast(U64) (rend - r);

    if (l_remainder) {
        M_CopySize(element, l, l_remainder);
        element += l_remainder;
    }

    if (r_remainder) { M_CopySize(element, r, r_remainder); }

    M_ReleaseTemp(temp);
}

internal void _MergeSortSplit(U8 *array, CompareFunc *Compare, U64 element_size, S64 start, S64 end) {
    if (start < end) {
        S64 middle = start + ((end - start) >> 1);

        _MergeSortSplit(array, Compare, element_size, start,      middle);
        _MergeSortSplit(array, Compare, element_size, middle + 1, end);

        _MergeSortMerge(array, Compare, element_size, start, middle, end);
    }
}

void _MergeSort(void *array, S64 count, CompareFunc *Compare, U64 element_size) {
    _MergeSortSplit((U8 *) array, Compare, element_size, 0, count - 1);
}

// quick sort implementation
//
internal S64 _QuickSortPartition(U8 *array, CompareFunc *Compare, U64 element_size, S64 lo, S64 hi) {
    S64 result = lo;

    M_Temp temp = M_AcquireTemp(0, 0);

    U8 *pivot = array + (hi * element_size);

    U8 *i = array + (lo * element_size);
    U8 *j = array + (lo * element_size);
    U8 *t = M_ArenaPush(temp.arena, U8, element_size); // for swapping

    for (S64 it = lo; it < hi; ++it) {
        if (Compare(j, pivot) <= 0) {
            // swap i and j
            //
            M_CopySize(t, i, element_size);
            M_CopySize(i, j, element_size);
            M_CopySize(j, t, element_size);

            i      += element_size;
            result += 1;
        }

        j += element_size;
    }

    // swap i with pivot
    //
    M_CopySize(t,     i,     element_size);
    M_CopySize(i,     pivot, element_size);
    M_CopySize(pivot, t,     element_size);

    M_ReleaseTemp(temp);

    return result;
}

internal void _QuickSortRange(U8 *array, CompareFunc *Compare, U64 element_size, S64 lo, S64 hi) {
    if (lo < hi) {
        S64 pivot = _QuickSortPartition(array, Compare, element_size, lo, hi);

        _QuickSortRange(array, Compare, element_size, lo, pivot - 1);
        _QuickSortRange(array, Compare, element_size, pivot + 1, hi);
    }
}

void _QuickSort(void *array, S64 count, CompareFunc *Compare, U64 element_size) {
    _QuickSortRange((U8 *) array, Compare, element_size, 0, count - 1);
}

//
// --------------------------------------------------------------------------------
// :impl_arena
// --------------------------------------------------------------------------------
//

#if OS_WINDOWS

//
// :win32_arena
//

void *M_Reserve(U64 size) {
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    return result;
}

B32 M_Commit(void *base, U64 size) {
    B32 result = VirtualAlloc(base, size, MEM_COMMIT, PAGE_READWRITE) != 0;
    return result;
}

void M_Decommit(void *base, U64 size) {
    VirtualFree(base, size, MEM_DECOMMIT);
}

void M_Release(void *base, U64 size) {
    (void) size;
    VirtualFree(base, 0, MEM_RELEASE);
}

U64 M_GetPageSize() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    U64 result = info.dwPageSize;
    return result;
}

U64 M_GetAllocationGranularity() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);

    U64 result = info.dwAllocationGranularity;
    return result;
}

#elif OS_MACOS
#error "macOS memory subsystem not implemented"
#elif OS_LINUX

//
// :linux_arena
//

#include <sys/mman.h>
#include <unistd.h>

#if !defined(MAP_ANON)
    #define MAP_ANON MAP_ANONYMOUS
#endif

void *M_Reserve(U64 size) {
    void *ptr = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

    void *result = (ptr == MAP_FAILED) ? 0 : ptr;
    return result;
}

B32 M_Commit(void *base, U64 size) {
    B32 result = mprotect(base, size, PROT_READ | PROT_WRITE) == 0;
    return result;
}

void M_Decommit(void *base, U64 size) {
    madvise(base, size, MADV_DONTNEED);
    mprotect(base, size, PROT_NONE);
}

void M_Release(void *base, U64 size) {
    munmap(base, size);
}

U64 M_GetPageSize() {
    U64 result = sysconf(_SC_PAGESIZE);
    return result;
}

U64 M_GetAllocationGranularity() {
    U64 result = sysconf(_SC_PAGESIZE);
    return result;
}

#elif OS_SWITCH

//
// :switch_arena
//
// we unfortunately don't have any other options to allocate memory with switchbrew,
// malloc/free are the only userspace general address space allocators available to
// us
//
#include <stdlib.h>

void *M_Reserve(U64 size) {
    void *result = malloc(size);
    return result;
}

B32 M_Commit(void *base, U64 size) {
    (void) base;
    (void) size;

    // already committed the entire region with malloc so nothing to do
    //
    B32 result = true;
    return result;
}

void M_Decommit(void *base, U64 size) {
    (void) base;
    (void) size;

    // no decommit available because malloc/free so nothing to do
    //
}

void M_Release(void *base, U64 size) {
    (void) size;

    free(base);
}

U64 M_GetPageSize() {
    U64 result = 4096;
    return result;
}

U64 M_GetAllocationGranularity() {
    U64 result = 4096;
    return result;
}

#endif

#define M_ARENA_MIN_OFFSET sizeof(M_Arena)

#if !defined(M_ARENA_COMMIT_SIZE)
    #define M_ARENA_COMMIT_SIZE KB(64)
#endif

#if !defined(M_ARENA_MAX_RESERVE_SWITCH)
    #define M_ARENA_MAX_RESERVE_SWITCH MB(8)
#endif

#if !defined(M_ARENA_GROW_RESERVE_SIZE)
    #define M_ARENA_GROW_RESERVE_SIZE MB(1)
#endif

internal M_Arena *__M_AllocSizedArena(U64 limit, U64 initial_commit, M_ArenaFlags flags) {
    M_Arena *result = 0;

    U64 page_size   = M_GetPageSize();
    U64 granularity = M_GetAllocationGranularity();

    // have at least the allocation granularity to reserve and at least the page size to commit
    //
    U64 to_reserve = Max(AlignUp(limit, granularity), granularity);
    U64 to_commit  = Clamp(page_size, AlignUp(initial_commit, page_size), to_reserve);

    void *base = M_Reserve(to_reserve);
    if (base != 0) {
        if (M_Commit(base, to_commit)) {
            result = cast(M_Arena *) base;

            result->current = result;
            result->prev    = 0;

            result->base        = 0;
            result->limit       = to_reserve;
            result->offset      = M_ARENA_MIN_OFFSET;
            result->last_offset = M_ARENA_MIN_OFFSET;

            result->committed = to_commit;

            result->flags = flags;
        }
    }

    Assert(result != 0);

    return result;
}

M_Arena *M_AllocArenaArgs(U64 limit, U64 initial_commit, M_ArenaFlags flags) {
#if OS_SWITCH
    // :note ~switch
    //
    // swtichbrew doesn't support virtual memory semantics, this means we can't just
    // ask for 64GiB of virtual address space to commit later and get away with it
    // like on other operating systems.
    //
    // to work around this we force growable arenas on and clamp the initial arena
    // size to a more reasonable size
    //
    flags &= ~M_ARENA_DONT_GROW;
    limit  = Min(limit, M_ARENA_MAX_RESERVE_SWITCH);
#endif

    M_Arena *result = __M_AllocSizedArena(limit, initial_commit, flags);
    return result;
}

M_Arena *M_AllocArena(U64 limit) {
    M_Arena *result = M_AllocArenaArgs(limit, M_ARENA_COMMIT_SIZE, 0);
    return result;
}

void M_ResetArena(M_Arena *arena) {
    M_Arena *current = arena->current;
    while (current->prev != 0) {
        void *base = cast(void *) current;
        U64   size = current->limit;

        current = current->prev;

        M_Release(base, size);
    }

    Assert(current == arena);
    Assert(current->committed >= M_ARENA_COMMIT_SIZE);

    void *decommit_base = cast(U8 *) current + M_ARENA_COMMIT_SIZE;
    U64   decommit_size = current->committed - M_ARENA_COMMIT_SIZE;

    if (decommit_size != 0) { M_Decommit(decommit_base, decommit_size); }

    current->offset      = M_ARENA_MIN_OFFSET;
    current->last_offset = M_ARENA_MIN_OFFSET;
    current->committed   = M_ARENA_COMMIT_SIZE;

    arena->current = current;
}

void M_ReleaseArena(M_Arena *arena) {
    M_Arena *current = arena->current;
    while (current != 0) {
        void *base = cast(void *) current;
        U64   size = current->limit;

        current = current->prev;

        M_Release(base, size);
    }
}

void *M_ArenaPushFrom(M_Arena *arena, U64 size, M_ArenaFlags flags, U64 alignment) {
    void *result = 0;

    M_Arena *current = arena->current;

    alignment  = Clamp(1, alignment, 4096);
    U64 offset = AlignUp(current->offset, alignment);
    U64 end    = offset + size;

    if (end > current->limit) {
        // not enough space in current arena so allocate a new one if growing is
        // permitted
        //
        if ((arena->flags & M_ARENA_DONT_GROW) == 0) {
            U64 reserve   = Max(size + M_ARENA_MIN_OFFSET, M_ARENA_GROW_RESERVE_SIZE);
            M_Arena *next = __M_AllocSizedArena(reserve, M_ARENA_COMMIT_SIZE, 0);

            next->base = current->base + current->limit;

            SLL_PushN(arena->current, next, prev);

            current = next;
            offset  = AlignUp(current->offset, alignment);
            end     = offset + size;
        }
    }

    if (end > current->committed) {
        void *commit_base = cast(U8 *) current + current->committed;
        U64 commit_offset = AlignUp(end, M_ARENA_COMMIT_SIZE);
        U64 commit_limit  = Min(commit_offset, current->limit);
        U64 commit_size   = commit_limit - current->committed;

        if (M_Commit(commit_base, commit_size)) { current->committed = commit_limit; }
    }

    if (current->committed >= end) {
        // we have managed to commit enough space for the allocation
        //
        result = cast(U8 *) current + offset;

        current->last_offset = current->offset;
        current->offset      = end;

        if ((flags & M_ARENA_NO_ZERO) == 0) { M_ZeroSize(result, size); }
    }

    Assert(result != 0);
    Assert(((U64) result & (alignment - 1)) == 0);

    return result;
}

void *M_ArenaPushCopyFrom(M_Arena *arena, void *src, U64 size, M_ArenaFlags flags, U64 alignment) {
    void *result = M_CopySize(M_ArenaPushFrom(arena, size, flags, alignment), src, size);
    return result;
}

U64 M_GetArenaOffset(M_Arena *arena) {
    M_Arena *current = arena->current;

    U64 result = (current->base + current->offset);
    return result;
}

void M_ArenaPopTo(M_Arena *arena, U64 offset) {
    M_Arena *current = arena->current;

    while (current->base > offset) {
        void *base = cast(void *) current;
        U64   size = current->limit;

        current = current->prev;

        M_Release(base, size);
    }

    U64 local_offset = Max(offset - current->base, M_ARENA_MIN_OFFSET);

    if (local_offset <= current->offset) {
        // as we have popped back explicitly the 'last_offset' is no longer valid so
        // set that to this offset thus calling 'pop last' after manually setting the
        // offset doesn't do anything
        //
        current->offset      = local_offset;
        current->last_offset = local_offset;
    }

    arena->current = current;
}

void M_ArenaPopSize(M_Arena *arena, U64 size) {
    U64 offset = M_GetArenaOffset(arena);

    // explicitly greater than because the arena takes some space for its header
    // information
    //
    if (offset > size) {
        M_ArenaPopTo(arena, offset - size);
    }
}

void M_ArenaPopLast(M_Arena *arena) {
    M_Arena *current = arena->current;

    // We know the last offset cannot span multiple arenas even with chained growth
    // because single push allocations must be contiguous and thus be contained in a
    // single arena
    //
    // this means popping the last allocation is a simple move of the offset
    //
    current->offset = current->last_offset;
}

// thread-local temporary arenas
//
#if !defined(M_TEMP_ARENA_COUNT)
    #define M_TEMP_ARENA_COUNT 2
#endif

#if !defined(M_TEMP_ARENA_RESERVE_SIZE)
    #define M_TEMP_ARENA_RESERVE_SIZE GB(4)
#endif

thread_static M_Arena *__tls_temp[M_TEMP_ARENA_COUNT];

M_Temp M_AcquireTemp(U32 count, M_Arena **conflicts) {
    M_Temp result;

    result.arena  = 0;
    result.offset = 0;

    for (U32 t = 0; t < M_TEMP_ARENA_COUNT; ++t) {
        if (!__tls_temp[t]) { __tls_temp[t] = M_AllocArena(M_TEMP_ARENA_RESERVE_SIZE); }

        M_Arena *arena = __tls_temp[t];
        for (U32 c = 0; c < count; ++c) {
            if (conflicts[c] == arena) {
                arena = 0;
                break;
            }
        }

        if (arena) {
            result.arena  = arena;
            result.offset = M_GetArenaOffset(arena);

            break;
        }
    }

    Assert(result.arena != 0);

    return result;
}

void M_ReleaseTemp(M_Temp temp) {
    M_ArenaPopTo(temp.arena, temp.offset);
}

//
// --------------------------------------------------------------------------------
// :impl_strings
// --------------------------------------------------------------------------------
//

Str8 Str8_Wrap(S64 count, U8 *data) {
    Str8 result;
    result.count = count;
    result.data  = data;

    return result;
}

internal S64 CountLengthZ(U8 *zstr) {
    S64 result = 0;

    while (zstr[result]) {
        result += 1;
    }

    return result;
}

Str8 Str8_WrapZ(U8 *zstr) {
    Str8 result;
    result.count = CountLengthZ(zstr);
    result.data  = zstr;

    return result;
}

Str8 Str8_WrapRange(U8 *start, U8 *end) {
    Str8 result;

    Assert(start <= end);

    result.count = cast(S64) (end - start);
    result.data  = start;

    return result;
}

// will be null-terminated
//
Str8 Str8_Copy(M_Arena *arena, Str8 str) {
    Str8 result;
    result.count = str.count;
    result.data  = cast(U8 *) M_CopySize(M_ArenaPush(arena, U8, str.count + 1), str.data, str.count);

    return result;
}

Str8 Str8_Concat(M_Arena *arena, Str8 a, Str8 b) {
    Str8 result;
    result.count = (a.count + b.count);
    result.data  = M_ArenaPush(arena, U8, result.count + 1);

    M_CopySize(result.data          , a.data, a.count);
    M_CopySize(result.data + a.count, b.data, b.count);

    return result;
}

// string equality
//
B32 Str8_Equal(Str8 a, Str8 b, Str8_EqualFlags flags) {
    B32 result;

    B32 ignore_case = (flags & STR8_EQUAL_FLAG_IGNORE_CASE) != 0;
    B32 inexact_rhs = (flags & STR8_EQUAL_FLAG_INEXACT_RHS) != 0;

    result = (a.count == b.count) || inexact_rhs;

    if (result) {
        S64 count = Min(a.count, b.count);

        for (S64 it = 0; it < count; ++it) {
            U8 ca, cb;
            if (ignore_case) {
                ca = Chr_ToUppercase(a.data[it]);
                cb = Chr_ToUppercase(b.data[it]);
            }
            else {
                ca = a.data[it];
                cb = b.data[it];
            }

            if (ca != cb) {
                result = false;
                break;
            }
        }
    }

    return result;
}

// @todo: probably want to move away from vsnprintf to allow us to have custom type
// printing etc.
//
#include <stdio.h> // vsnprintf

#if !defined(STR8_INITIAL_FORMAT_GUESS_SIZE)
    #define STR8_INITIAL_FORMAT_GUESS_SIZE 1024
#endif

internal S64 Str8_ProcessFormat(U8 *buffer, S64 count, const char *format, va_list args) {
    S64 result = vsnprintf((char *) buffer, count, format, args);
    return result;
}

Str8 Str8_FormatArgs(M_Arena *arena, const char *format, va_list args) {
    Str8 result;

    va_list copy;
    va_copy(copy, args);

    S64 buffer_size = STR8_INITIAL_FORMAT_GUESS_SIZE;
    U8 *buffer      = M_ArenaPush(arena, U8, buffer_size);

    S64 count = Str8_ProcessFormat(buffer, buffer_size, format, args);
    if (count >= buffer_size) {
        // not enough space in the initial guess so pop the entire buffer, push the
        // correct size and process format again
        //
        M_ArenaPopLast(arena);

        result.count = count;
        result.data  = M_ArenaPush(arena, U8, result.count + 1);

        Str8_ProcessFormat(result.data, result.count + 1, format, copy);
    }
    else if (count < 0) {
        // error occurred
        //
        M_ArenaPopLast(arena);

        result.count = 0;
        result.data  = 0;
    }
    else {
        // initial size was enough, pop any unused portion of the initial buffer
        //
        result.count = count;
        result.data  = buffer;

        // we keep the null-terminating byte
        //
        M_ArenaPopSize(arena, buffer_size - count - 1);
    }

    return result;
}

Str8 Str8_Format(M_Arena *arena, const char *format, ...) {
    Str8 result;

    va_list args;
    va_start(args, format);

    result = Str8_FormatArgs(arena, format, args);

    va_end(args);

    return result;
}

// string slicing, all counts are in bytes
//
Str8 Str8_Prefix(Str8 str, S64 count) {
    Str8 result;
    result.count = Min(str.count, count);
    result.data  = str.data;

    return result;
}

Str8 Str8_Suffix(Str8 str, S64 count) {
    Str8 result;
    result.count = Min(str.count, count);
    result.data  = str.data + (str.count - result.count);

    return result;
}

Str8 Str8_Advance(Str8 str, S64 count) {
    Str8 result;
    result.count = str.count - Min(str.count, count);
    result.data  = str.data  + Min(str.count, count);

    return result;
}

Str8 Str8_Remove(Str8 str, S64 count) {
    Str8 result;
    result.count = str.count - Min(str.count, count);
    result.data  = str.data;

    return result;
}

Str8 Str8_Slice(Str8 str, S64 start, S64 end) {
    Str8 result;

    Assert(start <= end);

    result.count = Min(str.count, end - start);
    result.data  = str.data + Min(str.count, start);

    return result;
}

// these are ascii only and are meant to be used with char literals
//
// including the found character
//
Str8 Str8_RemoveAfterFirst(Str8 str, U8 chr) {
    Str8 result = str;

    for (S64 it = 0; it < str.count; ++it) {
        if (str.data[it] == chr) {
            result = Str8_Prefix(str, it);
            break;
        }
    }

    return result;
}

Str8 Str8_RemoveAfterLast(Str8 str, U8 chr) {
    Str8 result = str;

    for (S64 it = str.count - 1; it >= 0; --it) {
        if (str.data[it] == chr) {
            result = Str8_Prefix(str, it);
            break;
        }
    }

    return result;
}

Str8 Str8_RemoveBeforeFirst(Str8 str, U8 chr) {
    Str8 result = str;

    for (S64 it = 0; it < str.count; ++it) {
        if (str.data[it] == chr) {
            result = Str8_Suffix(str, str.count - it - 1);
            break;
        }
    }

    return result;
}

Str8 Str8_RemoveBeforeLast(Str8 str, U8 chr) {
    Str8 result = str;

    for (S64 it = str.count - 1; it >= 0; --it) {
        if (str.data[it] == chr) {
            result = Str8_Suffix(str, str.count - it - 1);
            break;
        }
    }

    return result;
}

Str8 Str8_GetBasename(Str8 path) {
    Str8 result = path;

    for (S64 it = path.count - 1; it >= 0; --it) {
        if (Chr_IsPathSeparator(path.data[it])) {
            result = Str8_Suffix(path, path.count - it - 1);
            break;
        }
    }

    return result;
}

Str8 Str8_GetDirname(Str8 path) {
    // if a path separator isn't found, use 'current working directory'
    //
    Str8 result = Sl(".");

    for (S64 it = path.count - 1; it >= 0; --it) {
        if (Chr_IsPathSeparator(path.data[it])) {
            result = Str8_Prefix(path, it);
            break;
        }
    }

    return result;
}

Str8 Str8_GetExtension(Str8 path) {
    Str8 result = Sl(""); // if '.' is not found, no extension

    for (S64 it = path.count - 1; it >= 0; --it) {
        if (path.data[it] == '.') {
            result = Str8_Suffix(path, path.count - it - 1);
            break;
        }
    }

    return result;
}

Str8 Str8_StripExtension(Str8 path) {
    Str8 result = path;

    for (S64 it = path.count - 1; it >= 0; --it) {
        if (path.data[it] == '.') {
            result = Str8_Prefix(path, it);
            break;
        }
    }

    return result;
}

Codepoint UTF8_Decode(Str8 str) {
    Codepoint result;

    local_persist const U8 lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };

    local_persist const U32 masks[] = { 0x00, 0x7F, 0x1F, 0x0F, 0x07 };

    if (str.count > 0) {
        U8 *data = str.data;

        U32 len   = lengths[data[0] >> 3];
        U32 avail = cast(U32) Min(len, str.count);

        U32 codepoint = data[0] & masks[len];

        for (U32 it = 1; it < avail; ++it) {
            codepoint <<= 6;
            codepoint  |= (data[it] & 0x3F);
        }

        // don't go past the end of the string, may produce invalid codepoints
        //
        result.count = avail;
        result.value = codepoint;
    }
    else {
        result.value = '?';
        result.count =  1 ;
    }

    return result;
}

U32 UTF8_Encode(U8 *output, U32 codepoint) {
    U32 result;

    if (codepoint <= 0x7F) {
        output[0] = cast(U8) codepoint;
        result    = 1;
    }
    else if (codepoint <= 0x7FF) {
        output[0] = ((codepoint >> 6) & 0x1F) | 0xC0;
        output[1] = ((codepoint >> 0) & 0x3F) | 0x80;
        result    = 2;
    }
    else if (codepoint <= 0xFFFF) {
        output[0] = ((codepoint >> 12) & 0x0F) | 0xE0;
        output[1] = ((codepoint >>  6) & 0x3F) | 0x80;
        output[2] = ((codepoint >>  0) & 0x3F) | 0x80;
        result    = 3;
    }
    else if (codepoint <= 0x10FFFF) {
        output[0] = ((codepoint >> 18) & 0x07) | 0xF0;
        output[1] = ((codepoint >> 12) & 0x3F) | 0x80;
        output[2] = ((codepoint >>  6) & 0x3F) | 0x80;
        output[3] = ((codepoint >>  0) & 0x3F) | 0x80;
        result    = 4;
    }
    else {
        output[0] = '?';
        result    =  1 ;
    }

    return result;
}

// character utilities
//
B32 Chr_IsWhitespace(U8 c) {
    B32 result = (c == ' ') || (c == '\n') || (c == '\r') || (c == '\t') || (c == '\f') || (c == '\v');
    return result;
}

B32 Chr_IsAlpha(U8 c) {
    B32 result = Chr_IsUppercase(c) || Chr_IsLowercase(c);
    return result;
}

B32 Chr_IsUppercase(U8 c) {
    B32 result = (c >= 'A' && c <= 'Z');
    return result;
}

B32 Chr_IsLowercase(U8 c) {
    B32 result = (c >= 'a' && c <= 'z');
    return result;
}

B32 Chr_IsNumber(U8 c) {
    B32 result = (c >= '0' && c <= '9');
    return result;
}

B32 Chr_IsHex(U8 c) {
    B32 result = Chr_IsNumber(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    return result;
}

B32 Chr_IsSlash(U8 c) {
    B32 result = (c == '/') || (c == '\\');
    return result;
}

B32 Chr_IsPathSeparator(U8 c) {
#if OS_WINDOWS
    B32 result = (c == '/') || (c == '\\');
#else
    B32 result = (c == '/');
#endif

    return result;
}

U8 Chr_ToUppercase(U8 c) {
    U8 result = Chr_IsLowercase(c) ? (c - ('a' - 'A')) : c;
    return result;
}

U8 Chr_ToLowercase(U8 c) {
    U8 result = Chr_IsUppercase(c) ? (c + ('a' - 'A')) : c;
    return result;
}

//
// --------------------------------------------------------------------------------
// :impl_stream
// --------------------------------------------------------------------------------
//

internal STREAM_REFILL(Stream_RefillZeros) {
    Stream_Error result = stream->error;

    local_persist U8 zeros[64];

    stream->start = zeros;
    stream->end   = zeros + sizeof(zeros);
    stream->pos   = zeros;

    return result;
}

internal STREAM_REFILL(Stream_RefillMemory) {
    Stream_Error result = Stream_Fail(stream, STREAM_ERROR_READ_PAST_END);
    return result;
}

void Stream_Zero(Stream_Context *stream) {
    stream->bit_buffer = 0;
    stream->bit_count  = 0;

    stream->error = STREAM_ERROR_NONE;

    stream->RefillFunc = Stream_RefillZeros;
    stream->RefillFunc(stream);
}

void Stream_FromMemory(Stream_Context *stream, Str8 memory) {
    stream->bit_buffer = 0;
    stream->bit_count  = 0;

    stream->error = STREAM_ERROR_NONE;

    stream->start = memory.data;
    stream->end   = memory.data + memory.count;
    stream->pos   = memory.data;

    stream->RefillFunc = Stream_RefillMemory;
}

Stream_Error Stream_Fail(Stream_Context *stream, Stream_Error err) {
    Stream_Error result;

    stream->error      = err;
    stream->RefillFunc = Stream_RefillZeros;

    result = stream->RefillFunc(stream);
    return result;
}

// Bit functions
//
void Stream_RefillBits(Stream_Context *stream) {
    // @todo: think about how to handle this if we need more than 32-bits
    // in our bit buffer.
    //
    // If we have a larger amount it may cause over-reads where the codec
    // has specifically put mechanisms in place to prevent them. This could
    // potentially cause a refill call when it was unexpected by client code
    // and will desync the entire bitstream
    //
    // for example in PNG during the DEFLATE stage a BTYPE of 0 will first
    // align the bitstream to a byte boundary and then read two 16 bit
    // values, as 32-bits is the maximum it will read this guarantees the
    // bit buffer/count will be zero after. Thus when copying literals directly
    // (which updates the streams 'pos') there is no worry about having to
    // flush the bit buffer or over-reading.
    //
    // One option could be to fill the bit buffer without actually consuming bytes
    // and only update the "pos" pointer when consuming bits instead. However,
    // this has the issue of having to do a "dummy" refill to lookahead if
    // we reach the end of the stream section
    //
    // We could just require the max number of bits to be provided to the
    // this refill bits call
    //

    for (; stream->bit_count <= 24; stream->bit_count += 8) {
        if (stream->pos == stream->end) {
            // We have run out of bytes to put into the bit buffer so
            // attempt to refill, if on failure it will produce the virtual
            // zero stream and pad the extra bytes to zero
            //
            stream->RefillFunc(stream);
        }

        stream->bit_buffer |= (*stream->pos++ << stream->bit_count);
    }
}

void Stream_ConsumeBits(Stream_Context *stream, U64 count) {
    Assert(stream->bit_count >= count);

    stream->bit_buffer >>= count;
    stream->bit_count   -= count;
}

U64 Stream_PeekBits(Stream_Context *stream, U64 count) {
    Assert(count < 32);

    if (stream->bit_count < count) {
        // Not enough bit, refill!
        //
        Stream_RefillBits(stream);
    }

    // this works with zero count
    //
    U64 result = stream->bit_buffer & ((1ULL << count) - 1);
    return result;
}

// This will peek and then consume 'count' bits in one go
//
U64 Stream_ReadBits(Stream_Context *stream, U64 count) {
    U64 result = Stream_PeekBits(stream, count);

    Stream_ConsumeBits(stream, count);
    return result;
}

//
// --------------------------------------------------------------------------------
// :impl_logging
// --------------------------------------------------------------------------------
//

#if !defined(LOG_CONTEXT_ARENA_SIZE)
    #define LOG_CONTEXT_ARENA_SIZE MB(64)
#endif

thread_static Log_Context *__thread_logger;

Str8 Log_StrFromLevel(S32 level) {
    Str8 result = S("Custom");

    switch (level) {
        case LOG_DEBUG: { result = S("Debug");   } break;
        case LOG_INFO:  { result = S("Info");    } break;
        case LOG_WARN:  { result = S("Warning"); } break;
        case LOG_ERROR: { result = S("Error");   } break;
        default: {} break;
    }

    return result;
}

void Log_Init() {
    if (__thread_logger == 0) {
        M_Arena *arena  = M_AllocArena(LOG_CONTEXT_ARENA_SIZE);

        __thread_logger = M_ArenaPush(arena, Log_Context);
        __thread_logger->arena = arena;

        // Push a default scope for logging messages to
        //
        Log_PushScope();
    }
}

void Log_PushScope() {
    U64 offset = M_GetArenaOffset(__thread_logger->arena);

    Log_Scope *scope = M_ArenaPush(__thread_logger->arena, Log_Scope);
    scope->offset    = offset;

    SLL_Push(__thread_logger->scopes, scope);
}

Log_MessageArray Log_PopScope(M_Arena *arena) {
    Log_MessageArray result = { 0 };

    Log_Scope *scope = __thread_logger->scopes;
    Log_MessageList *messages = &scope->messages;

    // Copy all of the messages into the result array, then
    // remove pop the logger arena position back to the offset
    //
    if (messages->num_messages != 0) {
        result.count = messages->num_messages;
        result.items = M_ArenaPush(arena, Log_Message, result.count);

        Log_Message *src = messages->first;
        for (U32 it = 0; it < result.count; ++it) {
            Log_Message *dst = &result.items[it];

            dst->next = dst + 1;

            dst->code = src->code;

            dst->file = Str8_Copy(arena, src->file);
            dst->func = Str8_Copy(arena, src->func);
            dst->line = src->line;

            dst->message = Str8_Copy(arena, src->message);

            src = src->next;
        }

        result.items[result.count - 1].next = 0;
    }

    __thread_logger->scopes = scope->next;
    M_ArenaPopTo(__thread_logger->arena, scope->offset);

    if (__thread_logger->scopes == 0) {
        // If all scopes have been popped, push a default scope
        // so there is always one valid
        //
        Log_PushScope();
    }

    return result;
}

void Log_PushMessageArgs(S32 code, Str8 file, U32 line, Str8 func, const char *format, va_list args) {
    Assert(__thread_logger != 0);

    Log_Message *node = M_ArenaPush(__thread_logger->arena, Log_Message);

    // @todo: we probably don't have to copy the file/func each time
    // because they are likely coming from the statically defined macro
    // varaibles
    //
    node->code = code;

    node->file = Str8_Copy(__thread_logger->arena, file);
    node->func = Str8_Copy(__thread_logger->arena, func);
    node->line = line;

    node->message = Str8_FormatArgs(__thread_logger->arena, format, args);

    // Pull the top scope and push the message onto its list
    //
    Log_MessageList *messages = &__thread_logger->scopes->messages;

    SLL_Enqueue(messages->first, messages->last, node);
    messages->num_messages += 1;
}

void Log_PushMessage(S32 code, Str8 file, U32 line, Str8 func, const char *format, ...) {
    va_list args;
    va_start(args, format);

    Log_PushMessageArgs(code, file, line, func, format, args);

    va_end(args);
}

//
// --------------------------------------------------------------------------------
// :impl_filesystem
// --------------------------------------------------------------------------------
//

#if OS_WINDOWS

//
// :win32_filesystem
//

internal WCHAR *Win32_WideFromStr8(M_Arena *arena, Str8 str) {
    int    length = MultiByteToWideChar(CP_UTF8, 0, cast(char *) str.data, cast(int) str.count, 0, 0);
    WCHAR *result = M_ArenaPush(arena, WCHAR, length);

    MultiByteToWideChar(CP_UTF8, 0, cast(char *) str.data, cast(int) str.count, result, length);
    return result;
}

internal Str8 Win32_Str8FromWide(M_Arena *arena, WCHAR *str) {
    Str8 result;

    result.count = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0) - 1;
    result.data  = M_ArenaPush(arena, U8, result.count + 1);

    WideCharToMultiByte(CP_UTF8, 0, str, -1, cast(char *) result.data, cast(int) (result.count + 1), 0, 0);
    return result;
}

internal void Win32_ListPathRecurse(M_Arena *arena, FS_List *list, Str8 path, FS_ListFlags flags) {
    M_Temp temp  = M_AcquireTemp(1, &arena);

    // we have to append the wildcard to search
    //
    Str8 search_path = Sf(temp.arena, "%.*s\\*", Sv(path));
    WCHAR *wpath     = Win32_WideFromStr8(temp.arena, search_path);

    B32 recurse        = (flags & FS_LIST_RECURSIVE)      != 0;
    B32 include_hidden = (flags & FS_LIST_INCLUDE_HIDDEN) != 0;

    WIN32_FIND_DATAW find_data;
    HANDLE find = FindFirstFileW(wpath, &find_data);

    while (find != INVALID_HANDLE_VALUE) {
        Str8 basename = Win32_Str8FromWide(temp.arena, find_data.cFileName);

        B32 should_skip = false;
        B32 is_hidden   = true;

        if (basename.count == 2 && basename.data[0] == '.' && basename.data[1] == '.') {
            // always skip relative ..
            //
            should_skip = true;
        }
        else if (basename.count && basename.data[0] == '.') {
            // always skip relative . or if not including hidden files skip "unix-style" hidden files
            // beginning with a .
            //
            should_skip = ((basename.count == 1) || !include_hidden);
        }
        else {
            is_hidden = (find_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
        }

        should_skip = should_skip || (!include_hidden && is_hidden);

        if (!should_skip) {
            // Create and fill out the entry for the list
            //
            FS_Entry *entry = M_ArenaPush(arena, FS_Entry);

            FILETIME *ct = &find_data.ftCreationTime;
            FILETIME *at = &find_data.ftLastAccessTime;
            FILETIME *wt = &find_data.ftLastWriteTime;

            B32 is_dir = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

            entry->path   = Sf(arena, "%.*s/%.*s", Sv(path), Sv(basename));
            entry->size   = Compose_U64(find_data.nFileSizeHigh, find_data.nFileSizeLow);
            entry->props |= is_hidden ? FS_PROPERTY_IS_HIDDEN    : 0;
            entry->props |= is_dir    ? FS_PROPERTY_IS_DIRECTORY : 0;

            entry->times.written  = Compose_U64(wt->dwHighDateTime, wt->dwLowDateTime);
            entry->times.accessed = Compose_U64(at->dwHighDateTime, at->dwLowDateTime);
            entry->times.created  = Compose_U64(ct->dwHighDateTime, ct->dwLowDateTime);

            SLL_Enqueue(list->first, list->last, entry);

            list->num_entries += 1;

            if (recurse && is_dir) { Win32_ListPathRecurse(arena, list, entry->path, flags); }
        }

        if (!FindNextFileW(find, &find_data)) {
            break;
        }
    }

    M_ReleaseTemp(temp);
}

FS_List FS_ListPath(M_Arena *arena, Str8 path, FS_ListFlags flags) {
    FS_List result = { 0 };

    Win32_ListPathRecurse(arena, &result, path, flags);
    return result;
}

OS_Handle FS_OpenFile(Str8 path, FS_Access access) {
    OS_Handle result = OS_NilHandle();

    DWORD dwAccess   = 0;
    DWORD dwCreation = OPEN_EXISTING;

    if (access & FS_ACCESS_READ) {
        dwAccess   |= GENERIC_READ;
        dwCreation  = OPEN_EXISTING;
    }

    if (access & FS_ACCESS_WRITE) {
        dwAccess   |= GENERIC_WRITE;
        dwCreation  = OPEN_ALWAYS;
    }

    // resolve relative path and convert to UTF-16
    //
    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    HANDLE hFile = CreateFileW(wpath, dwAccess, FILE_SHARE_READ, 0, dwCreation, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile != INVALID_HANDLE_VALUE) {
        result.v[0] = cast(U64) hFile;
    }
    else {
        Log_Error("Failed to open file '%.*s' (0x%x)", Sv(path), GetLastError());
    }

    M_ReleaseTemp(temp);

    return result;
}

void FS_CloseFile(OS_Handle file) {
    if (OS_HandleValid(file)) {
        HANDLE hFile = cast(HANDLE) file.v[0];
        CloseHandle(hFile);
    }
}

B32 FS_RemoveFile(Str8 path) {
    B32 result;

    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    result = DeleteFileW(wpath);
    if (!result) {
        Log_Error("Failed to delete file '%.*s' (0x%x)", Sv(path), GetLastError());
    }

    M_ReleaseTemp(temp);

    return result;
}

S64 FS_ReadFile(OS_Handle file, Str8 data, U64 offset) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        HANDLE hFile  = cast(HANDLE) file.v[0];
        U8 *ptr       = data.data;
        S64 remaining = data.count;

        OVERLAPPED overlapped = { 0 };
        overlapped.Offset     = cast(DWORD) (offset >>  0);
        overlapped.OffsetHigh = cast(DWORD) (offset >> 32);

        while (remaining > 0) {
            DWORD nread   = 0;
            DWORD to_read = SaturateCast_U32(remaining);

            if (ReadFile(hFile, ptr, to_read, &nread, &overlapped)) {
                remaining -= nread;
                ptr       += nread;
                offset    += nread;

                overlapped.Offset     = cast(DWORD) (offset >>  0);
                overlapped.OffsetHigh = cast(DWORD) (offset >> 32);
            }
            else {
                Log_Error("Failed to read %d bytes at offset %lld (0x%x)", to_read, offset, GetLastError());
                break;
            }
        }

        result = cast(S64) (ptr - data.data);
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

S64 FS_WriteFile(OS_Handle file, Str8 data, U64 offset) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        HANDLE hFile  = cast(HANDLE) file.v[0];
        U8 *ptr       = data.data;
        S64 remaining = data.count;

        OVERLAPPED overlapped = { 0 };
        overlapped.Offset     = cast(DWORD) (offset >>  0);
        overlapped.OffsetHigh = cast(DWORD) (offset >> 32);

        while (remaining > 0) {
            DWORD nwritten = 0;
            DWORD to_write = SaturateCast_U32(remaining);

            if (WriteFile(hFile, ptr, to_write, &nwritten, &overlapped)) {
                remaining -= nwritten;
                ptr       += nwritten;
                offset    += nwritten;

                overlapped.Offset     = cast(DWORD) (offset >>  0);
                overlapped.OffsetHigh = cast(DWORD) (offset >> 32);
            }
            else {
                Log_Error("Failed to write %d bytes at offset %lld (0x%x)", to_write, offset, GetLastError());
                break;
            }
        }

        result = cast(S64) (ptr - data.data);
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

S64 FS_AppendFile(OS_Handle file, Str8 data) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        HANDLE hFile = cast(HANDLE) file.v[0];

        FILE_STANDARD_INFO info;
        if (GetFileInformationByHandleEx(hFile, FileStandardInfo, &info, sizeof(FILE_STANDARD_INFO))) {
            U64 offset = info.EndOfFile.QuadPart;
            result     = FS_WriteFile(file, data, offset);
        }
        else {
            Log_Error("Failed to get end-of-file offset (0x%x)", GetLastError());
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

FS_Properties FS_PropertiesFromHandle(OS_Handle file) {
    FS_Properties result = 0;

    if (OS_HandleValid(file)) {
        HANDLE hFile = cast(HANDLE) file.v[0];

        BY_HANDLE_FILE_INFORMATION info;
        if (GetFileInformationByHandle(hFile, &info)) {
            DWORD attrs = info.dwFileAttributes;

            M_Temp temp   = M_AcquireTemp(0, 0);
            Str8 path     = FS_PathFromHandle(temp.arena, file);
            Str8 basename = Str8_GetBasename(path);

            result |= (attrs & FILE_ATTRIBUTE_DIRECTORY)          ? FS_PROPERTY_IS_DIRECTORY : 0;
            result |= (attrs & FILE_ATTRIBUTE_HIDDEN)             ? FS_PROPERTY_IS_HIDDEN    : 0;
            result |= (basename.count && basename.data[0] == '.') ? FS_PROPERTY_IS_HIDDEN    : 0;

            M_ReleaseTemp(temp);
        }
        else {
            Log_Error("Failed to get file handle information (0x%x)", GetLastError());
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

FS_Time FS_TimeFromHandle(OS_Handle file) {
    FS_Time result = { 0 };

    if (OS_HandleValid(file)) {
        HANDLE hFile = cast(HANDLE) file.v[0];

        FILETIME ct, at, wt;
        if (GetFileTime(hFile, &ct, &at, &wt)) {
            result.created  = Compose_U64(ct.dwHighDateTime, ct.dwLowDateTime);
            result.accessed = Compose_U64(at.dwHighDateTime, at.dwLowDateTime);
            result.written  = Compose_U64(wt.dwHighDateTime, wt.dwLowDateTime);
        }
        else {
            Log_Error("Failed to get file times (0x%x)", GetLastError());
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

Str8 FS_PathFromHandle(M_Arena *arena, OS_Handle file) {
    Str8 result = { 0 };

    if (OS_HandleValid(file)) {
        HANDLE hFile = cast(HANDLE) file.v[0];
        M_Temp temp  = M_AcquireTemp(1, &arena);

        // Push a 1KiB buffer as an initial guess, if that is too small the
        // FILE_NAME_INFO will contain how long we need to make the buffer so pop
        // last and retry with the correct length
        //
        DWORD dwBufferSize    = KB(1);
        U8 *lpFileInformation = M_ArenaPush(temp.arena, U8, dwBufferSize);

        if (!GetFileInformationByHandleEx(hFile, FileNameInfo, lpFileInformation, dwBufferSize)) {
            DWORD err = GetLastError();
            if (err == ERROR_MORE_DATA) {
                FILE_NAME_INFO *name_info = cast(FILE_NAME_INFO *) lpFileInformation;
                dwBufferSize = name_info->FileNameLength + sizeof(FILE_NAME_INFO) + 2;

                M_ArenaPopLast(temp.arena);

                lpFileInformation = M_ArenaPush(temp.arena, U8, dwBufferSize);
                if (!GetFileInformationByHandleEx(hFile, FileNameInfo, lpFileInformation, dwBufferSize)) {
                    Log_Error("Failed to get file handle name information (0x%x)", GetLastError());
                    lpFileInformation = 0;
                }
            }
            else {
                Log_Error("Failed to get file handle name information (0x%x)", err);
                lpFileInformation = 0;
            }
        }

        if (lpFileInformation) {
            // We got the filename
            //
            FILE_NAME_INFO *name_info = cast(FILE_NAME_INFO *) lpFileInformation;
            result = Win32_Str8FromWide(arena, name_info->FileName);
        }

        M_ReleaseTemp(temp);
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

U64 FS_SizeFromHandle(OS_Handle file) {
    U64 result = 0;

    if (OS_HandleValid(file)) {
        HANDLE hFile = cast(HANDLE) file.v[0];

        LARGE_INTEGER size;
        if (GetFileSizeEx(hFile, &size)) {
            result = size.QuadPart;
        }
        else {
            Log_Error("Failed to get file size (0x%x)", GetLastError());
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

FS_Properties FS_PropertiesFromPath(Str8 path) {
    FS_Properties result = 0;

    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    DWORD attrs = GetFileAttributesW(wpath);
    if (attrs != INVALID_FILE_ATTRIBUTES) {
        result |= (attrs & FILE_ATTRIBUTE_DIRECTORY) ? FS_PROPERTY_IS_DIRECTORY : 0;
        result |= (attrs & FILE_ATTRIBUTE_HIDDEN)    ? FS_PROPERTY_IS_HIDDEN    : 0;
    }
    else {
        Log_Error("Failed to get file attributes for '%.*s' (0x%x)", Sv(path), GetLastError());
    }

    M_ReleaseTemp(temp);

    return result;
}

FS_Time FS_TimeFromPath(Str8 path) {
    FS_Time result = { 0 };

    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (GetFileAttributesExW(wpath, GetFileExInfoStandard, &attrs)) {
        FILETIME *ct = &attrs.ftCreationTime;
        FILETIME *at = &attrs.ftLastAccessTime;
        FILETIME *wt = &attrs.ftLastWriteTime;

        result.created  = Compose_U64(ct->dwHighDateTime, ct->dwLowDateTime);
        result.accessed = Compose_U64(at->dwHighDateTime, at->dwLowDateTime);
        result.written  = Compose_U64(wt->dwHighDateTime, wt->dwLowDateTime);
    }
    else {
        Log_Error("Failed to get file attributes for '%.*s' (0x%x)", Sv(path), GetLastError());
    }

    M_ReleaseTemp(temp);

    return result;
}

U64 FS_SizeFromPath(Str8 path) {
    U64 result = 0;

    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (GetFileAttributesExW(wpath, GetFileExInfoStandard, &attrs)) {
        result = Compose_U64(attrs.nFileSizeHigh, attrs.nFileSizeLow);
    }
    else {
        Log_Error("Failed to get file attributes for '%.*s' (0x%x)", Sv(path), GetLastError());
    }

    M_ReleaseTemp(temp);

    return result;
}

B32 FS_CreateDirectory(Str8 path) {
    B32 result = true;

    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    U64 it = 0;
    while (wpath[it] != 0) {
        if (wpath[it] == L'/' || wpath[it] == L'\\') {
            // We temporarily null-terminate this segment to create parent directories
            //
            WCHAR sep = wpath[it];
            wpath[it] = L'\0';

            if (!CreateDirectoryW(wpath, 0)) {
                DWORD err = GetLastError();
                if (err != ERROR_ALREADY_EXISTS) {
                    Str8 dir = Win32_Str8FromWide(temp.arena, wpath);
                    Log_Error("Failed to create directory '%.*s' (0x%x)", Sv(dir), err);

                    result = false;
                    break;
                }
            }

            // restore the separator
            //
            wpath[it] = sep;
        }

        it += 1;
    }

    if (result) {
        // We successfully created all of the parent directories (or they already existed) so
        // create the final directory
        //
        if (!CreateDirectoryW(wpath, 0)) {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS) {
                DWORD attrs = GetFileAttributesW(wpath);
                if ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    Log_Error("Attempted to create directory at '%.*s', but a file was found", Sv(path));
                    result = false;
                }
            }
            else {
                Log_Error("Failed to create directory '%.*s' (0x%x)", Sv(path), err);
                result = false;
            }
        }
    }

    M_ReleaseTemp(temp);

    return result;
}

B32 FS_RemoveDirectory(Str8 path) {
    B32 result;

    M_Temp temp  = M_AcquireTemp(0, 0);
    WCHAR *wpath = Win32_WideFromStr8(temp.arena, path);

    result = RemoveDirectoryW(wpath);
    M_ReleaseTemp(temp);

    return result;
}

// :folderid_hack
//
#include <shlobj.h>

#pragma comment(lib, "shell32.lib") // SHGetKnownFolderPath
#pragma comment(lib, "ole32.lib")   // CoTaskMemFree

#if LANG_CPP
    #define __FOLDERID_RoamingAppData FOLDERID_RoamingAppData
#else
    #define __FOLDERID_RoamingAppData &FOLDERID_RoamingAppData
#endif

Str8 FS_GetPath(M_Arena *arena, FS_PathType type) {
    Str8 result = { 0 };

    // @todo: will likely have some sort of "init" function in the future where we can cache some
    // of this stuff and then just copy it directly rather than having to do the work every time
    //

    M_Temp temp = M_AcquireTemp(1, &arena);

    switch (type) {
        case FS_PATH_EXE: {
            WCHAR *lpFilename;
            DWORD  nSize = 512;

            DWORD err, count;
            for (;;) {
                // guess the buffer size, doubling the size each time it fails
                //
                nSize *= 2;
                lpFilename = M_ArenaPush(temp.arena, WCHAR, nSize);

                // get the module filename, this gives us the full executable path. if the buffer
                // isn't big enough ERROR_INSUFFICIENT_BUFFER will be set. as the system doesn't
                // actually tell us what the length of the buffer _should_ be we have to keep guessing
                //
                count = GetModuleFileNameW(0, lpFilename, nSize);
                err   = GetLastError();

                if (err != ERROR_INSUFFICIENT_BUFFER) { break; }

                M_ArenaPopLast(temp.arena);
            }

            if (err == ERROR_SUCCESS) {
                // successfully go the path, chop off the basename as it includes the full executable
                // filename
                //
                while (count && lpFilename[count] != L'\\') {
                    count -= 1;
                }

                lpFilename[count] = L'\0';

                result = Win32_Str8FromWide(arena, lpFilename);
            }
            else {
                Log_Error("Failed to get executable path (0x%x)", err);
            }
        }
        break;
        case FS_PATH_USER: {
            // this is just annoying, this function uses references in C++ mode and a pointer in C mode
            // thus we have to do this hacky work around using a define :folderid_hack
            //
            PWSTR ppszPath;
            HRESULT hr = SHGetKnownFolderPath(__FOLDERID_RoamingAppData, 0, 0, &ppszPath);
            if (hr == S_OK) {
                result = Win32_Str8FromWide(arena, ppszPath);
            }
            else {
                // @todo: maybe we should fallback to attempt to read the environment variable %APPDATA%
                // and return that
                //
                Log_Error("Failed to get path (hr = 0x%x)", hr);
            }

            CoTaskMemFree(ppszPath);
        }
        break;
        case FS_PATH_TEMP: {
            DWORD  nBufferSize = MAX_PATH + 1; // apparently this is the max possible return value
            WCHAR *lpBuffer    = M_ArenaPush(temp.arena, WCHAR, nBufferSize);

            // :note documentation says to use GetTempPath2W but this is only really available
            // in very new sdk versions. there isn't really any difference between them anyway
            // when we eventually switch to have an "init" function we can test for the existence
            // and fallback to GetTempPathW if not available
            //
            nBufferSize = GetTempPathW(nBufferSize, lpBuffer);
            if (nBufferSize > 0) {
                // for whatever reason the temp path has a trailing backslash so we remove it
                //
                lpBuffer[nBufferSize - 1] = 0;

                result = Win32_Str8FromWide(arena, lpBuffer);
            }
            else {
                Log_Error("Failed to get temporary path (0x%x)", GetLastError());
            }
        }
        break;
        case FS_PATH_WORKING: {
            // :threading this is not thread-safe if we have a way of setting the working directory
            // as we don't at the moment it doesn't really matter
            //
            DWORD nBufferLength = GetCurrentDirectoryW(0, 0);
            if (nBufferLength > 0) {
                WCHAR *lpBuffer = M_ArenaPush(temp.arena, WCHAR, nBufferLength);
                if (GetCurrentDirectoryW(nBufferLength, lpBuffer)) {
                    result = Win32_Str8FromWide(arena, lpBuffer);
                }
            }
            else {
                Log_Error("Failed to get working path (0x%x)", GetLastError());
            }
        }
        break;
    }

    M_ReleaseTemp(temp);

    return result;
}

#elif OS_MACOS
    #error "macOS filesystem subsystem not implemented"
#elif OS_LINUX

//
// :linux_filesystem
//

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <linux/limits.h>

#define LINUX_DENTS_BUFFER_SIZE 1024

typedef struct linux_dirent64 linux_dirent64_t;
struct linux_dirent64 {
    __ino64_t      d_ino;    /* 64-bit inode number */
    __off64_t      d_off;    /* Not an offset; see getdents() */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
};

internal void Linux_ListPathRecursive(M_Arena *arena, FS_List *list, Str8 path, FS_ListFlags flags) {
    M_Temp temp = M_AcquireTemp(1, &arena);
    Str8 zpath  = Str8_Copy(temp.arena, path);

    B32 recurse        = (flags & FS_LIST_RECURSIVE)      != 0;
    B32 include_hidden = (flags & FS_LIST_INCLUDE_HIDDEN) != 0;

    char *buffer = M_ArenaPush(temp.arena, char, LINUX_DENTS_BUFFER_SIZE);

    int fd = open((const char *) zpath.data, O_RDONLY | O_DIRECTORY);
    if (fd >= 0) {
        for (;;) {
            ssize_t nread = syscall(SYS_getdents64, fd, buffer, LINUX_DENTS_BUFFER_SIZE);
            if (nread < 0) {
                Log_Error("Failed to list directory '%.*s' (%d)", Sv(path), errno);
                break;
            }
            else if (nread == 0) {
                // done
                //
                break;
            }

            for (ssize_t off = 0; off < nread;) {
                linux_dirent64_t *ent = cast(linux_dirent64_t *) (buffer + off);
                off += ent->d_reclen;

                B32 should_skip = false;
                B32 is_hidden   = false;
                B32 is_dir      = (ent->d_type == DT_DIR);

                Str8 basename   = Sz(ent->d_name);

                if (basename.count == 2 && basename.data[0] == '.' && basename.data[1] == '.') {
                    // always skip relative '..' entry
                    //
                    should_skip = true;
                }
                else if (basename.count && basename.data[0] == '.') {
                    // always skip relative '.' or skip names starting with '.' if
                    // the user hasn't requested the inclusion of hidden files
                    //
                    should_skip = ((basename.count == 1) || !include_hidden);
                    is_hidden   = true;
                }

                if (!should_skip) {
                    FS_Entry *entry = M_ArenaPush(arena, FS_Entry);

                    entry->path = Sf(arena, "%.*s/%.*s", Sv(path), Sv(basename));

                    entry->props |= is_hidden ? FS_PROPERTY_IS_HIDDEN    : 0;
                    entry->props |= is_dir    ? FS_PROPERTY_IS_DIRECTORY : 0;

                    struct stat stbuf;
                    if (stat((const char *) entry->path.data, &stbuf) == 0) {
                        entry->size = stbuf.st_size;
                        entry->times.written  = (1e9 * stbuf.st_mtim.tv_nsec) + stbuf.st_mtim.tv_sec;
                        entry->times.accessed = (1e9 * stbuf.st_atim.tv_nsec) + stbuf.st_atim.tv_sec;
                        entry->times.created  = 0;
                    }

                    SLL_Enqueue(list->first, list->last, entry);

                    list->num_entries += 1;

                    if (recurse && is_dir) { Linux_ListPathRecursive(arena, list, entry->path, flags); }
                }
            }
        }

        close(fd);
    }
    else {
        Log_Error("Failed to open '%.*s' for reading (%d)", Sv(path), errno);
    }

    M_ReleaseTemp(temp);
}

FS_List FS_ListPath(M_Arena *arena, Str8 path, FS_ListFlags flags) {
    FS_List result = { 0 };

    Linux_ListPathRecursive(arena, &result, path, flags);
    return result;
}

OS_Handle FS_OpenFile(Str8 path, FS_Access access) {
    OS_Handle result = OS_NilHandle();

    if (access) {
        M_Temp temp = M_AcquireTemp(0, 0);
        Str8 zpath  = Str8_Copy(temp.arena, path);

        int flags = O_RDONLY;
        mode_t mode = 0;

        if (access & FS_ACCESS_WRITE) {
            flags = ((access & FS_ACCESS_READ) ? O_RDWR : O_WRONLY) | O_CREAT;
            mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 0644 permissions
        }

        int fd = open((const char *) zpath.data, flags, mode);
        if (fd >= 0) {
            result.v[0] = cast(U64) fd;
        }
        else {
            Log_Error("Failed to open file '%.*s' (%d)", Sv(path), errno);
        }

        M_ReleaseTemp(temp);
    }
    return result;
}

void FS_CloseFile(OS_Handle file) {
    if (OS_HandleValid(file)) {
        int fd = cast(int) file.v[0];
        close(fd);
    }
}

B32 FS_RemoveFile(Str8 path) {
    M_Temp temp = M_AcquireTemp(0, 0);
    Str8 zpath  = Str8_Copy(temp.arena, path);

    B32 result = unlink((const char *) zpath.data) == 0;

    M_ReleaseTemp(temp);
    return result;
}

S64 FS_ReadFile(OS_Handle file, Str8 data, U64 offset) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        int    fd         = cast(int) file.v[0];
        U8    *ptr        = data.data;
        ssize_t remaining = data.count;

        while (remaining > 0) {
            ssize_t nread = pread(fd, ptr, remaining, offset);
            if (nread >= 0) {
                remaining -= nread;
                ptr       += nread;
                offset    += nread;
            }
            else {
                Log_Error("Failed to read %llu bytes at offset %llu (%d)", remaining, offset, errno);
                break;
            }
        }

        Assert(remaining >= 0);

        result = cast(S64) (ptr - data.data);
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

S64 FS_WriteFile(OS_Handle file, Str8 data, U64 offset) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        int     fd        = cast(int) file.v[0];
        U8     *ptr       = data.data;
        ssize_t remaining = data.count;

        while (remaining > 0) {
            ssize_t nwritten = pwrite(fd, ptr, remaining, offset);
            if (nwritten >= 0) {
                remaining -= nwritten;
                ptr       += nwritten;
                offset    += nwritten;
            }
            else {
                Log_Error("Failed to read %llu bytes at offset %llu (%d)", remaining, offset, errno);
                break;
            }
        }

        Assert(remaining >= 0);

        result = cast(S64) (ptr - data.data);
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

S64 FS_AppendFile(OS_Handle file, Str8 data) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        int fd = cast(int) file.v[0];

        struct stat stbuf;
        if (fstat(fd, &stbuf) == 0) {
            U64 offset = stbuf.st_size;
            result     = FS_WriteFile(file, data, offset);
        }
        else {
            Log_Error("Failed to get end-of-file offset (%d)", errno);
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

FS_Properties FS_PropertiesFromHandle(OS_Handle file) {
    FS_Properties result = 0;

    if (OS_HandleValid(file)) {
        int fd = cast(int) file.v[0];

        struct stat stbuf;
        if (fstat(fd, &stbuf) == 0) {
            M_Temp temp = M_AcquireTemp(0, 0);
            Str8 path   = FS_PathFromHandle(temp.arena, file);

            B32 is_dir    = ((stbuf.st_mode & S_IFMT) == S_IFDIR);
            B32 is_hidden = (path.count && path.data[0] == '.');

            result |= is_dir    ? FS_PROPERTY_IS_DIRECTORY : 0;
            result |= is_hidden ? FS_PROPERTY_IS_HIDDEN    : 0;

            M_ReleaseTemp(temp);
        }
        else {
            Log_Error("Failed to get file descriptor information (%d)", errno);
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

FS_Time FS_TimeFromHandle(OS_Handle file) {
    FS_Time result = { 0 };

    if (OS_HandleValid(file)) {
        int fd = cast(int) file.v[0];

        struct stat stbuf;
        if (fstat(fd, &stbuf) == 0) {
            result.written  = (1e9 * stbuf.st_mtim.tv_sec) + stbuf.st_mtim.tv_nsec;
            result.accessed = (1e9 * stbuf.st_atim.tv_sec) + stbuf.st_mtim.tv_nsec;
            result.created  = 0; // not available on unix ?
        }
        else {
            Log_Error("fstat failed on '%d' (%d)", fd, errno);
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

Str8 FS_PathFromHandle(M_Arena *arena, OS_Handle file) {
    Str8 result = { 0 };

    if (OS_HandleValid(file)) {
        M_Temp temp = M_AcquireTemp(1, &arena);

        int  fd      = cast(int) file.v[0];
        Str8 fd_path = Sf(temp.arena, "/proc/self/fd/%d", fd);

        char *buffer;
        S64 size = 512;

        ssize_t count;
        for (;;) {
            // we have to keep iterating towards the required buffer size
            // because readlink wont actually tell us
            //
            size *= 2;
            buffer = M_ArenaPush(temp.arena, char, size);

            count = readlink((const char *) fd_path.data, buffer, size);
            if (count < size) { break; }

            M_ArenaPopLast(temp.arena);
        }

        if (count > 0) {
            result.count = count;
            result.data  = M_ArenaPushCopy(arena, buffer, U8, count + 1);
        }
        else {
            Log_Error("readlink failed on '%.*s' (%d)", Sv(fd_path), errno);
        }

        M_ReleaseTemp(temp);
    }

    return result;
}

U64 FS_SizeFromHandle(OS_Handle file) {
    S64 result = 0;

    if (OS_HandleValid(file)) {
        int fd = cast(int) file.v[0];

        struct stat stbuf;
        if (fstat(fd, &stbuf) == 0) {
            result = stbuf.st_size;
        }
        else {
            Log_Error("fstat failed on '%d' (%d)", fd, errno);
        }
    }
    else {
        Log_Error("Invalid file handle");
    }

    return result;
}

FS_Properties FS_PropertiesFromPath(Str8 path) {
    FS_Properties result = 0;

    M_Temp temp = M_AcquireTemp(0, 0);
    Str8 zpath  = Str8_Copy(temp.arena, path);

    struct stat stbuf;
    if (stat((const char *) zpath.data, &stbuf) == 0) {
        B32 is_dir    = ((stbuf.st_mode & S_IFMT) == S_IFDIR);
        B32 is_hidden = (path.count && path.data[0] == '.');

        result |= is_dir    ? FS_PROPERTY_IS_DIRECTORY : 0;
        result |= is_hidden ? FS_PROPERTY_IS_HIDDEN    : 0;
    }
    else {
        Log_Error("stat failed on path '%.*s' (%d)", Sv(path), errno);
    }

    M_ReleaseTemp(temp);

    return result;
}

FS_Time FS_TimeFromPath(Str8 path) {
    FS_Time result = { 0 };

    M_Temp temp = M_AcquireTemp(0, 0);
    Str8 zpath  = Str8_Copy(temp.arena, path);

    struct stat stbuf;
    if (stat((const char *) zpath.data, &stbuf) == 0) {
        result.written  = (1e9 * stbuf.st_mtim.tv_sec) + stbuf.st_mtim.tv_nsec;
        result.accessed = (1e9 * stbuf.st_atim.tv_sec) + stbuf.st_mtim.tv_nsec;
        result.created  = 0; // not available on unix ?
    }
    else {
        Log_Error("stat failed on path '%.*s' (%d)", Sv(path), errno);
    }

    M_ReleaseTemp(temp);

    return result;
}

U64 FS_SizeFromPath(Str8 path) {
    U64 result = 0;

    M_Temp temp = M_AcquireTemp(0, 0);
    Str8 zpath  = Str8_Copy(temp.arena, path);

    struct stat stbuf;
    if (stat((const char *) zpath.data, &stbuf) == 0) {
        result = stbuf.st_size;
    }
    else {
        Log_Error("stat failed on path '%.*s' (%d)", Sv(path), errno);
    }

    M_ReleaseTemp(temp);

    return result;
}

B32 FS_CreateDirectory(Str8 path) {
    B32 result = true;

    M_Temp temp = M_AcquireTemp(0, 0);
    Str8 cpy    = Str8_Copy(temp.arena, path);

    char *zpath = cast(char *) cpy.data;

    S64 it = 0;
    while (zpath[it]) {
        if (zpath[it] == '/') {
            zpath[it] = 0;

            mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; // 0755
            if (mkdir(zpath, mode) != 0) {
                if (errno != EEXIST) {
                    Log_Error("Failed to create directory '%s' (%d)", zpath, errno);

                    result = false;
                    break;
                }
            }

            zpath[it] = '/';
        }

        it += 1;
    }

    if (result) {
        mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH; // 0755
        if (mkdir(zpath, mode) != 0) {
            result = false;
            if (errno == EEXIST) {
                struct stat stbuf;
                if (stat(zpath, &stbuf) == 0) {
                    result = ((stbuf.st_mode & S_IFMT) == S_IFDIR);
                }
            }
        }
    }

    M_ReleaseTemp(temp);

    return result;
}

B32 FS_RemoveDirectory(Str8 path) {
    M_Temp temp = M_AcquireTemp(0, 0);
    Str8 zpath  = Str8_Copy(temp.arena, path);

    B32 result = rmdir((const char *) zpath.data) == 0;

    M_ReleaseTemp(temp);

    return result;
}

Str8 FS_GetPath(M_Arena *arena, FS_PathType type) {
    Str8 result = { 0 };

    M_Temp temp = M_AcquireTemp(1, &arena);

    switch (type) {
        case FS_PATH_EXE: {
            char *buffer;
            S64 size = 512;

            ssize_t count;
            for (;;) {
                // we have to keep iterating towards the required buffer size
                // because readlink wont actually tell us
                //
                size *= 2;
                buffer = M_ArenaPush(temp.arena, char, size);

                count = readlink("/proc/self/exe", buffer, size);
                if (count < size) { break; }

                M_ArenaPopLast(temp.arena);
            }

            if (count > 0) {
                while (buffer[count] != '/') { count -= 1; }

                buffer[count] = 0;

                result.count = count;
                result.data  = M_ArenaPushCopy(arena, buffer, U8, count + 1);
            }
            else {
                Log_Error("readlink failed on /proc/self/exe (%d)", errno);
            }
        }
        break;
        case FS_PATH_USER: {
            // @todo: this can be changed to use our own loaded environment variables
            // which we can get by reading /proc/self/environ on an init call
            //
            if (__environ) {
                Str8 home_dir = { 0 };
                Str8 xdg_dir  = { 0 };

                for (U32 it = 0; __environ[it] != 0; it += 1) {
                    Str8 env = Sz(__environ[it]);
                    if (Str8_Equal(env, S("XDG_DATA_HOME"), STR8_EQUAL_FLAG_INEXACT_RHS)) {
                        xdg_dir = Str8_RemoveBeforeFirst(env, '=');

                        // we only really need this, $HOME is a fallback if
                        // this is unset
                        //
                        break;
                    }
                    else if (Str8_Equal(env, S("HOME"), STR8_EQUAL_FLAG_INEXACT_RHS)) {
                        home_dir = Str8_RemoveBeforeFirst(env, '=');
                    }
                }

                if (xdg_dir.count || home_dir.count) {
                    if (!xdg_dir.count) {
                        // this is the defined default as per specifiction
                        //
                        result = Sf(arena, "%.*s/.local/share", Sv(home_dir));
                    }
                    else {
                        result = Str8_Copy(arena, xdg_dir);
                    }
                }
                else {
                    Log_Error("Failed to get user home directory");
                }
            }
            else {
                Log_Error("__environ variable was null");
            }
        }
        break;
        case FS_PATH_TEMP: {
            Str8 temp_dir = Sl("/tmp");

            if (__environ) {
                // look for $TEMP or $TMP set by user and use that if it is
                // set, otherwise we fallback to the globally available /tmp
                //
                for (U32 it = 0; __environ[it] != 0; it += 1) {
                    Str8 env = Sz(__environ[it]);
                    if (Str8_Equal(env, S("TEMP"), STR8_EQUAL_FLAG_INEXACT_RHS)) {
                        temp_dir = Str8_RemoveBeforeFirst(env, '=');
                        break;
                    }
                    else if (Str8_Equal(env, S("TMP"), STR8_EQUAL_FLAG_INEXACT_RHS)) {
                        temp_dir = Str8_RemoveBeforeFirst(env, '=');
                        break;
                    }
                }

            }

            result = Str8_Copy(arena, temp_dir);
        }
        break;
        case FS_PATH_WORKING: {
            size_t size  = PATH_MAX;
            char *buffer = M_ArenaPush(temp.arena, char, size);

            for (;;) {
                if (getcwd(buffer, size)) {
                    break;
                }
                else if (errno != ERANGE) {
                    Log_Error("Failed to get current directory (%d)", errno);

                    buffer = 0;
                    break;
                }

                M_ArenaPopLast(temp.arena);

                size  *= 2;
                buffer = M_ArenaPush(temp.arena, char, size);
            }

            if (buffer) { result = Str8_Copy(arena, Sz(buffer)); }
        }
        break;
        default: {} break;
    }

    M_ReleaseTemp(temp);

    return result;
}

#elif OS_SWITCH
    #error "switchbrew filesystem subsystem not implemented"
#endif

Str8 FS_ReadEntireFile(M_Arena *arena, Str8 path) {
    Str8 result = { 0 };

    OS_Handle file = FS_OpenFile(path, FS_ACCESS_READ);

    result.count = FS_SizeFromHandle(file);
    result.data  = M_ArenaPush(arena, U8, result.count);

    FS_ReadFile(file, result, 0);

    FS_CloseFile(file);

    return result;
}

#endif  // CORE_C_

#endif  // CORE_MODULE || CORE_IMPL
