
#define trace_size 21
#define trace_x 1
#define trace_y 6
unsigned trace_curs, trace_top, trace_mode;
unsigned char trace_labels;

#define wat_x 34
#define wat_y 1
#define wat_sz 13
unsigned show_scrshot;
unsigned user_watches[3] = { 0x4000, 0x8000, 0xC000 };

#define mem_size 12
#define mem_x 34
#define mem_y 15
unsigned mem_curs, mem_top, mem_second, mem_sz = 8;
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

#define FRAME         0x01
#define FFRAME_FRAME  0x04

#define FFRAME_INSIDE 0x50
#define FFRAME_ERROR  0x52
#define FRM_HEADER    0xD0

unsigned ripper; // ripper mode (none/read/write)
__int64 debug_last_t; // used to find time delta

unsigned char txtscr[80*30*2];
struct {
   unsigned char x,y,dx,dy,c;
} frames[20]; unsigned nfr;


enum DBGWND {
   WNDNO, WNDMEM, WNDTRACE, WNDREGS
} activedbg = WNDTRACE;

void frame(unsigned x, unsigned y, unsigned dx, unsigned dy, unsigned char attr) {
   frames[nfr].x = x;
   frames[nfr].y = y;
   frames[nfr].dx = dx;
   frames[nfr].dy = dy;
   frames[nfr].c = attr;
   nfr++;
}

void tprint(unsigned x, unsigned y, char *str, unsigned char attr) {
   for (unsigned ptr = y*80 + x; *str; str++, ptr++) {
      txtscr[ptr] = *str; txtscr[ptr+80*30] = attr;
   }
}

void filledframe(unsigned x, unsigned y, unsigned dx, unsigned dy) {
   for (unsigned yy = y; yy < (y+dy); yy++)
      for (unsigned xx = x; xx < (x+dx); xx++)
         txtscr[yy*80+xx] = ' ',
         txtscr[yy*80+xx+30*80] = FFRAME_INSIDE;
   nfr = 0; // delete other frames while dialog
   frame(x,y,dx,dy,FFRAME_FRAME);
}

void debugflip() {
   if (!active) return;
   setpal(0);

   #define bptr gdibuf
   // print text
   int x,y;
   unsigned char *tptr = txtscr;
   for (y = 0; y < 16*30*640; y+=16*640) {
      for (x = 0; x < 80; x++, tptr++) {
         unsigned char ch = *tptr;
         unsigned char *fnt = &font16[ch*16];
         unsigned at = tptr[80*30] << 4;
         for (int yy = 0; yy < 16; yy++, fnt++) {
            *(unsigned*)(bptr+y+640*yy+x*8+0) = t.sctab8[0][(*fnt >>  4) + at];
            *(unsigned*)(bptr+y+640*yy+x*8+4) = t.sctab8[0][(*fnt & 0xF) + at];
         }
      }
   }
   if (show_scrshot) {
      unsigned char *dst = bptr + wat_y*16*640+wat_x*8;
      unsigned char *src = rbuf+temp.scx/4*(temp.b_top+192/2-wat_sz*16/2);
      src += temp.scx/8-37/2*2;
      for (unsigned y = 0; y < wat_sz*16; y++) {
         for (unsigned x = 0; x < 37; x++) {
            *(unsigned*)(dst+x*8+0) = t.sctab8[0][(src[x*2] >>  4) + src[2*x+1]*16];
            *(unsigned*)(dst+x*8+4) = t.sctab8[0][(src[x*2] & 0xF) + src[2*x+1]*16];
         }
         src += temp.scx/4, dst += 640;
      }
   }

   // show frames
   for (unsigned i = 0; i < nfr; i++) {
      unsigned char a1 = (frames[i].c | 0x08) * 0x11;
      y = frames[i].y*16-1;
      for (x = 8*frames[i].x-1; x < (frames[i].x+frames[i].dx)*8; x++) bptr[y*640+x] = a1;
      y = (frames[i].y+frames[i].dy)*16;
      for (x = 8*frames[i].x-1; x < (frames[i].x+frames[i].dx)*8; x++) bptr[y*640+x] = a1;
      x = frames[i].x*8-1;
      for (y = 16*frames[i].y; y < (frames[i].y+frames[i].dy)*16; y++) bptr[y*640+x] = a1;
      x = (frames[i].x+frames[i].dx)*8;
      for (y = 16*frames[i].y; y < (frames[i].y+frames[i].dy)*16; y++) bptr[y*640+x] = a1;
   }

   if (needclr) gdi_frame();
   SetDIBitsToDevice(temp.gdidc, temp.gx, temp.gy, 640, 480, 0, 0, 0, 480, bptr, &gdibmp.header, DIB_RGB_COLORS);
   #undef bptr
}

