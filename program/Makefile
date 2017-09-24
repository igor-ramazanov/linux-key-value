PROG	=	test
OBJS	=	main.o client.o command.o
CC	=	gcc
LDFLAGS	=	-pthread -ggdb3

all : $(PROG)

clean :
	rm -f $(OBJS) $(PROG)

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS)

.c.o :
	$(CC) -c $(CFLAGS) $<
