
const MAX_DEVICES = MAX_PHYS_HD_DRIVES+2*MAX_PHYS_CD_DRIVES;

PHYS_DEVICE phys[MAX_DEVICES];
int n_phys = 0;

/*
// this function is untested
void ATA_DEVICE::exec_mode_select()
{
   intrq = 1;
   command_ok();

   struct {
      SCSI_PASS_THROUGH_DIRECT p;
      unsigned char sense[0x40];
   } srb = { 0 }, dst;

   srb.p.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
   *(CDB*)&srb.p.Cdb = cdb;
   srb.p.CdbLength = sizeof(CDB);
   srb.p.DataIn = SCSI_IOCTL_DATA_OUT;
   srb.p.TimeOutValue = 10;
   srb.p.DataBuffer = transbf;
   srb.p.DataTransferLength = transcount;
   srb.p.SenseInfoLength = sizeof(srb.sense);
   srb.p.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);

   DWORD outsize;
   int r = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
                           &srb.p, sizeof(srb.p),
                           &dst, sizeof(dst),
                           &outsize, 0);

   if (!r) return;
   if (senselen = dst.p.SenseInfoLength) memcpy(sense, dst.sense, senselen);
   return;
}
*/

void init_hdd_cd()
{
   memset(&phys, 0, sizeof phys);
   if (conf.ide_skip_real) return;

   n_phys = 0;
   n_phys = ATA_PASSER::identify(phys + n_phys, MAX_DEVICES - n_phys);
   n_phys += ATAPI_PASSER::identify(phys + n_phys, MAX_DEVICES - n_phys);

   if (!n_phys) errmsg("HDD/CD emulator can't access physical drives");
}

void delstr_spaces(char *dst, char *src)
{
   for (; *src; src++)
      if (*src != ' ') *dst++ = *src;
   *dst = 0;
}

unsigned find_hdd_device(char *name)
{
   char s2[512]; delstr_spaces(s2, name);
//   if(temp.win9x)
   for (int drive = 0; drive < n_phys; drive++) {
      char s1[512]; delstr_spaces(s1, phys[drive].viewname);
      if (!stricmp(s1,s2)) return drive;
   }
   return -1;
}

void ATA_DEVICE::configure(IDE_CONFIG *cfg)
{
   atapi_p.close(); ata_p.close();

   c = cfg->c, h = cfg->h, s = cfg->s, lba = cfg->lba; readonly = cfg->readonly;

   phys_dev = -1;
   if (!*cfg->image) return;

   PHYS_DEVICE filedev, *dev;
   unsigned drive = find_hdd_device(cfg->image);
   if (drive == -1) {
      if (cfg->image[0] == '<') { errmsg("no physical device %s", cfg->image); *cfg->image = 0; return; }
      strcpy(filedev.filename, cfg->image);
      filedev.type = ATA_FILEHDD;
      dev = &filedev;
   } else {
      dev = phys + drive;
      if (dev->type == ATA_NTHDD) {
         // read geometry from id sector
         c = *(unsigned short*)(phys[drive].idsector+2);
         h = *(unsigned short*)(phys[drive].idsector+6);
         s = *(unsigned short*)(phys[drive].idsector+12);
         lba = *(unsigned*)(phys[drive].idsector+0x78);
         if (!lba) lba = c*h*s;
      }
   }
   DWORD errcode;
   if (dev->type == ATA_NTHDD || dev->type == ATA_FILEHDD) errcode = ata_p.open(dev), atapi = 0;
   if (dev->type == ATA_SPTI_CD || dev->type == ATA_ASPI_CD) errcode = atapi_p.open(dev), atapi = 1;
   if (errcode == NO_ERROR) return;
   errmsg("failed to open %s", cfg->image);
   err_win32(errcode);
   *cfg->image = 0;
}

void ATA_PORT::reset()
{
   dev[0].reset(ATA_DEVICE::RESET_HARD);
   dev[1].reset(ATA_DEVICE::RESET_HARD);
}

unsigned char ATA_PORT::read(unsigned n_reg)
{
#ifdef DUMP_HDD_IO
unsigned char val = dev[0].read(n_reg) & dev[1].read(n_reg); printf("R%X:%02X ", n_reg, val); return val;
#endif
   return dev[0].read(n_reg) & dev[1].read(n_reg);
}

unsigned ATA_PORT::read_data()
{
#ifdef DUMP_HDD_IO
unsigned val = dev[0].read_data() & dev[1].read_data(); printf("r%04X ", val & 0xFFFF); return val;
#endif
   return dev[0].read_data() & dev[1].read_data();
}

