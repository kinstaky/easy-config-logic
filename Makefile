INCLUDES := -Iinclude
DIRS := src standalone
OBJS :=


PROGRAMS := compare standardize convert config logic_test
SCALER_PROGRAMS := scaler scaler_server scaler_client

.PHONY: all
all: build-subdirs $(OBJS) install

.PHONY: install
install:
	@if [[ ! -d bin ]]; then \
		mkdir bin; \
	fi
	@for prog in $(PROGRAMS); do \
		cp standalone/$$prog bin/$$prog; \
	done
	@for prog in $(SCALER_PROGRAMS); do \
		cp standalone/scaler/$$prog bin/$$prog; \
	done
	

.PHONY: uninstall
uninstall:
	rm -rf bin

.PHONY: clean
clean: clean-subdirs
	rm -f $(OBJS)
	make uninstall

.PHONY: distclean
distclean:
	cd src; make clean
	cd standalone; make distclean


include common.mk