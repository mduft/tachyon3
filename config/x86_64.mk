# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#CROSS_COMPILER	:= x86_64-pc-elf-

CC				:= ${CROSS_COMPILER}gcc
CXX				:= ${CROSS_COMPILER}g++
LD				:= ${CROSS_COMPILER}ld
CPP				:= ${CROSS_COMPILER}cpp
OBJCOPY			:= ${CROSS_COMPILER}objcopy
NM				:= ${CROSS_COMPILER}nm

KERNEL_CPPFLAGS := -D__X86_64__
KERNEL_CFLAGS	:= -O0 -g -mcmodel=large -mno-red-zone -mno-sse $(KERNEL_CPPFLAGS)
KERNEL_CXXFLAGS	:= $(KERNEL_CFLAGS)
KERNEL_LDFLAGS	:= -z max-page-size=0x1000

KERNEL			:= $(BUILDDIR)/$(ARCH)-tachyon

KERNEL_ADD		:=
