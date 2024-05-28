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
//     - :utilities  | some basic utility functions
//     - :arena      | memory arena implementation
//     - :strings    | counted string helpers
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

// all return a pointer to the beginning of dst
//
function void *M_CopySize(void *dst, void *src, U64 size);
function void *M_FillSize(void *dst, U8 value, U64 size);
function void *M_ZeroSize(void *dst, U64 size);

function B32 M_CompareSize(void *a, void *b, U64 size);

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
function M_Temp M_GetTemp(U32 count, M_Arena **conflicts);

// release an acquired temporary arena
//
function void M_ReleaseTemp(M_Temp temp);

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
#define Ss(x) { sizeof(*(x)), (U8 *) (x) }
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

function Codepoint Utf8_Decode(Str8 str);
function U32 Utf8_Encode(U8 *output, U32 codepoint); // output expects enough space for the encoded codepoint

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
// :impl_utilities
// --------------------------------------------------------------------------------
//

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
#include <sys/mman.h>
#include <unistd.h>

#if !defined(MAP_ANON)
    #define MAP_ANON MAP_ANONYMOUS
#endif

void *OS_ReserveMemory(U64 size) {
    void *ptr = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

    void *result = (ptr == MAP_FAILED) ? 0 : ptr;
    return result;
}

B32 OS_CommitMemory(void *base, U64 size) {
    B32 result = mprotect(base, size, PROT_READ | PROT_WRITE) == 0;
    return result;
}

void OS_DecommitMemory(void *base, U64 size) {
    madvise(base, size, MADV_DONTNEED);
    mprotect(base, size, PROT_NONE);
}

void OS_ReleaseMemory(void *base, U64 size) {
    munmap(base, size);
}

U64 OS_GetPageSize() {
    U64 result = sysconf(_SC_PAGESIZE);
    return result;
}

U64 OS_GetAllocationGranularity() {
    U64 result = sysconf(_SC_PAGESIZE);
    return result;
}

#elif OS_SWITCH

// we unfortunately don't have any other options to allocate memory with switchbrew, malloc/free are
// the only userspace general address space allocators available to us
//
#include <stdlib.h>

void *OS_ReserveMemory(U64 size) {
    void *result = malloc(size);
    return result;
}

B32 OS_CommitMemory(void *base, U64 size) {
    (void) base;
    (void) size;

    // already committed the entire region with malloc so nothing to do
    //
    B32 result = true;
    return result;
}

void OS_DecommitMemory(void *base, U64 size) {
    (void) base;
    (void) size;

    // no decommit available because malloc/free so nothing to do
    //
}

void OS_ReleaseMemory(void *base, U64 size) {
    (void) size;

    free(base);
}

U64 OS_GetPageSize() {
    U64 result = 4096;
    return result;
}

U64 OS_GetAllocationGranularity() {
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

    U64 page_size   = OS_GetPageSize();
    U64 granularity = OS_GetAllocationGranularity();

    // have at least the allocation granularity to reserve and at least the page size to commit
    //
    U64 to_reserve = Max(AlignUp(limit, granularity), granularity);
    U64 to_commit  = Clamp(page_size, AlignUp(initial_commit, page_size), to_reserve);

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

            result->committed = to_commit;

            result->flags = flags;
        }
    }

    Assert(result != 0);

    return result;
}

M_Arena *M_AllocArenaArgs(U64 limit, U64 initial_commit, M_ArenaFlags flags) {
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

        OS_ReleaseMemory(base, size);
    }

    Assert(current == arena);
    Assert(current->committed >= M_ARENA_COMMIT_SIZE);

    void *decommit_base = cast(U8 *) current + M_ARENA_COMMIT_SIZE;
    U64   decommit_size = current->committed - M_ARENA_COMMIT_SIZE;

    if (decommit_size != 0) { OS_DecommitMemory(decommit_base, decommit_size); }

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

        OS_ReleaseMemory(base, size);
    }
}

void *M_ArenaPushFrom(M_Arena *arena, U64 size, M_ArenaFlags flags, U64 alignment) {
    void *result = 0;

    M_Arena *current = arena->current;

    alignment  = Clamp(1, alignment, 4096);
    U64 offset = AlignUp(current->offset, alignment);
    U64 end    = offset + size;

    if (end > current->limit) {
        // not enough space in current arena so allocate a new one if growing is permitted
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

        if (OS_CommitMemory(commit_base, commit_size)) { current->committed = commit_limit; }
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

        OS_ReleaseMemory(base, size);
    }

    U64 local_offset = Max(offset - current->base, M_ARENA_MIN_OFFSET);

    if (local_offset <= current->offset) {
        // as we have popped back explicitly the 'last_offset' is no longer valid so
        // set that to this offset thus calling 'pop last' after manually setting the offset
        // doesn't do anything
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

global_var thread_var M_Arena *__tls_temp[M_TEMP_ARENA_COUNT];

M_Temp M_GetTemp(U32 count, M_Arena **conflicts) {
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

// @todo: probably want to move away from vsnprintf to allow us to have custom type printing etc.
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
        // not enough space in the initial guess so pop the entire buffer, push the correct size and
        // process format again
        //
        M_ArenaPopLast(arena);

        result.count = count;
        result.data  = M_ArenaPush(arena, U8, result.count + 1);

        Str8_ProcessFormat(result.data, result.count + 1, format, copy);
    }
    else if (count < 0) {
        // error occurred
        //
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
    Str8 result = Sl("."); // if a path separator isn't found, use 'current working directory'

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

// return true if successfully encoded/decoded, false if an error occurred
//
Codepoint Utf8_Decode(Str8 str) {
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

        result.value = codepoint;
        result.count = avail; // don't go past the end of the string, may produce invalid codepoints
    }
    else {
        result.value = '?';
        result.count =  1 ;
    }

    return result;
}

U32 Utf8_Encode(U8 *output, U32 codepoint) {
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

#endif  // CORE_C_

#endif  // CORE_MODULE || CORE_IMPL
