Z80 prevcpu;
struct {
   unsigned char offs, width;
   unsigned char x,y;
   unsigned char lf,rt,up,dn;
} regs_layout[] = {
   { (int)&cpu.a        - (int)&cpu,  8,  3, 0, 0, 1, 0, 2 }, //  0 a
   { (int)&cpu.f        - (int)&cpu,  8,  5, 0, 0, 5, 1, 2 }, //  1 f
   { (int)&cpu.bc       - (int)&cpu, 16,  3, 1, 2, 6, 0, 3 }, //  2 bc
   { (int)&cpu.de       - (int)&cpu, 16,  3, 2, 3, 7, 2, 4 }, //  3 de
   { (int)&cpu.hl       - (int)&cpu, 16,  3, 3, 4, 8, 3, 4 }, //  4 hl
   { (int)&cpu.alt.af   - (int)&cpu, 16, 11, 0, 1, 9, 5, 6 }, //  5 af'
   { (int)&cpu.alt.bc   - (int)&cpu, 16, 11, 1, 2,10, 5, 7 }, //  6 bc'
   { (int)&cpu.alt.de   - (int)&cpu, 16, 11, 2, 3,11, 6, 8 }, //  7 de'
   { (int)&cpu.alt.hl   - (int)&cpu, 16, 11, 3, 4,12, 7, 8 }, //  8 hl'
   { (int)&cpu.sp       - (int)&cpu, 16, 19, 0, 5,13, 9,10 }, //  9 sp
   { (int)&cpu.pc       - (int)&cpu, 16, 19, 1, 6,10, 9,11 }, // 10 pc
   { (int)&cpu.ix       - (int)&cpu, 16, 19, 2, 7,15,10,12 }, // 11 ix
   { (int)&cpu.iy       - (int)&cpu, 16, 19, 3, 8,18,11,12 }, // 12 iy
   { (int)&cpu.i        - (int)&cpu,  8, 28, 0, 9,14,13,16 }, // 13 i
   { (int)&cpu.r_low    - (int)&cpu,  8, 30, 0,13,14,14,17 }, // 14 r
   { (int)&cpu.im       - (int)&cpu,  2, 26, 2,11,16,13,20 }, // 15 im
   { (int)&cpu.iff1     - (int)&cpu,  1, 30, 2,15,17,13,24 }, // 16 iff1
   { (int)&cpu.iff2     - (int)&cpu,  1, 31, 2,16,17,14,25 }, // 17 iff2
   { (int)&cpu.f        - (int)&cpu, 37, 24, 3,12,19,15,18 }, // 18 SF
   { (int)&cpu.f        - (int)&cpu, 36, 25, 3,18,20,15,19 }, // 19 ZF
   { (int)&cpu.f        - (int)&cpu, 35, 26, 3,19,21,15,20 }, // 20 F5
   { (int)&cpu.f        - (int)&cpu, 34, 27, 3,20,22,15,21 }, // 21 HF
   { (int)&cpu.f        - (int)&cpu, 33, 28, 3,21,23,15,22 }, // 22 F3
   { (int)&cpu.f        - (int)&cpu, 32, 29, 3,22,24,16,23 }, // 23 PV
   { (int)&cpu.f        - (int)&cpu, 31, 30, 3,23,25,16,24 }, // 24 NF
   { (int)&cpu.f        - (int)&cpu, 30, 31, 3,24,25,17,25 }, // 25 CF
};
void showregs() {
   unsigned char atr = (activedbg == WNDREGS) ? W_SEL : W_NORM;
   char line[40];
   tprint(regs_x,regs_y+0, "af:**** af'**** sp:**** ir: ****", atr);
   tprint(regs_x,regs_y+1, "bc:**** bc'**** pc:**** t:******", atr);
   tprint(regs_x,regs_y+2, "de:**** de'**** ix:**** im?,i:**", atr);
   tprint(regs_x,regs_y+3, "hl:**** hl'**** iy:**** ########", atr);

   if (cpu.halted && !cpu.iff1) {
      tprint(regs_x+26,regs_y+1,"DiHALT", (activedbg == WNDREGS) ? W_DIHALT1 : W_DIHALT2);
   } else sprintf(line, "%6d", cpu.t), tprint(regs_x+26,regs_y+1,line,atr);

   cpu.r_low = (cpu.r_low & 0x7F) + cpu.r_hi;
   for (unsigned i = 0; i < sizeof regs_layout / sizeof *regs_layout; i++) {
      unsigned mask = (1 << regs_layout[i].width) - 1;
      unsigned val = mask & *(unsigned*)((int)&cpu+regs_layout[i].offs);
      unsigned char atr1 = atr;
      if (activedbg == WNDREGS && i == regs_curs) atr1 = W_CURS;
      if (val != (mask & *(unsigned*)((int)&prevcpu+regs_layout[i].offs))) atr1 |= 0x08;
      char bf[16];
      switch (regs_layout[i].width) {
         case  8: sprintf(bf, "%02X", val); break;
         case 16: sprintf(bf, "%04X", val); break;
         case 1: case 2: sprintf(bf, "%X", val); break;
         default: *bf = 0;
      }
      tprint(regs_x + regs_layout[i].x, regs_y + regs_layout[i].y, bf, atr1);
   }
   static char flg[] = "SZ5H3PNCsz.h.pnc";
   for (unsigned char q = 0; q < 8; q++) {
      unsigned ln; unsigned char atr1 = atr;
      if (activedbg == WNDREGS && regs_curs == (unsigned)(q+18)) atr1 = W_CURS;
      ln = flg[q+((cpu.af & (0x80>>q)) ? 0 : 8)];
      if ((0x80>>q)&(cpu.f^prevcpu.f)) atr1 |= 0x08;
      tprint(regs_x+24+q,regs_y+3,(char*)&ln,  atr1);
   }
   tprint(regs_x, regs_y-1, "regs", W_TITLE);
   frame(regs_x,regs_y,32,4, FRAME);
}

