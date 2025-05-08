%{
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"

#define LIST     strcat(buf,yytext)

#define DEBUG(f, ...) printf("\e[33m\t" f "\e[m", __VA_ARGS__)

#define token(t)           { \
                                LIST; \
                                DEBUG("<%s>\n", #t); \
                                return t; \
                           }

#define tokenInteger(t, i) { \
                                LIST; \
                                DEBUG("<%s:%d>\n", #t, i); \
                                yylval.ival = i; \
                                return t; \
                           }

#define tokenReal(t, r)    { \
                                LIST; \
                                DEBUG("<%s:%f>\n", #t, r); \
                                yylval.rval = r; \
                                return t; \
                           }

#define tokenString(t, s)  { \
                                LIST; \
                                DEBUG("<%s:%s>\n", #t, s); \
                                char* dst = calloc(strlen(s), sizeof(char)); \
                                strcpy(dst, s); \
                                yylval.sval = dst; \
                                return t; \
                           }

#define MAX_LINE_LENG 256

int linenum = 1;
char buf[MAX_LINE_LENG];
static char string_buf[MAX_LINE_LENG];
%}

%option noyywrap
%s MULTI_COMMENT
%s STRING

%%
\/\/.*$                 { LIST; }
"/*"                    { LIST; BEGIN MULTI_COMMENT; }
<MULTI_COMMENT>[^*/\n]+ { LIST; }
<MULTI_COMMENT>"*/"     { LIST; BEGIN 0;}
<MULTI_COMMENT>[*/]     { LIST; }


<STRING>\"        {
                        BEGIN INITIAL;
                        tokenString(STRING_LITERAL, string_buf);
                  }
\"                { 
                        LIST;
                        string_buf[0] = '\0'; // reset string_buf
                        BEGIN STRING;
                  }
<STRING>[^"\n]+   { LIST; strcat(string_buf, yytext); }
<STRING>\"\"      { LIST; strcat(string_buf, "\""); }
<STRING>\n        { strcat(string_buf, "\n"); REJECT; }

"."		{ token('.'); }
","		{ token(','); }
":"		{ token(':'); }
";"		{ token(';'); }
"("		{ token('('); }
")"		{ token(')'); }
"["		{ token('['); }
"]"		{ token(']'); }
"{"		{ token('{'); }
"}"		{ token('}'); }
"++"		{ token(INCR); }
"+"		{ token('+'); }
"--"		{ token(DECR); }
"-"		{ token('-'); }
"*"		{ token('*'); }
"/"		{ token('/'); }
"%"		{ token('%'); }
"=="		{ token(EQ); }
">="		{ token(GE); }
"<="		{ token(LE); }
"!="		{ token(NE); }
"="		{ token('='); }
">"		{ token('>'); }
"<"		{ token('<'); }
"!"		{ token('!'); }
"&&"		{ token(AND); }
"||"		{ token(OR); }

"bool"		{ token(BOOL); }
"break"		{ token(BREAK); }
"case"		{ token(CASE); }
"char"		{ token(CHAR); }
"const"		{ token(CONST); }
"continue"	{ token(CONTINUE); }
"default"	{ token(DEFAULT); }
"do"		{ token(DO); }
"double"	{ token(DOUBLE); }
"else"		{ token(ELSE); }
"extern"	{ token(EXTERN); }
"false"		{ token(FALSE); }
"float"		{ token(FLOAT); }
"for"		{ token(FOR); }
"foreach"	{ token(FOREACH); }
"if"		{ token(IF); }
"int"		{ token(INT); }
"print"		{ token(PRINT); }
"println"	{ token(PRINTLN); }
"read"		{ token(READ); }
"return"	{ token(RETURN); }
"string"	{ token(STRING_yacc); }
"switch"	{ token(SWITCH); }
"true"		{ token(TRUE); }
"void"		{ token(VOID); }
"while"		{ token(WHILE); }

[a-zA-Z_][a-zA-Z_0-9]*  { tokenString(ID, yytext); }

[0-9]+\.[0-9]*([eE][+-]?[0-9]+)? { tokenReal(REAL_LITERAL, atof(yytext)); }

[0-9]+ { tokenInteger(INTEGER_LITERAL, atoi(yytext)); }


\n      {
        LIST;
        printf("\e[32mLine %03d:\e[m %s", linenum++, buf);
        buf[0] = '\0';
        }

[ \t\r]*  {LIST;}

.       {
        LIST;
        printf("\e[31mBad character at line No. %d:'%s'\n\e[m", linenum, yytext);
        return 256;
        }

%%
