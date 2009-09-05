@echo off

rem set include=c:\tools\msvc\include;c:\dev\lib
rem set lib=c:\tools\msvc\lib
set clexe=cl.exe

set cc=%1
shift

if "%cc%" == "rc" goto rc
if "%cc%" == "wc" goto wc
if "%cc%" == "ms" goto ms
if "%cc%" == "mss" goto mss
if "%cc%" == "msmerge" goto msmerge
if "%cc%" == "nms" goto ms
if "%cc%" == "msdebug" goto msdebug
if "%cc%" == "dll" goto dll
echo unknown compiler
goto exit

:rc
rc %1 %2 %3 %4 %5 %6 %7 %8 %9
goto exit

:msdebug
%clexe% /W3 /Zi /ZI /MLd /Fp"Debug.pch" /YX %1 %2 %3 %4 %5 %6 %7 %8 %9 /link /FILEALIGN:0x200
goto exit

:dll
%clexe% /W3 /LD /MD /Og /Ot /Oy /Ob1 /Oa /Gr /GA /G6 /YX %1 %2 %3 %4 %5 %6 %7 %8 %9 /link /ALIGN:4096 /FILEALIGN:0x200 /IGNORE:4108 /DLL
if errorlevel 1 goto exit
goto delobj

:ms
%clexe% /W3 /MD /Og /Ot /Oy /Ob1 /Oa /Gr /GA /G6 /YX %1 %2 %3 %4 %5 %6 %7 %8 %9 /link /ALIGN:4096 /FILEALIGN:0x200 /IGNORE:4108
if errorlevel 1 goto exit
goto delobj

:mss
%clexe% /W3 /O1 /MD /Og /Os /Oy /Ob1 /Oa /Gr /GA /G6 /YX %1 %2 %3 %4 %5 %6 %7 %8 %9 /link /ALIGN:4096 /FILEALIGN:0x200 /IGNORE:4108 /MERGE:.rdata=.text /IGNORE:4078,4108
if errorlevel 1 goto exit
goto delobj

:msmerge
%clexe% /W3 /MD /Og /Os /Oy /Ob1 /Oa /Gr /GA /G5 /YX %1 %2 %3 %4 %5 %6 %7 %8 %9 /link /ALIGN:4096 /FILEALIGN:0x200 /MERGE:.rdata=.text /MERGE:.data=.text /SECTION:.text,EWR /IGNORE:4078,4108
if errorlevel 1 goto exit
goto delobj

:wc
set include=c:\tools\watcom\h;c:\tools\watcom\h\nt
set WATCOM=c:\tools\watcom
c:\tools\watcom\binnt\wcl386 /ox /5r /ot /fh %1 %2 %3 %4 %5 %6 %7 %8 %9

:delobj
if %cc%! == nms! goto exit
del *.obj

:exit