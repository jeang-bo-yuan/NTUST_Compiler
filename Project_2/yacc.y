%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "symbol_table.h"
#include "expression.h"

extern int linenum;
struct SymbolTable_t* Symbol_Table = NULL;

int yylex();
void yyerror(char*);
/**
 * 新增一個變數到 symbol table，其型別為 Type_Info
 * 若失敗回傳 false。
 */
bool addVariable(const char* identifier, ExpressionNode_t* defaultValue);

// 確認該 Identifier 沒有在當前 scope 出現過
#define CHECK_NOT_IN_SYMBOL_TABLE(ID) { \
    if (lookup(Symbol_Table, ID) != NULL) { \
        yyerror("Variable or Function redifined."); \
        fprintf(stderr, "\tVariable or Function (%s) is redifined.\n", ID); \
        YYERROR; \
    } \
}

// 在全域的定義中，暫存 identifier 的值
static char* Global_Level_ID = NULL;

// Note: 因為 Var_Def 裡面不會出現 Var_Def； Func_Def 裡面不會出現 Func_Def。
//       所以可以用全域變數來儲存 Var_Def 和 Func_Def 解析出來的型別。
//
//       準確來說 Type 會存資訊進 Type_Info； Array_Dimensions 會存資訊進 DIMS_Buffer。

// 儲存變數的型別
static Type_Info_t Type_Info;
static unsigned DIMS_Buffer[MAX_ARRAY_DIMENSION];

// 儲存函數的型別
static Function_Type_Info_t Function_Info;
static Type_Info_t PARAM_Buffer[MAX_PARAMETER_NUM];

%}

%union {
    int     ival;
    float   fval;
    double  dval;
    char*   sval;
    ExpressionNode_t* expr;
}

// Keyword
%token BOOL DOUBLE FLOAT INT STRING_yacc
%token BREAK CASE CHAR CONST CONTINUE DEFAULT DO ELSE EXTERN FALSE FOR FOREACH IF PRINT PRINTLN READ RETURN SWITCH TRUE VOID WHILE
// Operator
%token INCR DECR EQ GE LE NE AND OR
// Literal
%token <ival> INTEGER_LITERAL
%token <fval> FLOAT_LITERAL
%token <dval> DOUBLE_LITERAL
%token <sval> STRING_LITERAL ID

%type <expr> Default_Value Expression

// 優先級低
%right '='
%left OR
%left AND
%left '!'
%left '<' LE EQ GE '>' NE
%left '+' '-'
%left '*' '/' '%'
%nonassoc INCR DECR
// 優先級高

%%
Program :   Type Array_Dimensions ID 
            { CHECK_NOT_IN_SYMBOL_TABLE($3); Global_Level_ID = $3; } 
            Global_Def_Tail
            { free(Global_Level_ID); Global_Level_ID = NULL; }
            Program
          | /* Empty */ ;
//Program : Var_Def Program | Func_Def Program | ;

Global_Def_Tail :   // Function 
                    '('
                    { 
                      memset(&Function_Info, 0, sizeof(Function_Info));
                      Function_Info.returnType = Type_Info;
                      // 為函數本體建立 symbol table（會儲存參數、區域變數）
                      Symbol_Table = create(Symbol_Table);
                    }
                    Parameter_Def_List
                    ')' 
                    '{' '}'
                    {
                      dump(Symbol_Table);
                      // 䆁放 Symbol Table，回到 global scope
                      Symbol_Table = freeSymbolTable(Symbol_Table);

                      // 將函數加入 Symbol Table
                      SymbolTableNode_t* function = insert(Symbol_Table, Global_Level_ID);
                      function->isFunction = true;
                      function->functionTypeInfo = Function_Info;
                    }
                  | // Variable Definition
                    {
                      // 避免有人寫出 `int[10] a;`
                      if (Type_Info.dimension > 0) {
                        yyerror("Syntax error on global variable definition!");
                        YYERROR;
                      }
                    }
                    Array_Dimensions Default_Value
                    {
                      if (addVariable(Global_Level_ID, $3) == false)
                          YYERROR;
                    }
                    ID_Def_List_Suffix ';';



// 變數定義 //////////////////////////////////////////////////////////////////////////// 
Var_Def: Type ID_Def_List ';';

