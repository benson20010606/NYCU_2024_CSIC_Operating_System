

MODULE_NAME := kfetch_mod_312512032
obj-m += $(MODULE_NAME).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
unload:
	sudo rmmod kfetch_mod_312512032.ko
load:
	sudo insmod kfetch_mod_312512032.ko
