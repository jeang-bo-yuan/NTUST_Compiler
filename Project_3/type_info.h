#pragma once
#include <stdbool.h>
#include <stdio.h>
#define MAX_ARRAY_DIMENSION 15
#define MAX_PARAMETER_NUM 15

extern const char* const PrimitiveTypeStr[6];
extern const char* const JASM_TypeStr[6];

/**
* 原生型別
*/
typedef enum PrimitiveType_t {
    pIntType = 0,
    pBoolType,
    pStringType,
    pFloatType,
    pDoubleType,
    pVoidType
} PrimitiveType_t;

/**
* 記錄「非函數」的型別資訊
*/
typedef struct Type_Info_t {
    unsigned isConst : 1;    // 是否為常數
    unsigned short type;     // 型別（PrimitiveType_t）
    unsigned dimension;      // 幾維陣列，（0 -> 不是陣列）
    unsigned* DIMS;          // 每個維度的大小，長度為 dimension
} Type_Info_t;

/**
 * Type_Info範例，非常數，非陣列型別
 */
extern const Type_Info_t BOOL_TYPE, INT_TYPE, FLOAT_TYPE, DOUBLE_TYPE, STRING_TYPE;

/**
 * 記錄函數的資訊
 */
typedef struct Function_Type_Info_t {
    Type_Info_t returnType;  // 回傳值的型別
    unsigned parameterNum;   // 有幾個參數
    Type_Info_t* parameters; // 所有參數的型別
} Function_Type_Info_t;


/**
 * 印出 T 的型別
 */
void printTypeInfo(FILE* file, const Type_Info_t T);

/**
 * 印出 T 的函數型別
 */
void printFunctionTypeInfo(FILE* file, const Function_Type_Info_t T);

/**
 * 若不考慮 isConst，檢查 T1 和 T2 是否是一樣的型別
 */
bool isSameTypeInfo_WithoutConst(const Type_Info_t T1, const Type_Info_t T2);
/**
 * 檢查 T1 和 T2 是否完全一致
 */
bool isSameTypeInfo(const Type_Info_t T1, const Type_Info_t T2);
