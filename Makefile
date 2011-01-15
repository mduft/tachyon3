# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#ARCH			:= x86
ARCH			:= x86_64
VERBOSE			:= 0

SHELL			:= bash

SOURCEDIR		:= $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
BUILDDIR		:= $(SOURCEDIR)/.build/$(ARCH)

ARCH_MAKEFILE	:= $(SOURCEDIR)/config/$(ARCH).mk
ARCH_LINKSCRIPT	:= $(SOURCEDIR)/config/$(ARCH).ld
ARCH_PPLSCRIPT  := $(BUILDDIR)/$(ARCH).ld

#.----------------------------------.
#| Include the platform config.     |
#'----------------------------------'

include $(ARCH_MAKEFILE)

GDB				:= x86_64-pc-linux-gnu-gdb

BASE_CPPFLAGS   := -Wall -Wextra -I$(SOURCEDIR)/src
BASE_CFLAGS		:= $(BASE_CPPFLAGS) -Werror -ffreestanding -nostartfiles -nostdlib -Wno-unused-parameter -std=gnu99
BASE_CXXFLAGS	:= $(BASE_CFLAGS) -fno-rtti -fno-exceptions -Wold-style-cast
BASE_LDFLAGS	 = -T $(ARCH_PPLSCRIPT) -Map=$(KERNEL).map

BASE_QFLAGS		:= -curses
BASE_GDBFLAGS	:= 

MAKE			:= $(MAKE) --no-print-directory

GRUB2_CFG		:= $(BUILDDIR)/boot/grub/grub.cfg
GRUB2_PXE		 = $(BUILDDIR)/boot/grub/i386-pc/core.0
GRUB2_ISO		 = $(BUILDDIR)/$(notdir $(KERNEL))-grub2.iso

#.----------------------------------.
#| The all target, keep it first!   |
#'----------------------------------'

all: all-kernel

.PHONY: all

#.----------------------------------.
#| Build internal/accumulated vars. |
#'----------------------------------'

KERNEL_SOURCES	:= \
	$(wildcard $(SOURCEDIR)/src/*.c) \
	$(wildcard $(SOURCEDIR)/src/*.S) \
	$(wildcard $(SOURCEDIR)/src/$(ARCH)/*.c) \
	$(wildcard $(SOURCEDIR)/src/$(ARCH)/*.S) \
	$(KERNEL_ADD)

KERNEL_CSOURCES := $(filter %.c,$(KERNEL_SOURCES))
KERNEL_SSOURCES := $(filter %.S,$(KERNEL_SOURCES))

KERNEL_COBJECTS := $(subst $(SOURCEDIR),$(BUILDDIR),$(KERNEL_CSOURCES:.c=.o))
KERNEL_SOBJECTS := $(subst $(SOURCEDIR),$(BUILDDIR),$(KERNEL_SSOURCES:.S=.o))

KERNEL_OBJECTS  := $(KERNEL_COBJECTS) $(KERNEL_SOBJECTS)

KCPPFLAGS	:= $(BASE_CPPFLAGS) $(CPPFLAGS) $(KERNEL_CPPFLAGS)
KCFLAGS		:= $(BASE_CFLAGS) $(CFLAGS) $(KERNEL_CFLAGS)
KLDFLAGS	:= $(BASE_LDFLAGS) $(LDFLAGS) $(KERNEL_LDFLAGS)

MAKE_BDIR 	 = test -d "$(dir $@)" || mkdir -p "$(dir $@)"

#.----------------------------------.
#| Specialized Rules                |
#'----------------------------------'

$(ARCH_PPLSCRIPT): $(ARCH_LINKSCRIPT)
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[CPP ] $(notdir $@)"; \
	 else \
	 	echo "$(CPP) $(KCPPFLAGS) -P -D__ASM__ -o \"$@\" \"$<\""; \
	 fi
	@$(CPP) $(KCPPFLAGS) -P -D__ASM__ -o "$@" "$<"

$(KERNEL): $(KERNEL_OBJECTS) $(ARCH_PPLSCRIPT)
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[LD  ] $(notdir $@)"; \
	 else \
	 	echo "$(LD) $(KLDFLAGS) -o \"$@\" $(KERNEL_OBJECTS)"; \
	 fi
	@$(LD) $(KLDFLAGS) -o "$@" $(KERNEL_OBJECTS)

$(KERNEL).dbg: $(KERNEL)
	@if test $(VERBOSE) = 0; then \
		echo "[SPLT] $(notdir $@)"; \
	 else \
		echo "$(OBJCOPY) --only-keep-debug \"$<\" \"$@\""; \
		echo "$(OBJCOPY) --strip-debug \"$<\""; \
		echo "$(OBJCOPY) --add-gnu-debuglink=\"$@\" \"$<\""; \
		echo "touch \"$@\""; \
	 fi
	@$(OBJCOPY) --only-keep-debug "$<" "$@"
	@$(OBJCOPY) --strip-debug "$<"
	@$(OBJCOPY) --add-gnu-debuglink="$@" "$<"
	@touch "$@"

all-kernel: $(KERNEL) $(KERNEL).dbg
	@printf "kernel ready: " 
	@(cd $(dir $(KERNEL)) && ls -hs $(notdir $(KERNEL)))

clean:
	@-rm -rf $(SOURCEDIR)/.build

#.----------------------------------.
#| General Template Rules           |
#'----------------------------------'

include $(foreach mf,$(subst .o,.Po,$(KERNEL_OBJECTS)),$(wildcard $(mf)))

$(KERNEL_COBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[CC  ] $(subst $(SOURCEDIR)/,,$<)"; \
	 else \
	 	echo "$(CC) $(KCFLAGS) -c -o \"$@\" \"$<\""; \
	 fi
	@$(CC) -MMD -MF "$(subst .o,.Po,$@)" $(KCFLAGS) -c -o "$@" "$<"

$(KERNEL_SOBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.S
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[AS  ] $(subst $(SOURCEDIR)/,,$<)"; \
	 else \
	 	echo "$(CC) $(KCFLAGS) -D__ASM__ -c -o \"$@\" \"$<\""; \
	 fi
	@$(CC) -MMD -MF "$(subst .o,.Po,$@)" $(KCFLAGS) -D__ASM__ -c -o "$@" "$<"

#.----------------------------------.
#| Helper rules                     |
#'----------------------------------'

all-archs:
	@for x in $(SOURCEDIR)/config/*.mk; do arch="$${x%.mk}"; arch=$${arch##*/}; echo "building $$arch ..."; \
	 $(MAKE) -f $(SOURCEDIR)/Makefile ARCH=$${arch} all; done