void ATA_PORT::write(unsigned n_reg, unsigned char data)
{
#ifdef DUMP_HDD_IO
printf("R%X=%02X ", n_reg, data);
#endif
   dev[0].write(n_reg, data);
   dev[1].write(n_reg, data);
}

void ATA_PORT::write_data(unsigned data)
{
#ifdef DUMP_HDD_IO
printf("w%04X ", data & 0xFFFF);
#endif
   dev[0].write_data(data);
   dev[1].write_data(data);
}

unsigned char ATA_PORT::read_intrq()
{
#ifdef DUMP_HDD_IO
unsigned char i = dev[0].read_intrq() & dev[1].read_intrq(); printf("i%d ", !!i); return i;
#endif
   return dev[0].read_intrq() & dev[1].read_intrq();
}

void ATA_DEVICE::reset_signature(RESET_TYPE mode)
{
   reg.count = reg.sec = reg.err = 1;
   reg.cyl = atapi? 0xEB14 : 0;
   reg.devhead &= (atapi && mode == RESET_SOFT)? 0x10 : 0;
   reg.status = (mode == RESET_SOFT || !atapi)? STATUS_DRDY : 0;
}

void ATA_DEVICE::reset(RESET_TYPE mode)
{
   reg.control = 0; // clear SRST
   intrq = 0;

   command_ok();
   reset_signature(mode);
}

void ATA_DEVICE::command_ok()
{
   state = S_IDLE;
   transptr = -1;
   reg.err = 0;
   reg.status = STATUS_DRDY | STATUS_DSC;
}

unsigned char ATA_DEVICE::read_intrq()
{
   if (!loaded() || ((reg.devhead ^ device_id) & 0x10) || (reg.control & CONTROL_nIEN)) return 0xFF;
   return intrq? 0xFF : 0x00;
}

unsigned char ATA_DEVICE::read(unsigned n_reg)
{
   if (!loaded()) return 0xFF;
   if ((reg.devhead ^ device_id) & 0x10) return 0xFF;
   if (n_reg == 7) intrq = 0;
   if (n_reg == 8) n_reg = 7; // read alt.status -> read status
   if (n_reg == 7 || (reg.status & STATUS_BSY)) {
//	   printf("state=%d\n",state); //Alone Coder
	   return reg.status;
   } // BSY=1 or read status
   // BSY = 0
   //// if (reg.status & STATUS_DRQ) return 0xFF;    // DRQ.  ATA-5: registers should not be queried while DRQ=1, but programs do this!
   // DRQ = 0
   return regs[n_reg];
}

unsigned ATA_DEVICE::read_data()
{
   if (!loaded()) return 0xFFFFFFFF;
   if ((reg.devhead ^ device_id) & 0x10) return 0xFFFFFFFF;
   if (/* (reg.status & (STATUS_DRQ | STATUS_BSY)) != STATUS_DRQ ||*/ transptr >= transcount) return 0xFFFFFFFF;
   // DRQ=1, BSY=0, data present
   unsigned result = *(unsigned*)(transbf + transptr*2); transptr++;
   if (transptr < transcount) return result;
   // look to state, prepare next block
   if (state == S_READ_ID || state == S_READ_ATAPI) command_ok();
   if (state == S_READ_SECTORS) {
      if (!--reg.count) command_ok();
      else { next_sector(); read_sectors(); }
   }

   return result;
}

char ATA_DEVICE::exec_ata_cmd(unsigned char cmd)
{
   // EXECUTE DEVICE DIAGNOSTIC for both ATA and ATAPI
   if (cmd == 0x90) { reset_signature(RESET_SOFT); return 1; }

   if (atapi) return 0;

   if ((cmd & 0xFE) == 0x20) { read_sectors(); return 1; } // cmd #21 obsolette, rqd for is-dos

   if ((cmd & 0xFE) == 0x30 && !readonly) { // cmd #31 obsolette
      if (seek()) state = S_WRITE_SECTORS, reg.status = STATUS_DRQ | STATUS_DSC, transptr = 0, transcount = 0x100;
      return 1;
   }

   if (cmd == 0xEC) { prepare_id(); return 1; }

   if (cmd == 0xE7) { // FLUSH CACHE
      if (ata_p.flush()) command_ok(), intrq = 1;
      else reg.status = STATUS_DRDY | STATUS_DWF | STATUS_DSC | STATUS_ERR; // 0x71
      return 1;
   }

   if (cmd == 0x10) { // ATA-5 drives should not implement recalibration command, but programs expect it
      command_ok();
      intrq = 1;
      return 1;
   }

   if (cmd == 0x70) { // ATA-5 drives should not implement seek command, but ATM CP/M expect it
      if (!seek()) return 1;
      command_ok();
      intrq = 1;
      return 1;
   }

   return 0;
}

