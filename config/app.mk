# Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
# This file is part of the 'tachyon' operating system.

all: all-target

.PHONY: all

include $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/config.mk)

all-target: $(TARGET)
	@printf "app ready: "
	@(cd $(dir $(TARGET)) && ls -hs $(notdir $(TARGET)))

$(TARGET): $(OBJECTS)
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[LD  ] $(notdir $@)"; \
	 else \
	 	echo "$(CC) $(CFLAGS) $(LDFLAGS) -o \"$@\" $(OBJECTS)"; \
	 fi
	@$(CC) $(CFLAGS) $(LDFLAGS) -o "$@" $^

