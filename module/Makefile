obj-m		+=	moddb.o
moddb-y		:=	main.o database.o command.o adaptor.o entry.o

all :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
