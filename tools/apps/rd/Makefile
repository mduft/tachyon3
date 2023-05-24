# Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
# This file is part of the 'tachyon' operating system.

TOPSRC	:= $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../)
TARGET	 = $(BUILDDIR)/rd

KCFLAGS := $(CFLAGS)

include $(TOPSRC)/config/app.mk

# override compiler to system compiler and linker
CC		:= gcc