ID_Def_List: ID Array_Dimensions Default_Value
             { 
                CHECK_NOT_IN_SYMBOL_TABLE($1);

                if (addVariable($1, $3) == false)
                    YYERROR;

                // free resource
                free($1);
             }
             ID_Def_List_Suffix
           ;

ID_Def_List_Suffix: ',' ID_Def_List | ;

Default_Value: '=' Expression   { $$ = $2; }
              |                 { $$ = NULL; } ;

// 函數定義 ////////////////////////////////////////////////////////////////////////////////

Parameter_Def_List: Non_Empty_Parameter_List 
                    {
                      Function_Info.parameters = calloc(Function_Info.parameterNum, sizeof(PARAM_Buffer[0]));
                      memcpy(Function_Info.parameters, PARAM_Buffer, Function_Info.parameterNum * sizeof(PARAM_Buffer[0]));
                    }
                  | /* Empty */;
Non_Empty_Parameter_List: 
                    Type ID Array_Dimensions
                    {
                      ++Function_Info.parameterNum;

                      if (Function_Info.parameterNum > MAX_PARAMETER_NUM) {
                        yyerror("Too much parameters!!!");
                        fprintf(stderr, "\tMax support %d parameters\n", MAX_PARAMETER_NUM);
                        YYERROR;
                      }

                      PARAM_Buffer[Function_Info.parameterNum - 1] = Type_Info;

                      // 存進 symbol table
                      SymbolTableNode_t* param = insert(Symbol_Table, $2);
                      param->isFunction = false;
                      param->isParameter = true;
                      param->typeInfo = Type_Info;

                      free($2);

                      // Note: 雖然 Type_Info 被同時複製到 PARAM_BUFFER 和 Symbol_Table，但不用擔心
                      //            「刪除 Symbol_Table 時 DIMS 也會被刪掉導致 PARAM_BUFFER 內出現迷途指標」的問題出現
                      //       因為 Symbol_Table 在被刪除時，不會去刪除參數的 typeInfo
                    }
                    Non_Empty_Parameter_Def_List_Suffix;

Non_Empty_Parameter_Def_List_Suffix: ',' Non_Empty_Parameter_List | /* Empty */ ;

