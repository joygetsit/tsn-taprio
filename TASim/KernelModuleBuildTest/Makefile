KDIR = /lib/modules/`uname -r`/build
EXTRA_CFLAGS = -Wall -g -DDEBUG

obj-m += TestKernelModuleBuilding.o

all:
	make -C $(KDIR) M=`pwd` modules

clean:
	make -C $(KDIR) M=`pwd` clean
