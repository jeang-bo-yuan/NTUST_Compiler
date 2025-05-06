#pragma once
#include <stdbool.h>

#define ID_CHARS 63
#define ID_FIRST_CHARS 53

struct SymbolTableNode_t {
    bool isEnd;
    struct SymbolTableNode_t* child[ID_CHARS];
};

// 使用 Trie
struct SymbolTable_t {
    struct SymbolTableNode_t* root[ID_FIRST_CHARS];
};

struct SymbolTable_t create();
//SymbolTableNode_t* lookup(SymbolTable_t* table, const char* S);
void insert(struct SymbolTable_t* table, const char* S);
void dump(const struct SymbolTable_t* table);
