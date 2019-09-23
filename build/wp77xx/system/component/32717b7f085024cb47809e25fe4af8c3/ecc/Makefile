# Makefile for tinydtls
#
# Copyright (c) 2011, 2012, 2013, 2014, 2015 Olaf Bergmann (TZI) and others.
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# and Eclipse Distribution License v. 1.0 which accompanies this distribution.
#
# The Eclipse Public License is available at http://www.eclipse.org/legal/epl-v10.html
# and the Eclipse Distribution License is available at 
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# Contributors:
#    Olaf Bergmann  - initial API and implementation
#    Hauke Mehrtens - memory optimization, ECC integration
#

# the library's version
VERSION:=0.8.6

# tools

SHELL = /bin/sh
MKDIR = mkdir

abs_builddir = /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/wp77xx/system/component/32717b7f085024cb47809e25fe4af8c3/ecc
top_builddir = ..
top_srcdir:= ..

ECC_SOURCES:= ecc.c testecc.c testfield.c test_helper.c
ECC_HEADERS:= ecc.h test_helper.h
FILES:=Makefile.in Makefile.contiki $(ECC_SOURCES) $(ECC_HEADERS) 
DISTDIR=$(top_builddir)/tinydtls-0.8.6

ifeq ("", "1")
include Makefile.contiki
else
ECC_OBJECTS:= $(patsubst %.c, %.o, $(ECC_SOURCES)) ecc_test.o
PROGRAMS:= testecc testfield
CPPFLAGS=-DSIERRA -DDTLSv12 -DWITH_SHA256
CFLAGS=-Wall -std=c99 -pedantic -w -std=gnu99 -DWITH_SHA256 -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/platform-specific/linux -fPIC --sysroot=/opt/swi/SWI9X07Y_02.25.02.01/sysroots/armv7a-neon-poky-linux-gnueabi -O2 -fno-omit-frame-pointer -DSIERRA_MDM9X28 -DSIERRA_WP76XX -DPCM_IF=SEC_AUX -DI2S_IF=SEC_MI2S -DUSB_IF=AFE_PCM -DCODEC_IF=PRI_MI2S -DMM_1_ALSA_MIXER_CTRL=MultiMedia1 -DMM_1_ALSA_PCM_DEVICE_ID=0 -DMM_2_ALSA_MIXER_CTRL=MultiMedia2 -DMM_2_ALSA_PCM_DEVICE_ID=13 -DADC_BEGIN_NUM=0 -DADC_COUNT=4 -DPDP_MAX_PROFILE=16 -DPDP_MAX_MULTIPDP=8 -DEUICC -I build/wp77xx/system -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/include -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/modules/WiFi -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/modemServices -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/positioning -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/atServices -DTEST_INCLUDE
LDLIBS=

.PHONY: all dirs clean install distclean .gitignore doc

.SUFFIXES:
.SUFFIXES:      .c .o

all: $(PROGRAMS)

ecc_test.o:	ecc.c ecc.h
	$(CC) $(CFLAGS) $(CPPFLAGS)  -c -o $@ $<

testecc: ecc_test.o test_helper.o

testfield: ecc_test.o test_helper.o

check:	
	echo DISTDIR: $(DISTDIR)
	echo top_builddir: $(top_builddir)

clean:
	@rm -f $(PROGRAMS) main.o $(LIB) $(OBJECTS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean ; \
	done

distclean:	clean
	@rm -rf $(DISTDIR)
	@rm -f *~ $(DISTDIR).tar.gz
endif # WITH_CONTIKI

dist:	$(FILES)
	test -d $(DISTDIR)/ecc || mkdir $(DISTDIR)/ecc
	cp -p $(FILES) $(DISTDIR)/ecc

install:	$(ECC_HEADERS)
	test -d $(includedir)/ecc || mkdir -p $(includedir)/ecc
	$(install) $(ECC_HEADERS) $(includedir)/ecc

.gitignore:
	echo "core\n*~\n*.[oa]\n*.gz\n*.cap\n$(PROGRAM)\n$(DISTDIR)\n.gitignore" >$@
