set LIB=%LIB%;lib32
:-Wcheck /O3 /Qip
: /fixed:no
: 
:/arch:IA32
icl-DDEBUG /Od /Qtrapuv /RTCsu /Zi -D_M_IX86_FP=2 Emul.cpp settings.RES -Fex32\unreal.exe /link /opt:ref,icf
if exist x32\unreal.exe copy /y x32\unreal.exe h:\zx\unreal