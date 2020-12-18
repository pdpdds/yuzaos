# src Makefile for zgv

# -----------------------------------------------------------------
# >>> NB: if you're looking to edit this to configure zgv, in the
# way you might have with older versions, edit `../config.mk' instead.
# -----------------------------------------------------------------


# This gets definitions for BINDIR etc.
include ../config.mk


# having /usr/local/{include,lib} like this is apparently needed for
# FreeBSD, but shouldn't cause problems on Linux.
#
CFLAGS+=-I/usr/local/include
ZGV_LIBS=-L/usr/local/lib

CFLAGS+=-DBACKEND_$(BACKEND)

ifeq ($(BACKEND),SVGALIB)
ZGV_LIBS+=-lvgagl -lvga
endif
ifeq ($(BACKEND),SDL)
ZGV_LIBS+=-lSDL
endif

CFLAGS+=-DRGB_DB_FILE=\"$(RGB_DB)\" -DETC_RCFILE=\"$(RCFILE)\"

ZGV_LIBS+=-ljpeg -lpng -ltiff -lz -lm
ifeq ($(PCDDEF),-DPCD_SUPPORT)
ZGV_LIBS+=-lpcd
endif

# `all' makes install-info too, as it's a bit weird to have something
# compile when you do `make install'. :-)
# (It would also be weird to have it made by the `doc' Makefile, IMHO.)

all: zgv install-info

ZGV_OBJS=zgv.o vgadisp.o font.o 3deffects.o mousecur.o \
	readnbkey.o rcfile.o helppage.o gnuhelp.o \
	magic.o resizepic.o copymove.o scrollbar.o rbmenu.o \
	readgif.o readjpeg.o readpnm.o \
	readbmp.o readtga.o readpng.o readpcx.o \
	readxvpic.o readmrf.o readxbm.o readxpm.o \
	modesel.o readpcd.o readtiff.o readprf.o zgv_io.o

zgv: $(ZGV_OBJS)
	$(CC) $(CFLAGS) -o zgv $(ZGV_OBJS) $(ZGV_LIBS)

bdf2h: bdf2h.o
	$(CC) $(CFLAGS) -o bdf2h bdf2h.o

install-info: install-info.c
	$(CC) $(INFODIRDEF) -o install-info install-info.c

# explicitly removes /usr/bin/{zgv,zgv-sdl} in case of old
# installation. Not nice to put this in the install target,
# but it's likely to cause problems if I don't.
# 
install: all
	@if [ "$(BACKEND)" = SVGALIB ]; then \
	  echo $(RM) /usr/bin/zgv; \
	  $(RM) /usr/bin/zgv; \
	  echo install -m 4755 -o root -g root -s zgv $(BINDIR); \
	  install -m 4755 -o root -g root -s zgv $(BINDIR); \
	fi
	@if [ "$(BACKEND)" = SDL ]; then \
	  echo $(RM) /usr/bin/zgv-sdl; \
	  $(RM) /usr/bin/zgv-sdl; \
	  echo install -m 755 -s zgv $(BINDIR)/zgv-sdl; \
	  install -m 755 -s zgv $(BINDIR)/zgv-sdl; \
	fi

uninstall:
	$(RM) $(BINDIR)/zgv $(BINDIR)/zgv-sdl

clean:
	$(RM) *.o *~ zgv bdf2h fontbmps.h install-info
	$(RM) rcfile_opt.h rcfile_var.h rcfile_short.h

# Dependancies
3deffects.o: 3deffects.c 3deffects.h font.h readnbkey.h zgv.h \
 rc_config.h rcfile.h mousecur.h rbmenu.h
bdf2h.o: bdf2h.c
copymove.o: copymove.c 3deffects.h font.h readnbkey.h zgv.h copymove.h
font.o: font.c zgv.h font.h fontbmps.h rc_config.h rcfile.h
gnuhelp.o: gnuhelp.c zgv.h helppage.h gnuhelp.h
helppage.o: helppage.c 3deffects.h font.h readnbkey.h rc_config.h \
 rcfile.h mousecur.h zgv.h helppage.h
