CXX=icl -c
LINK=xilink

#-RTCsu -Qtrapuv

CFLAGS_COMMON=-nologo -W3 -Wcheck -Qms0 -EHa- -GR- -Zi -MT -Oi \
         -D_CRT_SECURE_NO_DEPRECATE -DUSE_SND_EXTERNAL_BUFFER

!ifdef VGEMUL
CFLAGS_COMMON=$(CFLAGS_COMMON) -DVG_EMUL
!endif

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

LFLAGS= -debug -fixed:no -release

LIBS=$(LIBS) sndrender/snd.lib z80/z80.lib

all: x32/emul.exe

.c.obj::
	$(CXX) $(CFLAGS) $<

std.obj: std.cpp
	$(CXX) $(CXXFLAGS) -Yc"std.h" $?

.cpp.obj::
	$(CXX) $(CXXFLAGS) -Yu"std.h" $<

.rc.res:
	$(RC) $<

x32/emul.exe: emul.obj std.obj atm.obj cheat.obj config.obj dbgbpx.obj dbgcmd.obj dbglabls.obj \
	dbgmem.obj dbgoth.obj dbgpaint.obj dbgreg.obj dbgrwdlg.obj dbgtrace.obj \
        debug.obj draw.obj drawnomc.obj draw_384.obj dx.obj dxerr.obj dxovr.obj \
        dxrcopy.obj dxrend.obj dxrendch.obj dxrframe.obj dxr_4bpp.obj dxr_512.obj \
        dxr_advm.obj dxr_atm.obj dxr_atm0.obj dxr_atm2.obj dxr_atm6.obj dxr_atm7.obj \
        dxr_atmf.obj dxr_prof.obj dxr_rsm.obj dxr_text.obj dxr_vd.obj \
        emulkeys.obj emul_2203.obj fntsrch.obj font.obj font14.obj font16.obj \
        font8.obj fontatm2.obj fontdata.obj gs.obj gshlbass.obj gshle.obj \
        gsz80.obj gui.obj hdd.obj hddaspi.obj hddio.obj iehelp.obj init.obj \
        input.obj inputpc.obj io.obj keydefs.obj leds.obj mainloop.obj \
        memory.obj modem.obj opendlg.obj savesnd.obj sdcard.obj snapshot.obj \
        snd_bass.obj sound.obj sshot_png.obj tape.obj util.obj vars.obj \
        vs1001.obj wd93cmd.obj wd93crc.obj wd93dat.obj wd93trk.obj \
        wldr_fdi.obj wldr_isd.obj wldr_pro.obj wldr_td0.obj wldr_trd.obj wldr_udi.obj \
        z80.obj z80asm.obj zc.obj settings.res
	$(LINK) $(LFLAGS) -out:$@ -pdb:$*.pdb -map:$*.map $** $(LIBS)
clean:
	-del *.obj *.res *.map *.pdb *.pch *.pchi
