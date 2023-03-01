#set arch and cross compile tools
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

# set kernel root dir
BASEINCLUDE ?= ~/Linux/runninglinuxkernel_5.0/

mytest-objs := gcov_ko.o
obj-m := gcov_ko.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko;