install-info.o: install-info.c
magic.o: magic.c magic.h
modesel.o: modesel.c readnbkey.h modesel.h
mousecur.o: mousecur.c rc_config.h rcfile.h zgv.h
rbmenu.o: rbmenu.c 3deffects.h font.h rc_config.h rcfile.h zgv.h \
 rbmenu.h
rcfile.o: rcfile.c rc_config.h zgv.h rcfile.h rcfile_opt.h \
 rcfile_var.h rcfile_short.h
readbmp.o: readbmp.c zgv.h readbmp.h readpnm.h rcfile.h rc_config.h
readgif.o: readgif.c zgv.h readgif.h
readjpeg.o: readjpeg.c 3deffects.h zgv.h readjpeg.h vgadisp.h \
 rc_config.h rcfile.h
readmrf.o: readmrf.c zgv.h readmrf.h
readnbkey.o: readnbkey.c zgv.h readnbkey.h mousecur.h
readpcd.o: readpcd.c
readpcx.o: readpcx.c zgv.h readpcx.h readpnm.h rcfile.h rc_config.h
readpng.o: readpng.c zgv.h 3deffects.h readpng.h readpnm.h vgadisp.h \
 rc_config.h rcfile.h
readpnm.o: readpnm.c zgv.h rcfile.h rc_config.h readpnm.h
readprf.o: readprf.c zgv.h rcfile.h rc_config.h readpnm.h readprf.h
readtga.o: readtga.c zgv.h readtga.h readpnm.h rcfile.h rc_config.h
readtiff.o: readtiff.c zgv.h vgadisp.h rcfile.h rc_config.h readpnm.h \
 readtiff.h
readxbm.o: readxbm.c zgv.h readxbm.h
readxpm.o: readxpm.c zgv.h readpnm.h rcfile.h rc_config.h readxpm.h
readxvpic.o: readxvpic.c zgv.h readpnm.h readxvpic.h
resizepic.o: resizepic.c resizepic.h
scrollbar.o: scrollbar.c 3deffects.h zgv.h scrollbar.h
vgadisp.o: vgadisp.c zgv.h magic.h readgif.h readjpeg.h readpnm.h \
 readbmp.h readpng.h readtga.h readpcx.h readxvpic.h readmrf.h \
 readxbm.h readxpm.h readpcd.h readtiff.h readprf.h readnbkey.h \
 helppage.h 3deffects.h rc_config.h rcfile.h mousecur.h rbmenu.h \
 modesel.h vgadisp.h
zgv.o: zgv.c zgv.h readgif.h vgadisp.h readnbkey.h font.h 3deffects.h \
 helppage.h rc_config.h rcfile.h readpnm.h resizepic.h copymove.h \
 mousecur.h scrollbar.h rbmenu.h gnuhelp.h modesel.h magic.h
zgv_io.o: zgv_io.c rc_config.h rcfile.h zgv_io.h readnbkey.h

# more involved dependancy stuff
fontbmps.h: bdf2h \
	../fonts/timI12.bdf \
	../fonts/helvR08.bdf \
	../fonts/helvR12.bdf \
	../fonts/helvB12.bdf \
	../fonts/helvR18.bdf
	./bdf2h 0  <../fonts/timI12.bdf >fontbmps.h
	./bdf2h 1  <../fonts/helvR08.bdf >>fontbmps.h
	./bdf2h 2  <../fonts/helvR12.bdf >>fontbmps.h
	./bdf2h 2b <../fonts/helvB12.bdf >>fontbmps.h
	./bdf2h 3  <../fonts/helvR18.bdf >>fontbmps.h
rcfile_opt.h rcfile_var.h rcfile_short.h: options.src mkopts.awk
	$(AWK) -f mkopts.awk <options.src
