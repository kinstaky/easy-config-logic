# make flags
MAKEFLAGS += --warn-undefined-variables
# compile macros
CC := g++
# use bash
SHELL := /bin/bash

# compile flags 
CFLAGS := -std=c++17 -O3 -Werror -Wall -Wextra
OBJCFLAGS := $(CFLAGS) -c

# recursive wildcard
rwildcard=$(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2))$(filter $(subst *,%,$(2)),$(d)))

# recursive make and clean
.PHONY: build-subdirs
build-subdirs: $(DIRS)


.PHONY: $(DIRS)
$(DIRS):
	make -C $@ all


.PHONY: clean-subdirs
clean-subdirs:
	@for dir in $(DIRS); do \
		make -C $$dir clean; \
	done

# dependencies
$(OBJS):%.o:%.cpp
	$(CC) -o $@ $(OBJCFLAGS) $(INCLUDES) $<