char ATA_DEVICE::exec_atapi_cmd(unsigned char cmd)
{
   if (!atapi) return 0;

   // soft reset
   if (cmd == 0x08) { reset(RESET_SOFT); return 1; }
   if (cmd == 0xA1) { prepare_id(); return 1; }

   if (cmd == 0xA0) { // packet
      state = S_RECV_PACKET;
      reg.status = STATUS_DRQ;
      reg.intreason = INT_COD;
      transptr = 0, transcount = 6;
      return 1;
   }

   // "command aborted" with ATAPI signature
   reg.cyl = 0xEB14;
   return 0;
}

void ATA_DEVICE::write(unsigned n_reg, unsigned char data)
{
   if (!loaded()) return;
   if (n_reg == 1) { reg.feat = data; return; }
   if (n_reg != 7) {
      regs[n_reg] = data;
      if (reg.control & CONTROL_SRST) reset(RESET_SRST);
      return;
   }

   // execute command!
   if (((reg.devhead ^ device_id) & 0x10) && data != 0x90) return;
   if (!(reg.status & STATUS_DRDY) && !atapi) return;

   reg.err = 0; intrq = 0;

//{printf(" [");for (int q=1;q<9;q++) printf("-%02X",regs[q]);printf("]\n");}
   if (exec_atapi_cmd(data)) return;
   if (exec_ata_cmd(data)) return;
//printf(" *** unknown cmd %02X *** ", data);
   reg.status = STATUS_DSC | STATUS_DRDY | STATUS_ERR; reg.err = ERR_ABRT;
   state = S_IDLE; intrq = 1;
}

void ATA_DEVICE::write_data(unsigned data)
{
   if (!loaded()) return;
   if ((reg.devhead ^ device_id) & 0x10) return;
   if (/* (reg.status & (STATUS_DRQ | STATUS_BSY)) != STATUS_DRQ ||*/ transptr >= transcount) return;
   *(unsigned short*)(transbf + transptr*2) = (unsigned short)data; transptr++;
   if (transptr < transcount) return;
   // look to state, prepare next block
   if (state == S_WRITE_SECTORS) { write_sectors(); return; }
   if (state == S_RECV_PACKET) { handle_atapi_packet(); return; }
/*   if (state == S_MODE_SELECT) { exec_mode_select(); return; } */
}

char ATA_DEVICE::seek()
{
   unsigned pos;
   if (reg.devhead & 0x40) {
      pos = *(unsigned*)(regs+3) & 0x0FFFFFFF;
      if (pos >= lba) { seek_err: reg.status = STATUS_DRDY | STATUS_ERR; reg.err = ERR_IDNF; intrq = 1; return 0; }
   } else {
      if (reg.cyl >= c || (unsigned)(reg.devhead & 0x0F) >= h || reg.sec > s) goto seek_err;
      pos = (reg.cyl * h + (reg.devhead & 0x0F)) * s + reg.sec - 1;
//printf("\nchs %4d/%02d/%02d: %8d ", *(unsigned short*)(regs+4), (reg.devhead & 0x0F), reg.sec, pos);
   };
//printf("[seek %I64d]", ((__int64)pos) << 9);
   if (!ata_p.seek(pos)) {
      reg.status = STATUS_ERR | STATUS_DWF | STATUS_DRDY;
      reg.err = ERR_IDNF;
      intrq = 1;
      return 0;
   }
   return 1;
}

void ATA_DEVICE::write_sectors()
{
   intrq = 1;
//printf(" [write] ");
   if (!ata_p.write_sector(transbf)) {
      reg.status = STATUS_DRDY | STATUS_DSC | STATUS_ERR;
      reg.err = ERR_UNC;
      state = S_IDLE;
      return;
   }
   if (!--reg.count) { command_ok(); return; }
   next_sector(); if (!seek()) return;
   transptr = 0, transcount = 0x100;
   state = S_WRITE_SECTORS;
   reg.err = 0;
   reg.status = STATUS_DRQ | STATUS_DSC;
}

void ATA_DEVICE::read_sectors()
{
   intrq = 1;
   if (!seek()) return;
   DWORD sz = 0;
   if (!ata_p.read_sector(transbf)) {
      reg.status = 0x51; reg.err = ERR_UNC; state = S_IDLE;
      return;
   }
   transptr = 0, transcount = 0x100;
   state = S_READ_SECTORS;
   reg.err = 0;
   reg.status = STATUS_DRQ | STATUS_DSC;
}

