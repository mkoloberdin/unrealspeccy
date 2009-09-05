
int disasm_line(unsigned addr, char *line)
{
   unsigned char dbuf[16];
   for (int i = 0; i < 16; i++) dbuf[i] = rmdbg(addr+i);
   sprintf(line, "%04X ", addr); int ptr = 5;
   int len = disasm(dbuf, addr, trace_labels) - dbuf;
   //8000 ..DDCB0106 rr (ix+1)
   if (trace_labels) {
      char *lbl = mon_labels.find(am_r(addr));
      if (lbl) for (int k = 0; k < 10 && lbl[k]; line[ptr++] = lbl[k++]);
   } else {
      int len1 = len;
      if (len > 4) len1 = 4, *(short*)(line+ptr) = WORD2('.','.'), ptr+=2;
      for (i = len-len1; i < len; i++)
         sprintf(line+ptr, "%02X", dbuf[i]), ptr += 2;
   }
   while (ptr < 16) line[ptr++] = ' ';
   strcpy(line+ptr, asmbuf);
   return len;
}

void mon_dis()
{
   if (!query_file_addr(2)) return;
   FILE *ff = fopen(fname, "wt");
   if (!ff) return;
   for (unsigned a = addr; a <= end; ) {
      char line[64]; a += disasm_line(a, line);
      fprintf(ff, "%s\n", line);
   }
   fclose(ff);
}

#define TWF_BRANCH  0x010000
#define TWF_BRADDR  0x020000
#define TWF_LOOPCMD 0x040000
#define TWF_CALLCMD 0x080000

unsigned tracewndflags()
{
   unsigned readptr = cpu.pc, base = cpu.hl;
   unsigned char opcode = 0; unsigned char ed = 0;
   for (;;) {
      opcode = rmdbg(readptr++);
      if (opcode == 0xDD) base = cpu.ix;
      else if (opcode == 0xFD) base = cpu.iy;
      else if (opcode == 0xED) ed = 1;
      else break;
   }

   unsigned fl = 0;

   if (ed) {
      if ((opcode & 0xC7) != 0x45) return 0; // reti/retn
 ret:
      return rmdbg(cpu.sp) + 0x100*rmdbg(cpu.sp+1) + TWF_BRANCH + TWF_BRADDR;
   }

   if (opcode == 0xC9) goto ret;
   if (opcode == 0xC3) { jp: return rmdbg(readptr) + 0x100*rmdbg(readptr+1) + TWF_BRANCH + fl; }
   if (opcode == 0xCD) { fl = TWF_CALLCMD; goto jp; }

   static const unsigned char flags[] = { ZF,CF,PV,SF };

   if ((opcode & 0xC1) == 0xC0) {
      unsigned char flag = flags[(opcode >> 4) & 3];
      unsigned char res = cpu.f & flag;
      if (!(opcode & 0x08)) res ^= flag;
      if (!res) return 0;
      if ((opcode & 0xC7) == 0xC0) goto ret;// ret cc
      if ((opcode & 0xC7) == 0xC4) { fl = TWF_CALLCMD; goto jp; } // call cc
      if ((opcode & 0xC7) == 0xC2) { fl = TWF_LOOPCMD; goto jp; } // jp cc
   }

   if (opcode == 0xE9) return base + TWF_BRANCH + TWF_BRADDR; // jp (hl/ix/iy)

   if ((opcode & 0xC7) == 0xC7) return (opcode & 0x38) + TWF_CALLCMD + TWF_BRANCH; // rst #xx

   if ((opcode & 0xC7) == 0x00) {
      if (!opcode || opcode == 0x08) return 0;
      int offs = (signed char)rmdbg(readptr++);
      unsigned addr = offs + readptr + TWF_BRANCH;
      if (opcode == 0x18) return addr; // jr
      if (opcode == 0x10) return (cpu.b==1)? 0 : addr + TWF_LOOPCMD; // djnz

      unsigned char flag = flags[(opcode >> 4) & 1]; // jr cc
      unsigned char res = cpu.f & flag;
      if (!(opcode & 0x08)) res ^= flag;
      return res? addr+TWF_LOOPCMD : 0;
   }
   return 0;
}

