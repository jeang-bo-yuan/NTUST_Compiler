#include "exprToJasm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE* JASM_FILE;
extern char* JASM_CLASS_NAME;

void popExprResult(Type_Info_t type)
{
    switch (type.type) {
    case pIntType: case pFloatType: case pBoolType: case pStringType:
        fprintf(JASM_FILE, "pop\n");
        break;
    case pDoubleType:
        fprintf(JASM_FILE, "pop2\n");
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void exprToJasm(ExpressionNode_t *expr)
{
    // 直接載入常數 ////////////////////////////////////////////////////////////////////////////////
    if (expr->isConstExpr) {
        switch (expr->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "ldc %d\n",      expr->cIval); break;
        case pFloatType:  fprintf(JASM_FILE, "ldc %ef\n",     expr->cFval); break;
        case pDoubleType: fprintf(JASM_FILE, "ldc %e\n",      expr->cDval); break;
        case pBoolType:   fprintf(JASM_FILE, "ldc %d\n",      expr->cBval); break;
        case pStringType: fprintf(JASM_FILE, "ldc \"%s\"\n",  expr->cSval); break;
        }
    }
    // identifier /////////////////////////////////////////////////////////////////////////////////
    else if (expr->isID) {
        // local
        if (expr->localVariableIndex >= 0) {
            switch (expr->resultTypeInfo.type) {
            case pIntType:    fprintf(JASM_FILE, "iload %d\n", expr->localVariableIndex); break;
            case pFloatType:  fprintf(JASM_FILE, "fload %d\n", expr->localVariableIndex); break;
            case pDoubleType: fprintf(JASM_FILE, "dload %d\n", expr->localVariableIndex); break;
            case pBoolType:   fprintf(JASM_FILE, "iload %d\n", expr->localVariableIndex); break;
            case pStringType: perror("Not implemented - load string variable"); exit(-1);
            }
        }
        // global
        else {
            switch (expr->resultTypeInfo.type) {
            case pIntType:    fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pIntType]   , expr->sval); break;
            case pFloatType:  fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pFloatType] , expr->sval); break;
            case pDoubleType: fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pDoubleType], expr->sval); break;
            case pBoolType:   fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pBoolType]  , expr->sval); break;
            case pStringType: perror("Not implemented - getstatic string"); exit(-1);
            }
        }
    }
    // Operator ///////////////////////////////////////////////////////////////////////////////////
    else if (expr->isOP) {
        // Assign
        if (strcmp(expr->OP, "=") == 0) {
            assignToJasm(expr->leftOperand->sval, expr->leftOperand->localVariableIndex, expr->rightOperand);
        }
        
        // Logic
        else if (strcmp(expr->OP, "||") == 0) {
            orToJasm(expr->leftOperand, expr->rightOperand);
        }
        else if (strcmp(expr->OP, "&&") == 0) {
            andToJasm(expr->leftOperand, expr->rightOperand);
        }
        else if (strcmp(expr->OP, "!") == 0) {
            notToJasm(expr->rightOperand);
        }

        // Compare
        
        // Arithmetic
        else if (strcmp(expr->OP, "+") == 0) {
            if (expr->leftOperand)
                addToJasm(expr->leftOperand, expr->rightOperand);
            else
                posToJasm(expr->rightOperand);
        }
        else if (strcmp(expr->OP, "-") == 0) {
            if (expr->leftOperand)
                subToJasm(expr->leftOperand, expr->rightOperand);
            else
                negToJasm(expr->rightOperand);
        }
        else if (strcmp(expr->OP, "*") == 0) {
            mulToJasm(expr->leftOperand, expr->rightOperand);
        }
        else if (strcmp(expr->OP, "/") == 0) {
            divToJasm(expr->leftOperand, expr->rightOperand);
        }
        else if (strcmp(expr->OP, "%") == 0) {
            modToJasm(expr->leftOperand, expr->rightOperand);
        }
        // INCR && DECR
    }
}

// ASSIGN /////////////////////////////////////////////////////////////////////////////////////

