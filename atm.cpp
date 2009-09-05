
void atm_memswap()
{
   if (!conf.atm.mem_swap) return;
   // swap memory address bits A5-A7 and A8-A10
   for (unsigned start_page = 0; start_page < conf.ramsize*1024; start_page += 2048) {
      unsigned char buffer[2048], *bank = memory + start_page;
      for (unsigned addr = 0; addr < 2048; addr++)
         buffer[addr] = bank[(addr & 0x1F) + ((addr >> 3) & 0xE0) + ((addr << 3) & 0x700)];
      memcpy(bank, buffer, 2048);
   }
}

void set_atm_FF77(unsigned port, unsigned char val)
{
   if ((comp.pFF77 ^ val) & 1) atm_memswap();
   comp.pFF77 = val;
   comp.aFF77 = port;
   set_banks();
}

void set_atm_aFE(unsigned char addr)
{
   unsigned char old_aFE = comp.aFE;
   comp.aFE = addr;
   if ((addr ^ old_aFE) & 0x40) atm_memswap();
   if ((addr ^ old_aFE) & 0x80) set_banks();
}

void reset_atm()
{
   // spectrum colors -> palette indexes (RF_PALB - gg0rr0bb format)
   static unsigned char atm_pal[16] =
      { 0x00, 0x02, 0x10, 0x12, 0x80, 0x82, 0x90, 0x92,
        0x00, 0x03, 0x18, 0x1B, 0xC0, 0xC3, 0xD8, 0xDB };
   memcpy(comp.atm_pal, atm_pal, sizeof comp.atm_pal);
}

void atm_writepal(unsigned char val)
{
   comp.atm_pal[comp.border_attr] = t.atm_pal_map[val];
   temp.atm_pal_changed = 1;
}

unsigned char atm450_z(unsigned t)
{
   // PAL hardware gives 3 zeros in secret short time intervals
   if (conf.frame < 80000) { // NORMAL SPEED mode
      if ((unsigned)(t-7200) < 40 || (unsigned)(t-7284) < 40 || (unsigned)(t-7326) < 40) return 0;
   } else { // TURBO mode
      if ((unsigned)(t-21514) < 40 || (unsigned)(t-21703) < 80 || (unsigned)(t-21808) < 40) return 0;
   }
   return 0x80;
}
