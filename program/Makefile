PROG	=	test
OBJS	=	main.o
CC	=	gcc
CFLAGS	=	-Wall -pedantic -ggdb3
LDFLAGS	=	-pthread -ggdb3

all : $(PROG)

clean :
	rm -f $(OBJS) $(PROG)

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS)

.c.o :
	$(CC) -c $(CFLAGS) $<
