
enum WDSTATE
{
   WD_READY = 0,
   WD_WAIT,
   WD_SEEK0,
   WD_SEEK,
   WD_STEP,
   WD_SEEKDONE,
   WD_STARTREAD,
   WD_READSECTORS,
   WD_READDATA,
   WD_NOTFND,
   WD_WRITESECTORS,
   WD_STARTWRITE,
   WD_STARTWRITE2,
   WD_WRITEDATA,
   WD_FORMATTRACK,
   WD_FORMAT,
   WD_RESET
};

const INTRQ = 0x80,
      DRQ = 0x40;

const WDS_BUSY      = 0x01,
      WDS_INDEX     = 0x02,
      WDS_DRQ       = 0x02,
      WDS_TRK00     = 0x04,
      WDS_LOST      = 0x04,
      WDS_CRCERR    = 0x08,
      WDS_NOTFOUND  = 0x10,
      WDS_RECORDT   = 0x20,
      WDS_HEADL     = 0x20,
      WDS_WRFAULT   = 0x20,
      WDS_WRITEP    = 0x40,
      WDS_NOTRDY    = 0x80;

unsigned char fdd_track[4];
__int64 fdd_motor[4];


// test=0 - any ID, test=1 - correct crc&sec&trk, test=2 - correct side
void find_marker(int test, int needdata)
{
   if (conf.wd93_nodelay) {
      fdd_track[comp.wd.drive]=comp.wd.track;
      seek_track(comp.wd.drive, fdd_track[comp.wd.drive], comp.wd.side);
   }

   // seek_track() - assume it's already done
   comp.wd.state = WD_WAIT;
   unsigned char found = 0; unsigned t = MAX_TRACK_LEN;
   if (trkcache.trkd) {
      unsigned i = (unsigned)(comp.wd.next % (trkcache.trklen*trkcache.ts_byte)) / trkcache.ts_byte;
      for (t = 0; t < trkcache.trklen; t++, i = (i+1 >= trkcache.trklen)? 0 : i+1) {
         if (trkcache.trkd[i] != 0xA1 || trkcache.trkd[i+1] != 0xFE || !(trkcache.trki[i/8] & (1<<(i&7)))) continue;
         comp.wd.ptr = trkcache.trkd+i+2; comp.wd.len = 6; // nearest marker
         if (!test) { found = 1; break; }
         if (trkcache.trkd[i+2] != comp.wd.track || trkcache.trkd[i+4] != comp.wd.sector) continue;
         if (test > 1 && trkcache.trkd[i+3] != (unsigned char)comp.wd.side) continue;
         if (wd93_crc(trkcache.trkd+i+1, 5) != *(unsigned short*)(trkcache.trkd+i+6)) continue;
         if (!needdata) { found = 1; break; }
         for (unsigned j = i+8; j < trkcache.trklen-4; j++)
            if (trkcache.trkd[j] == 0xA1 && (trkcache.trki[j/8] & (1<<(j&7))) && !(trkcache.trki[(j+1)/8] & (1<<((j+1)&7)))) {
               if (trkcache.trkd[j+1] == 0xF8 || trkcache.trkd[j+1] == 0xFB) {
                  comp.wd.ptr = trkcache.trkd+j+2;
                  comp.wd.len = 0x80 << trkcache.trkd[i+5];
                  found = 1; break;
               }
               break;
            }
         break;
      }
      if (found && (comp.wd.ptr - trkcache.trkd + comp.wd.len > trkcache.trklen))
         comp.wd.len = trkcache.trklen-(comp.wd.ptr - trkcache.trkd);
   } else // no disk/track
      comp.wd.next += 20*Z80FQ; // no disk: infinite timeout?

   if (!found) comp.wd.ptr = 0, comp.wd.state2 = WD_NOTFND, t = 4*MAX_TRACK_LEN;
   comp.wd.ptr0 = comp.wd.ptr; comp.wd.len0 = comp.wd.len;
   comp.wd.next += ((__int64)t)*trkcache.ts_byte;
}

void find_marker()
{

}

