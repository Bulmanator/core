#define CORE_MODULE
#include "core.h"

#include <stdio.h>

typedef struct ListNode ListNode;
struct ListNode {
    ListNode *next;
    ListNode *prev;

    int value;
};

#define ExpectIntValue(a, v) printf("    Testing... %s == %d", #a, v); Assert((a) == (v)); printf(" ... passed\n");

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

    return 0;
}

int main(int argc, char **argv) {
    int result = ExecuteTests(argc, argv);
    return result;
}

