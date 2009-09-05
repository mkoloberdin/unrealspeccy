
void WD1793::process()
{
   time = comp.t_states + cpu.t;
   // inactive drives disregard HLT bit
   if (time > seldrive->motor && (system & 0x08)) seldrive->motor = 0;
   if (seldrive->rawdata) status &= ~WDS_NOTRDY; else status |= WDS_NOTRDY;

   if (!(cmd & 0x80)) { // seek / step commands
      status &= ~(WDS_TRK00 | WDS_INDEX);
      if (seldrive->motor && (system & 0x08)) status |= WDS_HEADL;
      if (!seldrive->track) status |= WDS_TRK00;
      // todo: test spinning
      if (seldrive->rawdata && seldrive->motor && ((time+tshift) % (Z80FQ/FDD_RPS) < (Z80FQ*4/1000)))
         status |= WDS_INDEX; // index every turn, len=4ms (if disk present)
   }

   for (;;) {

      switch (state) {

         // ----------------------------------------------------

         case S_IDLE:
            status &= ~WDS_BUSY;
            rqs = INTRQ;
            return;

         case S_WAIT:
            if (time < next) return;
            state = state2;
            break;

         // ----------------------------------------------------

         case S_DELAY_BEFORE_CMD:
            if (!conf.wd93_nodelay && (cmd & CMD_DELAY)) next += (Z80FQ*15/1000); // 15ms delay
            status = (status | WDS_BUSY) & ~(WDS_DRQ | WDS_LOST | WDS_NOTFOUND | WDS_RECORDT | WDS_WRITEP);
            state2 = S_CMD_RW; state = S_WAIT;
            break;

         case S_CMD_RW:
            if (((cmd & 0xE0) == 0xA0 || (cmd & 0xF0) == 0xF0) && conf.trdos_wp[drive]) {
               status |= WDS_WRITEP;
               state = S_IDLE;
               break;
            }

            if ((cmd & 0xC0) == 0x80 || (cmd & 0xF8) == 0xC0) {
               // read/write sectors or read am - find next AM
               end_waiting_am = next + 5*Z80FQ/FDD_RPS; // max wait disk 5 turns
               find_marker();
               break;
            }

            if ((cmd & 0xF8) == 0xF0) { // write track
               rqs = DRQ; status |= WDS_DRQ;
               next += 3*trkcache.ts_byte;
               state2 = S_WRTRACK; state = S_WAIT;
               break;
            }

            if ((cmd & 0xF8) == 0xE0) { // read track
               load(); rwptr = 0; rwlen = trkcache.trklen;
               state2 = S_READ; getindex();
               break;
            }

            // else unknown command
            state = S_IDLE;
            break;

        case S_FOUND_NEXT_ID:
            if (!seldrive->rawdata) { // no disk - wait again
               end_waiting_am = next + 5*Z80FQ/FDD_RPS;
        nextmk:
               find_marker();
               break;
            }
            if (next >= end_waiting_am) { nf: status |= WDS_NOTFOUND; state = S_IDLE; break; }
            if (foundid == -1) goto nf;

            status &= ~WDS_CRCERR;
            load();

            if (!(cmd & 0x80)) { // verify after seek
               if (trkcache.hdr[foundid].c != track) goto nextmk;
               if (!trkcache.hdr[foundid].c1) { status |= WDS_CRCERR; goto nextmk; }
               state = S_IDLE; break;
            }

            if ((cmd & 0xF0) == 0xC0) { // read AM
               rwptr = trkcache.hdr[foundid].id - trkcache.trkd;
               rwlen = 6;
         read_first_byte:
               data = trkcache.trkd[rwptr++]; rwlen--;
               rqs = DRQ; status |= WDS_DRQ;
               next += trkcache.ts_byte;
               state = S_WAIT;
               state2 = S_READ;
               break;
            }

            // else R/W sector(s)
            if (trkcache.hdr[foundid].c != track || trkcache.hdr[foundid].n != sector) goto nextmk;
            if ((cmd & CMD_SIDE_CMP_FLAG) && (((cmd >> CMD_SIDE_SHIFT) ^ trkcache.hdr[foundid].s) & 1)) goto nextmk;
            if (!trkcache.hdr[foundid].c1) { status |= WDS_CRCERR; goto nextmk; }

            if (cmd & 0x20) { // write sector(s)
               rqs = DRQ; status |= WDS_DRQ;
               next += trkcache.ts_byte*9;
               state = S_WAIT; state2 = S_WRSEC;
               break;
            }

            // read sector(s)
            if (!trkcache.hdr[foundid].data) goto nextmk;
            if (trkcache.hdr[foundid].data[-1] == 0xF8) status |= WDS_RECORDT; else status &= ~WDS_RECORDT;
            rwptr = trkcache.hdr[foundid].data - trkcache.trkd;
            rwlen = 128 << trkcache.hdr[foundid].l;
            goto read_first_byte;

         case S_READ:
            if (notready()) break;
            load();

            if (rwlen) {
               trdos_load = ROMLED_TIME;
               if (rqs & DRQ) status |= WDS_LOST;
               data = trkcache.trkd[rwptr++]; rwlen--;
               rqs = DRQ; status |= WDS_DRQ;
               if (!conf.wd93_nodelay) next += trkcache.ts_byte;
               state = S_WAIT;
               state2 = S_READ;
            } else {
               if ((cmd & 0xE0) == 0x80) { // read sector
                  if (!trkcache.hdr[foundid].c2) status |= WDS_CRCERR;
                  if (cmd & CMD_MULTIPLE) { sector++, state = S_CMD_RW; break; }
               }
               if ((cmd & 0xF0) == 0xC0) // read address
                  if (!trkcache.hdr[foundid].c1) status |= WDS_CRCERR;
               state = S_IDLE;
            }
            break;


         case S_WRSEC:
            load();
            if (rqs & DRQ) { status |= WDS_LOST; state = S_IDLE; break; }
            seldrive->optype |= 1;
            rwptr = trkcache.hdr[foundid].id + 6 + 11 + 11 - trkcache.trkd;
            for (rwlen = 0; rwlen < 12; rwlen++) trkcache.write(rwptr++, 0, 0);
            for (rwlen = 0; rwlen < 3; rwlen++)  trkcache.write(rwptr++, 0xA1, 1);
            trkcache.write(rwptr++, (cmd & CMD_WRITE_DEL)? 0xF8 : 0xFB, 0);
            rwlen = 128 << trkcache.hdr[foundid].l;
            state = S_WRITE; break;

         case S_WRITE:
            if (notready()) break;
            if (rqs & DRQ) status |= WDS_LOST, data = 0;
            trdos_save = ROMLED_TIME;
            trkcache.write(rwptr++, data, 0); rwlen--;
            if (rwptr == trkcache.trklen) rwptr = 0;
            trkcache.sf = JUST_SEEK; // invalidate sectors
            if (rwlen) {
               if (!conf.wd93_nodelay) next += trkcache.ts_byte;
               state = S_WAIT; state2 = S_WRITE;
               rqs = DRQ; status |= WDS_DRQ;
            } else {
               unsigned len = (128 << trkcache.hdr[foundid].l) + 1;
               unsigned char sc[2056];
               if (rwptr < len)
                  memcpy(sc, trkcache.trkd + trkcache.trklen - rwptr, rwptr), memcpy(sc + rwptr, trkcache.trkd, len - rwptr);
               else memcpy(sc, trkcache.trkd + rwptr - len, len);
               unsigned crc = wd93_crc(sc, len);
               trkcache.write(rwptr++, (BYTE)crc, 0);
               trkcache.write(rwptr++, (BYTE)(crc >> 8), 0);
               trkcache.write(rwptr, 0xFF, 0);
               if (cmd & CMD_MULTIPLE) { sector++, state = S_CMD_RW; break; }
               state = S_IDLE;
            }
            break;

         case S_WRTRACK:
            if (rqs & DRQ) { status |= WDS_LOST; state = S_IDLE; break; }
            seldrive->optype |= 2;
            state2 = S_WR_TRACK_DATA; getindex();
            end_waiting_am = next + 5*Z80FQ/FDD_RPS;
            break;

         case S_WR_TRACK_DATA:
         {
            if (notready()) break;
            trdos_format = ROMLED_TIME;
            if (rqs & DRQ) status |= WDS_LOST, data = 0;
            trkcache.seek(seldrive, seldrive->track, side, JUST_SEEK);
            trkcache.sf = JUST_SEEK; // invalidate sectors
            unsigned char marker = 0, byte = data;
            unsigned crc;
            if (data == 0xF5) byte = 0xA1, marker = 1, start_crc = rwptr+1;
            if (data == 0xF6) byte = 0xC2, marker = 1;
            if (data == 0xF7) crc = wd93_crc(trkcache.trkd+start_crc, rwptr-start_crc), byte = crc & 0xFF;
            trkcache.write(rwptr++, byte, marker); rwlen--;
            if (data == 0xF7) trkcache.write(rwptr++, (BYTE)(crc >> 8), 0), rwlen--; // second byte of CRC16

            if ((int)rwlen > 0) {
               if (!conf.wd93_nodelay) next += trkcache.ts_byte;
               state2 = S_WR_TRACK_DATA; state = S_WAIT;
               rqs = DRQ; status |= WDS_DRQ;
               break;
            }
            state = S_IDLE;
            break;
         }

         // ----------------------------------------------------

         case S_TYPE1_CMD:
            status = (status | WDS_BUSY) & ~(WDS_DRQ | WDS_CRCERR | WDS_SEEKERR);
            rqs = 0;

            if (!(cmd & 0xE0)) { // seek, restore
               if (!(cmd & 0x10)) track = 0xFF, data = 0;
               stepdirection = (data < track) ? -1 : 1;
            } else { // single step
               if (cmd & 0x40) stepdirection = (cmd & CMD_SEEK_DIR) ? -1 : 1;
               if (!(cmd & CMD_SEEK_TRKUPD)) track -= stepdirection; // compensate change in S_BEGINSTEP
            }
            state = S_BEGINSTEP;

         case S_BEGINSTEP:
            trdos_seek = ROMLED_TIME;
            if (conf.trdos_wp[drive]) status |= WDS_WRITEP;
            // disk spins 1.8 sec (9 turns) after command then stop
            seldrive->motor = next + 2*Z80FQ;

            // TRK00 sampled only in RESTORE command
            if (!seldrive->track && !(cmd & 0xF0)) { track = 0; state = S_VERIFY; break; }

            track += stepdirection;
            // step not performed, if fdc reset just after step command
            if (!conf.wd93_nodelay) next += 1*Z80FQ/1000;
            state = S_WAIT; state2 = S_STEP; break;

         case S_STEP:
         {
            seldrive->track += stepdirection;
            if (seldrive->track == (unsigned char)-1) seldrive->track = 0;
            if (seldrive->track >= MAX_PHYS_CYL) seldrive->track = MAX_PHYS_CYL;
            trkcache.clear();

            static const unsigned steps[] = { 6,12,20,30 };
            if (!conf.wd93_nodelay) next += (steps[cmd & CMD_SEEK_RATE]-1)*Z80FQ/1000;
            state2 = (cmd & 0xE0)? S_VERIFY : S_SEEK;
            state = S_WAIT; break;
         }

         case S_SEEK:
            if (data == track) { state = S_VERIFY; break; }
            stepdirection = (data < track) ? -1 : 1;
            state = S_BEGINSTEP; break;

         case S_VERIFY:
            if (!(cmd & CMD_SEEK_VERIFY)) { state = S_IDLE; break; }
            end_waiting_am = next + 6*Z80FQ/FDD_RPS; // max wait disk 6 turns
            load(); find_marker(); break;


         // ----------------------------------------------------

         case S_RESET: // seek to trk0, but don't be busy
            if (!seldrive->track) state = S_IDLE;
            else seldrive->track--, trkcache.clear();
            // if (!seldrive->track) track = 0;
            next += 6*Z80FQ/1000;
            break;


         default:
            printf("WD1793 in wrong state"); exit();
      }
   }
}

