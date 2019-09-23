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
#

# the library's version
VERSION:=0.8.6

# tools

SHELL = /bin/sh
MKDIR = mkdir

abs_builddir = /home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/build/wp77xx/system/component/32717b7f085024cb47809e25fe4af8c3/sha2
top_builddir = ..
top_srcdir:= ..

SOURCES:= sha2.c
HEADERS:=sha2.h
OBJECTS:= $(patsubst %.c, %.o, $(SOURCES))
CPPFLAGS=-DSIERRA -DDTLSv12 -DWITH_SHA256 -I$(top_srcdir)
CFLAGS=-Wall -std=c99 -pedantic -w -std=gnu99 -DWITH_SHA256 -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/3rdParty/Lwm2mCore/include/platform-specific/linux -fPIC --sysroot=/opt/swi/SWI9X07Y_02.25.02.01/sysroots/armv7a-neon-poky-linux-gnueabi -O2 -fno-omit-frame-pointer -DSIERRA_MDM9X28 -DSIERRA_WP76XX -DPCM_IF=SEC_AUX -DI2S_IF=SEC_MI2S -DUSB_IF=AFE_PCM -DCODEC_IF=PRI_MI2S -DMM_1_ALSA_MIXER_CTRL=MultiMedia1 -DMM_1_ALSA_PCM_DEVICE_ID=0 -DMM_2_ALSA_MIXER_CTRL=MultiMedia2 -DMM_2_ALSA_PCM_DEVICE_ID=13 -DADC_BEGIN_NUM=0 -DADC_COUNT=4 -DPDP_MAX_PROFILE=16 -DPDP_MAX_MULTIPDP=8 -DEUICC -I build/wp77xx/system -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/framework/include -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/modules/WiFi -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/modemServices -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/positioning -I/home/pankaj/Documents/yoctosrc_fx30LTECatM/legato-af_fx30ltem/interfaces/atServices
LDLIBS=
FILES:=Makefile.in $(SOURCES) $(HEADERS) README sha2prog.c sha2speed.c sha2test.pl 
DISTDIR=$(top_builddir)/tinydtls-0.8.6

.PHONY: all dirs clean install dist distclean .gitignore doc

.SUFFIXES:
.SUFFIXES:      .c .o

all:

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

dist:	$(FILES)
	test -d $(DISTDIR)/sha2 || mkdir $(DISTDIR)/sha2
	cp -p $(FILES) $(DISTDIR)/sha2
	test -d $(DISTDIR)/sha2/testvectors || mkdir $(DISTDIR)/sha2/testvectors
	cp -pr testvectors $(DISTDIR)/sha2/testvectors

install:	$(HEADERS)
	test -d $(includedir)/sha2 || mkdir -p $(includedir)/sha2
	$(install) $(HEADERS) $(includedir)/sha2

.gitignore:
	echo "core\n*~\n*.[oa]\n*.gz\n*.cap\n$(PROGRAM)\n$(DISTDIR)\n.gitignore" >$@