void rleft() { regs_curs = regs_layout[regs_curs].lf; }
void rright() { regs_curs = regs_layout[regs_curs].rt; }
void rup() { regs_curs = regs_layout[regs_curs].up; }
void rdown() { regs_curs = regs_layout[regs_curs].dn; }
void renter() {
   debugscr(), debugflip();
   unsigned char sz = regs_layout[regs_curs].width;
   unsigned val = ((1 << sz) - 1) & *(unsigned*)((int)&cpu + regs_layout[regs_curs].offs);
   unsigned char *ptr = (unsigned char*)&cpu + regs_layout[regs_curs].offs;
   if ((sz == 8 || sz == 16) && ((input.lastkey >= '0' && input.lastkey <= '9') || (input.lastkey >= 'A' && input.lastkey <= 'F')))
      PostThreadMessage(GetCurrentThreadId(), WM_KEYDOWN, input.lastkey, 1);
   switch (sz) {
      case 8:
         val = input2(regs_x + regs_layout[regs_curs].x, regs_y + regs_layout[regs_curs].y, val);
         if (val != -1) *ptr = val;
         break;
      case 16:
         val = input4(regs_x + regs_layout[regs_curs].x, regs_y + regs_layout[regs_curs].y, val);
         if (val != -1) *(unsigned short*)ptr = val;
         break;
      case 1:
         *ptr ^= 1; break;
      case 2:
         *ptr = (*ptr + 1) % 3; break;
      default: // flags
         *ptr ^= (1 << (sz-30));
   }
   cpu.r_hi = cpu.r_low & 0x80;
}
void ra() { regs_curs = 0; input.lastkey = 0; renter(); }
void rf() { regs_curs = 1; input.lastkey = 0; renter(); }
void rbc() { regs_curs = 2; input.lastkey = 0; renter(); }
void rde() { regs_curs = 3; input.lastkey = 0; renter(); }
void rhl() { regs_curs = 4; input.lastkey = 0; renter(); }
void rsp() { regs_curs = 9; input.lastkey = 0; renter(); }
void rpc() { regs_curs = 10; input.lastkey = 0; renter(); }
void rix() { regs_curs = 11; input.lastkey = 0; renter(); }
void riy() { regs_curs = 12; input.lastkey = 0; renter(); }
void ri() { regs_curs = 13; input.lastkey = 0; renter(); }
void rr() { regs_curs = 14; input.lastkey = 0; renter(); }
void rm() { regs_curs = 15; renter(); }
void r_1() { regs_curs = 16; renter(); }
void r_2() { regs_curs = 17; renter(); }
void rSF() { regs_curs = 18; renter(); }
void rZF() { regs_curs = 19; renter(); }
void rF5() { regs_curs = 20; renter(); }
void rHF() { regs_curs = 21; renter(); }
void rF3() { regs_curs = 22; renter(); }
void rPF() { regs_curs = 23; renter(); }
void rNF() { regs_curs = 24; renter(); }
void rCF() { regs_curs = 25; renter(); }

void rcodejump() { if (regs_layout[regs_curs].width == 16) activedbg = WNDTRACE, trace_curs = trace_top = *(unsigned short*)((int)&cpu + regs_layout[regs_curs].offs); }
void rdatajump() { if (regs_layout[regs_curs].width == 16) activedbg = WNDMEM, editor = ED_MEM, mem_curs = *(unsigned short*)((int)&cpu + regs_layout[regs_curs].offs); }

char dispatch_regs() {
   if ((input.lastkey >= '0' && input.lastkey <= '9') || (input.lastkey >= 'A' && input.lastkey <= 'F')) {
      renter();
      return 1;
   }
   return 0;
}