void WD1793::find_marker()
{
   if (conf.wd93_nodelay && seldrive->track != track) seldrive->track = track;
   load();

   foundid = -1; unsigned wait = 10*Z80FQ/FDD_RPS;

   if (seldrive->motor && seldrive->rawdata) {
      unsigned div = trkcache.trklen*trkcache.ts_byte;
      unsigned i = (unsigned)((next+tshift) % div) / trkcache.ts_byte;
      wait = -1;
      for (unsigned is = 0; is < trkcache.s; is++) {
         unsigned pos = trkcache.hdr[is].id - trkcache.trkd;
         unsigned dist = (pos > i)? pos-i : trkcache.trklen+pos-i;
         if (dist < wait) wait = dist, foundid = is;
      }

      if (foundid != -1) wait *= trkcache.ts_byte;
      else wait = 10*Z80FQ/FDD_RPS;

      if (conf.wd93_nodelay && foundid != -1) {
         // adjust tshift, that id appares right under head
         unsigned pos = trkcache.hdr[foundid].id - trkcache.trkd + 2;
         tshift = (unsigned)(((pos * trkcache.ts_byte) - (next % div) + div) % div);
         wait = 100; // delay=0 causes fdc to search infinitely, when no matched id on track
      }

   } // else no index pulses - infinite wait, but now wait 10spins, and re-test if disk inserted

   next += wait;
   if (seldrive->rawdata && next > end_waiting_am) next = end_waiting_am, foundid = -1;
   state = S_WAIT; state2 = S_FOUND_NEXT_ID;
}

