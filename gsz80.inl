Z80INLINE unsigned char rm(unsigned addr)
{
#ifdef Z80_DBG
   unsigned char *membit = membits + (addr & 0xFFFF);
   *membit |= MEMBITS_R;
   dbgbreak |= (*membit & MEMBITS_BPR);
   gscpu.dbgbreak |= (*membit & MEMBITS_BPR);
#endif

   unsigned char byte = *am_r(addr);
   if ((addr & 0xE000) == 0x6000)
       gs_byte_to_dac(addr, byte);
   return byte;
}

Z80INLINE void wm(unsigned addr, unsigned char val)
{
#ifdef Z80_DBG
   unsigned char *membit = membits + (addr & 0xFFFF);
   *membit |= MEMBITS_W;
   dbgbreak |= (*membit & MEMBITS_BPW);
   gscpu.dbgbreak |= (*membit & MEMBITS_BPW);
#endif

   u8 *bank = gsbankw[(addr >> 14) & 3];
#ifndef TRASH_PAGE
   if (!bank)
       return;
#endif
   bank[addr & (PAGE-1)] = val;
}

void z80loop()
{
   unsigned __int64 end = (float(cpu.t) * float(GSCPUFQI)) / float(conf.frame) + 0.5f;//((cpu.t * mult_gs2) >> MULT_GS_SHIFT); //; // t*GSCPUFQI/conf.frame;
   end += gscpu_t_at_frame_start;
   for (;;)
   { // while (gs_t_states+gscpu.t < end)
//      assert(end >= gs_t_states);
//      int max_gscpu_t = min(GSCPUINT, (int)(end - gs_t_states));
/*
      if(gs_t_states + gscpu.t >= end)
      {
          printf("%s->SyncLost %llu, %u, %llu\n", __FUNCTION__, gs_t_states, gscpu.t, end);
      }
      else
      {
          printf("%s->SyncOk %llu, %u, %llu\n", __FUNCTION__, gs_t_states, gscpu.t, end);
      }
*/
      while ((gs_t_states + gscpu.t < end) && (gscpu.t < GSCPUINT)) // (int)gscpu.t < max_gscpu_t
      {
#ifdef Z80_DBG
         debug_events(&gscpu);
#endif
         if (gscpu.halted)
         {
            unsigned st = (GSCPUINT - gscpu.t - 1) / 4 + 1;
            gscpu.t += 4 * st;
            gscpu.r_low += st;
            break;
         }
         stepi();
      }
      if (gscpu.t < GSCPUINT)
          break;

      if (gscpu.iff1 && gscpu.t != gscpu.eipos) // interrupt, but not after EI
         handle_int(&gscpu, 0xFF);

      gscpu.t -= GSCPUINT;
      gs_t_states += GSCPUINT;
   }
}
