rem post build batch file
rem %1:$(TargetDir) %2:$(TargetName) %3:$(TargetExt) %4:$(Platform)

if "%1" == "" goto end

rem set perl path
set path=d:\perl64\bin;d:\devel\bin\;%path%

rem copy lib files upto the lib folder

if "%3" == ".dll" goto dll
if "%4" == "x64" goto x64

:x86
if not exist ..\lib\x86 md ..\lib\x86
copy /b /y %1\%2.lib ..\lib\x86\
goto end

:x64
if not exist ..\lib\x64 md ..\lib\x64
copy /b /y %1\%2.lib ..\lib\x64\
goto end

:dll
if "%4" == "x64" goto x64_dll

:x86_dll
if not exist ..\libdll\x86 md ..\libdll\x86
copy /b /y %1\%2.dll ..\libdll\x86\
copy /b /y %1\%2.lib ..\libdll\x86\
goto end

:x64_dll
if not exist ..\libdll\x64 md ..\libdll\x64
copy /b /y %1\%2.dll ..\libdll\x64\
copy /b /y %1\%2.lib ..\libdll\x64\
goto end

:end
