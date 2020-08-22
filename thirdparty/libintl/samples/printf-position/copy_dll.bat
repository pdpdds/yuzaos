@echo off
copy /b /y ..\..\libdll\x86\libintl.dll .\Win32\DLL_Release\
copy /b /y ..\..\libdll\x86\libintld.dll .\Win32\DLL_Debug\
copy /b /y ..\..\libdll\x86\libintlu.dll .\Win32\DLL_Unicode_Release\
copy /b /y ..\..\libdll\x86\libintlud.dll .\Win32\DLL_Unicode_Debug\

copy /b /y ..\..\libdll\x64\libintl.dll .\x64\DLL_Release\
copy /b /y ..\..\libdll\x64\libintld.dll .\x64\DLL_Debug\
copy /b /y ..\..\libdll\x64\libintlu.dll .\x64\DLL_Unicode_Release\
copy /b /y ..\..\libdll\x64\libintlud.dll .\x64\DLL_Unicode_Debug\
