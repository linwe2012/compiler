CC = gcc
PROGRAM=xcc
FLEXFLAGS =
SRCS = lexer.c parser.c ast.c main.c
YFLAGS = -d
CCFLAGS = 

all: $(PROGRAM)

lexer.c: lex.l
	flex $(FLEXFLAGS) -olexer.c lex.l

parser.c: parser.y
	bison $(YFLAGS) -oparser.c parser.y

lexer: lexer.c parser.c

xcc: $(SRCS)
	$(CC) -o $@ $(SRCS) $(CCFLAGS) -lfl -lm