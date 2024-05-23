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

#if defined(__cplusplus)
}
#endif

#endif  // CORE_H_

#endif  // !CORE_IMPL
