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
#define CHECK_NOT_IN_CURRENT_SCOPE(ID) { \
    if (lookup(Symbol_Table, ID) != NULL) { \
        yyerror("Variable or Function redifined."); \
        fprintf(stderr, "\tVariable or Function (%s) is redifined.\n", ID); \
        YYERROR; \
    } \
}

// 檢查 Symbol Table Node 不是 NULL
#define CHECK_NODE_NOT_NULL(N, ID) { \
  if (N == NULL) { \
    yyerror("Identifier undefined!"); \
    fprintf(stderr, "\tFor ID = %s\n", ID); \
    YYERROR; \
  } \
}

// 是否在 global scope
#define IN_GLOBAL_SCOPE() (Symbol_Table->parent == NULL)

// 在 global scope 中，暫存 identifier 的值
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
%token BREAK CASE CHAR CONST CONTINUE DEFAULT DO ELSE EXTERN FALSE FOR FOREACH IF PRINT PRINTLN RANGE READ RETURN SWITCH TRUE VOID WHILE
// Operator
%token INCR DECR EQ GE LE NE AND OR
// Literal
%token <ival> INTEGER_LITERAL
%token <fval> FLOAT_LITERAL
%token <dval> DOUBLE_LITERAL
%token <sval> STRING_LITERAL ID

%type <expr> Default_Value Expression
%type <expr> ArrayIndexOP ArrayIndexOP_Suffix
%type <expr> FuncCallOP FuncCallOP_Params FuncCallOP_Params_Suffix

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
// 為了避免 Reduce / Reduce conflict 所以把「函數定義」和「全域變數定義」的前半部提出來
Program :   Type Array_Dimensions ID 
            { CHECK_NOT_IN_CURRENT_SCOPE($3); Global_Level_ID = $3; } 
            Global_Def_Tail
            { free(Global_Level_ID); Global_Level_ID = NULL; }
            Program
          | /* Empty */ ;

