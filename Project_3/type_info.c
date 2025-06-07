#include "type_info.h"
#include <stdio.h>

const char* const PrimitiveTypeStr[6] = {
    [pIntType] = "int",
    [pBoolType] = "bool",
    [pFloatType] = "float",
    [pDoubleType] = "double",
    [pStringType] = "string",
    [pVoidType] = "void"
};

//                                Const  Type         DIMS
const Type_Info_t BOOL_TYPE   = { false, pBoolType,   0, NULL},
                  INT_TYPE    = { false, pIntType,    0, NULL},
                  FLOAT_TYPE  = { false, pFloatType,  0, NULL},
                  DOUBLE_TYPE = { false, pDoubleType, 0, NULL},
                  STRING_TYPE = { false, pStringType, 0, NULL};

void printTypeInfo(FILE* file, const Type_Info_t T)
{
    // const
    if (T.isConst)
        fprintf(file, "const ");

    // primitive type
    fprintf(file, "%s", PrimitiveTypeStr[T.type]);

    // array dimension
    for (unsigned i = 0; i < T.dimension; i++) {
        fprintf(file, "[%u]", T.DIMS[i]);
    }
}

void printFunctionTypeInfo(FILE *file, const Function_Type_Info_t T)
{
    printTypeInfo(file, T.returnType);

    fprintf(file, "(");
    for (unsigned i = 0; i < T.parameterNum; ++i) {
        if (i != 0)
            fprintf(file, ", ");
        printTypeInfo(file, T.parameters[i]);
    }
    
    fprintf(file, ")");
}

bool isSameTypeInfo_WithoutConst(const Type_Info_t T1, const Type_Info_t T2)
{
    // 有一樣的 type 和 維度
    if (T1.type == T2.type && T1.dimension == T2.dimension) {
        // 檢查每一維的大小是否一樣
        for (unsigned i = 0; i < T1.dimension; ++i) {
            if (T1.DIMS[i] != T2.DIMS[i])
                return false;
        }

        return true;
    }

    return false;
}

bool isSameTypeInfo(const Type_Info_t T1, const Type_Info_t T2)
{
    return isSameTypeInfo_WithoutConst(T1, T2) && T1.isConst == T2.isConst;
}
