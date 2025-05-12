#include "expression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// decl
void yyerror(char* msg);

// 型別檢查函數 ///////////////////////////////////////////////////////////////////////////////////////////////

// 檢查 N 是 lvalue
static inline bool checkIsLvalue(ExpressionNode_t* N, const char* op) {
    if (isExprLvalue(N) == false) {
        yyerror("Expect a lvalue.");
        
        fprintf(stderr, "\tFor Operator: %s, expect a lvalue but got (Type = ", op);
        printTypeInfo(stderr, N->resultTypeInfo);
        fprintf(stderr, ") ");
        dumpExprTree(stderr, N);
        fprintf(stderr, "\n");
        
        return false;
    }
    return true;
}

// 檢查左右兩個運算元有同樣的型別
static inline bool checkSameType(ExpressionNode_t* L, ExpressionNode_t* R, const char* op) {
    if (isSameTypeInfo_WithoutConst(L->resultTypeInfo, R->resultTypeInfo) == false) {
        yyerror("Two operands have different type.");

        fprintf(stderr, "\tFor Operator: %s, ", op);
        printTypeInfo(stderr, L->resultTypeInfo);
        fprintf(stderr, " V.S. ");
        printTypeInfo(stderr, R->resultTypeInfo);
        fprintf(stderr, "\n");
        fprintf(stderr, "\tLeft = ");
        dumpExprTree(stderr, L);
        fprintf(stderr, "\n\tRight = ");
        dumpExprTree(stderr, R);
        fprintf(stderr, "\n");
        
        return false;
    }
    return true;
}

// 檢查是否為特定型別
static inline bool checkSpecificType(ExpressionNode_t* E, Type_Info_t type, const char* op) {
    if (isSameTypeInfo_WithoutConst(E->resultTypeInfo, type) == false) {
        yyerror("Type error.");
        
        fprintf(stderr, "\tOperator %s ONLY accept ", op);
        printTypeInfo(stderr, type);
        fprintf(stderr, " , but got (Type = ");
        printTypeInfo(stderr, E->resultTypeInfo);
        fprintf(stderr, ") ");
        dumpExprTree(stderr, E);
        fprintf(stderr, "\n");
        
        return false;
    }
    return true;
}

// 檢查「不是特定型別」
static inline bool checkNotSpecificType(ExpressionNode_t* E, Type_Info_t type, const char* op) {
    if (isSameTypeInfo_WithoutConst(E->resultTypeInfo, type) == true) {
        yyerror("Type error.");

        fprintf(stderr, "\tOperator %s DOESN'T accept ", op);
        printTypeInfo(stderr, type);
        fprintf(stderr, " , but got (Type = ");
        printTypeInfo(stderr, E->resultTypeInfo);
        fprintf(stderr, ") ");
        dumpExprTree(stderr, E);
        fprintf(stderr, "\n");
        
        return false;
    }
    return true;
}

// 檢查不是陣列型別
static inline bool checkNotArrayType(ExpressionNode_t* E, const char* op) {
    if (E->resultTypeInfo.dimension > 0) {
        yyerror("Type error.");

        fprintf(stderr, "\tOperator %s DOESN'T accept array type, but got (Type = ", op);
        printTypeInfo(stderr, E->resultTypeInfo);
        fprintf(stderr, ") ");
        dumpExprTree(stderr, E);
        fprintf(stderr, "\n");
        
        return false;
    }

    return true;
}

// 檢查不是 void
static inline bool checkNotVoidType(ExpressionNode_t* E, const char* op) {
    if (E->resultTypeInfo.type != pVoidType)
        return true;
    
    yyerror("Type error.");

    fprintf(stderr, "\tThe operand of operator %s is void type\n\tThe Operand = ", op);
    dumpExprTree(stderr, E);
    fprintf(stderr, "\n");
    
    return false;
}

// Helper Function ////////////////////////////////////////////////////////////////////////////////////////////////////

