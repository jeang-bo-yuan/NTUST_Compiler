#pragma once
#include "expression.h"

/**
 * 產生一串的 JASM code，在執行後 operand stack 最上方會是運算結果
 */
void exprToJasm(ExpressionNode_t* expr);

/**
 * = -> JASM
 */
void assignToJasm(const char* identifier, int localVariableIndex, ExpressionNode_t* expr);
