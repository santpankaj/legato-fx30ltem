#*******************************************************************************
# Copyright (C) Sierra Wireless Inc.
#*******************************************************************************

ifndef TARGET
  $(error No targets)
endif

ifdef VERBOSE
  V = $(VERBOSE)
else
  V ?= 0
endif

SILENT_0 := @
SILENT_1 :=
SILENT := $(SILENT_$(V))

MKAPP_0 := mkapp
MKAPP_1 := mkapp -v
MKAPP := $(MKAPP_$(V))

BUILDDIR := build
BINDIR := $(BUILDDIR)/bin
OBJDIR := $(BUILDDIR)/obj

all: atQmiLinker
clean: atQmiLinkerClean

.PHONY: atQmiLinker
ifeq ($(findstring $(TARGET),wp85 wp76xx wp750x), $(TARGET))
atQmiLinker:
	$(SILENT)echo "Building atQmiLinker for $(TARGET)"
	$(SILENT)$(MKAPP) -b -t $(TARGET) \
		-o $(BINDIR) -w $(OBJDIR) atQmiLinker.adef
	$(SILENT)cp -a $(BINDIR)/atQmiLinker.$(TARGET).app .
else
atQmiLinker:
	$(SILENT)echo "atQmiLinker is not supported by \"$(TARGET)\""
endif

.PHONY: atQmiLinkerClean
atQmiLinkerClean:
	$(SILENT)[ -d $(BUILDDIR) ] && \
		echo "Cleaning atQmiLinker" && \
		rm -rf $(BUILDDIR) atQmiLinker.$(TARGET).app || true
