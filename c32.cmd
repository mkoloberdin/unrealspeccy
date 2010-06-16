set LIB=lib32;%LIB%
:-Wcheck /O3 /Qip
: /fixed:no
: 
:/arch:IA32
: -DDEBUG /RTCsu /Od /Qtrapuv /Zi
icl -D_M_IX86_FP=2 -DDEBUG /Qms0 /RTCsu /Od /Qtrapuv /Zi /MT /Oi /arch:SSE2 emul.cpp settings.RES -Fex32\unreal.exe /link /opt:ref,icf /safeseh /nxcompat
:icl -D_M_IX86_FP=2 /Zi /MT /Oi /arch:SSE2 emul.cpp settings.RES -Fex32\unreal-p4.exe /link /opt:ref,icf /safeseh /nxcompat
:icl /MT /Oi /arch:IA32 emul.cpp settings.RES -Fex32\unreal-p1.exe /link /opt:ref,icf /safeseh /nxcompat
:if exist x32\unreal-p4.exe copy /y x32\unreal.exe h:\zx\unreal\unreal.exe
if exist x32\unreal.exe copy /y x32\unreal.exe h:\zx\unreal\unreal.exe
