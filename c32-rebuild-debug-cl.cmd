@echo off
SET INCLUDE=G:\WINDDK\6001.18000\inc\api;G:\WINDDK\6001.18000\inc\ddk;G:\Program Files\Microsoft Platform SDK\Include;G:\Program Files\Microsoft Visual Studio 8\vc\include;g:\Program Files\Microsoft Visual Studio 8\vc\atlmfc\include;G:\DX90SDK\Include;F:\Qt\3.3.2\qwt-4.2.0\include;f:\mgc\embedded\NucleusSIM\simulation\include
cd sndrender
nmake clean
nmake all SSE2=1 DEBUG=1 USE_CL=1
cd ..

cd z80
nmake clean
nmake all SSE2=1 DEBUG=1 USE_CL=1
cd ..

nmake clean
nmake all SSE2=1 DEBUG=1 USE_CL=1
