## Process this file with automake to create Makefile.in
# Copyright 1997, 1998, 2005, 2006, 2007, 2008, 2009 Free Software
# Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = ansi2knr

LN = ln

bin_PROGRAMS = grep egrep fgrep
grep_SOURCES  =  grep.c  search.c kwset.c dfa.c
egrep_SOURCES = egrep.c esearch.c kwset.c dfa.c
fgrep_SOURCES = fgrep.c fsearch.c kwset.c
noinst_HEADERS = grep.h dfa.h kwset.h getpagesize.h system.h mbsupport.h

LDADD = @INTLLIBS@ ../lib/libgreputils.a
datadir = $(prefix)/@DATADIRNAME@
localedir = $(datadir)/locale
INCLUDES = -I../intl -I$(top_srcdir)/lib -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST = \
             dosbuf.c \
             vms_fab.c vms_fab.h
