MODULE_NAME		:= kprobes
MODULE_LICENSE		:= GPL
MODULE_VERSION		:= 1.0
MODULE_AUTHOR		:= Muchun Song <songmuchun@bytedance.com>
MODULE_DESCRIPTION	:= Kprobe template for easy register kernel probes

ifneq ($(KERNELRELEASE),)
obj-m			:= $(MODULE_NAME).o
$(MODULE_NAME)-m	:= init.o
$(MODULE_NAME)-m	+= kprobe.o
$(MODULE_NAME)-m	+= $(patsubst $(PWD)/%.c,%.o, $(wildcard $(PWD)/trace/*.c))
ldflags-y		+= -r -T $(PWD)/kprobe.lds

name-fix   = $(squote)$(quote)$(subst $(comma),_,$(subst -,_,$1))$(quote)$(squote)
ccflags-y += -DCONFIG_MODULE_AUTHOR=$(call name-fix,$(MODULE_AUTHOR))
ccflags-y += -DCONFIG_MODULE_VERSION=$(call name-fix,$(MODULE_VERSION))
ccflags-y += -DCONFIG_MODULE_DESCRIPTION=$(call name-fix,$(MODULE_DESCRIPTION))
ccflags-y += -DCONFIG_MODULE_LICENSE=$(call name-fix,$(MODULE_LICENSE))
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
