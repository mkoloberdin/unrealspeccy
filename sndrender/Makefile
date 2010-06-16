CXX=icl -c

CFLAGS_COMMON=-nologo -W3 -Wcheck -Qms0 -EHa- -GR- -Zi -MT -Oi \
         -D_CRT_SECURE_NO_DEPRECATE -DUSE_SND_EXTERNAL_BUFFER

!ifdef SSE1
CFLAGS_COMMON=$(CFLAGS_COMMON) -QxK
!elseifdef SSE2
CFLAGS_COMMON=$(CFLAGS_COMMON) -arch:SSE2 -D_M_IX86_FP=2
!else
CFLAGS_COMMON=$(CFLAGS_COMMON) -arch:IA32
!endif

!ifdef DEBUG
CFLAGS_DEBUG=-Od -DDEBUG -D_DEBUG
!else
CFLAGS_RELEASE=-O3 -Qipo
!endif

CXXFLAGS=$(CFLAGS_COMMON) $(CFLAGS_DEBUG) $(CFLAGS_RELEASE) -Zc:forScope,wchar_t
CFLAGS=$(CFLAGS_COMMON) $(CFLAGS_DEBUG) $(CFLAGS_RELEASE) -Zc:wchar_t
LIB=xilib

all: snd.lib

.c.obj::
	$(CXX) $(CFLAGS) $<

.cpp.obj::
	$(CXX) $(CXXFLAGS) $<

snd.lib: sndbuffer.obj sndchip.obj sndcounter.obj sndrender.obj
	$(LIB) $(LIBFLAGS) -out:$@ $**
clean:
	-del *.obj *.lib
