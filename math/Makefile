CC_STDVER	:=	-std=c99
CC_TARGET	:=	-Og -save-temps -masm=intel -fverbose-asm

DIR_SRC		:=	src
DIR_CUT		:=	test
CUT_PREFIX	:=	cut_
CCASAN		:=
LDASAN		:=	$(CCASAN)

CCGCOV		:= -fprofile-arcs -ftest-coverage
LDGCOV		:= -lgcov --coverage

CCW			:= -Wall -Wextra
CCWNO		:= -Wno-bitwise-op-parentheses -Wno-shift-op-parentheses -Wno-parentheses -Wno-unused-variable

CCFLAGS 	:=	$(CC_TARGET) $(CC_STDVER) $(CCWARNS) $(CCWNO) $(CCGCOV) $(CCASAN)
LDFLAGS 	:=	$(CC_STDVER) $(LDGCOV) $(LDASAN) -L./vendor/fptest -lfptest

DEPS		:=
INDEPS		:=	$(wildcard $(DIR_SRC)/*.h)

BIN 		:=	bin

RESU_FEXT	:=	.mlog #extension choosen for math results tests

CUT_LIST	:=	elementaries
#sqrt exp log sin cos asin arithmetic pow

#$(subst .c,,$(subst $(DIR_CUT)/,,$(wildcard $(DIR_CUT)/$(CUT_PREFIX)*.c)))

CUT_LIST	:= $(addprefix $(CUT_PREFIX), $(CUT_LIST))

all:$(BIN) $(CUT_LIST)
	cp ./vendor/fptest/fptest.dll ./bin

run:all
	./bin/cut_elementaries

gcov:all
	./run_coverage.sh

#	@echo ""
#	@echo "Running tests..."
#	@echo "================================================================================="
#	@echo ""
#	@find ./$(BIN) -name '$(CUT_PREFIX)*.exe' -exec ./{} \;

cut_%:$(DEPS) test/cut_%.o
	$(CC) $(CCFLAGS) $(LDFLAGS) $^ -o $(BIN)/$@.exe

test/%.o:test/%.c
	$(CC) $(CCFLAGS) -c $< -o $@  -DMKF_RESULT_FNAME=$(call create_result_fname_from_object_output_path, $@)

clean:
	@rm -rf $(BIN) coverage_report fp_test_main.o fptest.dll
	@find . -name '*.o'   -exec rm {} \;
	@find . -name '*.i'   -exec rm {} \;
	@find . -name '*.s'   -exec rm {} \;
	@find . -name '*.exe'   -exec rm {} \;
	@find . -name '*.gc*'   -exec rm {} \;
	@find . -name '*.info'  -exec rm {} \;

clean_mlog:
	@find . -name '*.mlog' -exec rm {} \;

bin:
	@mkdir -p $(BIN)

help:
	@echo ""
	@echo "  call 'make run' to execute all tests with pattern: '$(CUT_PREFIX)*.c' allocated in '$(DIR_CUT)/' which are:"
	@echo ""
	@echo "  $(shell ls $(DIR_CUT)/$(CUT_PREFIX)*.c)"

define create_result_fname_from_object_output_path
\"$(addprefix RESULTS_,$(addsuffix $(RESU_FEXT),$(notdir $(patsubst %.o,%,$(1)))))\"
endef
