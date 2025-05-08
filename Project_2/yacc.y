%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "symbol_table.h"

extern int linenum;
struct SymbolTable_t Symbol_Table;

int yylex();
void yyerror(char*);

// 確認該 Identifier 沒有在當前 scope 出現過
#define CHECK_NOT_IN_SYMBOL_TABLE(ID) { \
    if (lookup(&Symbol_Table, ID) != NULL) { \
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
    double  rval;
    char*   sval;
    struct {
        int a;
    }a;
}

// Keyword
%token BOOL DOUBLE FLOAT INT STRING_yacc
%token BREAK CASE CHAR CONST CONTINUE DEFAULT DO ELSE EXTERN FALSE FOR FOREACH IF PRINT PRINTLN READ RETURN SWITCH TRUE VOID WHILE
// Operator
%token INCR DECR EQ GE LE NE AND OR
// Literal
%token <ival> INTEGER_LITERAL
%token <rval> REAL_LITERAL
%token <sval> STRING_LITERAL ID

// FIX ME, current : 0 -> No , 1 -> Have default value
%type <ival> Default_Value

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

                if (Type_Info.isConst) { 
                    if (Type_Info.dimension != 0) {
                        yyerror("Definition of Const Array is not supported.");
                        fprintf(stderr, "\tFor Variable (%s).\n", $1);
                        YYERROR;
                    }

                    if ($3 == 0) {
                        yyerror("Constant variable must have initial value");
                        fprintf(stderr, "\tFor Variable (%s).\n", $1);
                        YYERROR;
                    }
                }

                if (Type_Info.dimension != 0 && $3 == 1) {
                    yyerror("Array Initialization is not supported.");
                    fprintf(stderr, "\tFor Variable (%s).\n", $1);
                    YYERROR;
                }

                SymbolTableNode_t* Node = insert(&Symbol_Table, $1);
                Node->isFunction = false;
                Node->typeInfo = Type_Info;
                free($1);
             }
             ID_Def_List_Suffix
           ;

ID_Def_List_Suffix: ',' ID_Def_List | ;

Default_Value: '=' Expression   { $$ = 1; }
              |                 { $$ = 0; } ;

Expression: INTEGER_LITERAL | STRING_LITERAL | REAL_LITERAL;


// 型別 /////////////////////////////////////////////////
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
    Symbol_Table = create();

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
    else
        puts("\e[32mParsing Success!\e[m");

    dump(&Symbol_Table);
}
