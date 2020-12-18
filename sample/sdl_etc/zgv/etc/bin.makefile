# Makefile for zgv binary distribution

# Set BINDIR to directory for binaries,
# INFODIR to directory for info files,
# MANDIR to directory for man page.
# Usually it will be simpler to just set PREFIX.
#
PREFIX=/usr/local

# In theory it would be nice to put the info file and man page under
# /usr/local/share. However, it's not clear if this is widely
# supported yet, so for now the default is the traditional
# /usr/local/info and /usr/local/man/man1.
#
# If you want, though, or if you're installing with PREFIX=/usr,
# you can uncomment the following to get more FHS-like dirs such as
# /usr/local/share/info and /usr/local/share/man/man1.
#
# If you don't know what to do, leave it as-is.
#
#SHARE_INFIX=/share

BINDIR=$(PREFIX)/bin
INFODIR=$(PREFIX)$(SHARE_INFIX)/info
MANDIR=$(PREFIX)$(SHARE_INFIX)/man/man1

# Normally `make install' will update your `dir' file (in INFODIR),
# using a copy of texinfo's `install-info' bundled with zgv.
#
# But if you have a different way of keeping `dir' up-to-date (for
# example, perhaps your setup automatically handles this for you) you
# should uncomment this to prevent `make install' doing that. However,
# if you're installing in /usr/local, it's possible any automated
# update deliberately doesn't mess with /usr/local/info/dir...
#
# If you don't know what to do, leave it as-is.
#
#INFO_DIR_UPDATE=no


# You shouldn't need to modify anything below this line.


all: install

install:
	install -m 4755 -o root -g root -s zgv $(BINDIR)
	install -m 644 doc/zgv.1 $(MANDIR)
	install -m 644 doc/zgv doc/zgv-? $(INFODIR)
# Update info `dir' file
# Info always uses a dir file in preference to a dir.gz, so we don't use
# dir.gz unless it's the only game in town.
ifneq ($(INFO_DIR_UPDATE),no)
	if [ -f $(INFODIR)/dir.gz -a ! -f $(INFODIR)/dir ]; then \
	  gzip -d $(INFODIR)/dir.gz; \
	  ./install-info doc/zgv $(INFODIR)/dir; \
	  gzip $(INFODIR)/dir; \
	else \
	  ./install-info doc/zgv $(INFODIR)/dir; \
	  chmod a+r $(INFODIR)/dir; \
	fi
endif

# can't easily fix dir :-/, but do remove the files.
# explicitly removes /usr/man/man1/zgv.1* and /usr/info/zgv*
# in case of old installation.
uninstall:
	$(RM) $(BINDIR)/zgv
	$(RM) $(MANDIR)/zgv.1*
	$(RM) /usr/man/man1/zgv.1*
	$(RM) $(INFODIR)/zgv*
	$(RM) /usr/info/zgv*
