// for coverage in formatting
// #define STR8_INITIAL_FORMAT_GUESS_SIZE 16
//
// @todo: build a better test system, this is kinda bad

#define CORE_MODULE
#include "core.h"

#include <stdio.h>

typedef struct ListNode ListNode;
struct ListNode {
    ListNode *next;
    ListNode *prev;

    int value;
};

#define LogAssert(exp) do { \
    if (!(exp)) { \
        Log_Error("%s", #exp); \
        printf("... failed\n"); \
    } \
    else { \
        printf("... passed\n"); \
    } \
} while (0)

#define ExpectIntValue(a, v) printf("    Testing... %s == %llu", #a, (U64) v); LogAssert((a) == (v));
#define ExpectFloatValue(a, v) printf("    Testing... %s == %f", #a, v); LogAssert((a) == (v));
#define ExpectStrValue(a, v) printf("    Testing... %s == %s", #a, v); LogAssert(Str8_Equal(a, Sz(v), 0));
#define ExpectTrue(a) printf("    Testing... %s == true", #a); LogAssert(a);
#define ExpectFalse(a) printf("    Testing... %s == false", #a); LogAssert(!(a));

internal COMPARE_FUNC(CompareInt) {
    int ai = *(int *) a;
    int bi = *(int *) b;

    return (ai - bi);
}

internal COMPARE_FUNC(CompareListNode) {
    ListNode *node_a = cast(ListNode *) a;
    ListNode *node_b = cast(ListNode *) b;

    return (node_a->value - node_b->value);
}

