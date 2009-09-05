#include "std.h"
#pragma hdrstop

#define SND_TEST_FAILURES
//#define SND_TEST_SHOWSTAT
#define USE_SND_EXTERNAL_BUFFER
#include "mods.h"

//#include "9x.h"
#include "ddk.h"
#include "resource.h"

//------- Alone Coder ---------
#include <winioctl.h>

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER (DWORD)-1
#endif

__inline unsigned short _byteswap_ushort (unsigned short i){return (i>>8)|(i<<8);}
__inline unsigned long _byteswap_ulong(unsigned long i){return _byteswap_ushort((unsigned short)(i>>16))|(_byteswap_ushort((unsigned short)i)<<16);};
typedef unsigned int UINT32;
typedef signed int INT32;
typedef unsigned short UINT16;
typedef signed short INT16;
typedef unsigned char UINT8;
typedef signed char INT8;
int fmsoundon0=4; //Alone Coder
int tfmstatuson0=2; //Alone Coder
char pressedit = 0; //Alone Coder
unsigned frametime = 111111; //Alone Coder (GUI value for conf.frame)
//~------- Alone Coder ---------

int nmi_pending = 0;

bool ConfirmExit();
BOOL WINAPI ConsoleHandler(DWORD CtrlType);

#include "emul_2203.h" //Dexus
#include "sndrender/sndrender.h"
#include "emul.h"
#include "sndrender/sndchip.h"
#include "sndrender/sndcounter.h"
#include "funcs.h"
#include "debug.h"
#include "vars.cpp"
#include "util.cpp"
#include "iehelp.cpp"
#include "draw.cpp"
#include "atm.cpp"
#include "dx.cpp"

void m_nmi(ROM_MODE page);
void showhelp(char *anchor = 0)
{
   sound_stop(); //Alone Coder 0.36.6
   showhelppp(anchor); //Alone Coder 0.36.6
   sound_play(); //Alone Coder 0.36.6
}

#include "emul_2203.cpp" //Dexus
#include "sound.cpp"
#include "memory.cpp"
#include "z80.cpp"
#include "vs1001.h"
#include "vs1001.cpp"
#include "sdcard.h"
#include "sdcard.cpp"
#include "zc.h"
#include "zc.cpp"
#include "gs.cpp"
#include "tape.cpp"
#include "wd93crc.cpp"
#include "wd93trk.cpp"
#include "wd93dat.cpp"
#include "wd93cmd.cpp"
#include "hddaspi.cpp"
#include "hddio.cpp"
#include "hdd.cpp"
#include "input.cpp"
#include "modem.cpp"
#include "io.cpp"
#include "config.cpp"
#include "opendlg.cpp"
#include "snapshot.cpp"
#include "debug.cpp"
#include "leds.cpp"
#include "savesnd.cpp"
#include "fntsrch.cpp"
#include "gui.cpp"
#include "cheat.cpp"
#include "emulkeys.cpp"
#include "keydefs.cpp"
#include "mainloop.cpp"
#include "init.cpp"

LONG __stdcall filter(EXCEPTION_POINTERS *pp)
{
   color(CONSCLR_ERROR);
   printf("\nexception %08X at eip=%p\n",
                pp->ExceptionRecord->ExceptionCode,
                pp->ExceptionRecord->ExceptionAddress);
#if _M_IX86
   printf("eax=%08X ebx=%08X ecx=%08X edx=%08X\n"
          "esi=%08X edi=%08X ebp=%08X esp=%08X\n",
          pp->ContextRecord->Eax, pp->ContextRecord->Ebx,
          pp->ContextRecord->Ecx, pp->ContextRecord->Edx,
          pp->ContextRecord->Esi, pp->ContextRecord->Edi,
          pp->ContextRecord->Ebp, pp->ContextRecord->Esp);
#endif
#if _M_IX64
   printf("rax=%08X rbx=%08X rcx=%08X rdx=%08X\n"
          "rsi=%08X rdi=%08X rbp=%08X rsp=%08X\n",
          pp->ContextRecord->Rax, pp->ContextRecord->Rbx,
          pp->ContextRecord->Rcx, pp->ContextRecord->Rdx,
          pp->ContextRecord->Rsi, pp->ContextRecord->Rdi,
          pp->ContextRecord->Rbp, pp->ContextRecord->Rsp);
#endif
   color();
   return EXCEPTION_CONTINUE_SEARCH;
}

static bool Exit = false;

bool ConfirmExit()
{
    if(!conf.ConfirmExit)
        return true;

    return MessageBox(wnd, "Exit ?", "Unreal", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND) == IDYES;
}

BOOL WINAPI ConsoleHandler(DWORD CtrlType)
{
    switch(CtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        if(ConfirmExit())
            Exit = true;
        return TRUE;
    }
    return FALSE;
}

__declspec(noreturn)
int __cdecl main(int argc, char **argv)
{
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
   nowait = *(unsigned*)&csbi.dwCursorPosition;

   SetThreadAffinityMask(GetCurrentThread(), 1);

   SetConsoleCtrlHandler(ConsoleHandler, TRUE);

   color(CONSCLR_TITLE);
   printf("UnrealSpeccy " VERS_STRING_ " by SMT\n");
   printf("Integrated YM2203 version 1.4A by Tatsuyuki Satoh, Jarek Burczynski, Alone Coder\n"); //Dexus
   printf("Intel C++ Compiler: %d.%02d\n", __ICL/100, __ICL % 100);
   color();
//   printf(" *** source code available at http://sourceforge.net/projects/unrealspeccy/ ***\n");
//   printf(" *** source code is available at http://alonecoder.narod.ru/ ***\n");
   printf(" *** new versions & sources -> http://dlcorp.ucoz.ru/forum/17-255-1 ***\n");

#ifndef DEBUG
   SetUnhandledExceptionFilter(filter);
#endif

   SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
   init_all(argc-1, argv+1);
//   applyconfig();
   sound_play();
   color();
   mainloop(Exit);
   return 0;
}
