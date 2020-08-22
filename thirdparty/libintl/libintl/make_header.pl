#!/bin/perl
#
# make libgnuintl.h for VC++
#
while (<>) {
	s/\@HAVE_NEWLOCALE\@/0/g;
	s/\@HAVE_POSIX_PRINTF\@/1/g;
	s/\@HAVE_SNPRINTF\@/0/g;
	s/\@HAVE_ASPRINTF\@/0/g;
	s/\@HAVE_WPRINTF\@/0/g;
	print;
}
