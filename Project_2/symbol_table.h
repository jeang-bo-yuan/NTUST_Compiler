#pragma once
#include <stdbool.h>

#define ID_CHARS 63
#define ID_FIRST_CHARS 53
#define MAX_ARRAY_DIMENSION 10

typedef enum PrimitiveType_t {
    pIntType,
    pBoolType,
    pStringType,
    pFloatType,
    pDoubleType
} PrimitiveType_t;

typedef struct Type_Info_t {
    bool isConst;
    unsigned dimension;  // 幾維陣列，0 -> 不是陣列
    unsigned DIMS[MAX_ARRAY_DIMENSION];   // 每個維度的大小，只有 [0, dimension) 的資料是有用的，其餘為垃圾
    PrimitiveType_t type;
} Type_Info_t;

// Symbol Table ///////////////////////////////////////////////////////////////////

typedef struct SymbolTableNode_t {
    bool isEnd;
    Type_Info_t typeInfo;
    struct SymbolTableNode_t* child[ID_CHARS];
} SymbolTableNode_t;

// 使用 Trie
typedef struct SymbolTable_t {
    struct SymbolTableNode_t* root[ID_FIRST_CHARS];
} SymbolTable_t;


// Function Declaration //////////////////////////////////////////////////////////

SymbolTable_t create();
SymbolTableNode_t* lookup(SymbolTable_t* table, const char* S);
SymbolTableNode_t* insert(SymbolTable_t* table, const char* S);
void dump(const SymbolTable_t* table);
