
How to build:

STEP 0: Preparation

  Install Windows XP or higher.
  Install Microsoft Visual C++ 2008 or 2010.
  Install Microsoft SDK 7.1 (optional).
  Install patch command, MinGW or TortoiseGit in order to patch source files.


STEP 1: Getting the gettext archive

  Download the gettext version 0.19.5.1
  http://www.gnu.org/software/gettext/

  Extract libintl sources.
  copy to libintl/intl/ from gettext-runtime/intl/ on gettext archive.


STEP 2: Patching source files

  Patch libintl sources using patch command.

  cd libintl/intl/; patch -p0 < ../patch-0.19.5.1.patch

  Or, use TortoiseGitUDiff.


STEP 3: Configure

  Edit libintl/msvc/config.h if need.
  * Define SKIP_LC_MESSAGES to 1 if you don't need LC_MESSAGES sub-directory
    in locale directory.
  * Undefine HAVE_POSIX_PRINTF if you want to use printf() function supports
    format strings with positions.


STEP 4: Building libraries using MSVC solution file

  Build all using libintl_vc09.sln or libintl_vc10.sln.
  On success, copy library files to lib/* or libdll/*.

    lib/x86/ ........ 32bit static library.
      libintl.lib ..... Release         : MBCS and Release build
      libintld.lib .... Debug           : MBCS and Debug build
      libintlu.lib .... Unicode_Release : Unicode and Release build
      libintlud.lib ... Unicode_Debug   : Unicode and Debug build
    lib/x64/ ........ 64bit static library.
      libintl.lib ..... Release         : MBCS and Release build
      libintld.lib .... Debug           : MBCS and Debug build
      libintlu.lib .... Unicode_Release : Unicode and Release build
      libintlud.lib ... Unicode_Debug   : Unicode and Debug build

    libdll/x86/ ..... 32bit dynamic link library.
      libintl.dll ..... DLL_Release         : MBCS and Release build
      libintl.lib
      libintld.dll .... DLL_Debug           : MBCS and Debug build
      libintld.lib
      libintlu.dll .... DLL_Unicode_Release : Unicode and Release build
      libintlu.lib
      libintlud.dll ... DLL_Unicode_Debug   : Unicode and Debug build
      libintlud.lib
    libdll/x64/ ..... 64bit dynamic link library.
      libintl.dll ..... DLL_Release         : MBCS and Release build
      libintl.lib
      libintld.dll .... DLL_Debug           : MBCS and Debug build
      libintld.lib
      libintlu.dll .... DLL_Unicode_Release : Unicode and Release build
      libintlu.lib
      libintlud.dll ... DLL_Unicode_Debug   : Unicode and Debug build
      libintlud.lib


STEP 5: Testing

  Build sample program on samples directory and test them.

    If you test dll, copy dll file to application directory from libdll
  directory.



How to use:

    See gettext.h in samples directory to buildable on both MBCS and UNICODE
  environment.

