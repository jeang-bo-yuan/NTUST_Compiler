parser: lex.yy.c y.tab.c \
        symbol_table.c symbol_table.h \
		type_info.h type_info.c
	gcc -o parser lex.yy.c y.tab.c symbol_table.c type_info.c

lex.yy.c: lex.l
	lex lex.l

y.tab.c: yacc.y symbol_table.c symbol_table.h
	yacc -d yacc.y
