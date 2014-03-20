SOURCEDIR		:= $(abspath $(dir $(firstword $(MAKEFILE_LIST))))
BUILDDIR 		:= $(SOURCEDIR)/.build/$(ARCH)

SOURCES	:= \
	$(wildcard $(SOURCEDIR)/src/*.c) \
	$(wildcard $(SOURCEDIR)/src/*.S) \
	$(wildcard $(SOURCEDIR)/src/$(ARCH)/*.c) \
	$(wildcard $(SOURCEDIR)/src/$(ARCH)/*.S) \
    $(wildcard $(SOURCEDIR)/src/contrib/*/*.c) \
	$(KERNEL_ADD)

CSOURCES 	:= $(filter %.c,$(SOURCES))
SSOURCES 	:= $(filter %.S,$(SOURCES))

COBJECTS 	:= $(subst $(SOURCEDIR),$(BUILDDIR),$(CSOURCES:.c=.o))
SOBJECTS 	:= $(subst $(SOURCEDIR),$(BUILDDIR),$(SSOURCES:.S=.o))

OBJECTS  	:= $(COBJECTS) $(SOBJECTS)
MAKE_BDIR 	 = test -d "$(dir $@)" || mkdir -p "$(dir $@)"


#.----------------------------------.
#| General Template Rules           |
#'----------------------------------'

include $(foreach mf,$(subst .o,.Po,$(OBJECTS)),$(wildcard $(mf)))

$(COBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.c
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[CC  ] $(subst $(SOURCEDIR)/,,$<)"; \
	 else \
	 	echo "$(CC) $(KCFLAGS) -c -o \"$@\" \"$<\""; \
	 fi
	@$(CC) -MMD -MF "$(subst .o,.Po,$@)" -dA -dP -save-temps=obj $(KCFLAGS) -c -o "$@" "$<"

$(SOBJECTS): $(BUILDDIR)/%.o: $(SOURCEDIR)/%.S
	@-$(MAKE_BDIR)
	@if test $(VERBOSE) = 0; then \
		echo "[AS  ] $(subst $(SOURCEDIR)/,,$<)"; \
	 else \
	 	echo "$(CC) $(KCFLAGS) -D__ASM__ -c -o \"$@\" \"$<\""; \
	 fi
	@$(CC) -MMD -MF "$(subst .o,.Po,$@)" $(KCFLAGS) -D__ASM__ -c -o "$@" "$<"

clean:
	@-rm -rf $(SOURCEDIR)/.build

