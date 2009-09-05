
// input: ports 7FFD,1FFD,DFFD,FFF7,FF77, flags CF_TRDOS,CF_CACHEON
void set_banks()
{
   bankw[1] = bankr[1] = RAM_BASE_M + 5*PAGE;
   bankw[2] = bankr[2] = RAM_BASE_M + 2*PAGE;
   temp.base = memory + ((comp.p7FFD & 8) ? 7*PAGE : 5*PAGE);
   if (temp.base_2) temp.base_2 = temp.base;

   // these flags will be re-calculated
   comp.flags &= ~(CF_DOSPORTS | CF_LEAVEDOSRAM | CF_LEAVEDOSADR | CF_SETDOSROM);

   unsigned char *bank0, *bank3;

   if (comp.flags & CF_TRDOS) bank0 = (comp.p7FFD & 0x10)? base_dos_rom : base_sys_rom;
   else bank0 = (comp.p7FFD & 0x10)? base_sos_rom : base_128_rom;

   unsigned bank = (comp.p7FFD & 7);

   switch (conf.mem_model)
   {
      case MM_PENTAGON:
         bank += ((comp.p7FFD & 0xC0) >> 3) + (comp.p7FFD & 0x20);
         bank3 = RAM_BASE_M + (bank & temp.ram_mask)*PAGE;
         break;

      case MM_PROFSCORP:
         membits[0x0104] &= ~MEMBITS_R;
         membits[0x0108] &= ~MEMBITS_R;
         membits[0x010C] &= ~MEMBITS_R;
      case MM_SCORP:
         bank += ((comp.p1FFD & 0x10) >> 1) + ((comp.p1FFD & 0xC0) >> 2);
         bank3 = RAM_BASE_M + (bank & temp.ram_mask)*PAGE;
         if (comp.p1FFD & 2) bank0 = base_sys_rom;
         if (comp.p1FFD & 1) bank0 = RAM_BASE_M + 0*PAGE;
         if (conf.mem_model == MM_PROFSCORP) { if (bank0==base_sys_rom) comp.flags |= CF_PROFROM; else comp.flags &= ~CF_PROFROM; }
         break;

      case MM_KAY:
      {
         bank += ((comp.p1FFD & 0x10) >> 1) + ((comp.p1FFD & 0x80) >> 3) + ((comp.p7FFD & 0x80) >> 2);
         bank3 = RAM_BASE_M + (bank & temp.ram_mask)*PAGE;
         unsigned char rom1 = (comp.p1FFD >> 2) & 2;
         if (comp.flags & CF_TRDOS) rom1 ^= 2;
         switch (rom1+((comp.p7FFD & 0x10) >> 4)) {
            case 0: bank0 = base_128_rom; break;
            case 1: bank0 = base_sos_rom; break;
            case 2: bank0 = base_sys_rom; break;
            case 3: bank0 = base_dos_rom; break;
            default: __assume(0);
         }
         if (comp.p1FFD & 1) bank0 = RAM_BASE_M + 0*PAGE;
         break;
      }

      case MM_PROFI:
         bank += ((comp.pDFFD & 0x07) << 3); bank3 = RAM_BASE_M + (bank & temp.ram_mask)*PAGE;
         if (comp.pDFFD & 0x08) bankr[1] = bankw[1] = bank3, bank3 = RAM_BASE_M+7*PAGE;
         if (comp.pDFFD & 0x10) bank0 = RAM_BASE_M+0*PAGE;
         if (comp.pDFFD & 0x20) comp.flags |= CF_DOSPORTS;
         if (comp.pDFFD & 0x40) bankr[2] = bankw[2] = RAM_BASE_M + 6*PAGE;
         break;

      case MM_ATM450:
      {
         // RAM
         bank += ((comp.pFDFD & 0x07) << 3); // original ATM uses D2 as ROM address extension, not RAM
         bank3 = RAM_BASE_M + (bank & temp.ram_mask)*PAGE;
         if (!(comp.aFE & 0x80)) {
            bankw[1] = bankr[1] = RAM_BASE_M + 4*PAGE;
            bank0 = RAM_BASE_M;
            break;
         }

         // ROM
         if (comp.p7FFD & 0x20) comp.aFB &= ~0x80;
         if ((comp.flags & CF_TRDOS) && (comp.pFDFD & 8)) comp.aFB |= 0x80; // more priority, then 7FFD
         if (comp.aFB & 0x80) { bank0 = base_sys_rom; break; } // CPSYS signal
         // system rom not used on 7FFD.4=0 and DOS=1
         if (comp.flags & CF_TRDOS) bank0 = base_dos_rom;
         break;
      }

      case MM_ATM710:
      {
         if (!(comp.aFF77 & 0x200)) // ~cpm=0
            comp.flags |= CF_TRDOS;
         if (!(comp.aFF77 & 0x100)) { // pen=0
            bankr[1] = bankr[2] = bank3 = bank0 = ROM_BASE_M + PAGE * temp.rom_mask;
            break;
         }
         unsigned i = ((comp.p7FFD & 0x10) >> 2);
         for (unsigned bank = 0; bank < 4; bank++) {
            switch (comp.pFFF7[i+bank] & 0xC0) {
               case 0x00: // RAM from 7FFD
                  bankr[bank] = bankw[bank] = RAM_BASE_M + PAGE*( (comp.p7FFD & 7) + (comp.pFFF7[i+bank] & 0x38 & temp.ram_mask) );
                  break;
               case 0x40: // ROM from 7FFD
                  bankr[bank] = ROM_BASE_M + PAGE*((comp.pFFF7[i+bank] & temp.rom_mask & 0xFE) + ((comp.flags & CF_TRDOS)?1:0));
                  break;
               case 0x80: // RAM from FFF7
                  bankr[bank] = bankw[bank] = RAM_BASE_M + PAGE*(comp.pFFF7[i+bank] & temp.ram_mask);
                  break;
               case 0xC0: // ROM from FFF7
                  bankr[bank] = ROM_BASE_M + PAGE*(comp.pFFF7[i+bank] & temp.rom_mask);
                  break;
            }
         }
         bank0 = bankr[0]; bank3 = bankr[3];
         break;
      }
      default: bank3 = RAM_BASE_M + 0*PAGE;
   }

   bankw[0] = bankr[0] = bank0;
   bankw[3] = bankr[3] = bank3;

   if (bankr[0] >= ROM_BASE_M) bankw[0] = TRASH_M;
   if (bankr[1] >= ROM_BASE_M) bankw[1] = TRASH_M;
   if (bankr[2] >= ROM_BASE_M) bankw[2] = TRASH_M;
   if (bankr[3] >= ROM_BASE_M) bankw[3] = TRASH_M;


   unsigned char dosflags = CF_LEAVEDOSRAM;
   if (conf.mem_model == MM_PENTAGON || conf.mem_model == MM_PROFI) dosflags = CF_LEAVEDOSADR;

   if (comp.flags & CF_TRDOS) comp.flags |= dosflags | CF_DOSPORTS;
   else if ((comp.p7FFD & 0x10) && conf.trdos_present) { // B-48, inactive DOS, DOS present
      // for Scorp, ATM-1/2 and KAY, TR-DOS not started on executing RAM 3Dxx
      if (!((dosflags & CF_LEAVEDOSRAM) && bankr[0] < RAM_BASE_M+PAGE*MAX_RAM_PAGES))
         comp.flags |= CF_SETDOSROM;
   }

   if (comp.flags & CF_CACHEON) {
      unsigned char *cpage = CACHE_M;
      if (conf.cache == 32 && !(comp.p7FFD & 0x10)) cpage += PAGE;
      bankr[0] = bankw[0] = cpage;
      if (comp.pEFF7 & EFF7_ROCACHE) bankw[0] = TRASH_M;
   }

   if (comp.pEFF7 & EFF7_LOCKMEM)
      for (unsigned i = 0; i < 4; i++)
         if ((unsigned)(bankr[i] - (RAM_BASE_M+8*PAGE)) < PAGE*(MAX_RAM_PAGES-8)) { // hi-ram bank
            unsigned char *newram = ((bankr[i] - RAM_BASE_M) & (8*PAGE-1)) + RAM_BASE_M;
            if (bankr[i] == bankw[i]) bankw[i] = newram;
            bankr[i] = newram;
         }

   if (temp.led.osw && (trace_rom | trace_ram)) {
      for (unsigned i = 0; i < 4; i++) {
         unsigned bank = (bankr[i] - RAM_BASE_M) / PAGE;
         if (bank < MAX_PAGES) used_banks[bank] = 1;
      }
   }
}