static int ExecuteTests(int argc, char **argv) {
    // ... do nothing for now
    //
    (void) argc;
    (void) argv;

    Log_Init();

    // Basic platform macros
    //

    printf("-- Basic information\n");
    printf("  OS       : %s\n", OS_WINDOWS ? "Windows" : OS_MACOS ? "macOS" : OS_LINUX ? "Linux" : "Switchbrew");
    printf("  Compiler : %s\n", COMPILER_MSVC ? "cl.exe" : COMPILER_CLANG ? "clang" : "gcc");
    printf("  Arch     : %s\n", ARCH_AMD64 ? "amd64" : "aarch64");
    printf("  Language : %s\n", LANG_CPP ? "C++" : "C");

    printf("\n");

    // Utility macros
    //
    printf("-- Utility macros\n");
    {
        U32 arr[12];

        ExpectIntValue(cast(U32) 1.234f, 1);
        ExpectIntValue(ArraySize(arr), 12);
        ExpectIntValue(OffsetTo(ListNode, prev), 8);
        ExpectIntValue(AlignOf(U32), 4);
    }
    printf("\n");

    // Linked list macro testing
    //
    printf("-- Linked List macros\n");
    {
        ListNode nodes[10];
        for (U32 it = 0; it < ArraySize(nodes); ++it) {
            nodes[it].value = it;
        }

        // Queue
        //
        ListNode *head = 0;
        ListNode *tail = 0;

        SLL_Enqueue(head, tail, &nodes[5]);
        SLL_EnqueueN(head, tail, &nodes[2], next);

        SLL_EnqueueFront(head, tail, &nodes[4]);
        SLL_EnqueueFrontN(head, tail, &nodes[3], next);

        S32 queue_expect[] = { 3, 4, 5 };

        printf("Queue : \n");
        for (int it = 0; head->next != 0; it += 1) {
            ListNode *n = head;
            SLL_Dequeue(head, tail);

            ExpectIntValue(n->value, queue_expect[it]);
        }

        ExpectIntValue(head->value, 2);

        SLL_DequeueN(head, tail, next);

        Assert(head == 0 && tail == 0);
        printf("queue is now empty\n\n");

        // Stack
        //
        S32 stack_expect[] = { 5, 6, 2 };

        SLL_Push(head,  &nodes[7]);
        SLL_PushN(head, &nodes[2], next);
        SLL_PushN(head, &nodes[6], next);
        SLL_Push(head,  &nodes[5]);

        printf("Stack :\n");
        for (int it = 0; head->next != 0; it += 1) {
            ListNode *n = head;
            SLL_PopN(head, next);

            ExpectIntValue(n->value, stack_expect[it]);
        }

        ExpectIntValue(head->value, 7);

        SLL_Pop(head);

        Assert(head == 0);
        printf("stack is now empty\n\n");

        printf("DList :\n");

        S32 dlist_expect[] = { 8, 4, 3, 6, 7, 2 };
        S32 dlist_expect_after[] = { 8, 3, 6, 7 };

        DLL_InsertFront(head, tail, &nodes[4]);
        DLL_InsertFrontNP(head, tail, &nodes[8], next, prev);

        DLL_InsertBack(head, tail, &nodes[6]);
        DLL_InsertBackNP(head, tail, &nodes[2], next, prev);

        DLL_Insert(head, tail, &nodes[4], &nodes[3]);
        DLL_InsertNP(head, tail, &nodes[6], &nodes[7], next, prev);

        int x = 0;
        for (ListNode *n = head; n != 0; n = n->next) {
            ExpectIntValue(n->value, dlist_expect[x]);
            x += 1;
        }

        DLL_Remove(head, tail, &nodes[4]);
        DLL_RemoveNP(head, tail, &nodes[2], next, prev);

        printf("After removal :\n");
        x = 0;
        for (ListNode *n = head; n != 0; n = n->next) {
            ExpectIntValue(n->value, dlist_expect_after[x]);
            x += 1;
        }
    }
    printf("\n");

    printf("-- Intrinsics\n");
    {
        U32 v0 = 0x78;
        U64 v1 = 0x1F800;

        ExpectIntValue(CountLeadingZeros_U32(v0), 25);
        ExpectIntValue(CountLeadingZeros_U64(v1), 47);

        ExpectIntValue(CountTrailingZeros_U32(v0), 3);
        ExpectIntValue(CountTrailingZeros_U64(v1), 11);

        U32 v2 = 0xFF00FF00;
        U64 v3 = 0x00FF00FF;

        ExpectIntValue(RotateLeft_U32(v2, 5), 0xE01FE01F);
        ExpectIntValue(RotateLeft_U64(v3, 17), 0x1FE01FE0000);

        ExpectIntValue(RotateRight_U32(v2, 5), 0x7F807F8);
        ExpectIntValue(RotateRight_U64(v3, 17), 0x807F80000000007F);

        U32 v4 = 0x26B5;
        U64 v5 = 0x240C1C6222;

        ExpectIntValue(PopCount_U32(v4), 8);
        ExpectIntValue(PopCount_U64(v5), 12);
    }
    printf("\n");

    printf("-- Atomics\n");
    {
        // This isn't really a test for the atomics as there isn't any thread contention but
        // proves the produce correct results and as they are just using the compiler intrinsics
        // they are likely produce the correct interlocked instructions
        //

        volatile U32 v0 = 100;
        U32 v1 = AtomicAdd_U32(&v0, 25);

        ExpectIntValue(v0, 125);
        ExpectIntValue(v1, 100);

        volatile U64 v2 = 44000 | (1ULL << 35);
        U64 v3 = AtomicAdd_U64(&v2, 444);

        ExpectIntValue(v2, 44444 | (1ULL << 35));
        ExpectIntValue(v3, 44000 | (1ULL << 35));

        U32 v4 = AtomicExchange_U32(&v0, 1000);

        ExpectIntValue(v0, 1000);
        ExpectIntValue(v4, 125);

        U64 v5 = AtomicExchange_U64(&v2, 4344);

        ExpectIntValue(v2, 4344);
        ExpectIntValue(v5, 44444 | (1ULL << 35));

        void *v6 = (void *) 0x3939393939;
        void *v7 = AtomicExchange_Ptr(&v6, (void *) 0x4545454545);

        ExpectIntValue((U64) v6, 0x4545454545);
        ExpectIntValue((U64) v7, 0x3939393939);

        volatile U32 v8 = 10;

        ExpectTrue(AtomicCompareExchange_U32(&v8, 100, 10));
        ExpectFalse(AtomicCompareExchange_U32(&v8, 20,  10));
        ExpectIntValue(v8, 100);

        volatile U64 v9 = 2020;
        ExpectTrue(AtomicCompareExchange_U64(&v9, 4040, 2020));
        ExpectFalse(AtomicCompareExchange_U64(&v9, 10,   2020));
        ExpectIntValue(v9, 4040);

        void *v10 = (void *) 0x10101010;

        ExpectTrue(AtomicCompareExchange_Ptr(&v10, (void *) 0x20202020, (void *) 0x10101010));
        ExpectFalse(AtomicCompareExchange_Ptr(&v10, (void *) 0x30303030, (void *) 0x10101010));
        ExpectIntValue((U64) v10, 0x20202020);
    }
    printf("\n");

    printf("-- Utilities\n");
    {
        U64 v0 = cast(U64) U32_MAX + 3333333333ULL;
        U32 v1 = cast(U32) U16_MAX + 333333ULL;
        U16 v2 = cast(U64) U8_MAX  + 124ULL;

        ExpectIntValue(SaturateCast_U32(v0), U32_MAX);
        ExpectIntValue(SaturateCast_U16(v1), U16_MAX);
        ExpectIntValue(SaturateCast_U8 (v2), U8_MAX);

        U64 v3 = 333;
        U32 v4 = 333;
        U16 v5 = 124;

        ExpectIntValue(SafeTruncate_U32(v3), 333);
        ExpectIntValue(SafeTruncate_U16(v4), 333);
        ExpectIntValue(SafeTruncate_U8 (v5), 124);

        // these will all fail with an assertion
        //
        // SafeTruncate_U32(v0);
        // SafeTruncate_U16(v1);
        // SafeTruncate_U8(v2);
        //

        ExpectFloatValue(SafeRatio0_F32(1, 4), 0.25f);
        ExpectFloatValue(SafeRatio0_F64(1, 4), 0.25);

        ExpectFloatValue(SafeRatio1_F32(1, 4), 0.25f);
        ExpectFloatValue(SafeRatio1_F64(1, 4), 0.25);

        ExpectFloatValue(SafeRatio0_F32(1, 0), 0.0f);
        ExpectFloatValue(SafeRatio0_F64(1, 0), 0.0);

        ExpectFloatValue(SafeRatio1_F32(1, 0), 1.0f);
        ExpectFloatValue(SafeRatio1_F64(1, 0), 1.0);

        ExpectIntValue(NextPow2_U32(135), 256);
        ExpectIntValue(NextPow2_U64((1ULL << 33ULL) - 1), (1ULL << 33ULL));

        ExpectIntValue(PrevPow2_U32(4519), 4096);
        ExpectIntValue(PrevPow2_U64((1ULL << 35ULL) - 1), (1ULL << 34ULL));

        ExpectIntValue(NearestPow2_U32(3049), 2048);
        ExpectIntValue(NearestPow2_U64((1ULL << 35ULL) - 1), (1ULL << 35ULL));

        U32 s = 10, d = 0;

        M_CopySize(&d, &s, sizeof(U32));
        ExpectIntValue(d, 10);

        M_FillSize(&d, 0xFF, sizeof(U32));
        ExpectIntValue(d, U32_MAX);

        M_ZeroSize(&d, sizeof(U32));
        ExpectIntValue(d, 0);

        int some_array[]  = {
            4,   41,  86, 100, 100,
            32,  48,  48, 84,  31,
            56,  70,  81, 44,  32,
            44,  5,   18, 23,  22,
            10,  90,  8,  99,  84,
            9,   67,  32, 37,  12,
            56,  36,  48, 82,  49,
            99,  65,  91, 91,  80,
            78,  31,  32, 11,  69,
            20,  33,  2,  1,   0,
            4,   8,   2,  3,   4
        };

        int other_array[] = {
            93,  100, 11, 14,  12,
            87,  90,  56, 79,  100,
            77,  55,  19, 92,  91,
            9,   68,  70, 58,  48,
            50,  12,  43, 47,  86,
            55,  72,  72, 7,   65,
            1,   45,  32, 25,  71,
            86,  14,  81, 6,   90,
            36,  74,  61, 27,  1,
            31,  38,  39, 83,  76,
            100, 91,  58, 38,  88,
        };


        printf("    Unsorted: { ");

        for (U64 it = 0; it < ArraySize(some_array); ++it) {
            printf("%d ", some_array[it]);
        }

        printf("}\n");

        MergeSort(some_array, ArraySize(some_array), CompareInt);

        printf("    Sorted:   { ");

        U32 total = 0;
        for (U64 it = 0; it < ArraySize(some_array); ++it) {
            printf("%d ", some_array[it]);

            if (it != 0) { total += (some_array[it - 1] <= some_array[it]); }
        }

        printf("}\n");

        ExpectIntValue(total, ArraySize(some_array) - 1);

        printf("    Unsorted: { ");

        for (U64 it = 0; it < ArraySize(other_array); ++it) {
            printf("%d ", other_array[it]);
        }

        printf("}\n");

        QuickSort(other_array, ArraySize(other_array), CompareInt);

        printf("    Sorted:   { ");

        total = 0;
        for (U64 it = 0; it < ArraySize(other_array); ++it) {
            printf("%d ", other_array[it]);

            if (it != 0) { total += (other_array[it - 1] <= other_array[it]); }
        }

        printf("}\n");

        ExpectIntValue(total, ArraySize(other_array) - 1);

        ListNode nodes[10];

        nodes[0].value = 22;
        nodes[1].value = 9302;
        nodes[2].value = 2;
        nodes[3].value = 39;
        nodes[4].value = 190;
        nodes[5].value = 0;
        nodes[6].value = 459;
        nodes[7].value = 387;
        nodes[8].value = 548;
        nodes[9].value = 879;

        printf("Before sort:\n");
        for (U32 it = 0; it < 10; ++it) {
            printf("    node[%d].value = %d\n", it, nodes[it].value);
        }

        QuickSort(nodes, ArraySize(nodes), CompareListNode);

        printf("After sort:\n");
        for (U32 it = 0; it < 10; ++it) {
            printf("    node[%d].value = %d\n", it, nodes[it].value);
        }

        // Byte/bit swapping
        //
        U8  x8  = 0xF0;
        U16 x16 = 0xFF00;
        U32 x32 = 0xFFFF0000;
        U64 x64 = 0xFF00FF00FF00FF00;

        ExpectIntValue(SwapBytes_U16(x16), 0x00FF);
        ExpectIntValue(SwapBytes_U32(x32), 0x0000FFFF);
        ExpectIntValue(SwapBytes_U64(x64), 0x00FF00FF00FF00FF);

        ExpectIntValue(ReverseBits_U8(x8),   0x0F);
        ExpectIntValue(ReverseBits_U16(x16), 0x00FF);
        ExpectIntValue(ReverseBits_U32(x32), 0x0000FFFF);
        ExpectIntValue(ReverseBits_U64(x64), 0x00FF00FF00FF00FF);
    }
    printf("\n");

    printf("-- Arena\n");
    {
        M_Arena *arena = M_AllocArena(GB(8));

        ExpectIntValue(arena->committed, M_ARENA_COMMIT_SIZE);

        U32 *single = M_ArenaPush(arena, U32);
        ExpectIntValue(arena->offset, 68);

        single[0] = 22;

        U32 *array = M_ArenaPush(arena, U32, 32);
        ExpectIntValue(arena->offset, 196);

        for (U32 it = 0; it < 32; ++it) {
            array[it] = it;
        }

        M_ArenaPop(arena, U32, 32);
        ExpectIntValue(arena->offset, 68);

        U32 *arrayflag = M_ArenaPush(arena, U32, 32, M_ARENA_NO_ZERO);

        for (U32 it = 0; it < 32; ++it) {
            ExpectIntValue(arrayflag[it], it);
        }

        M_ArenaPop(arena, U32, 32);

        U32 *arrayalign = M_ArenaPush(arena, U32, 32, 0, 8);

        ExpectIntValue((U64) arrayalign & 7, 0);
        ExpectIntValue(arena->offset, 200);

        M_ArenaPopLast(arena);
        ExpectIntValue(arena->offset, 68);

        M_ArenaPop(arena, U32);

        ExpectIntValue(arena->offset, 64);

        U32 *a = M_ArenaPush(arena, U32, 10);

        for (U32 it = 0; it < 10; ++it) { a[it] = it; }

        U32 *b = M_ArenaPushCopy(arena, a, U32, 10);

        ExpectTrue(M_CompareSize(a, b, 10 * sizeof(U32)));

        M_Temp tempa = M_AcquireTemp(0, 0);
        M_Temp tempb = M_AcquireTemp(1, &tempa.arena);

        ExpectTrue(tempa.arena != tempb.arena);

        M_ArenaPush(tempa.arena, U32, 256);
        M_ReleaseTemp(tempa);

        // don't do this in production code, just testing to make sure the
        // arena was reset correctly
        //
        ExpectIntValue(tempa.arena->offset, 64);

        M_ResetArena(arena);
        M_ReleaseArena(arena);

        // should cause an access violation
        //
        // printf("current offset = %llu\n", arena->offset);
    }
    printf("\n");

    printf("-- Strings\n");
    {
        Str8 test = S("Some/path/with/file.txt");

        Str8 before_first = Str8_RemoveBeforeFirst(test, '/');
        Str8 before_last  = Str8_RemoveBeforeLast(test, '/');

        Str8 after_first = Str8_RemoveAfterFirst(test, '/');
        Str8 after_last  = Str8_RemoveAfterLast(test, '/');

        ExpectStrValue(before_first, "path/with/file.txt");
        ExpectStrValue(before_last,  "file.txt");
        ExpectStrValue(after_first,  "Some");
        ExpectStrValue(after_last,   "Some/path/with");

        Str8 dirname  = Str8_GetDirname(test);
        Str8 basename = Str8_GetBasename(test);
        Str8 ext      = Str8_GetExtension(test);

        Str8 noext = Str8_StripExtension(basename);

        ExpectStrValue(dirname,  "Some/path/with");
        ExpectStrValue(basename, "file.txt");
        ExpectStrValue(ext,      "txt");
        ExpectStrValue(noext,    "file");

        Str8 hira_a = Sl("\xE3\x81\x82");

        Codepoint c = UTF8_Decode(hira_a);

        ExpectIntValue(c.count, 3);
        ExpectIntValue(c.value, 0x3042);

        U8 value[4] = { 0 };
        U32 count = UTF8_Encode(value, c.value);

        ExpectIntValue(count, 3);
        ExpectIntValue(*(U32 *) value, 0x8281e3);

        M_Temp temp = M_AcquireTemp(0, 0);
        Str8 cpy = Str8_Copy(temp.arena, test);
        ExpectStrValue(cpy, "Some/path/with/file.txt");

        Str8 con = Str8_Concat(temp.arena, noext, ext);
        ExpectStrValue(con, "filetxt");

        Str8 range = Str8_WrapRange(dirname.data, dirname.data + 4);
        ExpectStrValue(range, "Some");

        Str8 formatted = Sf(temp.arena, "Hello, %s! %d With a REALLY LONG ENDING", "Sailor", 69105);
        ExpectStrValue(formatted, "Hello, Sailor! 69105 With a REALLY LONG ENDING");

        Str8 advanced = Str8_Advance(test, 5);
        Str8 removed  = Str8_Remove(test,  9);
        Str8 sliced   = Str8_Slice(test, 5, 9);

        ExpectStrValue(advanced, "path/with/file.txt");
        ExpectStrValue(removed,  "Some/path/with");
        ExpectStrValue(sliced,   "path");

        M_ReleaseTemp(temp);
    }
    printf("\n");

    printf("-- Characters\n");
    {
        ExpectTrue(Chr_IsWhitespace(' '));
        ExpectTrue(Chr_IsWhitespace('\n'));
        ExpectFalse(Chr_IsWhitespace('d'));

        ExpectTrue(Chr_IsAlpha('a'));
        ExpectTrue(Chr_IsAlpha('p'));
        ExpectTrue(Chr_IsAlpha('I'));

        ExpectFalse(Chr_IsAlpha(';'));
        ExpectFalse(Chr_IsAlpha(','));

        ExpectTrue(Chr_IsHex('0'));
        ExpectTrue(Chr_IsHex('7'));
        ExpectTrue(Chr_IsHex('a'));
        ExpectTrue(Chr_IsHex('F'));

        ExpectFalse(Chr_IsHex('i'));
        ExpectFalse(Chr_IsHex('p'));

        ExpectTrue(Chr_IsSlash('/'));
        ExpectTrue(Chr_IsSlash('\\'));

        ExpectTrue(Chr_IsPathSeparator('/'));
#if OS_WINDOWS
        ExpectTrue(Chr_IsPathSeparator('\\'));
#endif

        ExpectIntValue(Chr_ToUppercase('a'), 'A');
        ExpectIntValue(Chr_ToLowercase('A'), 'a');
    }
    printf("\n");

    printf("-- Logging\n");
    {
        Log_PushScope();

        Log_Debug("Hello, %d", 69105);
        Log_Info("Other INFO");

        Log_Warn("WARNING SOMETHING ISN'T RIGHT!");

        Log_Error("Error failed");

        const char *expected_messages[] = {
            ("Hello, 69105"),
            ("Other INFO"),
            ("WARNING SOMETHING ISN'T RIGHT!"),
            ("Error failed")
        };

        M_Temp temp = M_AcquireTemp(0, 0);
        Log_MessageArray messages = Log_PopScope(temp.arena);

        ExpectTrue(messages.count == ArraySize(expected_messages));

        for (U32 it = 0; it < messages.count; ++it) {
            Log_Message *n = &messages.items[it];
            ExpectStrValue(n->message, expected_messages[it]);
            ExpectIntValue(n->code, cast(S32) it - 4); // LOG_* codes are negative
        }

        ExpectStrValue(Log_StrFromLevel(LOG_ERROR), "Error");

        M_ReleaseTemp(temp);
    }
    printf("\n");

    printf("-- Stream\n");
    {
        U8 values[] = { 0, 1, 2, 3, 4 };

        Stream_Context zstream;
        Stream_Zero(&zstream);

        U32 *zero = Stream_Read(&zstream, U32);
        ExpectIntValue(zero[0], 0);

        Stream_Context mstream;

        Str8 data;
        data.count = ArraySize(values);
        data.data  = values;

        Stream_FromMemory(&mstream, data);

        U32 x = 0;
        while (mstream.pos != mstream.end) {
            ExpectIntValue(mstream.pos[0], values[x]);
            mstream.pos += 1;
            x += 1;
        }

        U32 bit_pattern = 0xCACACACA;

        data.count = sizeof(bit_pattern);
        data.data  = cast(U8 *) &bit_pattern;

        Stream_FromMemory(&mstream, data);

        for (U32 it = 0; it < 32; it += 4) {
            U64 bits = Stream_ReadBits(&mstream, 4);

            ExpectIntValue(bits, (U32) (((it >> 2) & 1) ? 0xC : 0xA));
        }

        ExpectIntValue(mstream.bit_buffer, 0);
        ExpectIntValue(mstream.bit_count,  0);
    }
    printf("\n");

    printf("-- File System\n");
    {
        M_Temp temp = M_AcquireTemp(0, 0);

        // system paths
        //
        Str8 exe_path     = FS_GetPath(temp.arena, FS_PATH_EXE);
        Str8 user_path    = FS_GetPath(temp.arena, FS_PATH_USER);
        Str8 temp_path    = FS_GetPath(temp.arena, FS_PATH_TEMP);
        Str8 working_path = FS_GetPath(temp.arena, FS_PATH_WORKING);

        printf("    exe     path = %.*s\n", Sv(exe_path));
        printf("    user    path = %.*s\n", Sv(user_path));
        printf("    temp    path = %.*s\n", Sv(temp_path));
        printf("    working path = %.*s\n", Sv(working_path));

        // directory listing
        //
        FS_List list = FS_ListPath(temp.arena, exe_path, FS_LIST_RECURSIVE);

        printf("    list of %.*s:\n", Sv(exe_path));

        U32 it = 0;
        for (FS_Entry *e = list.first; e != 0; e = e->next) {
            printf("      [%2d] = %.*s%c\n", it, Sv(e->path), (e->props & FS_PROPERTY_IS_DIRECTORY) ? '*' : ' ');
            it += 1;
        }

        // writing a file
        //
        OS_Handle file = FS_OpenFile(S("test.txt"), FS_ACCESS_WRITE);

        U64 offset = 0;

        offset += FS_WriteFile(file, S("Hello, World! | "), offset);
        offset += FS_WriteFile(file, S("Hello, World! | "), offset);
        offset += FS_WriteFile(file, S("Hello, World! | "), offset);

        FS_AppendFile(file, S("End of file"));

        ExpectIntValue(FS_PropertiesFromHandle(file), 0);
        ExpectIntValue(FS_SizeFromHandle(file), 59); // should be 59 bytes long after writing

        Str8 path = FS_PathFromHandle(temp.arena, file);
        Str8 basename = Str8_GetBasename(path);
        printf("    full path from handle is %.*s\n", Sv(path));
        ExpectStrValue(basename, "test.txt");

        FS_Time times = FS_TimeFromHandle(file);

        printf("    times from handle:\n");
        printf("      written:  %llu\n", times.written);
        printf("      accessed: %llu\n", times.accessed);
        printf("      created:  %llu\n", times.created);

        FS_CloseFile(file);

        // reading a file
        //
        Str8 contents = FS_ReadEntireFile(temp.arena, S("test.txt"));
        ExpectStrValue(contents, "Hello, World! | Hello, World! | Hello, World! | End of file");

        ExpectTrue(FS_CreateDirectory(S("test_dir")));

        // from path
        //
        ExpectIntValue(FS_PropertiesFromPath(S("test_dir")), FS_PROPERTY_IS_DIRECTORY);
        ExpectIntValue(FS_SizeFromPath(S("test.txt")), 59); // still 59 bytes after reading

        times = FS_TimeFromPath(S("test_dir"));

        printf("    times from path:\n");
        printf("      written:  %llu\n", times.written);
        printf("      accessed: %llu\n", times.accessed);
        printf("      created:  %llu\n", times.created);

        // cleanup
        //
        ExpectTrue(FS_RemoveFile(S("test.txt")));
        ExpectTrue(FS_RemoveDirectory(S("test_dir")));

        M_ReleaseTemp(temp);
    }
    printf("\n");

    printf("-- Leak\n");
    {
        // this will catch any leaked temporary memory calls
        //
        M_Temp a = M_AcquireTemp(0, 0);
        M_Temp b = M_AcquireTemp(1, &a.arena);

        ExpectIntValue(a.arena->offset, M_ARENA_MIN_OFFSET);
        ExpectIntValue(b.arena->offset, M_ARENA_MIN_OFFSET);
    }
    printf("\n");

    {
        M_Temp temp = M_AcquireTemp(0, 0);

        Log_MessageArray messages = Log_PopScope(temp.arena);
        if (messages.count != 0) {
            printf("[some tests failed]\n");

            for (U32 it = 0; it < messages.count; ++it) {
                Log_Message *msg = &messages.items[it];
                if (msg->code == LOG_ERROR) {
                    printf("  %.*s line %d failed: %.*s\n", Sv(msg->func), msg->line, Sv(msg->message));
                }
            }
        }
        else {
            printf("[all tests passed successfully]\n");
        }

        M_ReleaseTemp(temp);
    }

    return 0;
}

int main(int argc, char **argv) {
    int result = ExecuteTests(argc, argv);
    return result;
}
