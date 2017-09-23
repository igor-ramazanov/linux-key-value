PROG	=	test
OBJS	=	main.o
CC	=	gcc
CFLAGS	=	-Wall -pedantic
LDFLAGS	=	-pthread

all : $(PROG)

clean :
	rm -f $(OBJS) $(PROG)

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS)

.c.o :
	$(CC) -c $(CFLAGS) $<
