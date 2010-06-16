@echo off
cd sndrender
nmake clean
nmake all SSE2=1 DEBUG=1
cd ..

cd z80
nmake clean
nmake all SSE2=1 DEBUG=1
cd ..

nmake clean
nmake all SSE2=1 DEBUG=1
