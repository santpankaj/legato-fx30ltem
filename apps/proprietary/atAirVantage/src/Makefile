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

all: atAirVantage
clean: atAirVantageClean

.PHONY: atAirVantage
atAirVantage:
	$(SILENT)echo "Building atAirVantage for $(TARGET)"
	$(SILENT)$(MKAPP) -b -t $(TARGET) \
		-o $(BINDIR) -w $(OBJDIR) atAirVantage.adef
	$(SILENT)cp -a $(BINDIR)/atAirVantage.$(TARGET).app .

.PHONY: atAirVantageClean
atAirVantageClean:
	$(SILENT)[ -d $(BUILDDIR) ] && \
		echo "Cleaning atAirVantage" && \
		rm -rf $(BUILDDIR) atAirVantage.$(TARGET).app || true