static inline ExpressionNode_t* allocNewOperatorNode(
                                                    Type_Info_t resultType, 
                                                    const char* op, 
                                                    ExpressionNode_t* leftOperand, 
                                                    ExpressionNode_t* rightOperand) 
{
    ExpressionNode_t* newNode = calloc(1, sizeof(ExpressionNode_t));
    // 是 operator
    newNode->isOP = true;
    // 型別
    newNode->resultTypeInfo = resultType;
    // 代表 operator 字串
    strcpy(newNode->OP, op);
    // 左右運算元
    newNode->leftOperand = leftOperand;
    newNode->rightOperand = rightOperand;

    return newNode;
}

void dumpExprTree(FILE* file, ExpressionNode_t *root)
{
    if (root == NULL)
        return;

    // Array Index /////////////////////////////////
    if (root->isArrayIndexOP) {
        fprintf(file, " %s", root->sval);  // Array Name

        // index 組成的 linked list 放在 root->rightOperand                移至下個 index
        for (ExpressionNode_t* indexHead = root->rightOperand; indexHead; indexHead = indexHead->nextExpression) {
            fprintf(file, "[");
            dumpExprTree(file, indexHead);
            fprintf(file, "]");
        }
    }
    // Function Call ///////////////////////////////
    else if (root->isFuncCallOP) {
        fprintf(file, " %s(", root->sval);

        // 印出參數
        unsigned counter = 0;
        for (ExpressionNode_t* paramHead = root->rightOperand; paramHead; paramHead = paramHead->nextExpression) {
            if (counter++) 
                fprintf(file, ",");

            dumpExprTree(file, paramHead);
        }

        fprintf(file, ") ");
    }
    // Operator ////////////////////////////////////
    else if (root->isOP) {
        fprintf(file, " ( ");
        dumpExprTree(file, root->leftOperand);   // 左運算元
        fprintf(file, " %s ", root->OP);         // 運算子
        dumpExprTree(file, root->rightOperand);  // 右運算元
        fprintf(file, " ) ");
    }
    // Variable Name ///////////////////////////////
    else if (root->isID) {
        fprintf(file, " %s ", root->sval); // 印出 ID
    }
    
    // 如果不是編譯時期常數，則結束；否則，繼續往下並印出值
    if (!root->isConstExpr)
        return;

    // 加上 => 以區分原始 token 及運算結果
    if (root->isID || root->isOP)
        fprintf(file, "\e[35m=>");

    switch (root->resultTypeInfo.type) {
        case pIntType:    fprintf(file, " %d ",  root->cIval);                   break;
        case pFloatType:  fprintf(file, " %gf ", root->cFval);                   break;
        case pDoubleType: fprintf(file, " %g ",  root->cDval);                   break;
        case pStringType: fprintf(file, " \"%s\" ", root->cSval);                break;
        case pBoolType:   fprintf(file, " %s ", root->cBval ? "true" : "false"); break;
    }

    fprintf(file, "\e[m");
}

void freeExprTree(ExpressionNode_t *root)
{
    if (root == NULL)
        return;

    freeExprTree(root->leftOperand);
    freeExprTree(root->rightOperand);
    freeExprTree(root->nextExpression);

    // 䆁放 ID / Array Name / Function Name 的字串
    if (root->isID || root->isArrayIndexOP || root->isFuncCallOP)
        free(root->sval);

    // 䆁放字串運算結果
    // Note:  root 是 ID，代表 root->cSval 指向 symbol table 內的字串，這時不應該䆁放
    if (root->isConstExpr && root->resultTypeInfo.type == pStringType && !root->isID)
        free(root->cSval);

    // 䆁放root
    free(root);
}

bool isExprLvalue(ExpressionNode_t *root)
{
    return !root->resultTypeInfo.isConst && (root->isID || root->isArrayIndexOP);
}

