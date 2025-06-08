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

/**
 * = -> JASM
 */
void assignToJasm(const char* identifier, int localVariableIndex, ExpressionNode_t* expr);

/**
 * print expr -> JASM
 */
void printToJasm(ExpressionNode_t* expr);

/**
 * println expr -> JASM
 */
void printlnToJasm(ExpressionNode_t* expr);
