@echo off
cd sndrender
nmake clean -f Makefile.gcc
nmake all RELEASE=1 -f Makefile.gcc
cd ..

cd z80
nmake clean -f Makefile.gcc
nmake all RELEASE=1 -f Makefile.gcc
cd ..

nmake clean -f Makefile.gcc
nmake all RELEASE=1 -f Makefile.gcc