// Expression /////////////////////////////////////////////////////////////////////////////
Expression:
          '(' Expression ')'          { $$ = $2; } 
          | Expression '=' Expression { if (($$ = exprAssign($1, $3)) == NULL) YYERROR; }
          
          // LOGIC
          | Expression OR Expression  { if (($$ = exprOR($1, $3)    ) == NULL) YYERROR; }
          | Expression AND Expression { if (($$ = exprAND($1, $3)   ) == NULL) YYERROR; }
          | '!' Expression            { if (($$ = exprNOT($2)       ) == NULL) YYERROR; }

          // COMPARE
          | Expression '<' Expression { if (($$ = exprLT($1, $3)    ) == NULL) YYERROR; }
          | Expression LE Expression  { if (($$ = exprLE($1, $3)    ) == NULL) YYERROR; }
          | Expression EQ Expression  { if (($$ = exprEQ($1, $3)    ) == NULL) YYERROR; }
          | Expression GE Expression  { if (($$ = exprGE($1, $3)    ) == NULL) YYERROR; }
          | Expression '>' Expression { if (($$ = exprGT($1, $3)    ) == NULL) YYERROR; }
          | Expression NE Expression  { if (($$ = exprNE($1, $3)    ) == NULL) YYERROR; }

          // Arithmetic
          | Expression '+' Expression { if (($$ = exprAdd($1, $3)     ) == NULL) YYERROR; }
          | Expression '-' Expression { if (($$ = exprMinus($1, $3)   ) == NULL) YYERROR; }
          | Expression '*' Expression { if (($$ = exprMultiply($1, $3)) == NULL) YYERROR; }
          | Expression '/' Expression { if (($$ = exprDivide($1, $3)  ) == NULL) YYERROR; }
          | Expression '%' Expression { if (($$ = exprMod($1, $3)     ) == NULL) YYERROR; }
          | '+' Expression %prec INCR { if (($$ = exprPositive($2)    ) == NULL) YYERROR; }
          | '-' Expression %prec INCR { if (($$ = exprNegative($2)    ) == NULL) YYERROR; }

          // INCR && DECR
          | INCR Expression { if (($$ = exprPreIncr($2) ) == NULL) YYERROR; }
          | DECR Expression { if (($$ = exprPreDecr($2) ) == NULL) YYERROR; }
          | Expression INCR { if (($$ = exprPostIncr($1)) == NULL) YYERROR; }
          | Expression DECR { if (($$ = exprPostDecr($1)) == NULL) YYERROR; }
          

        /**
        * Literal & ID
        */
          | TRUE
          {
            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isConstExpr = true;
            $$->resultTypeInfo = BOOL_TYPE;
            $$->bval = true;
            $$->cBval = true;
          }
          | FALSE
          {
            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isConstExpr = true;
            $$->resultTypeInfo = BOOL_TYPE;
            $$->bval = false;
            $$->cBval = false;
          }
          | INTEGER_LITERAL
          {
            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isConstExpr = true;
            $$->resultTypeInfo = INT_TYPE;
            $$->ival = $1;
            $$->cIval = $1;
          }
          | STRING_LITERAL
          {
            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isConstExpr = true;
            $$->resultTypeInfo = STRING_TYPE;
            $$->sval = $1;
            $$->cSval = $1;
          }
          | FLOAT_LITERAL
          {
            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isConstExpr = true;
            $$->resultTypeInfo = FLOAT_TYPE;
            $$->fval = $1;
            $$->cFval = $1;
          }
          | DOUBLE_LITERAL
          {
            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isConstExpr = true;
            $$->resultTypeInfo = DOUBLE_TYPE;
            $$->dval = $1;
            $$->cDval = $1;
          }
          | ID
          {
            SymbolTableNode_t *N = lookup(Symbol_Table, $1);

            if (N == NULL) {
              yyerror("Variable undefined.");
              fprintf(stderr, "\tFor ID = %s\n", $1);
              YYERROR;
            }
            if (N->isFunction) {
              yyerror("Function name cannot exist alone.");
              fprintf(stderr, "\tFor ID = %s\n", $1);
              YYERROR;
            }

            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isID = true;
            $$->resultTypeInfo = N->typeInfo;
            $$->sval = $1; // ID

            // 是常數
            if (N->typeInfo.isConst) {
              // 從 symbol table 取出值，存進 expression node 的「編譯時期計算結果」
              $$->isConstExpr = true;

              switch ($$->resultTypeInfo.type) {
                case pIntType:    $$->cIval = N->ival; break;
                case pFloatType:  $$->cFval = N->fval; break;
                case pDoubleType: $$->cDval = N->dval; break;
                case pBoolType:   $$->cBval = N->bval; break;
                case pStringType: $$->cSval = N->sval; break;
              }
            }
          }
          ;


// 型別 ///////////////////////////////////////////////////////////////////////////////////
Type: { memset(&Type_Info, 0, sizeof(Type_Info)); } 
      Qualifier 
      PType;
    
Qualifier: CONST { Type_Info.isConst = true; } | ;

// Primitive Types
PType: BOOL         { Type_Info.type = pBoolType; } 
     | FLOAT        { Type_Info.type = pFloatType; } 
     | INT          { Type_Info.type = pIntType; }
     | DOUBLE       { Type_Info.type = pDoubleType; }
     | STRING_yacc  { Type_Info.type = pStringType; }
     | VOID         { Type_Info.type = pVoidType; }
     |              { yyerror("Missing Type"); YYERROR; } ;

// Array
Array_Dimensions: { // 單純為了初始化
                    Type_Info.dimension = 0;
                  } 
                  Array_Dimensions_Internal
                  { // 將 DIMS_Buffer 複製一份，放入 Type_Info
                    if (Type_Info.dimension > 0) {
                        Type_Info.DIMS = calloc(Type_Info.dimension, sizeof(DIMS_Buffer[0]));
                        memcpy(Type_Info.DIMS, DIMS_Buffer, Type_Info.dimension * sizeof(DIMS_Buffer[0]));
                    }
                  };

