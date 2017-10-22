obj-m		+=	shared_map.o
shared_map-y	:=	main.o map.o entry.o nlsocket.o message.o logger.o \
			storage.o file.o

all :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

debug :
	make EXTRA_CFLAGS="-DLOGGER_LOGLEVEL=4" -C \
	/lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
