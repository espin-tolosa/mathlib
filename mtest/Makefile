CCFLAGS := -Wall -Wextra -Wpedantic
LDFLAGS := -L. -lfptest


test_sin:
	$(CC) $(CCFLAGS) test_math_sin.c -o test_math_sin.exe $(LDFLAGS)

test_pow:
	$(CC) $(CCFLAGS) unity.c test_math_pow.c -o test_math_pow.exe $(LDFLAGS)

run_sin:test_sin
	./test_math_sin.exe

run_pow:test_pow
	./test_math_pow.exe