char WD1793::notready()
{
   // fdc is too fast in no-delay mode, wait until cpu handles DRQ, but not more 'end_waiting_am'
   if (!conf.wd93_nodelay || !(rqs & DRQ)) return 0;
   if (next > end_waiting_am) return 0;
   state2 = state; state = S_WAIT;
   next += trkcache.ts_byte;
   return 1;
}

void WD1793::getindex()
{
   unsigned trlen = trkcache.trklen*trkcache.ts_byte;
   unsigned ticks = (unsigned)((next+tshift) % trlen);
   if (!conf.wd93_nodelay) next += (trlen - ticks);
   rwptr = 0; rwlen = trkcache.trklen; state = S_WAIT;
}

void WD1793::load()
{
   trkcache.seek(seldrive, seldrive->track, side, LOAD_SECTORS);
}

unsigned char WD1793::in(unsigned char port)
{
   process();
   if (port & 0x80) return rqs | 0x3F;
   if (port == 0x1F) { rqs &= ~INTRQ; return status; }
   if (port == 0x3F) return track;
   if (port == 0x5F) return sector;
   if (port == 0x7F) { status &= ~WDS_DRQ; rqs &= ~DRQ; return data; }
   return 0xFF;
}

void WD1793::out(unsigned char port, unsigned char val)
{
   process();

   if (port == 0x1F) { // cmd

      // force interrupt
      if ((val & 0xF0) == 0xD0) {
         state = S_IDLE; rqs = INTRQ;
         status &= ~WDS_BUSY;
         return;
      }

      if (status & WDS_BUSY) return;
      cmd = val; next = comp.t_states + cpu.t;
      status |= WDS_BUSY; rqs = 0;

      //-----------------------------------------------------------------------

      if (cmd & 0x80) // read/write command
      {
         // abort if no disk
         if (status & WDS_NOTRDY) {
            state = S_IDLE; rqs = INTRQ;
            return;
         }

         // continue disk spinning
         if (seldrive->motor || conf.wd93_nodelay) seldrive->motor = next + 2*Z80FQ;

         state = S_DELAY_BEFORE_CMD;
         return;
      }

      // else seek/step command
      state = S_TYPE1_CMD;
      return;
   }

   //=======================================================================

   if (port == 0x3F) { track = val; return; }
   if (port == 0x5F) { sector = val; return; }
   if (port == 0x7F) { data = val, rqs &= ~DRQ, status &= ~WDS_DRQ; return; }

   if (port & 0x80) { // system
      system = val;
      drive = val & 3, side = 1 & ~(val >> 4);
      trkcache.clear();
      if (!(val & 0x04)) { // reset
         status = WDS_NOTRDY;
         rqs = INTRQ;
         seldrive->motor = 0;
         state = S_IDLE;
         #if 0 // move head to trk00
         steptime = 6 * (Z80FQ / 1000); // 6ms
         next += 1*Z80FQ/1000; // 1ms before command
         state = S_RESET;
           //seldrive->track = 0;
         #endif
      }
   }
}

