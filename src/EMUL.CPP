#include "std.h"
#pragma hdrstop

#include "mods.h"

#include "ddk.h"
#include "resource.h"

#include "emul.h"
#include "funcs.h"
#include "vars.cpp"
#include "util.cpp"
#include "iehelp.cpp"
#include "draw.cpp"
#include "atm.cpp"
#include "dx.cpp"
#include "sound.cpp"
#include "memory.cpp"
#include "z80.cpp"
#include "gs.cpp"
#include "tape.cpp"
#include "wd93dat.cpp"
#include "wd93cmd.cpp"
#include "hdd.cpp"
#include "input.cpp"
#include "modem.cpp"
#include "io.cpp"
#include "config.cpp"
#include "snapshot.cpp"
#include "debug.cpp"
#include "leds.cpp"
#include "savesnd.cpp"
#include "fntsrch.cpp"
#include "gui.cpp"
#include "emulkeys.cpp"
#include "keydefs.cpp"
#include "mainloop.cpp"

void init_all(int argc, char **argv)
{
   char *config = 0, legacy = 0;
   static const char vererror[] = "unsupported OS version";
   for (int i = 0; i < argc; i++) {
      if (argv[i][0] != '/' && argv[i][0] != '-') continue;
      if (!stricmp(argv[i]+1, "i") && i+1 < argc)
         config = argv[i+1], i++;
      #ifdef MOD_9X
      if (argv[i][1] == '9') legacy = 1;
      #endif
   }
   if (GetVersion() >> 31) {
      if (!legacy) printf(vererror), exit();
      #ifdef MOD_9X
      static const char verwarning[] =
         "detected windows version is not supported\r\n"
         "by current version of UnrealSpeccy and untested.\r\n\r\n"
         "you may click 'YES' to continue on your own risk.\r\n"
         "in this case you will experience crashes under\r\n"
         "some conditions. it's an OS-specific problem, please\r\n"
         "don't report it and consider using NT-based system";
      if (MessageBox(0, verwarning, vererror, MB_ICONERROR | MB_YESNO | MB_DEFBUTTON2) == IDNO) exit();
      #endif
   }
   init_z80tables();
   video_permanent_tables();
   init_ie_help();
   load_config(config);
   make_samples();
   #ifdef MOD_GS
   init_gs();
   #endif
   init_leds();
   init_dos();
   init_tape();
   init_hdd_cd();
   start_dx();
   applyconfig();
   main_reset();
   for (; argc; argc--, argv++) {
      if (**argv == '-' || **argv == '/') {
         if (argc > 1 && !stricmp(argv[0]+1, "i")) argc--, argv++;
         continue;
      }
      char fname[0x200], *temp;
      GetFullPathName(*argv, sizeof fname, fname, &temp);
      if (!loadsnap(fname))
         printf("error loading %s\n", *argv), load_errors = 1;
   }
/*
   if (load_errors) {
      printf("press ESC to exit, any other key to continue...");
      char c = getch(); printf(clrline);
      if (c == 0x1B) exit(0);
   }
*/
   SetCurrentDirectory(conf.workdir);
   timeBeginPeriod(1);
}

void __declspec(noreturn) exit()
{
   exitflag = 1;
   if (savesndtype) savesnddialog();
   done_dos();
   done_tape();
   done_dx();
   done_leds();
   save_nv();
   modem.close();
   done_ie_help();
   timeEndPeriod(1);
   printf("\nsee you later!");
   if (!nowait) SetConsoleTitle("press a key..."), getch();
   ExitProcess(0);
}

LONG __stdcall filter(EXCEPTION_POINTERS *pp)
{
   printf("\nexception %08X at eip=%08X\n",
                pp->ExceptionRecord->ExceptionCode,
                pp->ExceptionRecord->ExceptionAddress);
   printf("eax=%08X ebx=%08X ecx=%08X edx=%08X\n"
          "esi=%08X edi=%08X ebp=%08X esp=%08X\n",
          pp->ContextRecord->Eax, pp->ContextRecord->Ebx,
          pp->ContextRecord->Ecx, pp->ContextRecord->Edx,
          pp->ContextRecord->Esi, pp->ContextRecord->Edi,
          pp->ContextRecord->Ebp, pp->ContextRecord->Esp);
   return EXCEPTION_CONTINUE_SEARCH;
}

void __declspec(noreturn) __cdecl main(int argc, char **argv)
{
   Sleep(20); DWORD ticks = GetTickCount();
   unsigned __int64 clocks = rdtsc();

   CONSOLE_SCREEN_BUFFER_INFO csbi;
   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
   nowait = *(unsigned*)&csbi.dwCursorPosition;

   printf("UnrealSpeccy " VERS_STRING_ " by SMT\n");
   printf(" *** source code available at http://sourceforge.net/projects/unrealspeccy/ ***\n");

   SetUnhandledExceptionFilter(filter);
   SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

   char idstr[64]; fillCpuString(idstr); trim(idstr);
   unsigned cpuver = cpuid(1,0);
   temp.mmx = (cpuid(1,1) >> 23) & 1;

   init_all(argc-1, argv+1);

   Sleep(20);
   ticks = GetTickCount() - ticks;
   clocks = rdtsc() - clocks;
   conf.cpufq = (unsigned)(clocks*1000/ticks);

   printf("cpu: %s %d.%d.%d [%sMMX] at %d MHz\n",
      idstr,
      (cpuver>>8) & 0x0F, (cpuver>>4) & 0x0F, cpuver & 0x0F,
      temp.mmx ? nil : "non-",
      (unsigned)(conf.cpufq/1000000));

   applyconfig();
   sound_play();
   mainloop();
}
