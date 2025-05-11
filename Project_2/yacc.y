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

// 確認該 Identifier 沒有在當前 scope 出現過
#define CHECK_NOT_IN_SYMBOL_TABLE(ID) { \
    if (lookup(Symbol_Table, ID) != NULL) { \
        yyerror("Variable redifined."); \
        fprintf(stderr, "\tVariable (%s) is redifined.\n", ID); \
        YYERROR; \
    } \
}

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
Program : Var_Def Program | ;

// 變數定義/宣告 ///////////////////////////////////////////////////////////////////////// 
Var_Def: Type ID_Def_List ';';

ID_Def_List: ID Array_Dimensions Default_Value
             { 
                CHECK_NOT_IN_SYMBOL_TABLE($1);

                if (Type_Info.type == pVoidType) {
                    yyerror("Variable cannot be void type.");
                    fprintf(stderr, "\tFor Variable (%s).\n", $1);
                    YYERROR;
                }

                SymbolTableNode_t* Node = insert(Symbol_Table, $1);
                Node->isFunction = false;
                Node->typeInfo = Type_Info;

                if ($3) {
                  printf("\t\e[35mFor Variable:\e[m %s\n", $1);
                  printf("\t\e[35mDefault Value = \e[m ");
                  dumpExprTree(stdout, $3);
                  printf("\n");
                }

                if (Node->typeInfo.isConst && ($3 == NULL || $3->isConstExpr == false)) {
                  yyerror("Constant variable must have initial value that can be calculated at compile time.");
                  fprintf(stderr, "\tFor Variable (%s).\n", $1);
                  YYERROR;
                }

                if ($3) {
                  // 如果變數型別和 Expression 的型別不同
                  if (isSameTypeInfo_WithoutConst(Node->typeInfo, $3->resultTypeInfo) == false) {
                    yyerror("Variable and its default value have different type.");

                    fprintf(stderr, "\tFor variable: %s, ", $1);
                    printTypeInfo(stderr, Node->typeInfo);
                    fprintf(stderr, " V.S. ");
                    printTypeInfo(stderr, $3->resultTypeInfo);
                    fprintf(stderr, "\n");

                    YYERROR;
                  }

                  // 記錄起始值
                  if ($3->isConstExpr) {
                    switch ($3->resultTypeInfo.type) {
                      case pIntType:    Node->ival = $3->cIval; break;
                      case pFloatType:  Node->fval = $3->cFval; break;
                      case pDoubleType: Node->dval = $3->cDval; break;
                      case pBoolType:   Node->bval = $3->cBval; break;
                      case pStringType: {
                        Node->sval = calloc(strlen($3->cSval), sizeof(char));
                        strcpy(Node->sval, $3->cSval);
                        break;
                      }
                    }
                  }
                }

                // free resource
                free($1);
                freeExprTree($3);
             }
             ID_Def_List_Suffix
           ;

ID_Def_List_Suffix: ',' ID_Def_List | ;

Default_Value: '=' Expression   { $$ = $2; }
              |                 { $$ = NULL; } ;

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
                        Type_Info.DIMS = calloc(Type_Info.dimension, sizeof(unsigned));
                        memcpy(Type_Info.DIMS, DIMS_Buffer, Type_Info.dimension * sizeof(unsigned));
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
    }
}
