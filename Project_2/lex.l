%{
#include <stdlib.h>
#include "symbol_table.h"
#define LIST     strcat(buf,yytext)
#define token(t) {LIST; printf("<%s>\n",t);}
#define tokenInteger(t,i) {LIST; printf("<%s:%d>\n",t,i);}
#define tokenString(t,s) {LIST; printf("<%s:%s>\n",t,s);}

#define MAX_LINE_LENG 256

int linenum = 1;
char buf[MAX_LINE_LENG];
static char string_buf[MAX_LINE_LENG];
struct SymbolTable_t symbol_table;
%}

%option noyywrap
%Start MULTI_COMMENT
%Start STRING

%%
\/\/.*$                 { LIST; }
"/*"                    { LIST; BEGIN MULTI_COMMENT; }
<MULTI_COMMENT>[^*/\n]+ { LIST; }
<MULTI_COMMENT>"*/"     { LIST; BEGIN 0;}
<MULTI_COMMENT>[*/]     { LIST; }


<STRING>\"        {
                        tokenString("string", string_buf);
                        string_buf[0] = '\0';
                        BEGIN 0;
                  }
\"                { 
                        LIST;
                        BEGIN STRING;
                  }
<STRING>[^"\n]+   { LIST; strcat(string_buf, yytext); }
<STRING>\"\"      { LIST; strcat(string_buf, "\""); }
<STRING>\n        { strcat(string_buf, "\n"); REJECT; }

"."		{ token("'.'"); }
","		{ token("','"); }
":"		{ token("':'"); }
";"		{ token("';'"); }
"("		{ token("'('"); }
")"		{ token("')'"); }
"["		{ token("'['"); }
"]"		{ token("']'"); }
"{"		{ token("'{'"); }
"}"		{ token("'}'"); }
"++"		{ token("'++'"); }
"+"		{ token("'+'"); }
"--"		{ token("'--'"); }
"-"		{ token("'-'"); }
"*"		{ token("'*'"); }
"/"		{ token("'/'"); }
"%"		{ token("'%'"); }
"=="		{ token("'=='"); }
">="		{ token("'>='"); }
"<="		{ token("'<='"); }
"!="		{ token("'!='"); }
"="		{ token("'='"); }
">"		{ token("'>'"); }
"<"		{ token("'<'"); }
"!"		{ token("'!'"); }
"&&"		{ token("'&&'"); }
"||"		{ token("'||'"); }

"bool"		{ token("BOOL"); }
"break"		{ token("BREAK"); }
"case"		{ token("CASE"); }
"char"		{ token("CHAR"); }
"const"		{ token("CONST"); }
"continue"	{ token("CONTINUE"); }
"default"	{ token("DEFAULT"); }
"do"		{ token("DO"); }
"double"	{ token("DOUBLE"); }
"else"		{ token("ELSE"); }
"extern"	{ token("EXTERN"); }
"false"		{ token("FALSE"); }
"float"		{ token("FLOAT"); }
"for"		{ token("FOR"); }
"foreach"	{ token("FOREACH"); }
"if"		{ token("IF"); }
"int"		{ token("INT"); }
"print"		{ token("PRINT"); }
"println"	{ token("PRINTLN"); }
"read"		{ token("READ"); }
"return"	{ token("RETURN"); }
"string"	{ token("STRING"); }
"switch"	{ token("SWITCH"); }
"true"		{ token("TRUE"); }
"void"		{ token("VOID"); }
"while"		{ token("WHILE"); }

[a-zA-Z_][a-zA-Z_0-9]*  {
                        insert(&symbol_table, yytext);
                        tokenString("ID", yytext); 
                        }

[0-9]+\.[0-9]*([eE][+-]?[0-9]+)? { tokenString("real", yytext); }

[0-9]+ { tokenInteger("int", atoi(yytext)); }


\n      {
        LIST;
        printf("%d: %s", linenum++, buf);
        buf[0] = '\0';
        }

[ \t\r]*  {LIST;}

.       {
        LIST;
        printf("%d:%s\n", linenum+1, buf);
        printf("bad character:'%s'\n",yytext);
        exit(-1);
        }

%%
int main() {
        symbol_table = create();
        yylex();
        dump(&symbol_table);
}
