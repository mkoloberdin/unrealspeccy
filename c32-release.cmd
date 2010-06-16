@echo off
cd sndrender
nmake all SSE2=1 RELEASE=1
cd ..

cd z80
nmake all SSE2=1 RELEASE=1
cd ..

nmake all SSE2=1 RELEASE=1