char str[0x80];
unsigned inputhex(unsigned x, unsigned y, unsigned sz, char hex) {
   unsigned cr = 0;
   mousepos = 0;
   for (;;) {
      str[sz] = 0; for (unsigned i = strlen(str); i < sz; i++) str[i] = ' ';
      for (i = 0; i < sz; i++) {
         unsigned vl = (unsigned char)str[i]; tprint(x+i,y,(char*)&vl,(i==cr) ? W_INPUTCUR : W_INPUTBG);
      }
      debugflip();
      unsigned key;
      for (;;) {
         key = process_msgs();
         if (mousepos) return 0;
         if (!key) { Sleep(20); continue; }
         break;
      }
      if (key == VK_ESCAPE) return 0;
      if (key == VK_RETURN) {
         for (char *ptr = str+sz-1; *ptr == ' ' && ptr >= str; *ptr-- = 0);
         return 1;
      }
      if (key == VK_LEFT || key == VK_BACK) { if (cr) cr--; continue; }
      if (key == VK_RIGHT && cr != sz-1) cr++;
      if (key == VK_HOME) cr=0;
      if (key == VK_END) for (cr=sz-1; cr && str[cr]==' ' && str[cr-1] == ' '; cr--);
      if (key == VK_DELETE) { for (i = cr; i < sz-1; i++) str[i]=str[i+1]; str[sz-1] = ' '; }
      if (key == VK_INSERT) { for (i = sz-1; i > cr; i--) str[i]=str[i-1]; str[cr] = ' '; }
      if (hex) {
         if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'F')) str[cr++] = (unsigned char)key;
      } else {
         GetKeyboardState(kbdpc); unsigned short k;
         if (ToAscii(key,0,kbdpc,&k,0) == 1) str[cr++] = (char)k;
      }
      if (cr == sz) cr--;
   }
}
unsigned input4(unsigned x, unsigned y, unsigned val) {
   sprintf(str, "%04X", val);
   if (inputhex(x,y,4,1)) { sscanf(str, "%x", &val); return val; }
   return -1;
}
unsigned input2(unsigned x, unsigned y, unsigned val) {
   sprintf(str, "%02X", val);
   if (inputhex(x,y,2,1)) { sscanf(str, "%x", &val); return val; }
   return -1;
}

void debugscr();
unsigned find1dlg(unsigned start);
unsigned find2dlg(unsigned start);

#include "dbglabls.cpp"
#include "z80asm.cpp"
#include "dbgreg.cpp"
#include "dbgmem.cpp"
#include "dbgcmd.cpp"
#include "dbgtrace.cpp"
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
   unsigned mx = ((mousepos & 0xFFFF)-temp.gx)/8,
            my = (((mousepos >> 16) & 0x7FFF)-temp.gy)/16;
   if (my >= trace_y && my < trace_y+trace_size && mx >= trace_x && mx < trace_x+32) {
      needclr++; activedbg = WNDTRACE;
      trace_curs = trpc[my - trace_y];
      if (mx - trace_x < cs[1][0]) trace_mode = 0;
      else if (mx - trace_x < cs[2][0]) trace_mode = 1;
      else trace_mode = 2;
   }
   if (my >= mem_y && my < mem_y+mem_size && mx >= mem_x && mx < mem_x+37) {
      needclr++; activedbg = WNDMEM;
      unsigned dx = mx-mem_x;
      if (mem_dump) {
         if (dx >= 5) mem_curs = mem_top + (dx-5) + (my-mem_y)*32;
      } else {
         unsigned mem_se = (dx-5)%3;
         if (dx >= 29) mem_curs = mem_top + (dx-29) + (my-mem_y)*8, mem_ascii=1;
         if (dx >= 5 && mem_se != 2 && dx < 29)
            mem_curs = mem_top + (dx-5)/3 + (my-mem_y)*8,
            mem_second = mem_se, mem_ascii=0;
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

      /* ------------------------------------------------------------- */
void debug()
{
   sound_stop();
   temp.rflags = RF_MONITOR;
   set_video();
   trace_curs = cpu.pc;
   dbg_stopsp = dbg_stophere = -1;
   needclr = dbgbreak = 1;
   mousepos = 0;

   while (dbgbreak)
   {
      if (trace_labels)
         mon_labels.notify_user_labels();

repaint_dbg:
      trace_top &= 0xFFFF, trace_curs &= 0xFFFF;

      debugscr();
      if (trace_curs < trace_top || trace_curs >= trpc[trace_size] || asmii==-1)
         trace_top = trace_curs, debugscr();

      debugflip();

sleep:
      while (!dispatch(0)) {
         if (mousepos) handle_mouse();
         if (needclr) { needclr--; goto repaint_dbg; }
         Sleep(20);
      }
      if (activedbg == WNDREGS && dispatch_more(ac_regs) > 0) continue;
      if (activedbg == WNDTRACE && dispatch_more(ac_trace) > 0) continue;
      if (activedbg == WNDMEM && dispatch_more(ac_mem) > 0) continue;
      if (activedbg == WNDREGS && dispatch_regs()) continue;
      if (activedbg == WNDTRACE && dispatch_trace()) continue;
      if (activedbg == WNDMEM && dispatch_mem()) continue;
      if (needclr) { needclr--; continue; }
      goto sleep;
   }
   prevcpu = cpu;
   debug_last_t = comp.t_states + cpu.t;
   sound_play();
   apply_video();
}

void debug_events()
{
   dbgbreak |= (membits[cpu.pc & 0xFFFF] & MEMBITS_BPX);
   if ((cpu.pc & 0xFFFF) == dbg_stophere) dbgbreak = 1;
   if ((cpu.sp & 0xFFFF) == dbg_stopsp && (cpu.pc & 0xFFFF) > dbg_stophere && (cpu.pc & 0xFFFF) < dbg_stophere+0x100) dbgbreak = 1;

   if (cbpn) {
      cpu.r_low = (cpu.r_low & 0x7F) + cpu.r_hi;
      for (unsigned i = 0; i < cbpn; i++)
         if (calc(cbp[i])) dbgbreak = 1;
   }

   brk_port_in = brk_port_out = -1; // reset only when breakpoints active

   if (dbgbreak) { debug(); return; }
}