void assignToJasm(const char *identifier, int localVariableIndex, ExpressionNode_t *expr)
{
    exprToJasm(expr);

    // local
    if (localVariableIndex >= 0) {
        switch (expr->resultTypeInfo.type) {
        //                STORE                                                  LOAD BACK
        case pIntType:    fprintf(JASM_FILE, "istore %d\n", localVariableIndex); fprintf(JASM_FILE, "iload %d\n", localVariableIndex); break;
        case pFloatType:  fprintf(JASM_FILE, "fstore %d\n", localVariableIndex); fprintf(JASM_FILE, "fload %d\n", localVariableIndex); break;
        case pDoubleType: fprintf(JASM_FILE, "dstore %d\n", localVariableIndex); fprintf(JASM_FILE, "dload %d\n", localVariableIndex); break;
        case pBoolType:   fprintf(JASM_FILE, "istore %d\n", localVariableIndex); fprintf(JASM_FILE, "iload %d\n", localVariableIndex); break;
        case pStringType: perror("Not implemented - store string"); exit(-1);
        }
    }
    // global
    else {
        switch (expr->resultTypeInfo.type) {
        //                PUT                                                                             GET BACK
        case pIntType:    fprintf(JASM_FILE, "putstatic %s %s\n", JASM_TypeStr[pIntType]   , identifier); fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pIntType]   , identifier); break;
        case pFloatType:  fprintf(JASM_FILE, "putstatic %s %s\n", JASM_TypeStr[pFloatType] , identifier); fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pFloatType] , identifier); break;
        case pDoubleType: fprintf(JASM_FILE, "putstatic %s %s\n", JASM_TypeStr[pDoubleType], identifier); fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pDoubleType], identifier); break;
        case pBoolType:   fprintf(JASM_FILE, "putstatic %s %s\n", JASM_TypeStr[pBoolType]  , identifier); fprintf(JASM_FILE, "getstatic %s %s\n", JASM_TypeStr[pBoolType]  , identifier); break;
        case pStringType: perror("Not implemented - putstaticstring"); exit(-1);
        }
    }
}

// LOGIC //////////////////////////////////////////////////////////////////////////////////////////////////

void orToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    fprintf(JASM_FILE, "ior\n");
}

void andToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    fprintf(JASM_FILE, "iand\n");
}

void notToJasm(ExpressionNode_t *R)
{
    exprToJasm(R);
    fprintf(JASM_FILE, "iconst_1\nixor\n");
}

// ARITHMETIC ////////////////////////////////////////////////////////////////////////////

void addToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    switch (L->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "iadd\n"); break;
        case pFloatType:  fprintf(JASM_FILE, "fadd\n"); break;
        case pDoubleType: fprintf(JASM_FILE, "dadd\n"); break;
        case pStringType: perror("Not Implemented - String Concatanation"); exit(-1);
    }
}

void subToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    switch (L->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "isub\n"); break;
        case pFloatType:  fprintf(JASM_FILE, "fsub\n"); break;
        case pDoubleType: fprintf(JASM_FILE, "dsub\n"); break;
    }
}

void mulToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    switch (L->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "imul\n"); break;
        case pFloatType:  fprintf(JASM_FILE, "fmul\n"); break;
        case pDoubleType: fprintf(JASM_FILE, "dmul\n"); break;
    }
}

void divToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    switch (L->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "idiv\n"); break;
        case pFloatType:  fprintf(JASM_FILE, "fdiv\n"); break;
        case pDoubleType: fprintf(JASM_FILE, "ddiv\n"); break;
    }
}

void modToJasm(ExpressionNode_t *L, ExpressionNode_t *R)
{
    exprToJasm(L);
    exprToJasm(R);
    switch (L->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "irem\n"); break;
    }
}

void posToJasm(ExpressionNode_t *R)
{
    exprToJasm(R);
}

void negToJasm(ExpressionNode_t *R)
{
    exprToJasm(R);
    switch (R->resultTypeInfo.type) {
        case pIntType:    fprintf(JASM_FILE, "ineg\n"); break;
        case pFloatType:  fprintf(JASM_FILE, "fneg\n"); break;
        case pDoubleType: fprintf(JASM_FILE, "dneg\n"); break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

static void printStream_JASM(const char* func, ExpressionNode_t* expr)
{
    fprintf(JASM_FILE, "getstatic java.io.PrintStream java.lang.System.out\n");
    exprToJasm(expr);
    fprintf(JASM_FILE, "invokevirtual void java.io.PrintStream.%s(%s)\n", func, JASM_TypeStr[expr->resultTypeInfo.type]);
}

void printToJasm(ExpressionNode_t *expr)
{
    printStream_JASM("print", expr);
}

void printlnToJasm(ExpressionNode_t *expr)
{
    printStream_JASM("println", expr);
}
