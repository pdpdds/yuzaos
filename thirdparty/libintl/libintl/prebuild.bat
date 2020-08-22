rem pre build batch file

rem set perl path
rem set path=d:\perl64\bin;d:\devel\bin\;%path%

cd intl

rem make libgnuintl.h
rem perl ..\make_header.pl < libgnuintl.in.h > libgnuintl.h

rem copy libgnuintl.h
rem copy /y ..\msvc\libgnuintl.h .\

rem plural.c file include in gettext-0.19.4 archive.
rem bison plural.y -o plural.c

cd ..

