CC=gcc
R=/local/software/R-1.6.2
CPPFLAGS=-g -O0 -Wall -I$(R)/src/include -I.
LDFLAGS=-L$(R)/bin -lR -lstdc++

.SECONDARY:  # don't delete intermediate .c files

TESTS=$(wildcard tests/*.r)
TARGS=$(subst .r,.so,$(TESTS))

all: $(TARGS)

rcc: rcc.o get_name.o

tests/test%.so: tests/test%.c rcc_lib.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) -fpic -shared rcc_lib.o $< -o $@

tests/test%.c: tests/test%.r rcc 
	./rcc $<

tests/mgg00.so: tests/mgg00.c rcc_lib.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) -fpic -shared rcc_lib.o $< -o $@

tests/mgg00.c: tests/mgg00.r rcc
	./rcc tests/mgg00.r

clean:
	-rm -f rcc *.o tests/*.c tests/*.so sim00.out *~ run
