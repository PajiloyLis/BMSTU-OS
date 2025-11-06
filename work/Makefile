obj-m := mywq.o
work-objs := mywq.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install:
	sudo insmod work.ko

uninstall:
	sudo rmmod work

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
