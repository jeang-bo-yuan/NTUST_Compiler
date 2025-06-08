#pragma once
#include "expression.h"

/**
 * expression statement的結尾，要把最上面的值pop
 */
void popExprResult(Type_Info_t type);

/**
 * 產生一串的 JASM code，在執行後 operand stack 最上方會是運算結果
 */
void exprToJasm(ExpressionNode_t* expr);

// ASSIGN /////////////////////////
void assignToJasm(const char* identifier, int localVariableIndex, ExpressionNode_t* expr);

// LOGIC //////////////////////////
void orToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void andToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void notToJasm(ExpressionNode_t* R);

// COMPARE ////////////////////////
// ARITHMETIC /////////////////////
void addToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void subToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void mulToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void divToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void modToJasm(ExpressionNode_t* L, ExpressionNode_t* R);
void posToJasm(ExpressionNode_t* R);
void negToJasm(ExpressionNode_t* R);

// INCR & DECR ////////////////////
void prefixIncrToJasm(ExpressionNode_t* lvalue);
void suffixIncrToJasm(ExpressionNode_t* lvalue);
void prefixDecrToJasm(ExpressionNode_t* lvalue);
void suffixDecrToJasm(ExpressionNode_t* lvalue);

/**
 * print expr -> JASM
 */
void printToJasm(ExpressionNode_t* expr);

/**
 * println expr -> JASM
 */
void printlnToJasm(ExpressionNode_t* expr);
