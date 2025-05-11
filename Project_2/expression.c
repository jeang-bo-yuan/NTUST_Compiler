#include "expression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// decl
void yyerror(char* msg);

// 型別檢查函數 ///////////////////////////////////////////////////////////////////////////////////////////////

// 檢查 N 代表「非常數」的「Identifier」
static inline bool checkIsNonConstID(ExpressionNode_t* N, const char* op) {
    if (!N->isID || N->isConstExpr) {
        yyerror("Expect a variable that can be modified.");
        
        fprintf(stderr, "\tFor Operator: %s, get (Type = ", op);
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

    if (root->isOP) {
        fprintf(file, " ( ");
        dumpExprTree(file, root->leftOperand);
        fprintf(file, " %s ", root->OP);
        dumpExprTree(file, root->rightOperand);
        fprintf(file, " ) ");
    }
    // leaf node ///////////////////////////////////
    else if (root->isID) {
        fprintf(file, " %s ", root->sval); // 印出 ID
    }
    
    // 如果不是編譯時期常數，則結束；否則，繼續往下並印出值
    if (!root->isConstExpr)
        return;

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

// ASSIGN /////////////////////////////////////////////////////////////////////////////////////////////////////////////

ExpressionNode_t *exprAssign(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkIsNonConstID(leftOperand, "=") == false)
        return NULL;
    if (checkSameType(leftOperand, rightOperand, "=") == false)
        return NULL;

    ExpressionNode_t* newNode = allocNewOperatorNode(leftOperand->resultTypeInfo, "=", leftOperand, rightOperand);
    
    return newNode;
}

// LOGIC //////////////////////////////////////////////////////////////////////////////////////////////////////////////

ExpressionNode_t *exprOR(ExpressionNode_t *leftOperand, ExpressionNode_t *rightOperand)
{
    if (checkSameType(leftOperand, rightOperand, "||") == false)
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
    if (checkIsNonConstID(rightOperand, "prefix ++") == false)
        return NULL;
    if (checkSpecificType(rightOperand, INT_TYPE, "prefix ++") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "++", NULL, rightOperand);
}

ExpressionNode_t *exprPreDecr(ExpressionNode_t *rightOperand)
{
    if (checkIsNonConstID(rightOperand, "prefix --") == false)
        return NULL;
    if (checkSpecificType(rightOperand, INT_TYPE, "prefix --") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "--", NULL, rightOperand);
}

ExpressionNode_t *exprPostIncr(ExpressionNode_t *leftOperand)
{
    if (checkIsNonConstID(leftOperand, "postfix ++") == false)
        return NULL;
    if (checkSpecificType(leftOperand, INT_TYPE, "postfix ++") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "++", leftOperand, NULL);
}

ExpressionNode_t *exprPostDecr(ExpressionNode_t *leftOperand)
{
    if (checkIsNonConstID(leftOperand, "postfix --") == false)
        return NULL;
    if (checkSpecificType(leftOperand, INT_TYPE, "postfix --") == false)
        return NULL;

    return allocNewOperatorNode(INT_TYPE, "--", leftOperand, NULL);
}
