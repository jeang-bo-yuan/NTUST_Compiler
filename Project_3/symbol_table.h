#pragma once
#include <stdbool.h>
#include "type_info.h"
#include "expression.h"

#define ID_CHARS 63
#define ID_FIRST_CHARS 53

// Symbol Table ///////////////////////////////////////////////////////////////////

typedef struct SymbolTableNode_t {
    bool isEnd : 1;
    bool isFunction : 1;   // 是否是函數
    bool isParameter : 1;  // 是否是參數
    bool hasDefaultValue : 1;           // 是否有預設值（<- 這好像不需要，只是 Debug 時能印出比較多資訊）
    bool defaultValueIsConstExpr : 1;   // 預設值是 constexpr
    int localVariableIndex;  // JASM 中區域變數的 index，只有當 `不是全域變數 && !isFunction && (!typeInfo.isConst || isParameter)` 時才有 >= 0 的值，否則是 -1
    
    union {
        Type_Info_t          typeInfo;
        Function_Type_Info_t functionTypeInfo;
    };

    // 預設值
    union {
        int ival;      // 預設值為 int 常數
        float fval;
        double dval;
        char* sval;
        bool bval;
        ExpressionNode_t* expr; // 預設值為 expression，但不是常數
    };

    struct SymbolTableNode_t* child[ID_CHARS];
} SymbolTableNode_t;

/**
 * Symbol Table 為多層次架構，內層的 scope 的 symbol table 會指向外層 scope 的 symbol table。
 * @details trie
 */
typedef struct SymbolTable_t {
    unsigned nextLocalVariableIndex; // 下一個可分配的區域變數 index（Note: 只有 parent->parent == NULL 才可分配 index）
    struct SymbolTable_t* parent;
    struct SymbolTableNode_t* root[ID_FIRST_CHARS];
} SymbolTable_t;


// Function Declaration //////////////////////////////////////////////////////////

/**
 * 建立新的 symbol table
 */
SymbolTable_t* create(SymbolTable_t* parent);

/**
 * 䆁放 Symbol Table 然後回傳 parent
 */
SymbolTable_t* freeSymbolTable(SymbolTable_t* table);

/**
 * 查找（不查parent）
 */
SymbolTableNode_t* lookup(SymbolTable_t* table, const char* S);
/**
 * 查找，若在當前 scope 找不到，則嘗試往 parent scope 找
 */
SymbolTableNode_t* lookupRecursive(SymbolTable_t* table, const char* S);

/**
 * 插入
 */
SymbolTableNode_t* insert(SymbolTable_t* table, const char* S);

/**
 * 印出
 */
void dump(const SymbolTable_t* table);

/**
 * 替 node 指定區域變數的 index
 */
void assignIndex(SymbolTableNode_t* node, SymbolTable_t* table);
