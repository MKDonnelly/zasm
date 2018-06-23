
output_binary := zasm

c_sources :=	\
	./common.c	\
	./elfheader.c	\
	./zasm.c	\
	./assemble.c

c_objs = $(c_sources:%.c=%.o)

CFLAGS := 
CC := gcc
LD := ld

all: $(c_sources)
	$(CC) $(CFLAGS) $(c_sources) -o $(output_binary)

clean:
	@find . -name '*\.o' -exec \rm {} \;	
	@\rm zasm

.PHONY: all
