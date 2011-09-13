!ifdef USE_CL
CXX=cl -c
ICL_FLAGS_COMMON=
ICL_FLAGS_RELEASE=
ICL_IA32=
CL_FLAGS_RELEASE=-Ox -GL
LIB=lib
LIBFLAGS=-LTCG
!else
CXX=icl -c
ICL_FLAGS_COMMON=-Wcheck -Qms0
ICL_FLAGS_RELEASE=-O3 -Qipo
ICL_IA32=-arch:IA32
CL_FLAGS_RELEASE=
LIB=xilib
LIBFLAGS=
!endif

CFLAGS_COMMON=-nologo -W3 -EHa- -GR- -Zi -MT -MP -Oi $(ICL_FLAGS_COMMON) \
         -D_CRT_SECURE_NO_DEPRECATE -DUSE_SND_EXTERNAL_BUFFER -D_PREFIX_

!ifdef SSE1
CFLAGS_COMMON=$(CFLAGS_COMMON) -QxK
!elseifdef SSE2
CFLAGS_COMMON=$(CFLAGS_COMMON) -arch:SSE2 -D_M_IX86_FP=2
!else
CFLAGS_COMMON=$(CFLAGS_COMMON) $(ICL_IA32)
!endif

!ifdef DEBUG
CFLAGS_DEBUG=-Od -DDEBUG -D_DEBUG
!else
CFLAGS_RELEASE=$(CL_FLAGS_RELEASE) $(ICL_FLAGS_RELEASE)
!endif

CXXFLAGS=$(CFLAGS_COMMON) $(CFLAGS_DEBUG) $(CFLAGS_RELEASE) -Zc:forScope,wchar_t
CFLAGS=$(CFLAGS_COMMON) $(CFLAGS_DEBUG) $(CFLAGS_RELEASE) -Zc:wchar_t

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
