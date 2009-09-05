
#define MAX_PHYS_HD_DRIVES 8
#define MAX_PHYS_CD_DRIVES 8
struct {
   unsigned char hdd_id[MAX_PHYS_HD_DRIVES+MAX_PHYS_CD_DRIVES][512];
   unsigned hdd_size[MAX_PHYS_HD_DRIVES];
} phys;

// #define DUMP_HDD_IO

void swap_bytes(char *dst, BYTE *src, unsigned n_words)
{
   for (unsigned i = 0; i < n_words; i++) {
      char c1 = src[2*i], c2 = src[2*i+1];
      dst[2*i] = c2, dst[2*i+1] = c1;
   }
   dst[2*i] = 0; trim(dst);
}

void delstr_spaces(char *dst, char *src)
{
   for (; *src; src++)
      if (*src != ' ') *dst++ = *src;
   *dst = 0;
}

void make_ata_string(unsigned char *dst, unsigned n_words, char *src)
{
   for (unsigned i = 0; i < n_words*2 && src[i]; i++) dst[i] = src[i];
   while (i < n_words*2) dst[i++] = ' ';
   unsigned char tmp;
   for (i = 0; i < n_words*2; i += 2)
      tmp = dst[i], dst[i] = dst[i+1], dst[i+1] = tmp;
}

int ATA_DEVICE::pass_through(void *databuf, int bufsize)
{
   memset(databuf, 0, bufsize);

   struct {
      SCSI_PASS_THROUGH_DIRECT p;
      unsigned char sense[0x40];
   } srb = { 0 }, dst;

   srb.p.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
   *(CDB*)&srb.p.Cdb = cdb;
   srb.p.CdbLength = sizeof(CDB);
   srb.p.DataIn = SCSI_IOCTL_DATA_IN;
   srb.p.TimeOutValue = 10;
   srb.p.DataBuffer = databuf;
   srb.p.DataTransferLength = bufsize;
   srb.p.SenseInfoLength = sizeof(srb.sense);
   srb.p.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);

   DWORD outsize;
   int r = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT,
                           &srb.p, sizeof(srb.p),
                           &dst, sizeof(dst),
                           &outsize, 0);

   if (!r) return 0;

   passed_length = dst.p.DataTransferLength;
   if (senselen = dst.p.SenseInfoLength) memcpy(sense, dst.sense, senselen);

#ifdef DUMP_HDD_IO
printf("sense=%d,data=%d,res=%d (%d)\n", senselen, passed_length, outsize, sizeof(srb.p));
printf("srb:"); dump1((BYTE*)&dst, outsize);
printf("data:"); dump1((BYTE*)databuf, 0x40);
#endif

   return 1;
}

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


int ATA_DEVICE::read_atapi_id()
{
   memset(&cdb, 0, sizeof(CDB));
   memset(&transbf, 0, sizeof(transbf));

   INQUIRYDATA inq;
   cdb.CDB6INQUIRY.OperationCode = 0x12; // INQUIRY
   cdb.CDB6INQUIRY.AllocationLength = sizeof(inq);
   if (!pass_through(&inq, sizeof(inq))) return 0;

   char vendor[10], product[18], revision[6], id[22], ata_name[26];

   memcpy(vendor, inq.VendorId, sizeof(inq.VendorId)); vendor[sizeof(inq.VendorId)] = 0;
   memcpy(product, inq.ProductId, sizeof(inq.ProductId)); product[sizeof(inq.ProductId)] = 0;
   memcpy(revision, inq.ProductRevisionLevel, sizeof(inq.ProductRevisionLevel)); revision[sizeof(inq.ProductRevisionLevel)] = 0;
   memcpy(id, inq.VendorSpecific, sizeof(inq.VendorSpecific)); id[sizeof(inq.VendorSpecific)] = 0;

   trim(vendor); trim(product); trim(revision); trim(id);
   sprintf(ata_name, "%s %s", vendor, product);

   transbf[0] = 0xC0; // removable, accelerated DRQ, 12-byte packet
   transbf[1] = 0x85; // protocol: ATAPI, device type: CD-ROM

   make_ata_string(transbf+54, 20, ata_name);
   make_ata_string(transbf+20, 10, id);
   make_ata_string(transbf+46,  4, revision);

   transbf[0x63] = 0x0B; // caps: IORDY,LBA,DMA
   transbf[0x67] = 4; // PIO timing
   transbf[0x69] = 2; // DMA timing

   color(CONSCLR_HARDINFO);
   printf("%-40s %-20s  ", ata_name, id);
   color(CONSCLR_HARDITEM);
   printf("rev.%-4s\n", revision);
   return 1;
}