// ASSIGN /////////////////////////////////////////////////////////////////////////////////////////////////////////////

ExpressionNode_t *exprAssign(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkIsLvalue(leftOperand, "=") == false)
        return NULL;
    if (checkSameType(leftOperand, rightOperand, "=") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "=") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(leftOperand->resultTypeInfo, "=", leftOperand, rightOperand);
    
    return newNode;
}

// LOGIC //////////////////////////////////////////////////////////////////////////////////////////////////////////////

ExpressionNode_t *exprOR(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "||") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "||") == false)
        return NULL;
    if (checkSpecificType(leftOperand, BOOL_TYPE, "||") == false)
        return NULL;
    
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "||", leftOperand, rightOperand);

    // 編譯時期運算
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;
        newNode->cBval = (leftOperand->cBval || rightOperand->cBval);
    }

    return newNode;
}

ExpressionNode_t *exprAND(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "&&") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "&&") == false)
        return NULL;
    if (checkSpecificType(leftOperand, BOOL_TYPE, "&&") == false)
        return NULL;
    
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "&&", leftOperand, rightOperand);

    // 編譯時期運算
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;
        newNode->cBval = (leftOperand->cBval && rightOperand->cBval);
    }

    return newNode;
}

ExpressionNode_t *exprNOT(ExpressionNode_t *rightOperand)
{
    if (checkNotVoidType(rightOperand, "!") == false)
        return NULL;
    if (checkSpecificType(rightOperand, BOOL_TYPE, "!") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "!", NULL, rightOperand);

    // 編譯時期運算
    if (rightOperand->isConstExpr) {
        newNode->isConstExpr = true;
        newNode->cBval = (! rightOperand->cBval);
    }

    return newNode;
}

// COMPARE ////////////////////////////////////////////////////////////////////////////////////////////////////////////

ExpressionNode_t *exprLT(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand) 
{ 
    if (checkSameType(leftOperand, rightOperand, "<") == false) 
        return NULL; 
    if (checkNotVoidType(leftOperand, "<") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, "<") == false) 
        return NULL; 
    if (checkNotArrayType(leftOperand, "<") == false)
        return NULL;

    /* 新增一個節點，代表 compare 運算子 */ 
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "<", leftOperand, rightOperand); 

    /* 編譯時期運算*/ 
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) { 
        newNode->isConstExpr = true; 

        /* 依據運算元不同型別，作不同處理 */ 
        switch (leftOperand->resultTypeInfo.type) {   
        case pIntType:     newNode->cBval = leftOperand->cIval < rightOperand->cIval;              break; 
        case pFloatType:   newNode->cBval = leftOperand->cFval < rightOperand->cFval;              break; 
        case pDoubleType:  newNode->cBval = leftOperand->cDval < rightOperand->cDval;              break; 
        case pStringType:  newNode->cBval = strcmp(leftOperand->cSval, rightOperand->cSval) < 0;   break; 
        } 
    } 

    return newNode; 
}

ExpressionNode_t *exprLE(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand) 
{ 
    if (checkSameType(leftOperand, rightOperand, "<=") == false) 
        return NULL; 
    if (checkNotVoidType(leftOperand, "<=") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, "<=") == false) 
        return NULL; 
    if (checkNotArrayType(leftOperand, "<=") == false)
        return NULL;

    /* 新增一個節點，代表 compare 運算子 */ 
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "<=", leftOperand, rightOperand); 

    /* 編譯時期運算*/ 
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) { 
        newNode->isConstExpr = true; 

        /* 依據運算元不同型別，作不同處理 */ 
        switch (leftOperand->resultTypeInfo.type) {  
        case pIntType:     newNode->cBval = leftOperand->cIval <= rightOperand->cIval;              break; 
        case pFloatType:   newNode->cBval = leftOperand->cFval <= rightOperand->cFval;              break; 
        case pDoubleType:  newNode->cBval = leftOperand->cDval <= rightOperand->cDval;              break; 
        case pStringType:  newNode->cBval = strcmp(leftOperand->cSval, rightOperand->cSval) <= 0;   break; 
        } 
    } 

    return newNode; 
}