unsigned pc_trflags;
unsigned nextpc, trcurs_y; unsigned asmii;
unsigned trpc[40]; char asmpc[64], dumppc[12];
static const unsigned cs[3][2] = { {0,4}, {5,10}, {16,16} };

void showtrace()
{
   char line[40];

   trace_curs &= 0xFFFF, trace_top &= 0xFFFF, cpu.pc &= 0xFFFF;
   trace_mode = (trace_mode+3) % 3;

   pc_trflags = tracewndflags();
   nextpc = (cpu.pc + disasm_line(cpu.pc, line)) & 0xFFFF;
   unsigned pc = trace_top; asmii = -1;
   unsigned char atr0 = (activedbg == WNDTRACE) ? W_SEL : W_NORM;
   for (unsigned ii = 0; ii < trace_size; ii++) {
      pc &= 0xFFFF; trpc[ii] = pc;
      int len = disasm_line(pc, line);
      char *ptr = line+strlen(line);
      while (ptr < line+32) *ptr++ = ' '; line[32] = 0;

      unsigned char atr = (pc == cpu.pc)? W_TRACEPOS : atr0;
      if (membits[pc] & MEMBITS_BPX) atr = (atr&~7)|2;
      tprint(trace_x, trace_y+ii, line, atr);

      if (pc == trace_curs) {
         asmii = ii;
         if (activedbg == WNDTRACE)
            for (unsigned q = 0; q < cs[trace_mode][1]; q++)
               txtscr[80*30 + (trace_y+ii)*80 + trace_x + cs[trace_mode][0] + q] = W_CURS;
      }

      if (pc_trflags & TWF_BRANCH) {
         if (pc == cpu.pc) {
            unsigned addr = pc_trflags & 0xFFFF;
            unsigned arr = (addr <= cpu.pc)? 0x18 : 0x19; // up/down arrow
            unsigned char color = (pc == trace_curs && activedbg == WNDTRACE && trace_mode == 2)? W_TRACE_JINFO_CURS_FG : W_TRACE_JINFO_NOCURS_FG;
            if (pc_trflags & TWF_BRADDR) sprintf(line, "%04X%c", addr, arr), tprint_fg(trace_x+32-5, trace_y+ii, line, color);
            else tprint_fg(trace_x+32-1, trace_y+ii, (char*)&arr, color);
         }
         if (pc == (pc_trflags & 0xFFFF)) {
            unsigned arr = 0x11; // left arrow
            tprint_fg(trace_x+32-1, trace_y+ii, (char*)&arr, W_TRACE_JARROW_FOREGR);
         }
      }

      pc += len;
   }
   trpc[ii] = pc;

   unsigned char dbuf[16];
   for (int i = 0; i < 16; i++) dbuf[i] = rmdbg(trace_curs+i);
   int len = disasm(dbuf, trace_curs, 0) - dbuf; strcpy(asmpc, asmbuf);
   for (int i = 0; i < len && i < 5; i++)
      sprintf(dumppc + i*2, "%02X", rmdbg(trace_curs+i));

   tprint(trace_x, trace_y-1, "Z80", W_TITLE);
   frame(trace_x,trace_y,32,trace_size,FRAME);
}

void c_loadlbl_a()
{
   mon_labels.import_alasm();
}
void c_loadlbl_x()
{
   mon_labels.import_xas();
}
      /* ------------------------------------------------------------- */
unsigned save_pos[8] = { -1,-1,-1,-1,-1,-1,-1,-1 };
unsigned save_cur[8] = { -1,-1,-1,-1,-1,-1,-1,-1 };
unsigned stack_pos[32] = { -1 }, stack_cur[32] = { -1 };

