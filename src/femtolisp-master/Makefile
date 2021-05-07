
NAME = flisp
SRCS = flmain.c flisp.c builtins.c string.c equalhash.c table.c iostream.c
OBJS = $(SRCS:%.c=%.o)
DOBJS = $(SRCS:%.c=%.do)
EXENAME = $(NAME)
LIBTARGET = lib$(NAME)
LLTDIR = llt
LLT = $(LLTDIR)/libllt.a

FLAGS = -falign-functions -Wall -Wno-strict-aliasing -I$(LLTDIR) $(CFLAGS) -DUSE_COMPUTED_GOTO
LIBFILES = $(LLT)
LIBS = $(LIBFILES) -lm

DEBUGFLAGS = -ggdb -DDEBUG $(FLAGS)
SHIPFLAGS = -O3 -DNDEBUG $(FLAGS)

#default: release debug test
default: debug 

test:
	-cd tests && ../flisp unittest.lsp
	cd tests && ../flisp-gdb unittest.lsp

flmain.do: flmain.c flisp.h
flisp.do: flisp.c cvalues.c operators.c types.c flisp.h print.c read.c equal.c

flisp.o:  flisp.c cvalues.c operators.c types.c flisp.h print.c read.c equal.c
	
	
flmain.o: flmain.c flisp.h
csl_flmain.o : csl_flmain.c flisp.h
	
%.o: %.c
	$(CC) $(SHIPFLAGS) -c $< -o $@
%.do: %.c
	$(CC) $(DEBUGFLAGS) -c $< -o $@

$(LLT):
	cd $(LLTDIR) && $(MAKE)

$(LIBTARGET).da: $(DOBJS)
	rm -rf $@
	ar rs $@ $(DOBJS)

$(LIBTARGET).a: $(OBJS)
	rm -rf $@
	ar rs $@ $(OBJS)

debug: $(DOBJS) $(LIBFILES) $(LIBTARGET).da csl_flmain.do
	$(CC) $(DEBUGFLAGS) $(DOBJS) csl_flmain.do -o $(EXENAME)-gdb $(LIBS) $(LIBTARGET).da
#	$(MAKE) test

release: $(OBJS) $(LIBFILES) $(LIBTARGET).a csl_flmain.o
	$(CC) $(SHIPFLAGS) $(OBJS) csl_flmain.o -o $(EXENAME) $(LIBS) $(LIBTARGET).a
#	$(MAKE) test

clean:
	rm -f *.o
	rm -f *.do
	rm -f $(EXENAME)
	rm -f $(EXENAME)-gdb
	rm -f $(LIBTARGET).a
	rm -f $(LIBTARGET).da
