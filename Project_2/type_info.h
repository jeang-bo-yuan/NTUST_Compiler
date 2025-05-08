#pragma once
#define MAX_ARRAY_DIMENSION 15
#define MAX_PARAMETER_NUM 15

extern const char* const PrimitiveTypeStr[6];

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
    unsigned type : 3;       // 型別（PrimitiveType_t）
    unsigned dimension;  // 幾維陣列，（0 -> 不是陣列）
    unsigned* DIMS;   // 每個維度的大小，只有 [0, dimension) 的資料是有用的，其餘為垃圾
} Type_Info_t;

typedef struct Function_Type_Info_t {
    Type_Info_t returnType;  // 回傳值的型別
    unsigned parameterNum;   // 有幾個參數
    Type_Info_t* parameters; // 所有參數的型別
} Function_Type_Info_t;