void process_wd()
{
   seek_track(comp.wd.drive, fdd_track[comp.wd.drive], comp.wd.side);

   __int64 time = comp.t_states + cpu.t;
   for (int d = 0; d < 4; d++) // tofix: inactive drives disregard HLT bit
      if (time > fdd_motor[d] && (comp.wd.trdos_system & 0x08)) fdd_motor[d] = 0;
   if (!fdd_motor[comp.wd.drive]) comp.wd.status |= WDS_NOTRDY;
   if (!(comp.wd.cmd & 0x80)) { // seek / step commands
      comp.wd.status &= ~(WDS_TRK00 | WDS_INDEX);
      if (fdd_motor[comp.wd.drive] && (comp.wd.trdos_system & 0x08))
         comp.wd.status |= WDS_HEADL;
      if (!fdd_track[comp.wd.drive]) comp.wd.status |= WDS_TRK00;
      // todo: test spinning
      if (disk[comp.wd.drive].rawdata && fdd_motor[comp.wd.drive] && (time % (Z80FQ/FDD_RPS) < (Z80FQ*4/1000)))
         comp.wd.status |= WDS_INDEX; // index every turn, len=4ms (if disk present)
   }
   for (;;) {
      switch (comp.wd.state) {
         case WD_READY:
            comp.wd.status &= ~WDS_BUSY;
            comp.wd.rqs |= INTRQ;
            return;

         case WD_WAIT:
            if (!conf.wd93_nodelay && time < comp.wd.next) return;
            comp.wd.state = comp.wd.state2;
            break;

         case WD_NOTFND:
            comp.wd.status |= WDS_NOTFOUND;
            comp.wd.state = WD_READY;
            break;

         case WD_SEEK0:
            if (!fdd_track[comp.wd.drive] || !comp.wd.seek0cnt) {
               comp.wd.track = 0;
               comp.wd.state = WD_SEEKDONE;
               break;
            }
            if (comp.wd.seek0cnt == 0x100) comp.wd.track = 0;
            else comp.wd.track--;
            comp.wd.seek0cnt--;
seek_step:
            fdd_track[comp.wd.drive] += comp.wd.stepdirection;
            if (fdd_track[comp.wd.drive] == (unsigned char)-1) fdd_track[comp.wd.drive] = 0;
            if (fdd_track[comp.wd.drive] >= MAX_PHYS_CYL) fdd_track[comp.wd.drive] = MAX_PHYS_CYL;
            seek_track(comp.wd.drive, fdd_track[comp.wd.drive], comp.wd.side);
            if (comp.wd.cmd & 0x10) comp.wd.track += comp.wd.stepdirection;
            comp.wd.next += comp.wd.steptime;
            comp.wd.state2 = comp.wd.state;
            comp.wd.state = WD_WAIT;
            break;

         case WD_SEEK:
            if (comp.wd.track == comp.wd.data) {
               comp.wd.state = WD_SEEKDONE;
               break;
            }
            goto seek_step;

         case WD_STEP:
            comp.wd.state = WD_READY;
            goto seek_step;

         case WD_SEEKDONE:
            if (comp.wd.cmd & 4) { // read track number?
               comp.wd.state2 = WD_READY;
               find_marker(0,0);
               if (!comp.wd.ptr || comp.wd.ptr[0] != comp.wd.track)
                  comp.wd.state2 = WD_NOTFND;
               break;
            }
            comp.wd.state = WD_READY;
            comp.wd.rqs = INTRQ;
            break;

         case WD_RESET: // seek to trk0, but don't be busy
            if (!fdd_track[comp.wd.drive]) comp.wd.state = WD_READY;
            else fdd_track[comp.wd.drive]--, seek_track(comp.wd.drive, fdd_track[comp.wd.drive], comp.wd.side);
            comp.wd.next += comp.wd.steptime;
            break;

         case WD_READDATA:
            if (conf.wd93_nodelay && (comp.wd.rqs & DRQ) && comp.wd.next > time) return;
            if (comp.wd.len) {
               trdos_load = ROMLED_TIME;
               if (comp.wd.rqs & DRQ) comp.wd.status |= WDS_LOST;
               comp.wd.data = *comp.wd.ptr++;
               comp.wd.len--;
               comp.wd.rqs = DRQ;
               comp.wd.status |= WDS_DRQ;
               comp.wd.next += trkcache.ts_byte;
               comp.wd.state = WD_WAIT;
               comp.wd.state2 = WD_READDATA;
            } else {
               comp.wd.state = WD_READY;
               if ((comp.wd.cmd & 0xE0) == 0x80) { // read sector
                  if (comp.wd.ptr0[-1] == 0xF8) comp.wd.status |= WDS_RECORDT;
                  if (*(unsigned short*)(comp.wd.ptr0+comp.wd.len0) != wd93_crc(comp.wd.ptr0-1, comp.wd.len0+1))
                     comp.wd.status |= WDS_CRCERR;
                  if (comp.wd.cmd & 0x10)
                     comp.wd.state = WD_READSECTORS, comp.wd.sector++;
               }
               if ((comp.wd.cmd & 0xF0) == 0xC0) { // read address
                  if (*(unsigned short*)(comp.wd.ptr0+4) != wd93_crc(comp.wd.ptr0-1, 5)) comp.wd.status |= WDS_CRCERR;
               }
            }
            break;

         case WD_READSECTORS:
            comp.wd.state2 = WD_STARTREAD;
            find_marker(comp.wd.cmd & 2 ? 2 : 1, 1);
            break;

         case WD_STARTREAD:
            comp.wd.state2 = WD_READDATA;
            comp.wd.state = WD_WAIT;
            comp.wd.next += trkcache.ts_byte;
            comp.wd.rqs = DRQ;
            comp.wd.status |= WDS_DRQ;
            comp.wd.data = *comp.wd.ptr++;
            comp.wd.len--;
            break;

         case WD_WRITESECTORS:
            comp.wd.state2 = WD_STARTWRITE;
            find_marker(comp.wd.cmd & 2 ? 2 : 1, 1);
            break;

         case WD_STARTWRITE: // set DRQ and wait 22 bytes
            comp.wd.state = WD_WAIT;
            comp.wd.state2 = WD_STARTWRITE2;
            comp.wd.rqs |= DRQ;
            comp.wd.status |= WDS_DRQ;
            comp.wd.next += 22*trkcache.ts_byte;
            break;

         case WD_STARTWRITE2:
            if (comp.wd.rqs & DRQ) {
               if (conf.wd93_nodelay && comp.wd.next > time) return;
               comp.wd.state = WD_READY;
               comp.wd.status |= WDS_LOST;
               break;
            }
            disk[comp.wd.drive].optype |= 1;
            comp.wd.ptr[-1] = (comp.wd.cmd & 1) ? 0xF8 : 0xFB;
            *comp.wd.ptr = comp.wd.data;
            trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr-trkcache.trkd) & 7));
            comp.wd.ptr++;
            comp.wd.len--;
            comp.wd.state = WD_WAIT;
            comp.wd.state2 = WD_WRITEDATA;
            comp.wd.status |= WDS_DRQ;
            comp.wd.rqs |= DRQ;
            comp.wd.next += 12*trkcache.ts_byte;
            break;

         case WD_WRITEDATA:
            if (conf.wd93_nodelay && (comp.wd.rqs & DRQ) && comp.wd.next > time) return;
            if (comp.wd.rqs & DRQ) comp.wd.status |= WDS_LOST, comp.wd.data = 0;
            trdos_save = ROMLED_TIME;
            *comp.wd.ptr = comp.wd.data;
            trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr-trkcache.trkd) & 7));
            comp.wd.ptr++;
            comp.wd.len--;
            if (comp.wd.len) {
               comp.wd.next += trkcache.ts_byte;
               comp.wd.state = WD_WAIT;
               comp.wd.state2 = WD_WRITEDATA;
               comp.wd.rqs = DRQ;
               comp.wd.status |= WDS_DRQ;
            } else {
               comp.wd.state = WD_READY;
               if ((comp.wd.cmd & 0xE0) == 0xA0) { // write sector
                  *(unsigned short*)(comp.wd.ptr0 + comp.wd.len0) = wd93_crc(comp.wd.ptr0-1, comp.wd.len0+1);
                  trkcache.trki[(comp.wd.ptr0 + comp.wd.len0 - trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr0 + comp.wd.len0 - trkcache.trkd) & 7));
                  trkcache.trki[(comp.wd.ptr0 + comp.wd.len0 + 1 - trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr0 + comp.wd.len0 + 1 - trkcache.trkd) & 7));
                  if (comp.wd.cmd & 0x10)
                     comp.wd.state = WD_WRITESECTORS, comp.wd.sector++;
               }
            }
            break;

         case WD_FORMATTRACK:
            if (comp.wd.rqs & DRQ) {
               if (conf.wd93_nodelay && comp.wd.next > time) return;
               comp.wd.state = WD_READY;
               comp.wd.status |= WDS_LOST;
               break;
            }
            disk[comp.wd.drive].optype |= 2;
            *comp.wd.ptr = comp.wd.data;
            trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr-trkcache.trkd) & 7));
            comp.wd.ptr++;
            comp.wd.len--;
            comp.wd.state = WD_WAIT;
            comp.wd.state2 = WD_FORMAT;
            comp.wd.status |= WDS_DRQ;
            comp.wd.rqs |= DRQ;
            comp.wd.next += trkcache.ts_byte;
            comp.wd.ptr0 = comp.wd.ptr;
            break;

         case WD_FORMAT:
         {
            if (conf.wd93_nodelay && (comp.wd.rqs & DRQ) && comp.wd.next > time) return;
            trdos_format = ROMLED_TIME;
            if (comp.wd.rqs & DRQ) comp.wd.status |= WDS_LOST, comp.wd.data = 0;
            unsigned char marker = 0, data = comp.wd.data;
            unsigned crc;
            if (data == 0xF5) data = 0xA1, marker = 1, comp.wd.ptr0 = comp.wd.ptr+1;
            if (data == 0xF6) data = 0xC2, marker = 1;
            if (data == 0xF7) crc = wd93_crc(comp.wd.ptr0, comp.wd.ptr-comp.wd.ptr0), data = crc & 0xFF;
            *comp.wd.ptr = data;
            if (marker) trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] |= (1<<((comp.wd.ptr-trkcache.trkd) & 7));
            else trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr-trkcache.trkd) & 7));
            comp.wd.ptr++;
            comp.wd.len--;
            if (comp.wd.data == 0xF7) { // second byte of CRC16
               *comp.wd.ptr = crc >> 8;
               trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr-trkcache.trkd) & 7));
               comp.wd.ptr++; comp.wd.len--;
            }
            if ((int)comp.wd.len > 0) {
               comp.wd.next += trkcache.ts_byte;
               comp.wd.state = WD_WAIT;
               comp.wd.state2 = WD_FORMAT;
               comp.wd.rqs = DRQ;
               comp.wd.status |= WDS_DRQ;
            } else {
               comp.wd.state = WD_READY;
            }
            break;
         }

         default:
            printf("WD1793 in wrong state"); exit();
      }
   }
}

