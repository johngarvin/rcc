include Makefile.$(HOSTTYPE)

clean:
	-rm -f rcc *.o tests/*.c tests/*.so sim00.out *~ run
