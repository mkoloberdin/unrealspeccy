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

all: z80.lib

.c.obj::
	$(CXX) $(CFLAGS) $<

.cpp.obj::
	$(CXX) $(CXXFLAGS) $<

z80.lib: daa_tabs.obj op_cb.obj op_dd.obj op_ddcb.obj op_ed.obj \
	op_fd.obj op_noprefix.obj tables.obj
	$(LIB) $(LIBFLAGS) -out:$@ $**
clean:
	-del *.obj *.lib
