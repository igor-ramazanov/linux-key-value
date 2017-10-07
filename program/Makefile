PROG	=	test
OBJS	=	main.o nlsocket.o shared_map.o message.o
CC	=	gcc
CFLAGS	=	-ggdb3
LDFLAGS	=	-pthread -ggdb3

all : $(PROG)

clean :
	rm -f $(OBJS) $(PROG)

$(PROG) : $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS)

.c.o :
	$(CC) -c $(CFLAGS) $<