Global_Def_Tail :   // Function 
                    '('
                    { // Reset + Return Type + 為函數本體建立 symbol table（會儲存參數、區域變數）
                      memset(&Function_Info, 0, sizeof(Function_Info));
                      // Return type
                      if (Type_Info.type == pVoidType && (Type_Info.dimension > 0 || Type_Info.isConst)) {
                        yyerror("Invalid Return Type");
                        fprintf(stderr, "\t");
                        printTypeInfo(stderr, Type_Info);
                        fprintf(stderr, " is invalid\n");
                        YYERROR;
                      }
                      Function_Info.returnType = Type_Info;
                      // 為函數本體建立 symbol table（會儲存參數、區域變數）
                      Symbol_Table = create(Symbol_Table);
                    }
                    Parameter_Def_List
                    ')' 
                    {  // 將函數加入 Global Symbol Table
                      SymbolTableNode_t* function = insert(Symbol_Table->parent, Global_Level_ID);
                      function->isFunction = true;
                      function->functionTypeInfo = Function_Info;
                    }
                    '{' Statements '}'
                    { // 䆁放 Symbol Table，回到 global scope
                      dump(Symbol_Table);
                      Symbol_Table = freeSymbolTable(Symbol_Table);
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
                CHECK_NOT_IN_CURRENT_SCOPE($1);

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
                      CHECK_NOT_IN_CURRENT_SCOPE($2);

                      if (Type_Info.type == pVoidType) {
                        yyerror("Parameter cannot be void type.");
                        fprintf(stderr, "\tFor Parameter (%s)\n", $2);
                        YYERROR;
                      }

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

// Statements /////////////////////////////////////////////////////////////////////////////
Statements: One_Simple_Statement Statements 
          | Var_Def Statements
          | /* Empty */ ;

One_Simple_Statement:
               Expression ';'         { printf("\t\e[36mExpr = \e[m");  dumpExprTree(stdout, $1); puts(""); freeExprTree($1); }
             | PRINT Expression ';'   { printf("\t\e[36mprint \e[m");   dumpExprTree(stdout, $2); puts(""); freeExprTree($2); }
             | PRINTLN Expression ';' { printf("\t\e[36mprintln \e[m"); dumpExprTree(stdout, $2); puts(""); freeExprTree($2); }
             | RETURN Expression ';'
             { 
                if (isSameTypeInfo_WithoutConst(Function_Info.returnType, $2->resultTypeInfo)) {
                  printf("\t\e[36mreturn \e[m");  dumpExprTree(stdout, $2); puts(""); freeExprTree($2); 
                }
                else {
                  yyerror("Type Error!");
                  fprintf(stderr, "\tFunction (%s) return type = ", Global_Level_ID);
                  printTypeInfo(stderr, Function_Info.returnType);
                  fprintf(stderr, " , but type of expression being returned = ");
                  printTypeInfo(stderr, $2->resultTypeInfo);
                  fprintf(stderr, "\n");
                  YYERROR;
                }
             }
             | RETURN ';'
             {
                if (Function_Info.returnType.type == pVoidType) {
                  printf("\t\e[36mreturn\e[m\n");
                }
                else {
                  yyerror("Type Error!");
                  fprintf(stderr, "\tFunction (%s) return type = ", Global_Level_ID);
                  printTypeInfo(stderr, Function_Info.returnType);
                  fprintf(stderr, " , but nothing is returned.\n");
                  YYERROR;
                }
             }
             | READ Expression ';' 
             { 
                if (isExprLvalue($2)) {
                  printf("\t\e[36mread \e[m");  dumpExprTree(stdout, $2); puts(""); freeExprTree($2);
                }
                else {
                  yyerror("Cannot read value into rvalue!");
                  fprintf(stderr, "\tExpect a lvalue, but got (type = ");
                  printTypeInfo(stderr, $2->resultTypeInfo);
                  fprintf(stderr, ") ");
                  dumpExprTree(stderr, $2);
                  fprintf(stderr, "\n");
                  YYERROR;
                }
             }
             | ';'
             | Block_of_Statements
             | Control_Flow
             ;

Block_of_Statements: '{'         { Symbol_Table = create(Symbol_Table); } 
                     Statements 
                     '}'         { dump(Symbol_Table); Symbol_Table = freeSymbolTable(Symbol_Table); }
                     ;

Control_Flow: IF '(' Condition_Expression ')' One_Simple_Statement
            | IF '(' Condition_Expression ')' One_Simple_Statement ELSE One_Simple_Statement
            | WHILE '(' Condition_Expression ')' One_Simple_Statement
            | FOR '(' For_Initial_Expression ';' For_Condition_Expression ';' For_Update_Expression ')' One_Simple_Statement
            | FOREACH '(' ID ':' Integer_Expression RANGE Integer_Expression ')'
              {
                SymbolTableNode_t* N = lookupRecursive(Symbol_Table, $3);

                CHECK_NODE_NOT_NULL(N, $3);

                if (!N->isFunction && isSameTypeInfo(N->typeInfo, INT_TYPE)) {
                  printf("\t\e[36mForeach \e[m%s\n", $3);
                  free($3);
                }
                else {
                  yyerror("Type Error!");
                  fprintf(stderr, "\tExpect a identifier of int type, but got (type = ");
                  if (N->isFunction)
                    printFunctionTypeInfo(stderr, N->functionTypeInfo);
                  else
                    printTypeInfo(stderr, N->typeInfo);
                  fprintf(stderr, ", ID = %s)\n", $3);
                  YYERROR;
                }
              }
              One_Simple_Statement
            ;

For_Initial_Expression:    Expression { printf("\t\e[36mInitial Expression =  \e[m"); dumpExprTree(stdout, $1); puts(""); freeExprTree($1); }
                         | /* Empty */;
For_Condition_Expression : Condition_Expression 
                         | /* Empty */ { puts("\t\e[36mCondition =  true\e[m"); };
For_Update_Expression:     Expression  { printf("\t\e[36mUpdate Expression =  \e[m");  dumpExprTree(stdout, $1); puts(""); freeExprTree($1); }
                         | /* Empty */;

Condition_Expression: Expression 
                      {
                        if (isSameTypeInfo_WithoutConst($1->resultTypeInfo, BOOL_TYPE)) {
                          printf("\t\e[36mCondition = \e[m"); dumpExprTree(stdout, $1); puts(""); freeExprTree($1);
                        }
                        else {
                          yyerror("Type error!");
                          fprintf(stderr, "\tExpect a boolean expression, but got (Type = ");
                          printTypeInfo(stderr, $1->resultTypeInfo);
                          fprintf(stderr, ") ");
                          dumpExprTree(stderr, $1);
                          fprintf(stderr, "\n");
                          YYERROR;
                        }
                      }
                      ;

Integer_Expression: Expression 
                    {
                      if (isSameTypeInfo_WithoutConst($1->resultTypeInfo, INT_TYPE)) {
                        printf("\t\e[36mInteger Expression = \e[m"); dumpExprTree(stdout, $1); puts(""); freeExprTree($1);
                      }
                      else {
                        yyerror("Type error!");
                        fprintf(stderr, "\tExpect a integer expression, but got (Type = ");
                        printTypeInfo(stderr, $1->resultTypeInfo);
                        fprintf(stderr, ") ");
                        dumpExprTree(stderr, $1);
                        fprintf(stderr, "\n");
                        YYERROR;
                      }
                    }
                    ;

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
          
          // 陣列存取
          | ID ArrayIndexOP 
          { 
            SymbolTableNode_t* N = lookupRecursive(Symbol_Table, $1); 
            CHECK_NODE_NOT_NULL(N, $1);

            if (N->isFunction) {
              yyerror("Array Name expected, but got Function Name.");
              fprintf(stderr, "\tFor ID = %s\n", $1);
              YYERROR;
            }

            if (($$ = exprArrayIndexOP($1, N->typeInfo, $2)) == NULL)
              YYERROR;

            // Note: 不用 free($1)，因為 freeExprTree 會把它清掉
          }
          | ID FuncCallOP
          {
            SymbolTableNode_t* N = lookupRecursive(Symbol_Table, $1);
            CHECK_NODE_NOT_NULL(N, $1);

            if (!N->isFunction) {
              yyerror("Function Name expected, but got Variable.");
              fprintf(stderr, "\tFor ID = %s\n", $1);
              YYERROR;
            }

            if (($$ = exprFuncCallOP($1, N->functionTypeInfo, $2)) == NULL)
              YYERROR;
          }

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
            SymbolTableNode_t *N = lookupRecursive(Symbol_Table, $1);

            CHECK_NODE_NOT_NULL(N, $1);
            
            if (N->isFunction) {
              yyerror("Function name cannot exist alone.");
              fprintf(stderr, "\tFor ID = %s\n", $1);
              YYERROR;
            }

            $$ = calloc(1, sizeof(ExpressionNode_t));
            $$->isID = true;
            $$->resultTypeInfo = N->typeInfo;
            $$->sval = $1; // ID

            // 是常數，且有常數值
            if (N->typeInfo.isConst && N->hasDefaultValue && N->defaultValueIsConstExpr) {
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

ArrayIndexOP: '[' Expression ']' ArrayIndexOP_Suffix
              {
                $2->nextExpression = $4; // 將所有 index 的 expression 串成 linked list
                $$ = $2;
              };

ArrayIndexOP_Suffix: ArrayIndexOP  { $$ = $1; }
                   | /* Empty */   { $$ = NULL; } ;

FuncCallOP: '(' FuncCallOP_Params ')' { $$ = $2; }
          | '(' ')'                   { $$ = NULL; };

FuncCallOP_Params: Expression FuncCallOP_Params_Suffix
                   {
                    $1->nextExpression = $2;
                    $$ = $1;
                   };

FuncCallOP_Params_Suffix: ',' FuncCallOP_Params  { $$ = $2; }
                        | /* Empty */            { $$ = NULL; };

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
     |              { Type_Info.type = pVoidType; } ;

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

  // 全域變數的預設值 只能 是「編譯時期常數」
  if (IN_GLOBAL_SCOPE() && defaultValue && defaultValue->isConstExpr == false) {
    yyerror("Global variable's initial value must be calculated at compile time.");
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