unsigned char in_wd93(unsigned char port)
{
   process_wd();
   if (port & 0x80) return comp.wd.rqs;
   if (port == 0x1F) { comp.wd.rqs &= ~INTRQ; return comp.wd.status; }
   if (port == 0x3F) return comp.wd.track;
   if (port == 0x5F) return comp.wd.sector;
   if (port == 0x7F) { comp.wd.status &= ~WDS_DRQ; comp.wd.rqs &= ~DRQ; return comp.wd.data; }
   return 0xFF;
}

void out_wd93(unsigned char port, unsigned char val)
{
   process_wd();

   if (port == 0x1F) { // cmd
      comp.wd.rqs &= ~INTRQ;

      if ((val & 0xF0) == 0xD0) {
         comp.wd.state = WD_READY;
         comp.wd.status &= ~WDS_BUSY;
         return;
      }

      if (comp.wd.status & WDS_BUSY) return;
      comp.wd.cmd = val;
      comp.wd.next = comp.t_states + cpu.t;
      comp.wd.status = WDS_BUSY;
      comp.wd.rqs = 0;

      //=======================================================================

      if ((val & 0xF0) == 0x00) { // seek trk 0
         comp.wd.state2 = WD_SEEK0;
         comp.wd.stepdirection = -1;
         comp.wd.seek0cnt = 0x100;
      setup_seek:
         static char steps[] = { 6,12,20,30 };
         comp.wd.steptime = steps[val & 3] * (Z80FQ / 1000);
         comp.wd.next += 1*Z80FQ/1000; // 1ms delay before command
         comp.wd.state = WD_WAIT;
         if (conf.trdos_wp[comp.wd.drive]) comp.wd.status |= WDS_WRITEP;

         // disk spins 1.8 sec (9 turns) after command then stop
         fdd_motor[comp.wd.drive] = comp.wd.next + 2*Z80FQ;
         return;
      }
      if ((val & 0xF0) == 0x10) { // seek
         //if (comp.wd.data & 0x80) { comp.wd.state = WD_READY; return; }
         comp.wd.state2 = WD_SEEK;
         if (comp.wd.track < comp.wd.data) comp.wd.stepdirection = +1;
         else if (comp.wd.track > comp.wd.data) comp.wd.stepdirection = -1;
         else comp.wd.stepdirection = 0;
         goto setup_seek;
      }
      if ((val & 0xE0) == 0x20) { // step
         comp.wd.state2 = WD_STEP;
         goto setup_seek;
      }
      if ((val & 0xC0) == 0x40) { // step-in, step-out
         comp.wd.state2 = WD_STEP;
         comp.wd.stepdirection = (val & 0x20) ? -1 : 1;
         goto setup_seek;
      }

      //=======================================================================

      if (!trkcache.trkd) { // other commands work with surface - abort if no track
         comp.wd.state = WD_NOTFND;
         return;
      }

      // abort if disk is not spinning
      if (!fdd_motor[comp.wd.drive]) {
         comp.wd.status = WDS_NOTRDY | WDS_NOTFOUND;
         comp.wd.state = WD_READY;
         return;
      } else fdd_motor[comp.wd.drive] = comp.wd.next + 2*Z80FQ;

      if ((val & 0xE0) == 0x80) { // read sector
         comp.wd.state = WD_READSECTORS;
         return;
      }

      if ((val & 0xF0) == 0xC0) { // read address
         comp.wd.state2 = WD_STARTREAD;
         find_marker(0, 0);
         return;
      }

      if ((val & 0xE0) == 0xA0) { // write sector
         if (conf.trdos_wp[comp.wd.drive]) { comp.wd.status |= WDS_WRITEP; return; }
         comp.wd.state = WD_WRITESECTORS;
         return;
      }

      //=======================================================================

      if ((val & 0xF0) == 0xE0) { // read track
         comp.wd.state2 = WD_STARTREAD;
      trk_op:
         comp.wd.ptr = trkcache.trkd;
         comp.wd.len = trkcache.trklen;
         comp.wd.state = WD_WAIT;
         unsigned offset = (unsigned)(comp.wd.next % (trkcache.trklen*trkcache.ts_byte));
         comp.wd.next += (trkcache.trklen*trkcache.ts_byte - offset); // wait index
         return;
      }

      if ((val & 0xF0) == 0xF0) { // format track
         if (conf.trdos_wp[comp.wd.drive]) { comp.wd.status |= WDS_WRITEP; return; }
         comp.wd.state2 = WD_FORMATTRACK;
         comp.wd.rqs |= DRQ, comp.wd.status |= WDS_DRQ;
         goto trk_op;
      }
   }

   //=======================================================================

   if (port == 0x3F) { comp.wd.track = val; return; }
   if (port == 0x5F) { comp.wd.sector = val; return; }
   if (port == 0x7F) { comp.wd.data = val, comp.wd.rqs &= ~DRQ, comp.wd.status &= ~WDS_DRQ; return; }

   if (port & 0x80) { // system
      comp.wd.trdos_system = val;
      comp.wd.drive = val & 3;
      comp.wd.side = 1 & ~(val >> 4);
      if (!(val & 0x04)) { // reset
         comp.wd.status = 0x80;
         comp.wd.rqs = INTRQ;
         fdd_motor[comp.wd.drive] = 0;
         comp.wd.state = WD_READY;
         // move head to trk00
         #if 0
           //fdd_track[comp.wd.drive] = 0;
         comp.wd.steptime = 6 * (Z80FQ / 1000); // 6ms
         comp.wd.next += 1*Z80FQ/1000; // 1ms before command
         comp.wd.state = WD_RESET;
         #endif
      }
   }
}

