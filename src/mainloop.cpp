
void spectrum_frame()
{
   if (!temp.inputblock || input.keymode != K_INPUT::KM_DEFAULT)
      input.make_matrix();

   init_snd_frame();
   init_frame();

   (dbgchk) ? z80dbg::z80loop() : z80fast::z80loop();
   if (modem.open_port) modem.io();

   flush_snd_frame();
   flush_frame();
   showleds();

   if (!cpu.iff1 || // int disabled in CPU
        cpu.t == cpu.eipos || // interrupt not after EI
        (conf.mem_model == MM_ATM710 && !(comp.pFF77 & 0x20))) // int disabled by ATM hardware
   {
      unsigned char *mp = am_r(cpu.pc);
      if (cpu.halted) strcpy(statusline, "CPU HALTED"), statcnt = 10;
      if (*(unsigned short*)mp == WORD2(0x18,0xFE) ||
          ((*mp == 0xC3) && *(unsigned short*)(mp+1) == (unsigned short)cpu.pc))
         strcpy(statusline, "CPU STOPPED"), statcnt = 10;
   } else {
      unsigned intad;
      if (cpu.im < 2) {
         intad = 0x38;
      } else { // im2
         unsigned lo = ((comp.flags & CF_DOSPORTS)? conf.floatdos : conf.floatbus)? ((BYTE)rdtsc()) : 0xFF;
         unsigned vec =lo+cpu.i*0x100;
         intad = z80dbg::rm(vec)+0x100*z80dbg::rm(vec+1);
      }

      cpu.t += (cpu.im < 2) ? 13 : 19;
      z80dbg::wm(--cpu.sp, cpu.pch);
      z80dbg::wm(--cpu.sp, cpu.pcl);
      cpu.pc = intad;
      cpu.halted = 0;
      cpu.iff1 = cpu.iff2 = 0;
   }

   comp.t_states += conf.frame;
   cpu.t -= conf.frame;
   comp.frame_counter++;
}

void mainloop()
{
   unsigned char skipped = 0;
   for (;;)
   {
      if (skipped < temp.frameskip) skipped++, temp.vidblock = 1;
      else skipped = temp.vidblock = 0;
      temp.sndblock = !conf.sound.enabled;
      temp.inputblock = temp.vidblock;
      spectrum_frame();

      // message handling before flip (they paint to rbuf)
      if (!temp.inputblock) dispatch(conf.atm.xt_kbd? ac_main_xt : ac_main);
      if (needrestart) break;
      if (!temp.vidblock) flip();
      if (!temp.sndblock) do_sound();
   }
}