ExpressionNode_t *exprEQ(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand) 
{ 
    if (checkSameType(leftOperand, rightOperand, "==") == false) 
        return NULL; 
    if (checkNotVoidType(leftOperand, "==") == false)
        return NULL;

    /* 新增一個節點，代表 compare 運算子 */ 
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "==", leftOperand, rightOperand); 

    /* 編譯時期運算*/ 
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) { 
        newNode->isConstExpr = true; 

        /* 依據運算元不同型別，作不同處理 */ 
        switch (leftOperand->resultTypeInfo.type) {   
        case pIntType:     newNode->cBval = leftOperand->cIval == rightOperand->cIval;              break; 
        case pFloatType:   newNode->cBval = leftOperand->cFval == rightOperand->cFval;              break; 
        case pDoubleType:  newNode->cBval = leftOperand->cDval == rightOperand->cDval;              break; 
        case pStringType:  newNode->cBval = strcmp(leftOperand->cSval, rightOperand->cSval) == 0;   break; 
        case pBoolType:    newNode->cBval = leftOperand->cBval == rightOperand->cBval;              break;
        } 
    } 

    return newNode; 
}

ExpressionNode_t *exprGE(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand) 
{ 
    if (checkSameType(leftOperand, rightOperand, ">=") == false) 
        return NULL; 
    if (checkNotVoidType(leftOperand, ">=") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, ">=") == false) 
        return NULL; 
    if (checkNotArrayType(leftOperand, ">=") == false)
        return NULL;

    /* 新增一個節點，代表 compare 運算子 */ 
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, ">=", leftOperand, rightOperand); 

    /* 編譯時期運算*/ 
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) { 
        newNode->isConstExpr = true; 

        /* 依據運算元不同型別，作不同處理 */ 
        switch (leftOperand->resultTypeInfo.type) {  
        case pIntType:     newNode->cBval = (leftOperand->cIval >= rightOperand->cIval);              break; 
        case pFloatType:   newNode->cBval = leftOperand->cFval >= rightOperand->cFval;              break; 
        case pDoubleType:  newNode->cBval = leftOperand->cDval >= rightOperand->cDval;              break; 
        case pStringType:  newNode->cBval = strcmp(leftOperand->cSval, rightOperand->cSval) >= 0;   break; 
        } 
    } 

    return newNode; 
}

ExpressionNode_t *exprGT(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand) 
{ 
    if (checkSameType(leftOperand, rightOperand, ">") == false) 
        return NULL; 
    if (checkNotVoidType(leftOperand, ">") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, ">") == false) 
        return NULL; 
    if (checkNotArrayType(leftOperand, ">") == false)
        return NULL;

    /* 新增一個節點，代表 compare 運算子 */ 
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, ">", leftOperand, rightOperand); 

    /* 編譯時期運算*/ 
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) { 
        newNode->isConstExpr = true; 

        /* 依據運算元不同型別，作不同處理 */ 
        switch (leftOperand->resultTypeInfo.type) {  
        case pIntType:     newNode->cBval = leftOperand->cIval > rightOperand->cIval;              break; 
        case pFloatType:   newNode->cBval = leftOperand->cFval > rightOperand->cFval;              break; 
        case pDoubleType:  newNode->cBval = leftOperand->cDval > rightOperand->cDval;              break; 
        case pStringType:  newNode->cBval = strcmp(leftOperand->cSval, rightOperand->cSval) > 0;   break; 
        } 
    } 

    return newNode; 
}

