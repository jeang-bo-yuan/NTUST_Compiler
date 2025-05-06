#include "symbol_table.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

// Memory Pool ///////////////////
union Internal_Memory_t {
    struct SymbolTableNode_t node;
    union Internal_Memory_t* next;
};
static union Internal_Memory_t* MemoryPool = NULL;

static void RefreshMemoryPool() {
    // 預先分配 1024 個節點，並串成 linked list
    MemoryPool = calloc(1024, sizeof(union Internal_Memory_t));

    for (int i = 1; i < 1024; i++) {
        MemoryPool[i - 1].next = &(MemoryPool[i]);
    }
}

static struct SymbolTableNode_t* AllocNode() {
    // 如果 memory pool 為空的，則刷新
    if (MemoryPool == NULL)
        RefreshMemoryPool();

    // 取出最前面的元素並回傳
    struct SymbolTableNode_t* Result = &(MemoryPool->node);
    MemoryPool = MemoryPool->next;
    memset(Result, 0, sizeof(*Result));
    return Result;
}
///////////////////////////////////

static int Char2Idx(char c) {
    const char* S = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    return strchr(S, c) - S;
}

static char Idx2Char (int i) {
    const char* S = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    return S[i];
}

struct SymbolTable_t create() {
    struct SymbolTable_t Result;

    // 初始化為 NULL
    for (int i = 0; i < ID_FIRST_CHARS; i++)
        Result.root[i] = NULL;

    return Result;
}

void insert(struct SymbolTable_t* table, const char* S) {
    int idx = Char2Idx(*S);
    S++;
    struct SymbolTableNode_t** curr = &(table->root[idx]);

    do {
        if (*curr == NULL)
            *curr = AllocNode();

        // go down
        if (*S) {
            idx = Char2Idx(*S);
            S++;
            curr = &((**curr).child[idx]);
        }
        // reach the end
        else {
            (**curr).isEnd = true;
            break;
        }
    } while (true);
}


static char internal_buf[256];
static int buf_len = 0;
static void DumpNode(struct SymbolTableNode_t* N) {
    if (N == NULL) 
        return;

    if (N->isEnd)
        puts(internal_buf);

    for (int i = 0; i < ID_CHARS; ++i) {
        internal_buf[buf_len++] = Idx2Char(i);
        DumpNode(N->child[i]);
        internal_buf[buf_len--] = '\0';
    }
}

void dump(const struct SymbolTable_t* table) {
    internal_buf[0] = '\0';
    buf_len = 0;

    puts("\nSymbol Table:");
    for (int i = 0; i < ID_FIRST_CHARS; ++i) {
        internal_buf[buf_len++] = Idx2Char(i);
        DumpNode(table->root[i]);
        internal_buf[buf_len--] = '\0';
    }
}


