#set arch and cross compile tools
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-

CFLAGS_GCOV	:= -fprofile-arcs -ftest-coverage \
	$(call cc-option,-fno-tree-loop-im) \
	$(call cc-disable-warning,maybe-uninitialized,)
export CFLAGS_GCOV

GCOV_PROFILE := y
# set kernel root dir
BASEINCLUDE ?= ~/Linux/runninglinuxkernel_5.0/
#CFLAGS := -fprofile-arcs -ftest-coverage
export CFLAGS
mytest-objs := gcov.o
obj-m := gcov.o

all:
	echo $(_c_flags)
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko;
