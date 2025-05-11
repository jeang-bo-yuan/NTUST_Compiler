parser: lex.yy.c y.tab.c \
        symbol_table.h symbol_table.c\
		type_info.h type_info.c \
		expression.h expression.c
	gcc -g -o parser lex.yy.c y.tab.c symbol_table.c type_info.c expression.c

lex.yy.c: lex.l
	lex lex.l

y.tab.c: yacc.y symbol_table.c \
         symbol_table.h type_info.h expression.h
	yacc -d yacc.y
	# 在 y.tab.h 前 include expression.h
	echo '#include "expression.h"' | cat - y.tab.h > temp && mv temp y.tab.h
