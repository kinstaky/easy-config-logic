INCLUDES := -I../include
DIRS :=
OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

include ../common.mk

SYNTAX_OBJS := $(patsubst %.cpp,%.o,$(call rwildcard,../src/syntax,*.cpp))
STANDARDIZE_OBJS := $(patsubst %.cpp,%.o,$(call rwildcard,../src/standardize,*.cpp))
CONFIG_OBJS := $(patsubst %.cpp,%.o,$(call rwildcard,../src/config,*.cpp))

PROGRAMS := compare standardize convert config logic_test
compare_OBJS := compare_logical_expression.o $(SYNTAX_OBJS)
standardize_OBJS := standard_logic_adapter.o $(SYNTAX_OBJS) $(STANDARDIZE_OBJS)
convert_OBJS := convert.o $(SYNTAX_OBJS) $(STANDARDIZE_OBJS) $(CONFIG_OBJS)
config_OBJS := config.o i2c.o $(SYNTAX_OBJS) $(STANDARDIZE_OBJS) $(CONFIG_OBJS)
logic_test_OBJS := logic_test.o $(SYNTAX_OBJS) $(STANDARDIZE_OBJS) $(CONFIG_OBJS)


define PROGRAM_template =
$(1): $$($(1)_OBJS)
	$$(CC) -o $$@ $$(CFLAGS) $$^
endef
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))


.PHONY: all
all: build-subdirs $(PROGRAMS)

.PHONY: distclean
distclean:
	rm -f $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS)

