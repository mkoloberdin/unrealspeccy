@echo off
cd sndrender
nmake all SSE2=1 DEBUG=1 -f Makefile.gcc
cd ..

cd z80
nmake all SSE2=1 DEBUG=1 -f Makefile.gcc
cd ..

nmake all SSE2=1 DEBUG=1 -f Makefile.gcc
