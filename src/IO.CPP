
#define in_trdos in_wd93
#define out_trdos out_wd93

void out(unsigned port, unsigned char val)
{
   port &= 0xFFFF;
   brk_port_out = port; brk_port_val = val;
   if (comp.flags & CF_DOSPORTS) {
      if (conf.ide_scheme == IDE_ATM && (port & 0x1F) == 0x0F) {
         if (port & 0x100) { comp.ide_write = val; return; }
      write_hdd_5:
         port >>= 5;
      write_hdd:
         port &= 7;
         if (port) hdd.write(port, val);
         else hdd.write_data(val + (comp.ide_write << 8));
         return;
      }
      if ((port & 0x18A3) == (0xFFFE & 0x18A3)) { // SMUC
         if (conf.smuc) {
            if ((port & 0xA044) == (0xDFBA & 0xA044)) { // clock
               if (comp.pFFBA & 0x80) cmos_write(val);
               else comp.cmos_addr = val;
               return;
            }
            if ((port & 0xA044) == (0xFFBA & 0xA044)) { // SMUC system port
               if ((val & 1) && (conf.ide_scheme == IDE_SMUC)) hdd.reset();
               comp.nvram.write(val);
               comp.pFFBA = val;
               return;
            }
            if ((port & 0xA044) == (0x7FBA & 0xA044)) { comp.p7FBA = val; return; }
         }
         if ((port & 0x8044) == (0xFFBE & 0x8044) && conf.ide_scheme == IDE_SMUC) {
            if (!(port & 0x2000)) { comp.ide_write = val; return; }
            port >>= 8; goto write_hdd;
         }
      }
      unsigned char p1 = (unsigned char)port;
      if (conf.mem_model == MM_ATM710) {
         if ((p1 & 0x9F) == 0x17) { set_atm_FF77(port, val); return; }
         if ((p1 & 0x9F) == 0x97) { comp.pFFF7[((comp.p7FFD & 0x10) >> 2) + ((port >> 14) & 3)] = val ^ 0xFF; set_banks(); return; }
         if ((p1 & 0x9F) == 0x9F && !(comp.aFF77 & 0x4000)) atm_writepal(val); // don't return - write to TR-DOS system port
      }
      if ((p1 & 0x1F) == 0x1F) { out_trdos(p1, val); return; }
      // don't return - out to port #FE works in trdos!
   } else {
      if ((unsigned char)port == 0x1F && conf.sound.ay_scheme == AY::SCHEME_POS) { comp.active_ay = val & 1; return; }
      if (!(port & 6) && (conf.ide_scheme == IDE_NEMO || conf.ide_scheme == IDE_NEMO_A8)) {
         unsigned hi_byte = (conf.ide_scheme == IDE_NEMO)? (port & 1) : (port & 0x100);
         if (hi_byte) { comp.ide_write = val; return; }
         if ((port & 0x18) == 0x08) { if ((port & 0xE0) == 0xC0) hdd.write(8, val); return; } // CS1=0,CS0=1,reg=6
         if ((port & 0x18) != 0x10) return; // invalid CS0,CS1
         goto write_hdd_5;
      }
   }

   if (!(port & 1)) { // port #FE
      // speaker & mic
      if ((comp.pFE ^ val) & 0x18) flush_dig_snd();
      spkr_dig = (val & 0x10) ? conf.sound.beeper : 0;
      mic_dig = (val & 0x08) ? conf.sound.micout : 0;

      unsigned char new_border = (val & 7); ; if (conf.mem_model == MM_ATM710) new_border += ((port & 8) ^ 8);
      if (comp.border_attr ^ new_border) update_screen();
      comp.border_attr = new_border;

      comp.pFE = val;
      // do not return! intro to navy seals (by rst7) uses out #FC for to both FE and 7FFD
   }
   if (!(port & 2)) {

      if ((unsigned char)port == 0xDD) { // port DD - covox
         flush_dig_snd();
         covDD_vol = val*conf.sound.covoxDD/0x100;
         return;
      }

      if (!(port & 0x8000)) { // zx128 port

         if (port == 0x1FFD && (conf.mem_model == MM_SCORP || conf.mem_model == MM_PROFSCORP)) {
            comp.p1FFD = val;
            set_banks(); return;
         }
         // if (conf.mem_model == MM_ATM710 && (port & 0x8202) != (0x7FFD & 0x8202)) return; // strict 7FFD decoding on ATM-2

         // 7FFD
         if (conf.mem_model == MM_MYPENT && (val & 0x80) && (comp.flags & CF_SYSTEMROM)) comp.flags &= ~CF_SYSTEMROM, set_banks();
         if ((comp.p7FFD & 0x20) && (conf.ramsize != 1024 || (conf.mem_model != MM_PENTAGON && conf.mem_model != MM_MYPENT))) return; //locked
         if ((comp.p7FFD ^ val) & 0x08) update_screen();
         comp.p7FFD = val; set_banks(); return;
      }
      if (port == 0xDFFD && conf.mem_model == MM_PROFI) {
         comp.pDFFD = val; set_banks(); return;
      }
      if (port == 0xFDFD && conf.mem_model == MM_ATM450) {
         comp.pFDFD = val; set_banks(); return;
      }
      if ((port & 0xC0FF) == 0xC0FD) { // A15=A14=1, FxFD - AY select register
         if (conf.sound.ay_scheme == AY::SCHEME_CHRV && ((val & 0xFE) == 0xFE)) comp.active_ay = val & 1;
         unsigned ay = (conf.sound.ay_scheme == AY::SCHEME_QUADRO)? ay = (port >> 12) & 1 : comp.active_ay;
         comp.ay[ay].select(val);
         return;
      }
      if ((port & 0xC000)==0x8000 && conf.sound.ay_scheme) {  // BFFD - AY data register
         unsigned ay = (conf.sound.ay_scheme == AY::SCHEME_QUADRO)? ay = (port >> 12) & 1 : comp.active_ay;
         comp.ay[ay].write(val); // port=0 used in ZX-Time#6 for border effects
         if (comp.ay[ay].activereg == 14 && conf.input.mouse == 2) input.aymouse_wr(val);
         return;
      }
      return;
   }

   #ifdef MOD_GS
   if ((port & 0xF7) == 0xB3 && conf.gs_type) { out_gs(port, val); return; }
   #endif

   if ((port & 0xAF) == 0x0F) { // soundrive
      if ((unsigned char)port == 0x0F) comp.p0F = val;
      if ((unsigned char)port == 0x1F) comp.p1F = val;
      if ((unsigned char)port == 0x4F) comp.p4F = val;
      if ((unsigned char)port == 0x5F) comp.p5F = val;
      flush_dig_snd();
      sd_l = (conf.sound.sd * (comp.p0F+comp.p1F)) >> 8;
      sd_r = (conf.sound.sd * (comp.p4F+comp.p5F)) >> 8;
      return;
   }
   if (!(port & 4)) { // port FB - covox
      flush_dig_snd();
      covFB_vol = val*conf.sound.covoxFB/0x100;
      return;
   }
   if (port == 0xEFF7) { comp.pEFF7 = (comp.pEFF7 & conf.EFF7_mask) | (val & ~conf.EFF7_mask); return; }
   if (conf.cmos && (comp.pEFF7 & EFF7_CMOS)) {
      if (port == 0xDFF7) { comp.cmos_addr = val; return; }
      if (port == 0xBFF7) { cmos_write(val); return; }
   }
   if ((port & 0xF8FF) == 0xF8EF && modem.open_port) modem.write((port >> 8) & 7, val);
}