void init_hdd_cd()
{
   memset(&phys, 0, sizeof phys);
   if (conf.ide_skip_real) return;

   unsigned found = 0, drive;
   for (drive = 0; drive < MAX_PHYS_HD_DRIVES; drive++) {
      char devname[0x200]; sprintf(devname, "\\\\.\\PhysicalDrive%d", drive);
      HANDLE hDevice = CreateFile(devname,
                               GENERIC_READ | GENERIC_WRITE, // R/W required!
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                               0, OPEN_EXISTING, 0, 0);

      DWORD errcode = GetLastError();
      if (hDevice == INVALID_HANDLE_VALUE && errcode == ERROR_FILE_NOT_FOUND) continue;
      color(CONSCLR_HARDITEM); printf("hd%d: ", drive);
      if (hDevice == INVALID_HANDLE_VALUE) { color(CONSCLR_ERROR), printf("access failed\n"); err_win32(errcode); continue; }

      SENDCMDINPARAMS in = { 512 };
      in.irDriveRegs.bCommandReg = ID_CMD;
      struct { SENDCMDOUTPARAMS out; char xx[512]; } res_buffer;
      res_buffer.out.cBufferSize = 512; DWORD sz;

      DISK_GEOMETRY geo;
      int res1 = DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA, &in, sizeof in, &res_buffer, sizeof res_buffer, &sz, 0);
      int res2 = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, 0, 0, &geo, sizeof geo, &sz, 0);
      if (geo.BytesPerSector != 512) { color(CONSCLR_ERROR); printf("unsupported sector size (%d bytes)\n", geo.BytesPerSector); continue; }
      CloseHandle(hDevice);

      if (!res1) { color(CONSCLR_ERROR), printf("identification failed\n"); continue; }

      memcpy(phys.hdd_id[drive], res_buffer.out.bBuffer, 512);
      char model[42], serial[22];
      swap_bytes(model, res_buffer.out.bBuffer+54, 20);
      swap_bytes(serial, res_buffer.out.bBuffer+20, 10);

      unsigned drivesize = geo.Cylinders.LowPart * geo.SectorsPerTrack * geo.TracksPerCylinder;
      phys.hdd_size[drive] = drivesize;

      unsigned shortsize = drivesize / 2; char mult = 'K';
      if (shortsize >= 100000) {
         shortsize /= 1024, mult = 'M';
         if (shortsize >= 100000) shortsize /= 1024, mult = 'G';
      }

      color(CONSCLR_HARDINFO);
      printf("%-40s %-20s ", model, serial);
      color(CONSCLR_HARDITEM);
      printf("%8d %cb\n", shortsize, mult);
      if (drivesize > 0xFFFFFFF) color(CONSCLR_WARNING), printf("     warning! LBA48 is not supported. only first 128GB visible\n", drive);

      found++;
   }

   ATA_DEVICE dev;

   for (drive = 0; drive < MAX_PHYS_CD_DRIVES; drive++) {
      char devname[0x200]; sprintf(devname, "\\\\.\\CdRom%d", drive);
      HANDLE hDevice = CreateFile(devname,
                               GENERIC_READ | GENERIC_WRITE, // R/W required!
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                               0, OPEN_EXISTING, 0, 0);

      DWORD errcode = GetLastError();
      if (hDevice == INVALID_HANDLE_VALUE && errcode == ERROR_FILE_NOT_FOUND) continue;
      color(CONSCLR_HARDITEM); printf("cd%d: ", drive);
      if (hDevice == INVALID_HANDLE_VALUE) { color(CONSCLR_ERROR), printf("access failed\n"); err_win32(errcode); continue; }

      dev.hDevice = hDevice;
      int res = dev.read_atapi_id();
      CloseHandle(hDevice); dev.hDevice = INVALID_HANDLE_VALUE;
      if (!res) { color(CONSCLR_ERROR), printf("identification failed\n"); continue; }
      memcpy(phys.hdd_id[MAX_PHYS_HD_DRIVES+drive], dev.transbf, 512);
      found++;
   }

   if (!found) errmsg("HDD/CD emulator can't access physical drives");
}

