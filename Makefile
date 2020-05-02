MODULE_NAME		:= kprobes

ifneq ($(KERNELRELEASE),)
obj-m			:= $(MODULE_NAME).o
$(MODULE_NAME)-m	:= init.o
$(MODULE_NAME)-m	+= kprobe.o
ldflags-y		+= -r -T $(PWD)/kprobe.lds
else
PWD			:= $(shell pwd)
KERNEL_HEAD		:= $(shell uname -r)
KERNELDIR		:= /lib/modules/$(KERNEL_HEAD)/build

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

install:
	insmod $(MODULE_NAME).ko

remove:
	rmmod $(MODULE_NAME)

endif
