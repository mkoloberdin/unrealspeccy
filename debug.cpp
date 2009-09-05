
#ifdef MOD_MONITOR

#define trace_size 21
#define trace_x 1
#define trace_y 6

unsigned char trace_labels;

#define wat_x 34
#define wat_y 1
#define wat_sz 13
unsigned show_scrshot;
unsigned user_watches[3] = { 0x4000, 0x8000, 0xC000 };

#define mem_size 12
#define mem_x 34
#define mem_y 15
unsigned mem_sz = 8;
unsigned mem_disk, mem_track, mem_max;
enum { ED_MEM, ED_PHYS, ED_LOG };
unsigned char mem_ascii, mem_dump, editor = ED_MEM;

#define regs_x 1
#define regs_y 1
unsigned regs_curs;

#define stack_x 72
#define stack_y 12
#define stack_size 10

#define ay_x  31
#define ay_y  28

#define time_x 1
#define time_y 28

#define copy_x 1
#define copy_y 28

#define banks_x 72
#define banks_y 22

#define ports_x 72
#define ports_y 1
unsigned dbg_extport; unsigned char dgb_extval; // extended memory port like 1FFD or DFFD

#define dos_x 72
#define dos_y 6

#define W_SEL      0x17
#define W_NORM     0x07
#define W_CURS     0x30
#define BACKGR     0x50
#define FRAME_CURS 0x02
#define W_TITLE    0x59
#define W_OTHER    0x40
#define W_OTHEROFF 0x47
#define BACKGR_CH  0xB1
#define W_AYNUM    0x4F
#define W_AYON     0x41
#define W_AYOFF    0x40
#define W_BANK     0x40
#define W_BANKRO   0x41
#define W_DIHALT1  0x1A
#define W_DIHALT2  0x0A
#define W_TRACEPOS 0x70
#define W_INPUTCUR 0x60
#define W_INPUTBG  0x40

#define W_TRACE_JINFO_CURS_FG   0x0D
#define W_TRACE_JINFO_NOCURS_FG 0x02
#define W_TRACE_JARROW_FOREGR   0x0D

#define FRAME         0x01
#define FFRAME_FRAME  0x04

#define FFRAME_INSIDE 0x50
#define FFRAME_ERROR  0x52
#define FRM_HEADER    0xD0

#define MENU_INSIDE   0x70
#define MENU_HEADER   0xF0

#define MENU_CURSOR   0xE0
#define MENU_ITEM     MENU_INSIDE
#define MENU_ITEM_DIS 0x7A

unsigned ripper; // ripper mode (none/read/write)
__int64 debug_last_t; // used to find time delta

enum DBGWND {
   WNDNO, WNDMEM, WNDTRACE, WNDREGS
} activedbg = WNDTRACE;

void debugscr();
unsigned find1dlg(unsigned start);
unsigned find2dlg(unsigned start);

#include "dbgpaint.cpp"
#include "dbglabls.cpp"
#include "z80asm.cpp"
#include "dbgreg.cpp"
#include "dbgmem.cpp"
#include "dbgtrace.cpp"
#include "dbgrwdlg.cpp"
#include "dbgcmd.cpp"
#include "dbgbpx.cpp"
#include "dbgoth.cpp"

void debugscr()
{
   memset(txtscr, BACKGR_CH, sizeof txtscr/2);
   memset(txtscr+sizeof txtscr/2, BACKGR, sizeof txtscr/2);
   nfr = 0;

   showregs();
   showtrace();
   showmem();
   showwatch();
   showstack();
   show_ay();
   showbanks();
   showports();
   showdos();

#if 1
   show_time();
#else
   tprint(copy_x, copy_y, "\x1A", 0x9C);
   tprint(copy_x+1, copy_y, "UnrealSpeccy " VERS_STRING, 0x9E);
   tprint(copy_x+20, copy_y, "by SMT", 0x9D);
   tprint(copy_x+26, copy_y, "\x1B", 0x9C);
   frame(copy_x, copy_y, 27, 1, 0x0A);
#endif
}

void handle_mouse()
{
   Z80 &cpu = CpuMgr.Cpu();
   unsigned mx = ((mousepos & 0xFFFF)-temp.gx)/8,
            my = (((mousepos >> 16) & 0x7FFF)-temp.gy)/16;
   if (my >= trace_y && my < trace_y+trace_size && mx >= trace_x && mx < trace_x+32)
   {
      needclr++; activedbg = WNDTRACE;
      cpu.trace_curs = cpu.trpc[my - trace_y];
      if (mx - trace_x < cs[1][0]) cpu.trace_mode = 0;
      else if (mx - trace_x < cs[2][0]) cpu.trace_mode = 1;
      else cpu.trace_mode = 2;
   }
   if (my >= mem_y && my < mem_y+mem_size && mx >= mem_x && mx < mem_x+37)
   {
      needclr++; activedbg = WNDMEM;
      unsigned dx = mx-mem_x;
      if (mem_dump)
      {
         if (dx >= 5)
             cpu.mem_curs = cpu.mem_top + (dx-5) + (my-mem_y)*32;
      }
      else
      {
         unsigned mem_se = (dx-5)%3;
         if (dx >= 29) cpu.mem_curs = cpu.mem_top + (dx-29) + (my-mem_y)*8, mem_ascii=1;
         if (dx >= 5 && mem_se != 2 && dx < 29)
            cpu.mem_curs = cpu.mem_top + (dx-5)/3 + (my-mem_y)*8,
            cpu.mem_second = mem_se, mem_ascii=0;
      }
   }
   if (mx >= regs_x && my >= regs_y && mx < regs_x+32 && my < regs_y+4) {
      needclr++; activedbg = WNDREGS;
      for (unsigned i = 0; i < sizeof regs_layout / sizeof *regs_layout; i++) {
         unsigned delta = 1;
         if (regs_layout[i].width == 16) delta = 4;
         if (regs_layout[i].width == 8) delta = 2;
         if (my-regs_y == regs_layout[i].y && mx-regs_x-regs_layout[i].x < delta) regs_curs = i;
      }
   }
   if (mousepos & 0x80000000) { // right-click
      enum { IDM_BPX=1, IDM_SOME_OTHER };
      HMENU menu = CreatePopupMenu();
      if (activedbg == WNDTRACE) {
         AppendMenu(menu, MF_STRING, IDM_BPX, "breakpoint");
      } else {
         AppendMenu(menu, MF_STRING, 0, "I don't know");
         AppendMenu(menu, MF_STRING, 0, "what to place");
         AppendMenu(menu, MF_STRING, 0, "to menu, so");
         AppendMenu(menu, MF_STRING, 0, "No Stuff Here");
      }
      int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN,
         (mousepos & 0xFFFF) + temp.client.left,
         ((mousepos>>16) & 0x7FFF) + temp.client.top, 0, wnd, 0);
      DestroyMenu(menu);
      if (cmd == IDM_BPX) cbpx();
      //if (cmd == IDM_SOME_OTHER) some_other();
      //needclr++;
   }
   mousepos = 0;
}

