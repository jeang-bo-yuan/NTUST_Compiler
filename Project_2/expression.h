#pragma once
#include "type_info.h"
#include <stdbool.h>

/**
 * 用來構建運算樹
 * @details isArrayIndexOP 、 isFuncCallOP 、 isOP 是互斥的
 */
typedef struct ExpressionNode_t {
    unsigned isArrayIndexOP : 1;  // 是否為陣列存取運算子（如果是的話，sval 存 Array    Name，rightOperand 存所有 index expression 的 linked list）
    unsigned isFuncCallOP : 1;    // 是否為函數呼叫運算子（如果是的話，sval 存 Function Name，rightOperand 存所有 actual paramenter 的 linked list）
    unsigned isOP : 1;            // 這個節點是運算子（樹的中間節點，但不是 ArrayIndexOP 也不是 FuncCallOP）
    unsigned isConstExpr : 1;     // 是否為常數表達示（可在編譯時期確定值）
    unsigned isID : 1;            // 這個節點是否代表一個identifier （如果是的話，sval 存 identifier name）

    Type_Info_t resultTypeInfo;   // 計算結果是什麼型別

    // 原本讀到的token長什麼樣子
    union {
        int ival;    // -> int literal
        double dval; // -> double literal
        float fval;  // -> float literal
        bool bval;  // -> bool literal
        char* sval;  // -> string literal | ID | Array Name | Function Name
        char OP[8];  // -> operator
    };

    // 編譯時期計算結果（只有 isConstExpr == true 時，這裡的值才有意義）
    union {
        int cIval;    // -> int
        double cDval; // -> double
        float cFval;  // -> float
        bool cBval;  // -> bool
        char* cSval;  // -> string
    };
    
    struct ExpressionNode_t* leftOperand;
    struct ExpressionNode_t* rightOperand;
    struct ExpressionNode_t* nextExpression; // 串成 linked list 時使用，只有 ArrayIndexOP 和 FuncCallOP 要用到
} ExpressionNode_t;

/**
 * 印出運算樹
 */
void dumpExprTree(FILE* file, ExpressionNode_t* root);
/**
 * 䆁放運算樹
 */
void freeExprTree(ExpressionNode_t* root);

/**
 * Expression 是否為 lvalue （可出現在等號左邊）
 * 
 * 判斷標準： 「結果不是 const」 && (「是 identifier」 || 「是 ArrayIndexOP」)
 */
bool isExprLvalue(ExpressionNode_t* root);

// ASSIGN
ExpressionNode_t* exprAssign(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
// LOGIC
ExpressionNode_t* exprOR(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprAND(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprNOT(ExpressionNode_t* rightOperand);
// COMAPRE
ExpressionNode_t* exprLT(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprLE(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprEQ(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprGE(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprGT(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprNE(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
// ARIITHMETIC
ExpressionNode_t* exprAdd(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprMinus(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprMultiply(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprDivide(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprMod(ExpressionNode_t* leftOperand, ExpressionNode_t* rightOperand);
ExpressionNode_t* exprPositive(ExpressionNode_t* rightOperand);
ExpressionNode_t* exprNegative(ExpressionNode_t* rightOperand);
// INCR && DECR
ExpressionNode_t* exprPreIncr(ExpressionNode_t* rightOperand);
ExpressionNode_t* exprPreDecr(ExpressionNode_t* rightOperand);
ExpressionNode_t* exprPostIncr(ExpressionNode_t* leftOperand);
ExpressionNode_t* exprPostDecr(ExpressionNode_t* leftOperand);

