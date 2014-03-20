# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

ARCH			:= x86_64
VERBOSE			:= 0
SHELL			:= bash
MAKE			:= $(MAKE) --no-print-directory

CONFIGDIR		:= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
ARCH_MAKEFILE	:= $(CONFIGDIR)/$(ARCH).mk
ENV_MAKEFILE	:= $(CONFIGDIR)/../.package/env.mk

include $(ENV_MAKEFILE)
include $(ARCH_MAKEFILE)
include $(CONFIGDIR)/compile.mk
