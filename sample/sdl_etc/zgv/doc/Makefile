# Makefile for zgv docs
#
# Only info version is made by default, do `make dvi' to make a .dvi
# file.


# This gets definitions for INFODIR, MANDIR, etc.
include ../config.mk


all: info man

info: zgv
dvi: zgv.dvi

zgv: zgv.texi
	makeinfo zgv.texi

# `-c' removes the huge number of associated files created by TeX.
# This saves doing a `make clean' from hell. :-)
zgv.dvi: zgv.texi
	@echo "=== NB: If you don't want A4 paper, edit config.mk! ==="
	texi2dvi -c $(USE_A4_DEF) zgv.texi

# This explicitly mentions `gawk' because makeman requires it
# (and can't easily be made not to, since it really does need gensub).
# To avoid problems on gawkless systems, a prebuilt zgv.1 is included
# in the source tgz.
#
man: zgv.1
zgv.1: zgv.texi makeman.awk
	@echo 'Making man page from zgv.texi...'
	gawk -f makeman.awk <zgv.texi >zgv.1

# explicitly removes /usr/share/man/man1/zgv.1* and /usr/share/info/zgv*
# in case of old installation. Not nice to have this in the install
# target, but in practice I think it's for the best.
#
# I think this one's getting just a tiny bit confusing :-}
#
ifeq ($(INFO_DIR_UPDATE),no)
install:
else
install: ../src/install-info
endif
	$(RM) /usr/share/man/man1/zgv.1* /usr/share/info/zgv*
	install -m 644 zgv.1 $(MANDIR)
	install -m 644 zgv zgv-? $(INFODIR)
# Update info `dir' file.
# Info always uses a dir file in preference to a dir.gz, so we don't use
# dir.gz unless it's the only game in town.
ifneq ($(INFO_DIR_UPDATE),no)
	if [ -f $(INFODIR)/dir.gz -a ! -f $(INFODIR)/dir ]; then \
	  gzip -d $(INFODIR)/dir.gz; \
	  ../src/install-info zgv $(INFODIR)/dir; \
	  gzip $(INFODIR)/dir; \
	else \
	  ../src/install-info zgv $(INFODIR)/dir; \
	  chmod a+r $(INFODIR)/dir; \
	fi
endif

../src/install-info: ../src/install-info.c
	cd ../src && $(MAKE) install-info

# can't easily fix dir :-/, but do remove the files.
uninstall:
	$(RM) $(MANDIR)/zgv.1*
	$(RM) $(INFODIR)/zgv*

clean:
	$(RM) *~ zgv.dvi
