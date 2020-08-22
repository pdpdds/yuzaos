@REM This is a generated file.
@echo off
setlocal
SET srcdir="C:\Users\juhan\Downloads\pcre-master\pcre-master"
SET pcretest="C:\Users\juhan\Downloads\pcre-master\pcre-master\build\DEBUG\pcretest.exe"
if not [%CMAKE_CONFIG_TYPE%]==[] SET pcretest="C:\Users\juhan\Downloads\pcre-master\pcre-master\build\%CMAKE_CONFIG_TYPE%\pcretest.exe"
call %srcdir%\RunTest.Bat
if errorlevel 1 exit /b 1
echo RunTest.bat tests successfully completed
