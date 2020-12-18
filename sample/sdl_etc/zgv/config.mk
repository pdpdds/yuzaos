# This file configures zgv's Makefiles. You should edit the settings
# below as needed.


# ---------------------- Compilation options ----------------------

# Set the C compiler to use, and options for it.
# This is likely to be what you'll want for most systems:
#
CC=gcc
CFLAGS=-O2 -Wall -fomit-frame-pointer -finline-functions
#
# If you're brave enough to try compiling zgv on a non-x86 system :-),
# this might be a better bet:
#
#CC=gcc
#CFLAGS=-O2 -Wall -finline-functions

# Set the awk interpreter to use for a script used while compiling.
# (This should be a `new' awk, such as gawk or mawk.) This setting
# should work for Linux and *BSD.
#
AWK=awk

# Set display backend to use. svgalib is the `native' one and
# is likely to remain preferable. The SDL backend is now quite
# usable, but tends to be rather slower.
#
BACKEND=SVGALIB
#BACKEND=SDL


# --------------------- Installation options ----------------------

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

# Set the location/filename of the system-wide configuration file. You
# may prefer to have this under /etc, for example.
#
RCFILE=$(PREFIX)/etc/zgv.conf


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


# ------------------- Format-related options ----------------------

# Uncomment this if you want zgv to support Kodak's Photo-CD format
# (`.pcd' files). This requires libpcd (part of the xpcd package).
#
#PCDDEF=-DPCD_SUPPORT


# -------------------- Miscellaneous options -----------------------

# Name of X's named-colour file (for XPM support); not required,
# but if the file's not found, only "black" and "white" and hex
# colours will be recognised, which will mean many XPMs won't be
# readable. This setting should be ok.
#
RGB_DB=/usr/X11R6/lib/X11/rgb.txt

# Finally, an option for `make dvi' in the `doc' directory. You only need
# worry about what this is set to if you plan to make a printed manual.
#
# Normally the .dvi file created will be formatted for printing on A4
# paper (210x297mm, 8.27x11.69"); if you'll be printing on non-A4,
# you'll probably want to comment this out. (It'll still warn you to
# edit this file when you do `make dvi', but that's only because
# doc/Makefile isn't as smart about that as it should be. :-))
#
USE_A4_DEF=-t @afourpaper