void set_scorp_profrom(unsigned read_address)
{
   static unsigned char switch_table[] =
   {
      0,1,2,3,
      3,3,3,2,
      2,2,0,1,
      1,0,1,0
   };
   comp.profrom_bank = switch_table[read_address*4 + comp.profrom_bank] & temp.profrom_mask;
   unsigned char *base = ROM_BASE_M + (comp.profrom_bank * 64*1024);
   base_128_rom = base + 0*PAGE;
   base_sos_rom = base + 1*PAGE;
   base_sys_rom = base + 2*PAGE;
   base_dos_rom = base + 3*PAGE;
   set_banks();
}

Z80INLINE unsigned char *am_r(unsigned addr)
{
#ifdef MOD_VID_VD
   if (comp.vdbase && (unsigned)((addr & 0xFFFF) - 0x4000) < 0x1800) return comp.vdbase + (addr & 0x1FFF);
#endif
   return bankr[(addr >> 14) & 3] + (addr & (PAGE-1));
}

void wmdbg(unsigned addr, unsigned char val)
{
   *am_r(addr) = val;
}

unsigned char rmdbg(unsigned addr)
{
   return *am_r(addr);
}

void set_mode(ROM_MODE mode)
{
   if (mode == RM_NOCHANGE) return;
   if (mode == RM_CACHE) { comp.flags |= CF_CACHEON; set_banks(); return; }
   // no RAM/cache/SERVICE
   comp.p1FFD &= ~7;
   comp.pDFFD &= ~0x10;
   comp.flags &= ~CF_CACHEON;
   // comp.aFF77 |= 0x100; // enable ATM memory
   switch (mode) {
      case RM_128:
         comp.flags &= ~CF_TRDOS;
         comp.p7FFD &= ~0x10;
         break;
      case RM_SOS:
         comp.flags &= ~CF_TRDOS;
         comp.p7FFD |= 0x10;
         break;
      case RM_SYS:
         comp.flags |= CF_TRDOS;
         comp.p7FFD &= ~0x10;
         break;
      case RM_DOS:
         comp.flags |= CF_TRDOS;
         comp.p7FFD |=  0x10;
         break;
   }
   set_banks();
}

