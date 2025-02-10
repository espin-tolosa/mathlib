CC_STDVER	:=	-std=c99
CC_OPTIM	:=	-O3
CC_WARNS	:=	-Werror -Wall -Wextra -Wpedantic
CC_TEMPS	:=	-save-temps
CC_WNO		:=	-Wno-gnu-line-marker -Wno-unused
INC_SRC		:=	 src
DIR_SRC		:=	 src
DIR_CUT		:=	 test
DIR_INSTALL :=	 ../math/vendor/fptest

TESTED_FUNCS:=	PREV_FLOAT NEXT_FLOAT

TESTED_FUNCS:=$(addprefix -DTEST_FP_, $(TESTED_FUNCS) )

# Sanitizers
CC_ASAN		:=
LD_ASAN		:=

# Linked libraries
LD_CC		:=	-lm -s -shared -Wl,--subsystem,windows

# Add address sanitizer when using clang compiler
ifeq ($(shell $(CC) --version | grep -i clang),)
# Not Clang, no action
else
CC_ASAN  += -fsanitize=address
LD_ASAN  += -fsanitize=address -static-libsan
endif

CCFLAGS 	:=	$(CC_OPTIM) $(CC_STDVER) $(CC_WARNS) $(CC_WNO) $(CC_TEMPS) $(CC_ASAN) $(CC_FSAN)
LDFLAGS 	:=	$(LD_ASAN) $(LD_CC)

# Compiler deps
INCS		:=	$(wildcard $(INC_SRC)/*.h)
SRCS		:=	$(wildcard $(DIR_SRC)/*.c)
OBJS		:=	$(SRCS:.c=.o)
BINS		:=	fptest.dll

all:$(BINS)

%.o:%.c $(INCS)
	@$(CC) $(CCFLAGS) -c $< -o $@ -DADD_EXPORTS

$(BINS):$(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	@$(RM) -rf $(BINS) $(DIR_SRC)/*.o $(DIR_SRC)/*.s $(DIR_SRC)/*.i

# References

# 1. -Wno-gnu-line-marker included for: clang -Wpedantic -save-temps, (see https://github.com/llvm/llvm-project/issues/63284)

test:
	@$(CC) -c -o test_dll.o test_dll.c $(TESTED_FUNCS)
	@$(CC) -o test_dll.exe -s test_dll.o -L. -lfptest
	./test_dll.exe

install:clean all
ifndef DIR_INSTALL
	@echo "DIR_INSTALL is not defined, please add a relative or absolute path to install fptest.h and fptest.dll"
else
	cp $(BINS) 	$(DIR_INSTALL)/$(BINS);
	cp src/fptest.h $(DIR_INSTALL)/fptest.h
endif
