# top-level Makefile for zgv

# -----------------------------------------------------------------
# >>> NB: if you're looking to edit this to configure zgv, in the
# way you might have with older versions, edit `config.mk' instead.
# -----------------------------------------------------------------


# The main targets of interest are:
#
# all		the default; make everything except info
#		(it warns if the info is out of date, though)
# info		make info (requires texinfo's `makeinfo')
# install	install everything
# uninstall	can't imagine what use you could possibly have for this :^)
# clean		clean up
#
# src-tgz	make source distribution tar.gz
# bin-tgz	make binary distribution tar.gz
# tgz		make both


# version number, needed for src/bin distrib-making stuff below.
#
VERS=5.9



all: src man infowarn

src: zgv src/install-info

# We try this the whole time, as the dependancies are a bit
# complicated to duplicate here.
zgv:
	cd src && $(MAKE) zgv

src/install-info: src/install-info.c
	cd src && $(MAKE) install-info

man: doc/zgv.1

doc/zgv.1: doc/zgv.texi doc/makeman.awk
	cd doc && $(MAKE) zgv.1

# Like in GNU stuff, info files aren't automatically remade,
# as I don't want to assume everyone has texinfo's `makeinfo' handy.
# So the `infowarn' below is mainly to warn me if the info gets
# out of date. :-)
info: doc/zgv

doc/zgv: doc/zgv.texi
	cd doc && $(MAKE) info

# Warn if the info is out of date. This *is* automatically done.
# It's a bit kludgey though, using doc/zgv-1... :-)
infowarn: doc/zgv-1

doc/zgv-1: doc/zgv.texi
	@echo '================================================'
	@echo 'WARNING: info files out of date, do "make info"!'
	@echo '================================================'


clean:
	cd src && $(MAKE) clean
	cd doc && $(MAKE) clean
	$(RM) *~ etc/*~

install: all
	cd src && $(MAKE) install
	cd doc && $(MAKE) install

uninstall:
	cd src && $(MAKE) uninstall
	cd doc && $(MAKE) uninstall


# The rest of the file is devoted to making the src/bin distributions.
# The src one is just as-is, but the bin one is pretty fiddly.

tgz: src-tgz bin-tgz

# The easy one :-)
src-tgz: ../zgv-$(VERS).tar.gz
  
# Based on the example in ESR's Software Release Practice HOWTO.
# The exclusion of any `sav' dir is because I sometimes will make a
# copy of the zgv src in such a dir before changing things, in case
# I screw it up. :-)
#
../zgv-$(VERS).tar.gz: info clean
	$(RM) ../zgv-$(VERS)
	@cd ..;ln -s zgv zgv-$(VERS)
	cd ..;tar zchvf zgv-$(VERS).tar.gz --exclude='*/sav' zgv-$(VERS)
	@cd ..;$(RM) zgv-$(VERS)


# The bin distrib is a bit of a pain though. Correction: it's
# a *lot* of a pain...
#
# The basic idea is to make a temporary zgv-$VERS-bin dir (with a
# `doc' subdir), stick symlinks in there as needed, make the tar.gz,
# then blast the dir.
#
bin-tgz: ../zgv-$(VERS)-bin.tar.gz
	
# Well, and obviously we need to make the binaries too. Duh! :-)
# `src' makes zgv and install-info.
#
../zgv-$(VERS)-bin.tar.gz: src man
	rm -fr zgv-$(VERS)-bin
	mkdir zgv-$(VERS)-bin
	mkdir zgv-$(VERS)-bin/doc
	cd zgv-$(VERS)-bin; ln -s ../{COPYING,README,README.fonts} .
	cd zgv-$(VERS)-bin; ln -s ../{TODO,ChangeLog,NEWS,SECURITY} .
	cd zgv-$(VERS)-bin; ln -s ../etc/bin.makefile Makefile
	cd zgv-$(VERS)-bin; ln -s ../etc/README.bin .
	cd zgv-$(VERS)-bin/doc; ln -s ../../doc/sample.zgvrc .
	cd zgv-$(VERS)-bin/doc; ln -s ../../doc/{zgv,zgv-?,zgv.1} .
	cd zgv-$(VERS)-bin; ln -s ../src/{zgv,install-info} .
	strip zgv-$(VERS)-bin/{zgv,install-info}
	tar zchvf ../zgv-$(VERS)-bin.tar.gz zgv-$(VERS)-bin
	rm -fr zgv-$(VERS)-bin
	$(MAKE) clean
