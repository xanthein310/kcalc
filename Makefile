KDIR=/lib/modules/$(shell uname -r)/build

obj-m += calc.o
obj-m += livepatch_calc.o
calc-objs += main.o expression.o
livepatch_calc-objs += livepatch-calc.o expression.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS)
	make -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

check: all
	scripts/test.sh

clean:
	make -C $(KDIR) M=$(PWD) clean
