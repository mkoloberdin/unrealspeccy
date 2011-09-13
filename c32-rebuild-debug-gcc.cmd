@echo off
cd sndrender
nmake clean -f Makefile.gcc
nmake all SSE2=1 DEBUG=1 -f Makefile.gcc
cd ..

cd z80
nmake clean -f Makefile.gcc
nmake all SSE2=1 DEBUG=1 -f Makefile.gcc
cd ..

nmake clean -f Makefile.gcc
nmake all SSE2=1 DEBUG=1 -f Makefile.gcc
