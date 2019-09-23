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

all: qmiAirVantage
clean: qmiAirVantageClean

.PHONY: qmiAirVantage
qmiAirVantage:
	$(SILENT)echo "Building qmiAirVantage for $(TARGET)"
	$(SILENT)$(MKAPP) -b -t $(TARGET) \
		-o $(BINDIR) -w $(OBJDIR) qmiAirVantage.adef
	$(SILENT)cp -a $(BINDIR)/qmiAirVantage.$(TARGET).app .

.PHONY: qmiAirVantageClean
qmiAirVantageClean:
	$(SILENT)[ -d $(BUILDDIR) ] && \
		echo "Cleaning qmiAirVantage" && \
		rm -rf $(BUILDDIR) qmiAirVantage.$(TARGET).app || true
