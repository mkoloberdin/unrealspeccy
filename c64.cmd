:-Wcheck /O3 /Qip
set ICL="%ICPP_COMPILER11%\bin\IA32_Intel64\icl.exe"
set LIB="G:\Program Files\Microsoft Platform SDK\Lib\AMD64";"G:\Program Files\Microsoft Visual Studio 8\VC\lib\amd64";"G:\WINDDK\6001.18000\lib\wlh\amd64";"%ICPP_COMPILER11%\lib\intel64";lib64
:/Od /Qtrapuv /RTCsu /Zi
%ICL% -DDEBUG -D_M_IX86_FP=2 /O2 emul.cpp runtmchk.lib settings.RES -Fex64\unreal.exe
:if exist unreal.exe copy /y unreal.exe h:\zx\unreal
