EXECS 	= fake-arm socket-client

SRC	= fake-arm.c socke-client.c interpreter.c parameters.c
INCL  	= 

GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0)
GLIB_LDFLAGS = $(shell pkg-config --libs glib-2.0)

IPATH   = -I.
OBJOPT  = -O0 -Wall -fPIC     # option for obj
EXEOPT  = -O0 -Wall # -lgsl -lgslcblas -lm  -lbms -lretmono  # option for exe (-lefence ...)
LPATH   = # -L../../lib           # path for librairies ... 

CC 	= cc
MAKE 	= make
SHELL	= /bin/sh
OBJS 	= $(SRC:.c=.o) 
RM 	= /bin/rm -f 	
COMP	= gzip -9v
UNCOMP	= gzip -df
STRIP	= strip

CFLAGS  = $(OBJOPT) $(IPATH) -D_GNU_SOURCE -g $(GLIB_CFLAGS)
LDFLAGS = $(EXEOPT) $(LPATH) $(GLIB_LDFLAGS) -pthread

ALL = $(EXECS)

all: $(ALL)

clean:
	rm -f *.o *~ 

distclean: clean
	-$(RM) $(ALL)

.c.o:
	$(CC) $(CFLAGS) -c $< 

fake-arm: interpreter.o parameters.o commands.o interrupt.o

curves: main
	./main

%.dat: %
	./$<

%.eps: %.gp
	$(GP-TO-EPSLATEX) in $< out $@

%.pdf: %.eps
	epstopdf $<