ExpressionNode_t *exprNE(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand) 
{ 
    if (checkSameType(leftOperand, rightOperand, "!=") == false) 
        return NULL; 
    if (checkNotVoidType(leftOperand, "!=") == false)
        return NULL;

    /* 新增一個節點，代表 compare 運算子 */ 
    ExpressionNode_t* newNode = allocNewOperatorNode(BOOL_TYPE, "!=", leftOperand, rightOperand); 

    /* 編譯時期運算*/ 
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) { 
        newNode->isConstExpr = true; 

        /* 依據運算元不同型別，作不同處理 */ 
        switch (leftOperand->resultTypeInfo.type) {   
        case pIntType:     newNode->cBval = leftOperand->cIval != rightOperand->cIval;              break; 
        case pFloatType:   newNode->cBval = leftOperand->cFval != rightOperand->cFval;              break; 
        case pDoubleType:  newNode->cBval = leftOperand->cDval != rightOperand->cDval;              break; 
        case pStringType:  newNode->cBval = strcmp(leftOperand->cSval, rightOperand->cSval) != 0;   break; 
        case pBoolType:    newNode->cBval = leftOperand->cBval != rightOperand->cBval;              break;
        } 
    } 

    return newNode; 
}

// Arithmetic /////////////////////////////////////////////////////////////////////////////////////

ExpressionNode_t *exprAdd(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "+") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "+") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, "+") == false)
        return NULL;
    if (checkNotArrayType(leftOperand, "+") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(leftOperand->resultTypeInfo, "+", leftOperand, rightOperand);

    /* 編譯時期運算 */
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;

        switch (leftOperand->resultTypeInfo.type) {
            case pIntType:     newNode->cIval = leftOperand->cIval + rightOperand->cIval;   break;
            case pFloatType:   newNode->cFval = leftOperand->cFval + rightOperand->cFval;   break;
            case pDoubleType:  newNode->cDval = leftOperand->cDval + rightOperand->cDval;   break;
            case pStringType: {
                // 字串串接
                newNode->cSval = calloc(strlen(leftOperand->cSval) + strlen(rightOperand->cSval) + 1, sizeof(char));
                strcpy(newNode->cSval, leftOperand->cSval);
                strcat(newNode->cSval, rightOperand->cSval);
            }
        }
    }

    return newNode;
}

ExpressionNode_t *exprMinus(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "-") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "-") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, "-") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, STRING_TYPE, "-") == false)
        return NULL;
    if (checkNotArrayType(leftOperand, "-") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(leftOperand->resultTypeInfo, "-", leftOperand, rightOperand);

    /* 編譯時期計算 */
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;

        switch (leftOperand->resultTypeInfo.type) {
            case pIntType:    newNode->cIval = leftOperand->cIval - rightOperand->cIval;   break;
            case pFloatType:  newNode->cFval = leftOperand->cFval - rightOperand->cFval;   break;
            case pDoubleType: newNode->cDval = leftOperand->cDval - rightOperand->cDval;   break;
        }
    }

    return newNode;
}

ExpressionNode_t *exprMultiply(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "*") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "*") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, "*") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, STRING_TYPE, "*") == false)
        return NULL;
    if (checkNotArrayType(leftOperand, "*") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(leftOperand->resultTypeInfo, "*", leftOperand, rightOperand);

    /* 編譯時期計算 */
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;

        switch (leftOperand->resultTypeInfo.type) {
            case pIntType:    newNode->cIval = leftOperand->cIval * rightOperand->cIval;   break;
            case pFloatType:  newNode->cFval = leftOperand->cFval * rightOperand->cFval;   break;
            case pDoubleType: newNode->cDval = leftOperand->cDval * rightOperand->cDval;   break;
        }
    }

    return newNode;
}

