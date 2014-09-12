TARGET = hello.ko

CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)

obj-m := pagefault.o

all:
	make -C $(KDIR) M=$(PWD) modules
	#make -C /lib/modules/`uname -r`/build M=`pwd` modules

clean:
	rm -f *.o pagefault.ko .*.cmd .*.flags *.mod.c