void TCpuMgr::CopyToPrev()
{
    for(int i = 0; i < Count; i++)
        PrevCpus[i] = *Cpus[i];
}
      /* ------------------------------------------------------------- */
void debug()
{
   sound_stop();
   temp.rflags = RF_MONITOR;
   needclr = dbgbreak = 1;
   set_video();

   Z80 *cpu = &CpuMgr.Cpu();
   Z80 *prevcpu = &CpuMgr.PrevCpu();
   cpu->trace_curs = cpu->pc;
   cpu->dbg_stopsp = cpu->dbg_stophere = -1;
   cpu->dbg_loop_r1 = 0, cpu->dbg_loop_r2 = 0xFFFF;
   mousepos = 0;

   while (dbgbreak) // debugger event loop
   {
      if (trace_labels)
         mon_labels.notify_user_labels();

      cpu = &CpuMgr.Cpu();
      prevcpu = &CpuMgr.PrevCpu();
repaint_dbg:
      cpu->trace_top &= 0xFFFF;
      cpu->trace_curs &= 0xFFFF;

      debugscr();
      if (cpu->trace_curs < cpu->trace_top || cpu->trace_curs >= cpu->trpc[trace_size] || asmii==-1)
      {
         cpu->trace_top = cpu->trace_curs;
         debugscr();
      }

      debugflip();

sleep:
      while(!dispatch(0))
      {
         if (mousepos)
             handle_mouse();
         if (needclr)
         {
             needclr--;
             goto repaint_dbg;
         }
         Sleep(20);
      }
      if (activedbg == WNDREGS && dispatch_more(ac_regs) > 0)
          continue;
      if (activedbg == WNDTRACE && dispatch_more(ac_trace) > 0)
          continue;
      if (activedbg == WNDMEM && dispatch_more(ac_mem) > 0)
          continue;
      if (activedbg == WNDREGS && dispatch_regs())
          continue;
      if (activedbg == WNDTRACE && dispatch_trace())
          continue;
      if (activedbg == WNDMEM && dispatch_mem())
          continue;
      if (needclr)
      {
          needclr--;
          continue;
      }
      goto sleep;
   }

   *prevcpu = *cpu;
//   CpuMgr.CopyToPrev();
   debug_last_t = comp.t_states + cpu->t;
   apply_video();
   sound_play();
}

void debug_events()
{
   Z80 &cpu = CpuMgr.Cpu();

   unsigned pc = cpu.pc & 0xFFFF;
   unsigned char *membit = membits + pc;
   *membit |= MEMBITS_X;
   dbgbreak |= (*membit & MEMBITS_BPX);

   if (pc == cpu.dbg_stophere)
       dbgbreak = 1;

   if ((cpu.sp & 0xFFFF) == cpu.dbg_stopsp)
   {
      if (pc > cpu.dbg_stophere && pc < cpu.dbg_stophere + 0x100)
          dbgbreak = 1;
      if (pc < cpu.dbg_loop_r1 || pc > cpu.dbg_loop_r2)
          dbgbreak = 1;
   }

   if (cbpn)
   {
      cpu.r_low = (cpu.r_low & 0x7F) + cpu.r_hi;
      for (unsigned i = 0; i < cbpn; i++)
      {
         if (calc(cbp[i]))
             dbgbreak = 1;
      }
   }

   brk_port_in = brk_port_out = -1; // reset only when breakpoints active

   if (dbgbreak)
       debug();
}

#endif // MOD_MONITOR

unsigned char isbrk() // is there breakpoints active or any other reason to use debug z80 loop?
{
#ifndef MOD_DEBUGCORE
   return 0;
#else

   #ifdef MOD_MEMBAND_LED
   if (conf.led.memband & 0x80000000) return 1;
   #endif

   if (conf.mem_model == MM_PROFSCORP) return 1; // breakpoint on read ROM switches ROM bank

   #ifdef MOD_MONITOR
   if (cbpn) return 1;
   unsigned char res = 0;
   for (int i = 0; i < 0x10000; i++) res |= membits[i];
   return (res & (MEMBITS_BPR | MEMBITS_BPW | MEMBITS_BPX));
   #endif

#endif
}