ExpressionNode_t *exprDivide(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "/") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "/") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, BOOL_TYPE, "/") == false)
        return NULL;
    if (checkNotSpecificType(leftOperand, STRING_TYPE, "/") == false)
        return NULL;
    if (checkNotArrayType(leftOperand, "/") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(leftOperand->resultTypeInfo, "/", leftOperand, rightOperand);

    /* 編譯時期計算 */
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;

        switch (leftOperand->resultTypeInfo.type) {
            case pIntType:    newNode->cIval = leftOperand->cIval / rightOperand->cIval;   break;
            case pFloatType:  newNode->cFval = leftOperand->cFval / rightOperand->cFval;   break;
            case pDoubleType: newNode->cDval = leftOperand->cDval / rightOperand->cDval;   break;
        }
    }

    return newNode;
}

ExpressionNode_t *exprMod(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "%") == false)
        return NULL;
    if (checkNotVoidType(leftOperand, "%") == false)
        return NULL;
    if (checkSpecificType(leftOperand, INT_TYPE, "%") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(INT_TYPE, "%", leftOperand, rightOperand);

    /* 編譯時期運算 */
    if (leftOperand->isConstExpr && rightOperand->isConstExpr) {
        newNode->isConstExpr = true;
        newNode->cIval = leftOperand->cIval % rightOperand->cIval;
    }

    return newNode;
}