__inline unsigned char in1(unsigned port)
{
   port &= 0xFFFF;
   brk_port_in = port;
   if (conf.cache) {
      if ((unsigned char)port == 0xFB) { // open cache
         comp.flags |= CF_CACHEON; set_banks(); return 0xFF;
      }
      if ((unsigned char)port == 0x7B) { // close cache
         comp.flags &= ~CF_CACHEON; set_banks(); return 0xFF;
      }
   }

   if (comp.flags & CF_DOSPORTS) {
      if (conf.ide_scheme == IDE_ATM && (port & 0x1F) == 0x0F) {
         if (port & 0x100) return comp.ide_read;
      read_hdd_5:
         port >>= 5;
      read_hdd:
         port &= 7;
         if (port) return hdd.read(port);
         unsigned v = hdd.read_data();
         comp.ide_read = (unsigned char)(v >> 8);
         return (unsigned char)v;
      }
      if ((port & 0x18A3) == (0xFFFE & 0x18A3)) { // SMUC
         if (conf.smuc) {
            if ((port & 0xA044) == (0xDFBA & 0xA044)) return cmos_read(); // clock
            if ((port & 0xA044) == (0xFFBA & 0xA044)) return comp.nvram.out; // SMUC system port
            if ((port & 0xA044) == (0x7FBA & 0xA044)) return comp.p7FBA | 0x3F;
         }
         if ((port & 0x8044) == (0xFFBE & 0x8044) && conf.ide_scheme == IDE_SMUC) {
            if (!(port & 0x2000)) return comp.ide_read;
            port >>= 8; goto read_hdd;
         }
      }

      unsigned char p1 = (unsigned char)port;
      if ((p1 & 0x1F) == 0x1F) return in_trdos(p1);
   } else {
      if (!(port & 6) && (conf.ide_scheme == IDE_NEMO || conf.ide_scheme == IDE_NEMO_A8)) {
         unsigned hi_byte = (conf.ide_scheme == IDE_NEMO)? (port & 1) : (port & 0x100);
         if (hi_byte) return comp.ide_read;
         comp.ide_read = 0xFF;
         if ((port & 0x18) == 0x08) return ((port & 0xE0) == 0xC0)? hdd.read(8) : 0xFF; // CS1=0,CS0=1,reg=6
         if ((port & 0x18) != 0x10) return 0xFF; // invalid CS0,CS1
         goto read_hdd_5;
      }
   }

   #ifdef MOD_GS
   if ((port & 0xF7) == 0xB3 && conf.gs_type) return in_gs(port);
   #endif

   if (!(port & 0x20)) { // kempstons
      port = (port & 0xFFFF) | 0xFA00; // A13,A15 not used in decoding
      if ((port == 0xFADF || port == 0xFBDF || port == 0xFFDF) && conf.input.mouse == 1) { // mouse
         input.mouse_joy_led |= 1;
         if (port == 0xFBDF) return input.kempston_mx();
         if (port == 0xFFDF) return input.kempston_my();
         return input.mbuttons;
      }
      if (conf.input.kjoy) { input.mouse_joy_led |= 2; return input.kjoy; }
   }
   if (!(port & 1)) { // port #FE
      if ((cpu.pc & 0xFFFF) == 0x0564 && bankr[0][0x0564]==0x1F && conf.tape_autostart && !comp.tape.play_pointer) start_tape();
      return input.read(port >> 8);
   }
   if ((port & 0x8202) == (0x7FFD & 0x8202) && (conf.mem_model == MM_ATM710 || conf.ide_scheme == IDE_ATM)) { // ATM-2 IDE+DAC/ADC
      unsigned char irq = 0x40;
      if (conf.ide_scheme == IDE_ATM) irq = (hdd.read_intrq() & 0x40);
      return irq + 0x3F;
   }
   if ((unsigned char)port == 0xFD && conf.sound.ay_scheme) {
      if ((port & 0xC0FF) != 0xC0FD) return 0xFF;
      unsigned ay = (conf.sound.ay_scheme == AY::SCHEME_QUADRO)? ay = (port >> 12) & 1 : comp.active_ay;
      unsigned reg = comp.ay[ay].activereg;
      if (reg >= 0x10) return 0xFF;
      // else FxFD - read selected AY register
      if (reg == 14 && conf.input.mouse == 2) { input.mouse_joy_led |= 1; return input.aymouse_rd(); }
      return comp.ay[ay].reg[reg];
   }
   if ((unsigned char)port == 0xFF) {
      update_screen();
      if (vmode != 2) return 0xFF; // ray is not in paper
      return temp.base[vcurr->atr_offs + (cpu.t - vcurr[-1].next_t)/4];
   }
   if (port == 0xBFF7 && conf.cmos && (comp.pEFF7 & EFF7_CMOS)) return cmos_read();
   if ((port & 0xF8FF) == 0xF8EF && modem.open_port) return modem.read((port >> 8) & 7);

   return 0xFF;
}

unsigned char in(unsigned port)
{
   brk_port_val = in1(port);
   return brk_port_val;
}

#undef in_trdos
#undef out_trdos
