INCLUDES :=
DIRS :=
OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

include ../../common.mk

PROGRAMS := scaler scaler_server scaler_client


define PROGRAM_template =
$(1): $(1).o
	$$(CC) -o $$@ $$(CFLAGS) -pthread $$^
endef
$(foreach prog,$(PROGRAMS),$(eval $(call PROGRAM_template,$(prog))))


.PHONY: all
all: $(PROGRAMS)

.PHONY: distclean
distclean:
	rm -f $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS)

