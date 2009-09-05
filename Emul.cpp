#include "std.h"
#pragma hdrstop

#define USE_SND_EXTERNAL_BUFFER
#include "mods.h"

//#include "9x.h"
#include "ddk.h"
#include "resource.h"

//------- Alone Coder ---------
#include <winioctl.h> 
#define INVALID_SET_FILE_POINTER -1
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

#include "emul_2203.h" //Dexus
#include "sndrender/sndrender.h"
#include "emul.h"
#include "sndrender/sndchip.h"
#include "sndrender/sndcounter.h"
#include "funcs.h"
#include "vars.cpp"
#include "util.cpp"
#include "iehelp.cpp"
#include "draw.cpp"
#include "atm.cpp"
#include "dx.cpp"

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
   printf("\nexception %08X at eip=%08X\n",
                pp->ExceptionRecord->ExceptionCode,
                pp->ExceptionRecord->ExceptionAddress);
   printf("eax=%08X ebx=%08X ecx=%08X edx=%08X\n"
          "esi=%08X edi=%08X ebp=%08X esp=%08X\n",
          pp->ContextRecord->Eax, pp->ContextRecord->Ebx,
          pp->ContextRecord->Ecx, pp->ContextRecord->Edx,
          pp->ContextRecord->Esi, pp->ContextRecord->Edi,
          pp->ContextRecord->Ebp, pp->ContextRecord->Esp);
   color();
   return EXCEPTION_CONTINUE_SEARCH;
}

void __declspec(noreturn) __cdecl main(int argc, char **argv)
{
   CONSOLE_SCREEN_BUFFER_INFO csbi;
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
   nowait = *(unsigned*)&csbi.dwCursorPosition;

   color(CONSCLR_TITLE);
   printf("UnrealSpeccy " VERS_STRING_ " by SMT\n");
   printf("Integrated YM2203 version 1.4A by Tatsuyuki Satoh, Jarek Burczynski, Alone Coder\n"); //Dexus
   color();
//   printf(" *** source code available at http://sourceforge.net/projects/unrealspeccy/ ***\n");
   printf(" *** source code is available at http://alonecoder.narod.ru/ ***\n");

   SetUnhandledExceptionFilter(filter);
   SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

   init_all(argc-1, argv+1);
   applyconfig();
   sound_play();
   color();
   mainloop();
}
