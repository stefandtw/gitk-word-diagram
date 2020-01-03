CC = cc
LDLIBS = -lfl -lcgraph -lgvc
DEBUG_CFLAGS=-ggdb3 -fvar-tracking-assignments -Og


build: diffwords

parser.c: diff.y
	bison -d -o $@ $<

lexer.c: diff.l
	flex -o $@ $<

diffwords: ast.h ast.c \
	graph.h graph.c \
	graph_factory.h graph_factory.c \
	graph_walk.h graph_walk.c \
	graph_filter.h graph_filter.c \
	output_plain.h output_plain.c \
	output_gv.h output_gv.c \
	lexer.c parser.c
	${CC} ${DEBUG_CFLAGS} -o $@ $^ ${LDLIBS}

install:
	install diffwords /usr/bin/diffwords
	install gitk-wo /usr/bin/gitk-wo

uninstall:
	rm -f /usr/bin/diffwords
	rm -f /usr/bin/gitk-wo

clean:
	rm -f parser.h parser.c
	rm -f lexer.h lexer.c
	rm -f diffwords

.PHONY: build install uninstall clean