void trdos_traps()
{
   unsigned pc = (cpu.pc & 0xFFFF);
   if (pc < 0x3E01) return;

   if (pc == 0x3E01 && bankr[0][0x3E01] == 0x0D) { cpu.a = cpu.c = 1; return; } // no delays
   if (pc == 0x3FEC && bankr[0][0x3FED] == 0xA2 &&
              (comp.wd.state == WD_READDATA || (comp.wd.state2 == WD_READDATA && comp.wd.state == WD_WAIT))) {
      trdos_load = ROMLED_TIME;
      if (comp.wd.rqs & DRQ) {
         z80dbg::wm(cpu.hl, comp.wd.data); // move byte from controller
         cpu.hl++, cpu.b--;
         comp.wd.rqs &= ~DRQ; comp.wd.status &= ~WDS_DRQ;
      }
      while (comp.wd.len) { // move others
         z80dbg::wm(cpu.hl, *comp.wd.ptr++);
         comp.wd.len--; cpu.hl++; cpu.b--;
      }
      cpu.pc += 2; // skip INI
      return;
   }
   if (pc == 0x3FD1 && bankr[0][0x3FD2] == 0xA3 &&
              (comp.wd.rqs & DRQ) && (comp.wd.len>1) && (comp.wd.state == WD_WRITEDATA || (comp.wd.state2 == WD_WRITEDATA && comp.wd.state == WD_WAIT))) {
      trdos_save = ROMLED_TIME;
      while (comp.wd.len > 1) {
         *comp.wd.ptr = z80dbg::rm(cpu.hl); cpu.hl++; cpu.b--;
         trkcache.trki[(comp.wd.ptr-trkcache.trkd)/8] &= ~(1<<((comp.wd.ptr-trkcache.trkd) & 7));
         comp.wd.ptr++; comp.wd.len--;
      }
      cpu.pc += 2; // skip OUTI
      return;
   }
}

void init_dos()
{

}

void done_dos()
{
   for (int i = 0; i < 4; i++) testdisk(i), freedisk(i);
}
