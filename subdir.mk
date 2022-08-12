.PHONY: all
all: build-subdirs $(OBJS)

.PHONY: clean
clean: clean-subdirs
	rm -f $(OBJS)