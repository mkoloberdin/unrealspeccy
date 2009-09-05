
Z80INLINE unsigned char rm(unsigned addr)
{
#ifdef Z80_DBG
   unsigned char *membit = membits + (addr & 0xFFFF);
   *membit |= MEMBITS_R; dbgbreak |= (*membit & MEMBITS_BPR);
#endif
   return *am_r(addr);
}

Z80INLINE void wm(unsigned addr, unsigned char val)
{
#ifdef Z80_DBG
   unsigned char *membit = membits + (addr & 0xFFFF);
   *membit |= MEMBITS_W; dbgbreak |= (*membit & MEMBITS_BPW);
#endif

#ifdef MOD_VID_VD
   if (comp.vdbase && (unsigned)((addr & 0xFFFF) - 0x4000) < 0x1800) { comp.vdbase[addr & 0x1FFF] = val; return; }
#endif

   unsigned char *a = bankw[(addr >> 14) & 3];
#ifndef TRASH_PAGE
   if (!a) return;
#endif
   a += (addr & (PAGE-1));
   if ((unsigned)(a - temp.base_2) < 0x1B00) {
      if (*a == val) return;
      update_screen();
   }
   *a = val;
}

Z80INLINE unsigned char m1_cycle(Z80 *cpu)
{
   cpu->r_low++;// = (cpu->r & 0x80) + ((cpu->r+1) & 0x7F);
   cpu->t += 4;
   return rm(cpu->pc++);
}

#include "z80/cmd.cpp"

__inline void step()
{
   if (comp.flags & CF_SETDOSROM) {
      if (cpu.pch == 0x3D) comp.flags |= CF_TRDOS, set_banks();
   } else if (comp.flags & CF_LEAVEDOSADR) {
      if (cpu.pch & 0xC0) // PC > 3FFF closes TR-DOS
         close_dos: comp.flags &= ~CF_TRDOS, set_banks();
      if (conf.trdos_traps) comp.wd.trdos_traps();
   } else if (comp.flags & CF_LEAVEDOSRAM) {
      // executing RAM closes TR-DOS
      if (bankr[(cpu.pc >> 14) & 3] < RAM_BASE_M+PAGE*MAX_RAM_PAGES) goto close_dos;
      if (conf.trdos_traps) comp.wd.trdos_traps();
   }
   //if ((cpu.pc & 0xFFFF) == 0x056B) tape_traps();
   if (comp.tape.play_pointer && !conf.sound.enabled) fast_tape();

//todo if(comp.turbo)cpu.t-=tbias[cpu.dt]
   if (cpu.pch & temp.evenM1_C0) cpu.t += (cpu.t & 1);
//~todo
   unsigned oldt=cpu.t; //0.37
   unsigned char opcode = m1_cycle(&cpu);
   (normal_opcode[opcode])(&cpu);
//todo if(comp.turbo)cpu.t-=tbias[cpu.t-oldt]
   if( ((conf.mem_model == MM_PENTAGON)&&((comp.pEFF7 & EFF7_GIGASCREEN)==0))
	 ||((conf.mem_model == MM_ATM710)&&(comp.pFF77 & 8))
	 ) cpu.t -= (cpu.t-oldt)>>1; //0.37
//~todo   
#ifdef Z80_DBG
   if ((comp.flags & CF_PROFROM) && ((membits[0x104] | membits[0x108] | membits[0x10C]) & MEMBITS_R)) {
      if (membits[0x104] & MEMBITS_R) set_scorp_profrom(1);
      if (membits[0x108] & MEMBITS_R) set_scorp_profrom(2);
      if (membits[0x10C] & MEMBITS_R) set_scorp_profrom(3);
   }
#endif
}

__inline void z80loop()
{
   cpu.haltpos = 0;

   // INT check separated from main Z80 loop to improve emulation speed
   while (cpu.t < conf.intlen) {
      if (cpu.iff1 && cpu.t != cpu.eipos && // int enabled in CPU not issued after EI
           !(conf.mem_model == MM_ATM710 && !(comp.pFF77 & 0x20))) // int enabled by ATM hardware
         handle_int(&cpu, (comp.flags & CF_Z80FBUS)? (BYTE)rdtsc() : 0xFF);
#ifdef Z80_DBG
      debug_events();
#endif
      step();
      if (cpu.halted) break;
   }

   cpu.eipos = -1;

   while (cpu.t < conf.frame) {
#ifdef Z80_DBG
      debug_events();
#endif
      if (cpu.halted) {
         //cpu.t += 4, cpu.r = (cpu.r & 0x80) + ((cpu.r+1) & 0x7F); continue;
         unsigned st = (conf.frame-cpu.t-1)/4+1;
         cpu.t += 4*st;
         cpu.r_low += st;
         break;
      }
      step();
   }
}