void print_device_name(char *dst, int drive)
{
   unsigned char *idsec = phys.hdd_id[drive];
   char model[42], serial[22];
   swap_bytes(model, idsec+54, 20);
   swap_bytes(serial, idsec+20, 10);
   sprintf(dst, "<%s,%s>", model, serial);
}

unsigned find_hdd_device(char *name)
{
   for (unsigned drive = 0; drive < MAX_PHYS_HD_DRIVES+MAX_PHYS_CD_DRIVES; drive++) {
      if (!phys.hdd_id[drive][0]) continue;
      char combo[128]; print_device_name(combo, drive);
      char s1[512], s2[512]; delstr_spaces(s1, combo), delstr_spaces(s2, name);
      if (!stricmp(s1,s2)) return drive;
   }
   return -1;
}

void ATA_DEVICE::configure(IDE_CONFIG *cfg)
{
   if (hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice), hDevice = INVALID_HANDLE_VALUE;
   c = cfg->c, h = cfg->h, s = cfg->s, lba = cfg->lba; readonly = cfg->readonly;
   phys_dev = -1; atapi = 0;
   if (!*cfg->image) return;
   char *devname = cfg->image; char xx[32]; DWORD open_mode;
   if (devname[0] == '<') { // find physical drive
      unsigned drive = find_hdd_device(devname);
      if (drive < MAX_PHYS_HD_DRIVES) {
         sprintf(devname = xx, "\\\\.\\PhysicalDrive%d", drive);
         // read geometry from id sector
         c = *(unsigned short*)(phys.hdd_id[drive]+2);
         h = *(unsigned short*)(phys.hdd_id[drive]+6);
         s = *(unsigned short*)(phys.hdd_id[drive]+12);
         lba = *(unsigned*)(phys.hdd_id[drive]+0x78);
         if (!lba) lba = c*h*s;
      } else if (drive < MAX_PHYS_HD_DRIVES + MAX_PHYS_CD_DRIVES) {
         sprintf(devname = xx, "\\\\.\\CdRom%d", drive - MAX_PHYS_HD_DRIVES);
         atapi = 1;
      } else { errmsg("no physical device %s", cfg->image); *cfg->image = 0; return; }

      phys_dev = drive; open_mode = OPEN_EXISTING;
   } else
      open_mode = OPEN_ALWAYS;

   hDevice = CreateFile(devname,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          0, open_mode, 0, 0);
   if (hDevice == INVALID_HANDLE_VALUE) {
      errmsg("failed to open %s", cfg->image);
      err_win32();
      *cfg->image = 0;
   }
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
   if (hDevice == INVALID_HANDLE_VALUE || ((reg.devhead ^ device_id) & 0x10) || (reg.control & CONTROL_nIEN)) return 0xFF;
   return intrq? 0xFF : 0x00;
}

