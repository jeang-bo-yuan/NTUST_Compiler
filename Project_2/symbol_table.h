#pragma once
#include <stdbool.h>
#include "type_info.h"

#define ID_CHARS 63
#define ID_FIRST_CHARS 53

// Symbol Table ///////////////////////////////////////////////////////////////////

typedef struct SymbolTableNode_t {
    bool isEnd;
    bool isFunction;
    
    union {
        Type_Info_t          typeInfo;
        Function_Type_Info_t functionTypeInfo;
    };

    // 預設值
    union {
        int ival;
        float fval;
        double dval;
        char* sval;
        bool bval;
    };

    struct SymbolTableNode_t* child[ID_CHARS];
} SymbolTableNode_t;

/**
 * Symbol Table 為多層次架構，內層的 scope 的 symbol table 會指向外層 scope 的 symbol table。
 * @details trie
 */
typedef struct SymbolTable_t {
    struct SymbolTable_t* parent;
    struct SymbolTableNode_t* root[ID_FIRST_CHARS];
} SymbolTable_t;


// Function Declaration //////////////////////////////////////////////////////////

/**
 * 建立新的 symbol table
 */
SymbolTable_t* create(SymbolTable_t* parent);
/**
 * 查找（不查parent）
 */
SymbolTableNode_t* lookup(SymbolTable_t* table, const char* S);
/**
 * 插入
 */
SymbolTableNode_t* insert(SymbolTable_t* table, const char* S);
/**
 * 印出
 */
void dump(const SymbolTable_t* table);
