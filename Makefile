output_binary := zasm

c_sources :=	\
	./common.c	\
	./elfheader.c	\
	./lexer.c	\
	./zasm.c	\
	./assemble.c	\
	./opcode_table.c

c_objs = $(c_sources:%.c=%.o)

CFLAGS := -g
CC := gcc
LD := ld

all: $(c_sources)
	$(CC) $(CFLAGS) $(c_sources) -o $(output_binary)

clean:
	@find . -name '*\.o' -exec \rm {} \;	
	@\rm zasm

.PHONY: all