Array_Dimensions_Internal: '[' INTEGER_LITERAL ']' 
                            {
                                Type_Info.dimension++;

                                if (Type_Info.dimension > MAX_ARRAY_DIMENSION) {
                                    yyerror("Array dimension is too high.");
                                    fprintf(stderr, "\tMax support:  %d-D array.\n", MAX_ARRAY_DIMENSION);
                                    YYERROR;
                                }

                                if ($2 == 0) {
                                    yyerror("Array size cannot be zero");
                                    YYERROR;
                                }

                                // 將維度的大小存進 buffer
                                DIMS_Buffer[Type_Info.dimension - 1] = $2;
                            } 
                            Array_Dimensions_Internal
                           | /* Empty */ ;





%%

/* file descriptor of source program in LEX
 * https://stackoverflow.com/questions/1796520/in-lex-how-to-make-yyin-point-to-a-file-with-the-main-function-in-yacc
 */
extern FILE *yyin;

void yyerror(char* msg)
{
    fprintf(stderr, "\e[31mError: %s\e[m\n", msg);
}


bool addVariable(const char* identifier, ExpressionNode_t* defaultValue) {
  // 避免 void type
  if (Type_Info.type == pVoidType) {
    yyerror("Variable cannot be void type.");
    fprintf(stderr, "\tFor Variable (%s).\n", identifier);
    return false;
  }

  SymbolTableNode_t* Node = insert(Symbol_Table, identifier);
  Node->isFunction = false;
  Node->typeInfo = Type_Info;

  // 印出預設值
  if (defaultValue) {
    printf("\t\e[35mFor Variable:\e[m %s\n", identifier);
    printf("\t\e[35mDefault Value = \e[m ");
    dumpExprTree(stdout, defaultValue);
    printf("\n");
  }

  // 檢查 const 變數有預設值
  if (Node->typeInfo.isConst && (defaultValue == NULL || defaultValue->isConstExpr == false)) {
    yyerror("Constant variable must have initial value that can be calculated at compile time.");
    fprintf(stderr, "\tFor Variable (%s).\n", identifier);
    return false;
  }

  if (defaultValue) {
    Node->hasDefaultValue = true;

    // 如果變數型別和 Expression 的型別不同
    if (isSameTypeInfo_WithoutConst(Node->typeInfo, defaultValue->resultTypeInfo) == false) {
      yyerror("Variable and its default value have different type.");

      fprintf(stderr, "\tFor variable: %s, ", identifier);
      printTypeInfo(stderr, Node->typeInfo);
      fprintf(stderr, " V.S. ");
      printTypeInfo(stderr, defaultValue->resultTypeInfo);
      fprintf(stderr, "\n");

      return false;
    }

    // 記錄起始值
    if (defaultValue->isConstExpr) {
      Node->defaultValueIsConstExpr = true;  

      switch (defaultValue->resultTypeInfo.type) {
        case pIntType:    Node->ival = defaultValue->cIval; break;
        case pFloatType:  Node->fval = defaultValue->cFval; break;
        case pDoubleType: Node->dval = defaultValue->cDval; break;
        case pBoolType:   Node->bval = defaultValue->cBval; break;
        case pStringType: {
          Node->sval = calloc(strlen(defaultValue->cSval) + 1, sizeof(char));
          strcpy(Node->sval, defaultValue->cSval);
          break;
        }
      }

      freeExprTree(defaultValue);
    }
    else
      Node->expr = defaultValue;
  }

  return true;
}


int main (int argc, char *argv[])
{
    Symbol_Table = create(Symbol_Table);

    /* open the source program file */
    if (argc == 2) {
        yyin = fopen(argv[1], "r"); /* open input file */

        if (yyin == NULL) {
            yyerror("Cannot open file");
            exit(-1);
        }
    }
    else if (argc != 1) {
        yyerror("Usage\n");
        yyerror("\tparser           -> use stdin\n");
        yyerror("\tparser <file>    -> read from file\n");
        exit(-1);
    }

    /* perform parsing */
    if (yyparse() == 1) /* parsing */
        fprintf(stderr, "\e[31mError at line No. %i\e[m\n", linenum); /* syntax error */
    else {
        puts("\e[32mParsing Success!\e[m");
        dump(Symbol_Table);
        Symbol_Table = freeSymbolTable(Symbol_Table);
    }
}
