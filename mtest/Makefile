CCFLAGS := -Wall -Wextra -Wpedantic -O0
LDFLAGS := -L. -lfptest

PREFIX	:= test_math

ifeq ($(shell (which $(CC) || where $(CC)) 2>/dev/null),)
    CC := gcc
endif

.PHONY: to_integer sin pow

COMPILE	:= $(CC) $(CCFLAGS) $(LDFLAGS)

run_%:%
	@./$(PREFIX)_$<.exe

arithmetic:
	@$(COMPILE) $(PREFIX)_$@.c -o $(PREFIX)_$@.exe

to_integer:
	@$(COMPILE) $(PREFIX)_$@.c -o $(PREFIX)_$@.exe -frounding-math -fsignaling-nans

sin:
	@$(COMPILE) $(PREFIX)_$@.c -o $(PREFIX)_$@.exe

pow:
	@$(COMPILE) $(PREFIX)_$@.c -o $(PREFIX)_$@.exe
