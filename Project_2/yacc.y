%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "symbol_table.h"

extern int linenum;
struct SymbolTable_t symbol_table;

int yylex();
void yyerror(char*);

enum PrimitiveType_t {
    pIntType,
    pBoolType,
    pStringType,
    pFloatType,
    pDoubleType
};

static struct {
    bool isConst;
    enum PrimitiveType_t type;
} Type_Info;

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

%%
Program : Var_Def Program | ;

Var_Def: Type ID_Def_List ';';

ID_Def_List: ID
             { 
                if (Type_Info.isConst) { 
                    yyerror("Constant variable must have initial value");
                    YYERROR;
                }

                printf("Insert (type:%d) %s into symbol table", Type_Info.type, $1);
             }
             ID_Def_List_Suffix
           | 
             ID '=' Expression 
             {
                printf("Insert (type:%d) %s into symbol table", Type_Info.type, $1);
             }
             ID_Def_List_Suffix;

ID_Def_List_Suffix: ',' ID_Def_List | ;

Expression: INTEGER_LITERAL;


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
     | STRING_yacc  { Type_Info.type = pStringType; };





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
    symbol_table = create();

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

    dump(&symbol_table);
}