void push_pos()
{
   memmove(&stack_pos[1], &stack_pos[0], sizeof stack_pos - sizeof *stack_pos);
   memmove(&stack_cur[1], &stack_cur[0], sizeof stack_cur - sizeof *stack_cur);
   stack_pos[0] = trace_top; stack_cur[0] = trace_curs;
}

unsigned cpu_up(unsigned ip)
{
   unsigned char buf1[0x10];
   unsigned p1 = (ip > sizeof buf1) ? ip - sizeof buf1 : 0;
   for (unsigned i = 0; i < sizeof buf1; i++) buf1[i] = rmdbg(p1+i);
   unsigned char *dispos = buf1, *prev;
   do {
      prev = dispos;
      dispos = disasm(dispos, 0, 0);
   } while ((unsigned)(dispos-buf1+p1) < ip);
   return prev-buf1+p1;
}

void cgoto()
{
   unsigned v = input4(trace_x, trace_y, trace_top);
   if (v != -1) trace_top = trace_curs = v;
}

void csetpc() { cpu.pc = trace_curs; }

void center()
{
   if (!trace_mode) sprintf(str, "%04X", trace_curs);
   else if (trace_mode == 1) strcpy(str, dumppc);
   else strcpy(str, asmpc);

   if (input.lastkey != VK_RETURN) *str = 0, PostThreadMessage(GetCurrentThreadId(), WM_KEYDOWN, input.lastkey, 1);

   for (;;) {
      if (!inputhex(trace_x+cs[trace_mode][0], trace_y + trcurs_y + asmii, cs[trace_mode][1], trace_mode < 2)) break;
      if (!trace_mode) {
         push_pos(); sscanf(str, "%X", &trace_top); trace_curs = trace_top;
         for (unsigned i = 0; i < asmii; i++) trace_top = cpu_up(trace_top);
         break;
      } else if (trace_mode == 1) {
         for (char *p = str+strlen(str)-1; p >= str && *p == ' '; *p-- = 0);
         unsigned char dump[8]; unsigned i;
         for (p = str, i = 0; ishex(*p) && ishex(p[1]); p+=2)
            dump[i++] = hex(p);
         if (*p) continue;
         for (unsigned j = 0; j < i; j++)
            wmdbg(trace_curs+j, dump[j]);
         break;
      } else {
         unsigned sz;
         if (sz = assemble_cmd((unsigned char*)str, trace_curs)) {
            for (unsigned i = 0; i < sz; i++) wmdbg(trace_curs+i, asmresult[i]);
            showtrace();
            void cdown(); cdown();
            break;
         }
      }
   }
}

char dispatch_trace()
{
   if (input.lastkey >= 'A' && input.lastkey < 'Z') { center(); return 1; }
   return 0;
}

void cfindtext()
{
   unsigned char oldmode = editor; editor = ED_MEM;
   unsigned rs = find1dlg(trace_curs);
   editor = oldmode;
   if (rs != -1) trace_top = trace_curs = rs;
}
void cfindcode() {
   unsigned char oldmode = editor; editor = ED_MEM;
   unsigned rs = find2dlg(trace_curs);
   editor = oldmode;
   if (rs != -1) trace_top = trace_curs = rs;
}

void cbpx()
{
   membits[trace_curs] ^= MEMBITS_BPX;
}

void cfindpc() { trace_top = trace_curs = cpu.pc; }

void cup()
{
   if (trace_curs > trace_top) {
      for (unsigned i = 1; i < trace_size; i++)
         if (trpc[i] == trace_curs) trace_curs = trpc[i-1];
   } else trace_top = trace_curs = cpu_up(trace_curs);
}

