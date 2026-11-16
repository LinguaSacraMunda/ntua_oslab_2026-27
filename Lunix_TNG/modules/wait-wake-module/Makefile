obj-m := wait_wake.o
wait_wake-objs := module.o chrdev.o

# If KERNELDIR is not already set, set it to the build tree of the current kernel
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all: modules

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(KERNEL_VERBOSE) $(KERNEL_MAKE_ARGS) modules

clean: 
	$(MAKE) -C $(KERNELDIR) M=$(PWD) $(KERNEL_VERBOSE) $(KERNEL_MAKE_ARGS) clean