unsigned char ATA_DEVICE::read(unsigned n_reg)
{
   if (hDevice == INVALID_HANDLE_VALUE) return 0xFF;
   if ((reg.devhead ^ device_id) & 0x10) return 0xFF;
   if (n_reg == 7) intrq = 0;
   if (n_reg == 8) n_reg = 7; // read alt.status -> read status
   if (n_reg == 7 || (reg.status & STATUS_BSY)) return reg.status; // BSY=1 or read status
   // BSY = 0
   //// if (reg.status & STATUS_DRQ) return 0xFF;    // DRQ.  ATA-5: registers should not be queried while DRQ=1, but programs do this!
   // DRQ = 0
   return regs[n_reg];
}

unsigned ATA_DEVICE::read_data()
{
   if (hDevice == INVALID_HANDLE_VALUE) return 0xFFFFFFFF;
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
      if (FlushFileBuffers(hDevice)) command_ok(), intrq = 1;
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
   if (hDevice == INVALID_HANDLE_VALUE) return;
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
   if (hDevice == INVALID_HANDLE_VALUE) return;
   if ((reg.devhead ^ device_id) & 0x10) return;
   if (/* (reg.status & (STATUS_DRQ | STATUS_BSY)) != STATUS_DRQ ||*/ transptr >= transcount) return;
   *(unsigned short*)(transbf + transptr*2) = (unsigned short)data; transptr++;
   if (transptr < transcount) return;
   // look to state, prepare next block
   if (state == S_WRITE_SECTORS) { write_sectors(); return; }
   if (state == S_RECV_PACKET) { handle_atapi_packet(); return; }
   if (state == S_MODE_SELECT) { exec_mode_select(); return; }
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
   LARGE_INTEGER offset; offset.QuadPart = ((__int64)pos) << 9;
//printf("[seek %I64d]", offset.QuadPart);
   DWORD code = SetFilePointer(hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
   if (code == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
      intrq = 1;
      reg.status = STATUS_ERR | STATUS_DWF | STATUS_DRDY;
      reg.err = ERR_IDNF;
      return 0;
   }
   return 1;
}

void ATA_DEVICE::write_sectors()
{
   intrq = 1; DWORD sz;
//printf(" [write] ");
   if (!WriteFile(hDevice, transbf, 512, &sz, 0) || sz != 512) {
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
   if (!ReadFile(hDevice, transbf, 512, &sz, 0)) {
      reg.status = 0x51; reg.err = ERR_UNC; state = S_IDLE;
      return;
   }
   if (sz != 512) memset(transbf+sz, 0, 512-sz); // on EOF, or truncated file, read 00
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
   memcpy(&cdb, transbf, 12);

   intrq = 1;

   if (cdb.MODE_SELECT10.OperationCode == 0x55) { // MODE SELECT requires additional data from host

      state = S_MODE_SELECT;
      reg.status = STATUS_DRQ;
      reg.intreason = 0;
      transptr = 0;
      transcount = cdb.MODE_SELECT10.ParameterListLength[0]*0x100 + cdb.MODE_SELECT10.ParameterListLength[1];
      return;
   }

   if (cdb.CDB6READWRITE.OperationCode == 0x03 && senselen) { // REQ.SENSE - read cached
      memcpy(transbf, sense, senselen);
      passed_length = senselen; senselen = 0; // next time read from device
      goto ok;
   }

   if (pass_through(transbf, sizeof(transbf))) {
      if (senselen) { reg.err = sense[2] << 4; goto err; } // err = sense key
    ok:
      if (!passed_length) { command_ok(); return; }
      reg.atapi_count = passed_length;
      reg.intreason = INT_IO;
      reg.status = STATUS_DRQ;
      transcount = (passed_length+1)/2;
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
      memcpy(transbf, phys.hdd_id[phys_dev], 512);
   }

   state = S_READ_ID;
   transptr = 0, transcount = 0x100;
   intrq = 1;
   reg.status = STATUS_DRQ | STATUS_DSC;
   reg.err = 0;
}