ExpressionNode_t *exprPositive(ExpressionNode_t *rightOperand)
{
    if (checkNotVoidType(rightOperand, "unary +") == false)
        return NULL;
    if (checkNotSpecificType(rightOperand, BOOL_TYPE, "unary +") == false)
        return NULL;
    if (checkNotSpecificType(rightOperand, STRING_TYPE, "unary +") == false)
        return NULL;
    if (checkNotArrayType(rightOperand, "unary +") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(rightOperand->resultTypeInfo, "+", NULL, rightOperand);

    /* 編譯時期計算 */
    if (rightOperand->isConstExpr) {
        newNode->isConstExpr = true;

        switch (rightOperand->resultTypeInfo.type) {
        case pIntType:    newNode->cIval = + rightOperand->cIval;  break;
        case pFloatType:  newNode->cFval = + rightOperand->cFval;  break;
        case pDoubleType: newNode->cDval = + rightOperand->cDval;  break;
        }
    }

    return newNode;
}

ExpressionNode_t *exprNegative(ExpressionNode_t *rightOperand)
{
    if (checkNotVoidType(rightOperand, "unary -") == false)
        return NULL;
    if (checkNotSpecificType(rightOperand, BOOL_TYPE, "unary -") == false)
        return NULL;
    if (checkNotSpecificType(rightOperand, STRING_TYPE, "unary -") == false)
        return NULL;
    if (checkNotArrayType(rightOperand, "unary -") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(rightOperand->resultTypeInfo, "-", NULL, rightOperand);

    /* 編譯時期計算 */
    if (rightOperand->isConstExpr) {
        newNode->isConstExpr = true;

        switch (rightOperand->resultTypeInfo.type) {
        case pIntType:    newNode->cIval = - rightOperand->cIval;  break;
        case pFloatType:  newNode->cFval = - rightOperand->cFval;  break;
        case pDoubleType: newNode->cDval = - rightOperand->cDval;  break;
        }
    }

    return newNode;
}

// INCR && DECR

ExpressionNode_t *exprPreIncr(ExpressionNode_t *rightOperand)
{
    if (checkNotVoidType(rightOperand, "prefix ++") == false)
        return NULL;
    if (checkIsLvalue(rightOperand, "prefix ++") == false)
        return NULL;
    if (checkSpecificType(rightOperand, INT_TYPE, "prefix ++") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "++", NULL, rightOperand);
}

ExpressionNode_t *exprPreDecr(ExpressionNode_t *rightOperand)
{
    if (checkNotVoidType(rightOperand, "prefix --") == false)
        return NULL;
    if (checkIsLvalue(rightOperand, "prefix --") == false)
        return NULL;
    if (checkSpecificType(rightOperand, INT_TYPE, "prefix --") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "--", NULL, rightOperand);
}

ExpressionNode_t *exprPostIncr(ExpressionNode_t *leftOperand)
{
    if (checkNotVoidType(leftOperand, "postfix ++") == false)
        return NULL;
    if (checkIsLvalue(leftOperand, "postfix ++") == false)
        return NULL;
    if (checkSpecificType(leftOperand, INT_TYPE, "postfix ++") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "++", leftOperand, NULL);
}

ExpressionNode_t *exprPostDecr(ExpressionNode_t *leftOperand)
{
    if (checkNotVoidType(leftOperand, "postfix --") == false)
        return NULL;
    if (checkIsLvalue(leftOperand, "postfix --") == false)
        return NULL;
    if (checkSpecificType(leftOperand, INT_TYPE, "postfix --") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "--", leftOperand, NULL);
}

// Special /////////////////////////////////////////////////////////////////

ExpressionNode_t *exprArrayIndexOP(char *identifier, const Type_Info_t T, ExpressionNode_t *indices)
{
    ExpressionNode_t* result = calloc(1, sizeof(ExpressionNode_t));
    //
    result->isArrayIndexOP = true;
    // 結果為陣列中的元素
    result->resultTypeInfo.isConst = T.isConst;
    result->resultTypeInfo.type    = T.type;
    // 記錄 Array Name
    result->sval = identifier;
    // 運算元
    result->rightOperand = indices;

    unsigned indicesNum = 0;
    // 檢查 indices 都是 int 且長度 == T.dimension
    while (indices) {
        indicesNum++;

        // 不是 Int
        if (isSameTypeInfo_WithoutConst(indices->resultTypeInfo, INT_TYPE) == false) {
            yyerror("Type Error!");
            fprintf(stderr, "\tExpect a int for array index, but got (Type = ");
            printTypeInfo(stderr, indices->resultTypeInfo);
            fprintf(stderr, ") ");
            dumpExprTree(stderr, indices);
            fprintf(stderr, "\n");
            
            return NULL;
        }

        indices = indices->nextExpression;
    }
    
    // 長度不一致
    if (indicesNum != T.dimension) {
        yyerror("Number of indices doesn't match!");
        fprintf(stderr, "\t(ID = %s) is %u-D Array, but there are %u indices\n", identifier, T.dimension, indicesNum);
        return NULL;
    }

    return result;
}

ExpressionNode_t *exprFuncCallOP(char *identifier, const Function_Type_Info_t T, ExpressionNode_t *params)
{
    ExpressionNode_t* result = calloc(1, sizeof(ExpressionNode_t));
    //
    result->isFuncCallOP = true;
    // 結果為回傳值
    result->resultTypeInfo = T.returnType;
    // 記錄 Function Name
    result->sval = identifier;
    // 記錄運算元（參數）
    result->rightOperand = params;

    unsigned paramsNum = 0;
    // 檢查參數數量及型別
    while (params) {
        if (paramsNum < T.parameterNum) {
            if (isSameTypeInfo_WithoutConst(T.parameters[paramsNum], params->resultTypeInfo) == false) {
                yyerror("Type error!");
                fprintf(stderr, "\tExpect a parameter with type = ");
                printTypeInfo(stderr, T.parameters[paramsNum]);
                fprintf(stderr, ", but got (Type = ");
                printTypeInfo(stderr, params->resultTypeInfo);
                fprintf(stderr, ") ");
                dumpExprTree(stderr, params);
                fprintf(stderr, "\n");
                return NULL;
            }
        }
        else {
            yyerror("Too many parameters.");
            fprintf(stderr, "\tFor Function = %s, expect %d parameters\n", identifier, T.parameterNum);
            return NULL;
        }

        paramsNum++;
        params = params->nextExpression;
    }

    if (paramsNum < T.parameterNum) {
        yyerror("Too less parameters.");
        fprintf(stderr, "\tFor Function = %s, expect %d parameters\n", identifier, T.parameterNum);
        return NULL;
    }

    return result;
}