all-archs-iso:
	@for x in $(SOURCEDIR)/config/*.mk; do arch="$${x%.mk}"; arch=$${arch##*/}; echo "building $$arch ..."; \
	 $(MAKE) -f $(SOURCEDIR)/Makefile ARCH=$${arch} iso; done

#
# qemu direct kernel boot.
#

KQFLAGS := $(BASE_QFLAGS) $(QFLAGS)

qemu: $(KERNEL)
	@arch=$(ARCH); arch=$${arch%%-*}; qe=$$(type -p qemu-system-$${arch}); test -z "$${qe}" && qe=qemu; \
	 echo "using $${qe}..."; $${qe} $(KQFLAGS) -kernel $(KERNEL);

qemu-dbg:
	@$(MAKE) -f $(SOURCEDIR)/Makefile qemu QFLAGS="$(QFLAGS) -s -S"

gdb:
	@gdb=$$(type -p cgdb); test -x "$${gdb}" || gdb=$(GDB); test -x "$${gdb}" || gdb=gdb; \
	 echo "using $${gdb}..."; \
	$${gdb} -x $(SOURCEDIR)/config/misc/gdb-qemu.gdbinit $(KERNEL)
	
#
# grub2 network boot
#

qemu-netboot-grub2: $(KERNEL) $(GRUB2_CFG) $(GRUB2_PXE)
	@arch=$(ARCH); arch=$${arch%%-*}; qe=$$(type -p qemu-system-$${arch}); test -z "$${qe}" && qe=qemu; \
	 echo "using $${qe}..."; $${qe} $(KQFLAGS) -net nic -net user,tftp=$(BUILDDIR),bootfile=$(subst $(BUILDDIR),,$(GRUB2_PXE)) -boot n;

qemu-netboot-grub2-dbg:
	@$(MAKE) -f $(SOURCEDIR)/Makefile qemu-netboot-grub2 QFLAGS="$(QFLAGS) -s -S"

#
# direct network boot - multiboot loaded by gPXE.
#

qemu-netboot: $(KERNEL)
	@arch=$(ARCH); arch=$${arch%%-*}; qe=$$(type -p qemu-system-$${arch}); test -z "$${qe}" && qe=qemu; \
	 echo "using $${qe}..."; $${qe} $(KQFLAGS) -net nic -net user,tftp=$(BUILDDIR),bootfile=/$(notdir $(KERNEL)) -boot n;

qemu-netboot-dbg:
	@$(MAKE) -f $(SOURCEDIR)/Makefile qemu-netboot QFLAGS="$(QFLAGS) -s -S"

#
# cdrom iso boot.
#

qemu-cd: $(GRUB2_ISO)
	@arch=$(ARCH); arch=$${arch%%-*}; qe=$$(type -p qemu-system-$${arch}); test -z "$${qe}" && qe=qemu; \
	 echo "using $${qe}..."; $${qe} $(KQFLAGS) -cdrom $(GRUB2_ISO) -boot d;

qemu-cd-dbg:
	@$(MAKE) -f $(SOURCEDIR)/Makefile qemu-cd-dbg QFLAGS="$(QFLAGS) -s -S"

#
# BOCHS related stuff
#

BOCHS_RC	:= $(BUILDDIR)/bochsrc.txt

$(BOCHS_RC): $(SOURCEDIR)/config/misc/bochsrc.in $(GRUB2_ISO)
	@-$(MAKE_BDIR)
	@sed -e "s,@GRUB2_ISO@,$(GRUB2_ISO),g" < "$<" > "$@"

bochs-cd: $(BOCHS_RC) $(GRUB2_ISO)
	@bochs="$$(type -p bochs)"; test -x "$${bochs}" || { echo "bochs not found!"; exit 1; }; \
	 $${bochs} -qf $(BOCHS_RC) || true

#
# GRUB2 related stuff
#

$(GRUB2_CFG): $(SOURCEDIR)/config/misc/grub2.cfg
	@-$(MAKE_BDIR)
	@sed -e "s,@KERNEL@,$(notdir $(KERNEL)),g" < "$<" > "$@"

$(GRUB2_PXE):
	@mknet=$$(type -p grub-mknetdir); test -x "$${mknet}" || { echo "grub-mknetdir not found!"; exit 1; }; \
	 $${mknet} --net-directory=$(BUILDDIR);

$(GRUB2_ISO): $(GRUB2_CFG) $(KERNEL)
	@-rm -f "$@"
	@mkresc=$$(type -p grub-mkrescue); test -x "$${mkresc}" || { echo "grub-mkrescue not found!"; exit 1; }; \
	 $${mkresc} -o "$@" --modules="biosdisk iso9660 multiboot sh" $(BUILDDIR)
	@echo "iso image \"$@\" built..."

iso: $(GRUB2_ISO)

