obj-m		+=	shared_map.o
shared_map-y		:=	main.o map.o entry.o nlsocket.o message.o
#shared_map-y		:=	main.o database.o command.o adaptor.o entry.o

all :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
