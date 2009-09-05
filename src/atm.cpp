
void set_atm_FF77(unsigned port, unsigned char val)
{
   static unsigned char old_FF77 = 0;
   if (((old_FF77 ^ val) & 1) && conf.atm.mem_swap) { // swap memory address bits A5-A7 and A8-A10
      for (unsigned start_page = 0; start_page < conf.ramsize; start_page += 2048) {
         unsigned char buffer[2048], *bank = memory + start_page;
         for (unsigned addr = 0; addr < 2048; addr++)
            buffer[addr] = bank[(addr & 0x1F) + ((addr >> 3) & 0xE0) + ((addr << 3) & 0x700)];
         memcpy(bank, buffer, 2048);
      }
   }
   old_FF77 = comp.pFF77 = val;
   comp.aFF77 = port;
   set_banks();
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

