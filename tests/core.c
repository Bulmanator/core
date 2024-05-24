#define CORE_MODULE
#include "core.h"

#include <stdio.h>

typedef struct ListNode ListNode;
struct ListNode {
    ListNode *next;
    ListNode *prev;

    int value;
};

#define ExpectIntValue(a, v) printf("    Testing... %s == %llu", #a, (U64) v); Assert((a) == (v)); printf(" ... passed\n");

static int ExecuteTests(int argc, char **argv) {
    // ... do nothing for now
    //
    (void) argc;
    (void) argv;

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

        //printf("  Truncated value of 1.234f = %u\n", cast(U32) 1.234f);
 //       printf("  ArraySize(arr) = %llu\n", ArraySize(arr));
 //       printf("  OffsetTo(ListNode, prev) = %llu\n", OffsetTo(ListNode, prev));
 //       printf("  AlignOf(U32) = %llu\n", AlignOf(U32));
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

        ExpectIntValue(AtomicCompareExchange_U32(&v8, 100, 10), (B32) true);
        ExpectIntValue(AtomicCompareExchange_U32(&v8, 20,  10), (B32) false);
        ExpectIntValue(v8, 100);

        volatile U64 v9 = 2020;
        ExpectIntValue(AtomicCompareExchange_U64(&v9, 4040, 2020), (B32) true);
        ExpectIntValue(AtomicCompareExchange_U64(&v9, 10,   2020), (B32) false);
        ExpectIntValue(v9, 4040);

        void *v10 = (void *) 0x10101010;

        ExpectIntValue(AtomicCompareExchange_Ptr(&v10, (void *) 0x20202020, (void *) 0x10101010), (B32) true);
        ExpectIntValue(AtomicCompareExchange_Ptr(&v10, (void *) 0x30303030, (void *) 0x10101010), (B32) false);
        ExpectIntValue((U64) v10, 0x20202020);
    }
    printf("\n");

    printf("-- Arena\n");
    {
        M_Arena *arena = M_AllocArena(GB(8));

        printf("    Arena pointer = %p\n", arena);

        M_ResetArena(arena);
        M_ReleaseArena(arena);
    }
    printf("\n");

    return 0;
}

int main(int argc, char **argv) {
    int result = ExecuteTests(argc, argv);
    return result;
}