void ATA_DEVICE::next_sector()
{
   if (reg.devhead & 0x40) { // LBA
      *(unsigned*)&reg.sec = (*(unsigned*)&reg.sec & 0xF0000000) + ((*(unsigned*)&reg.sec+1) & 0x0FFFFFFF);
      return;
   }
   // need to recalc CHS for every sector, coz ATA registers
   // should contain current position on failure
   if (reg.sec < s) { reg.sec++; return; }
   reg.sec = 1; unsigned char head = (reg.devhead & 0x0F) + 1;
   if (head < h) { reg.devhead = (reg.devhead & 0xF0)+head; return; }
   reg.devhead &= 0xF0; reg.cyl++;
}

void ATA_DEVICE::handle_atapi_packet()
{
#ifdef DUMP_HDD_IO
{printf(" [packet"); for (int i = 0; i < 12; i++) printf("-%02X", transbf[i]); printf("]\n");}
#endif
   memcpy(&atapi_p.cdb, transbf, 12);

   intrq = 1;

   if (atapi_p.cdb.MODE_SELECT10.OperationCode == 0x55) { // MODE SELECT requires additional data from host

      state = S_MODE_SELECT;
      reg.status = STATUS_DRQ;
      reg.intreason = 0;
      transptr = 0;
      transcount = atapi_p.cdb.MODE_SELECT10.ParameterListLength[0]*0x100 + atapi_p.cdb.MODE_SELECT10.ParameterListLength[1];
      return;
   }

   if (atapi_p.cdb.CDB6READWRITE.OperationCode == 0x03 && atapi_p.senselen) { // REQ.SENSE - read cached
      memcpy(transbf, atapi_p.sense, atapi_p.senselen);
      atapi_p.passed_length = atapi_p.senselen; atapi_p.senselen = 0; // next time read from device
      goto ok;
   }

   if (atapi_p.pass_through(transbf, sizeof transbf)) {
      if (atapi_p.senselen) { reg.err = atapi_p.sense[2] << 4; goto err; } // err = sense key //win9x hangs on drq after atapi packet when emulator does goto err (see walkaround in SEND_ASPI_CMD)
    ok:
      if (!atapi_p.cdb.CDB6READWRITE.OperationCode) atapi_p.passed_length = 0; // bugfix in cdrom driver: TEST UNIT READY has no data
      if (!atapi_p.passed_length /* || atapi_p.passed_length == sizeof transbf */ ) { command_ok(); return; }
      reg.atapi_count = atapi_p.passed_length;
      reg.intreason = INT_IO;
      reg.status = STATUS_DRQ;
      transcount = (atapi_p.passed_length+1)/2;
	  //printf("transcount=%d\n",transcount); //32768 in win9x
      transptr = 0;
      state = S_READ_ATAPI;
   } else { // bus error
      reg.err = 0;
    err:
      state = S_IDLE;
      reg.status = STATUS_DSC | STATUS_ERR | STATUS_DRDY;
   }
}

void ATA_DEVICE::prepare_id()
{
   if (phys_dev == -1) {
      memset(transbf, 0, 512);
      make_ata_string(transbf+54, 20, "UNREAL SPECCY HARD DRIVE IMAGE");
      make_ata_string(transbf+20, 10, "0000");
      make_ata_string(transbf+46,  4, VERS_STRING);
      *(unsigned short*)transbf = 0x045A;
      ((unsigned short*)transbf)[1] = (unsigned short)c;
      ((unsigned short*)transbf)[3] = (unsigned short)h;
      ((unsigned short*)transbf)[6] = (unsigned short)s;
      *(unsigned*)(transbf+60*2) = lba;
      ((unsigned short*)transbf)[20] = 3; // a dual ported multi-sector buffer capable of simultaneous transfers with a read caching capability
      ((unsigned short*)transbf)[21] = 512; // cache size=256k
      ((unsigned short*)transbf)[22] = 4; // ECC bytes
      ((unsigned short*)transbf)[49] = 0x200; // LBA supported
      ((unsigned short*)transbf)[80] = 0x3E; // support specifications up to ATA-5
      ((unsigned short*)transbf)[81] = 0x13; // ATA/ATAPI-5 T13 1321D revision 3
      ((unsigned short*)transbf)[82] = 0x60; // supported look-ahead and write cache

      // make checksum
      transbf[510] = 0xA5;
      unsigned char cs = 0;
      for (unsigned i = 0; i < 511; i++) cs += transbf[i];
      transbf[511] = 0-cs;
   } else { // copy as is...
      memcpy(transbf, phys[phys_dev].idsector, 512);
   }

   state = S_READ_ID;
   transptr = 0, transcount = 0x100;
   intrq = 1;
   reg.status = STATUS_DRQ | STATUS_DSC;
   reg.err = 0;
}