void cdown()
{
   for (unsigned i = 0; i < trace_size; i++)
      if (trpc[i] == trace_curs) {
         trace_curs = trpc[i+1];
         if (i+1 == trace_size) trace_top = trpc[1];
         break;
      }
}
void cleft()  { trace_mode--; }
void cright() { trace_mode++; }
void chere() { dbgbreak = 0, dbgchk = 1; dbg_stophere = trace_curs; }
void cpgdn() {
   unsigned curs = 0;
   for (unsigned i = 0; i < trace_size; i++)
      if (trace_curs == trpc[i]) curs = i;
   trace_top = trpc[trace_size];
   showtrace(); trace_curs = trpc[curs];
}
void cpgup() {
   unsigned curs = 0;
   for (unsigned i = 0; i < trace_size; i++)
      if (trace_curs == trpc[i]) curs = i;
   for (i = 0; i < trace_size; i++) trace_top = cpu_up(trace_top);
   showtrace(); trace_curs = trpc[curs];
}
void pop_pos() {
   if (stack_pos[0] == -1) return;
   trace_curs = stack_cur[0], trace_top = stack_pos[0];
   memcpy(&stack_pos[0], &stack_pos[1], sizeof stack_pos - sizeof *stack_pos);
   memcpy(&stack_cur[0], &stack_cur[1], sizeof stack_cur - sizeof *stack_cur);
   stack_pos[(sizeof stack_pos / sizeof *stack_pos)-1] = -1;
}
void cjump() {
   char *ptr = 0;
   for (char *p = asmpc; *p; p++)
      if (ishex(p[0]) & ishex(p[1]) & ishex(p[2]) & ishex(p[3])) ptr = p;
   if (!ptr) return;
   push_pos();
   unsigned addr; sscanf(ptr, "%04X", &addr);
   trace_curs = trace_top = addr;
}
void cdjump() {
   char *ptr = 0;
   for (char *p = asmpc; *p; p++)
      if (ishex(p[0]) & ishex(p[1]) & ishex(p[2]) & ishex(p[3])) ptr = p;
   if (!ptr) return;
   unsigned addr; sscanf(ptr, "%04X", &addr);
   mem_curs = addr; activedbg = WNDMEM; editor = ED_MEM;
}
void cfliplabels() {
   trace_labels = !trace_labels; showtrace();
}
void csave(unsigned n) {
   save_pos[n] = trace_top;
   save_cur[n] = trace_curs;
}
void crest(unsigned n) {
   if (save_pos[n] == -1) return;
   push_pos();
   trace_top = save_pos[n], trace_curs = save_cur[n];
}
void csave1() { csave(0); }
void csave2() { csave(1); }
void csave3() { csave(2); }
void csave4() { csave(3); }
void csave5() { csave(4); }
void csave6() { csave(5); }
void csave7() { csave(6); }
void csave8() { csave(7); }
void crest1() { crest(0); }
void crest2() { crest(1); }
void crest3() { crest(2); }
void crest4() { crest(3); }
void crest5() { crest(4); }
void crest6() { crest(5); }
void crest7() { crest(6); }
void crest8() { crest(7); }

void mon_step()
{
   debug_last_t = comp.t_states + cpu.t;
   prevcpu = cpu; cpu.halted = 0; z80dbg::step();
   trace_curs = cpu.pc;
}

void mon_stepover()
{
   unsigned char trace = 0;

   // call,rst
   if (pc_trflags & TWF_CALLCMD) dbg_stopsp = cpu.sp & 0xFFFF, dbg_stophere = nextpc;

   // jr cc,$-xx, jp cc,$-xx
   else if ((pc_trflags & TWF_LOOPCMD) && (pc_trflags & 0xFFFF) < (cpu.pc & 0xFFFF))
      dbg_stopsp = cpu.sp & 0xFFFF, dbg_stophere = nextpc,
      dbg_loop_r1 = pc_trflags & 0xFFFF, dbg_loop_r2 = cpu.pc & 0xFFFF;

   else if (pc_trflags & TWF_BRANCH) trace = 1;

   else dbg_stophere = nextpc;

   if (trace) mon_step();
   else dbgbreak = 0, dbgchk = 1;
}
