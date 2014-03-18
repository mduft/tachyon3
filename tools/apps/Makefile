# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

APPDIR			:= $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
BINDIR			:= $(APPDIR)/bin
CC				:= gcc
CFLAGS			:= -g -O0

RD_SOURCES		:= $(wildcard $(APPDIR)/rd/*.c)

all: $(BINDIR) $(BINDIR)/rd

.PHONY: all

$(BINDIR):
	-mkdir $(BINDIR)

$(BINDIR)/rd: $(RD_SOURCES)
	$(CC) $(CFLAGS) $^ -o $@