unsigned char cmosBCD(unsigned char binary)
{
   if (!(cmos[11] & 4)) binary = (binary % 10) + 0x10*((binary/10)%10);
   return binary;
}

unsigned char cmos_read()
{
   unsigned char reg = comp.cmos_addr; SYSTEMTIME st;
   if (conf.cmos == 2) reg &= 0x3F;

   if ((1 << reg) & ((1<<0)|(1<<2)|(1<<4)|(1<<6)|(1<<7)|(1<<8)|(1<<9)))
      GetLocalTime(&st);
   switch (reg) {
      case 0:     return cmosBCD((BYTE)st.wSecond);
      case 2:     return cmosBCD((BYTE)st.wMinute);
      case 4:     return cmosBCD((BYTE)st.wHour);
      case 6:     return 1+(((BYTE)st.wDayOfWeek+8-conf.cmos) % 7);
      case 7:     return cmosBCD((BYTE)st.wDay);
      case 8:     return cmosBCD((BYTE)st.wMonth);
      case 9:     return cmosBCD(st.wYear % 100);
      case 10:    return 0x20;
      case 11:    return (cmos[11] & 4) | 2;
      case 12:    return 0;
      case 13:    return 0x80;
   }
   return cmos[reg];
}

__inline void cmos_write(unsigned char val)
{
   if (conf.cmos == 2) comp.cmos_addr &= 0x3F;
   cmos[comp.cmos_addr] = val;
}

void NVRAM::write(unsigned char val)
{
   const SCL = 0x40, SDA = 0x10, WP = 0x20,
         SDA_1 = 0xFF, SDA_0 = 0xBF,
         SDA_SHIFT_IN = 4;

   if ((val ^ prev) & SCL) // clock edge, data in/out
   {
      if (val & SCL) // nvram reads SDA
      {
         if (state == RD_ACK)
         {
            if (val & SDA) goto idle; // no ACK, stop
            // move next byte to host
            state = SEND_DATA;
            dataout = nvram[address];
            address = (address+1) & 0x7FF;
            bitsout = 0; goto exit; // out_z==1;
         }

         if ((1<<state) & ((1<<RCV_ADDR)|(1<<RCV_CMD)|(1<<RCV_DATA))) {
            if (out_z) // skip nvram ACK before reading
               datain = 2*datain + ((val >> SDA_SHIFT_IN) & 1), bitsin++;
         }

      } else { // nvram sets SDA

         if (bitsin == 8) // byte received
         {
            bitsin = 0;
            if (state == RCV_CMD) {
               if ((datain & 0xF0) != 0xA0) goto idle;
               address = (address & 0xFF) + ((datain << 7) & 0x700);
               if (datain & 1) { // read from current address
                  dataout = nvram[address];
                  address = (address+1) & 0x7FF;
                  bitsout = 0;
                  state = SEND_DATA;
               } else
                  state = RCV_ADDR;
            } else if (state == RCV_ADDR) {
               address = (address & 0x700) + datain;
               state = RCV_DATA; bitsin = 0;
            } else if (state == RCV_DATA) {
               nvram[address] = datain;
               address = (address & 0x7F0) + ((address+1) & 0x0F);
               // state unchanged
            }

            // EEPROM always acknowledges
            out = SDA_0; out_z = 0; goto exit;
         }

         if (state == SEND_DATA) {
            if (bitsout == 8) { state = RD_ACK; out_z = 1; goto exit; }
            out = (dataout & 0x80)? SDA_1 : SDA_0; dataout *= 2;
            bitsout++; out_z = 0; goto exit;
         }

         out_z = 1; // no ACK, reading
      }
      goto exit;
   }

   if ((val & SCL) && ((val ^ prev) & SDA)) // start/stop
   {
      if (val & SDA) { idle: state = IDLE; } // stop
      else state = RCV_CMD, bitsin = 0; // start
      out_z = 1;
   }

   // else SDA changed on low SCL


 exit:
   if (out_z) out = (val & SDA)? SDA_1 : SDA_0;
   prev = val;
}