void WD1793::trdos_traps()
{
   unsigned pc = (cpu.pc & 0xFFFF);
   if (pc < 0x3E01) return;

   if (pc == 0x3E01 && bankr[0][0x3E01] == 0x0D) { cpu.a = cpu.c = 1; return; } // no delays
   if (pc == 0x3FEC && bankr[0][0x3FED] == 0xA2 &&
              (state == S_READ || (state2 == S_READ && state == S_WAIT))) {
      trdos_load = ROMLED_TIME;
      if (rqs & DRQ) {
         z80dbg::wm(cpu.hl, data); // move byte from controller
         cpu.hl++, cpu.b--;
         rqs &= ~DRQ; status &= ~WDS_DRQ;
      }
      while (rwlen) { // move others
         z80dbg::wm(cpu.hl, trkcache.trkd[rwptr++]); rwlen--;
         cpu.hl++; cpu.b--;
      }
      cpu.pc += 2; // skip INI
      return;
   }
   if (pc == 0x3FD1 && bankr[0][0x3FD2] == 0xA3 &&
              (rqs & DRQ) && (rwlen>1) && (state == S_WRITE || (state2 == S_WRITE && state == S_WAIT))) {
      trdos_save = ROMLED_TIME;
      while (rwlen > 1) {
         trkcache.write(rwptr++, z80dbg::rm(cpu.hl), 0); rwlen--;
         cpu.hl++; cpu.b--;
      }
      cpu.pc += 2; // skip OUTI
      return;
   }
}
