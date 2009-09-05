
#define MAX_PHYS_DRIVES 8
unsigned char hdd_id[MAX_PHYS_DRIVES][512];
unsigned hdd_size[MAX_PHYS_DRIVES];

void swap_bytes(void *dst0, void *src0, unsigned n_words)
{
   unsigned short *dst = (unsigned short*)dst0, *src = (unsigned short*)src0;
   for (unsigned i = 0; i < n_words; i++)
      dst[i] = (src[i] >> 8) + (src[i] << 8);
   // trim right spaces
   char *txt = (char*)dst0;
   for (int l = n_words*2; l && (!txt[l-1] || txt[l-1] == ' '); l--);
   txt[l] = 0;
}

void delstr_spaces(char *dst, char *src)
{
   for (; *src; src++)
      if (*src != ' ') *dst++ = *src;
   *dst = 0;
}

void init_hdd()
{
   memset(hdd_id, 0, sizeof hdd_id);
   memset(hdd_size, 0, sizeof hdd_size);
   if (conf.ide_skip_real) return;

   unsigned found = 0;
   for (unsigned drive = 0; drive < MAX_PHYS_DRIVES; drive++) {
      char devname[0x200]; sprintf(devname, "\\\\.\\PhysicalDrive%d", drive);
      HANDLE hDevice = CreateFile(devname,
                               GENERIC_READ | GENERIC_WRITE, // R/W required!
                               FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                               0, OPEN_EXISTING, 0, 0);
      if (hDevice == INVALID_HANDLE_VALUE) continue;
      SENDCMDINPARAMS in = { 512 };
      in.irDriveRegs.bCommandReg = ID_CMD;
      struct { SENDCMDOUTPARAMS out; char xx[512]; } res_buffer;
      res_buffer.out.cBufferSize = 512; DWORD sz;

      if (DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA, &in, sizeof in, &res_buffer, sizeof res_buffer, &sz, 0)) {
         memcpy(hdd_id[drive], res_buffer.out.bBuffer, 512);
         char model[42], serial[22];
         swap_bytes(model, res_buffer.out.bBuffer+54, 20);
         swap_bytes(serial, res_buffer.out.bBuffer+20, 10);

         DISK_GEOMETRY geo; DWORD drivesize = 0;
         if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, 0, 0, &geo, sizeof geo, &sz, 0)) {
            if (geo.BytesPerSector == 512) drivesize = geo.Cylinders.LowPart*geo.SectorsPerTrack*geo.TracksPerCylinder;
         }
         hdd_size[drive] = drivesize; drivesize /= 2; char mult = 'K';

         if (drivesize >= 100000) {
            drivesize /= 1024, mult = 'M';
            if (drivesize >= 100000) drivesize /= 1024, mult = 'G';
         }
         printf("hd%d: %-40s %-20s %8d %cb\n", drive, model, serial, drivesize, mult);
         found++;
      }
   }
   if (!found) printf("warning! HDD emulator can't access physical drives\n");
}

void print_device_name(char *dst, int drive)
{
   char model[42], serial[22];
   swap_bytes(model, hdd_id[drive]+54, 20);
   swap_bytes(serial, hdd_id[drive]+20, 10);
   sprintf(dst, "<%s,%s>", model, serial);
}

unsigned find_hdd_device(char *name)
{
   for (unsigned drive = 0; drive < MAX_PHYS_DRIVES; drive++) {
      if (!hdd_size[drive]) continue;
      char combo[128]; print_device_name(combo, drive);
      char s1[512], s2[512]; delstr_spaces(s1, combo), delstr_spaces(s2, name);
      if (!stricmp(s1,s2)) return drive;
   }
   return -1;
}

void ATA_DEVICE::configure(IDE_CONFIG *cfg)
{
   if (hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice), hDevice = INVALID_HANDLE_VALUE;
   c = cfg->c, h = cfg->h, s = cfg->s, lba = cfg->lba; readonly = cfg->readonly; phys_hdd = -1;
   if (!*cfg->image) return;
   char *devname = cfg->image; char xx[32]; DWORD open_mode;
   if (devname[0] == '<') { // find physical drive
      unsigned drive = find_hdd_device(devname);
      if (drive >= MAX_PHYS_DRIVES) { printf("error: no physical drive %s\n", devname); return; }
      sprintf(devname = xx, "\\\\.\\PhysicalDrive%d", drive);
      // read geometry from id sector
      lba = hdd_size[drive]; c = *(unsigned short*)(hdd_id[drive]+2);
      h = *(unsigned short*)(hdd_id[drive]+6); s = *(unsigned short*)(hdd_id[drive]+12);
      phys_hdd = drive; open_mode = OPEN_EXISTING;
   } else open_mode = OPEN_ALWAYS;
   hDevice = CreateFile(devname,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                          0, open_mode, 0, 0);
   if (hDevice == INVALID_HANDLE_VALUE)
      printf("error: failed to open %s: %08X\n", cfg->image, GetLastError());
}

void ATA_PORT::reset()
{
   dev[0].reset();
   dev[1].reset();
}

unsigned char ATA_PORT::read(unsigned reg)
{
//unsigned char val = dev[0].read(reg) & dev[1].read(reg); printf("R%X:%02X ", reg, val); return val;
   return dev[0].read(reg) & dev[1].read(reg);
}

unsigned ATA_PORT::read_data()
{
//unsigned val = dev[0].read_data() & dev[1].read_data(); printf("r%04X ", val & 0xFFFF); return val;
   return dev[0].read_data() & dev[1].read_data();
}

void ATA_PORT::write(unsigned reg, unsigned char data)
{
//printf("R%X=%02X ", reg, data);
   dev[0].write(reg, data);
   dev[1].write(reg, data);
}

void ATA_PORT::write_data(unsigned data)
{
//printf("w%04X ", data & 0xFFFF);
   dev[0].write_data(data);
   dev[1].write_data(data);
}

unsigned char ATA_PORT::read_intrq()
{
//unsigned char i = dev[0].read_intrq() & dev[1].read_intrq(); printf("i%d ", !!i); return i;
   return dev[0].read_intrq() & dev[1].read_intrq();
}

void ATA_DEVICE::reset()
{
   regs[8] = 0;
   intrq = 0;
   command_ok();
}

void ATA_DEVICE::command_ok()
{
   state = S_IDLE;
   transptr = -1;
   regs[1] = 0;
   regs[7] = 0x50;
}

unsigned char ATA_DEVICE::read_intrq()
{
   if (hDevice == INVALID_HANDLE_VALUE || ((regs[6] ^ device_id) & 0x10) || (regs[8] & 2)) return 0xFF;
   return intrq? 0xFF : 0x00;
}

unsigned char ATA_DEVICE::read(unsigned reg)
{
   if (hDevice == INVALID_HANDLE_VALUE) return 0xFF;
   if ((regs[6] ^ device_id) & 0x10) return 0xFF;
   if (reg == 7) intrq = 0;
   if (reg == 8) reg = 7; // read status
   if (reg == 7 || (regs[7] & 0x80)) return regs[7]; // BSY=1 or read status
   // BSY = 0
   //// if (regs[7] & 0x08) return 0xFF;    // DRQ.  ATA-5: registers should not be queried while DRQ=1, but programs do this!
   // DRQ = 0
   return regs[reg];
}

unsigned ATA_DEVICE::read_data()
{
   if (hDevice == INVALID_HANDLE_VALUE) return 0xFFFFFFFF;
   if ((regs[6] ^ device_id) & 0x10) return 0xFFFFFFFF;
   if (/* (regs[7] & 0x88) != 0x08 ||*/ transptr >= 0x100) return 0xFFFFFFFF;
   // DRQ=1, BSY=0, data present
   unsigned result = *(unsigned*)(transbf + transptr*2); transptr++;
   if (transptr < 0x100) return result;
   // look to state, prepare next block
   if (state == S_READ_ID) command_ok();
   if (state == S_READ_SECTORS) {
      if (!--regs[2]) command_ok();
      else { next_sector(); read_sectors(); }
   }

   return result;
}

void ATA_DEVICE::write(unsigned reg, unsigned char data)
{
   if (hDevice == INVALID_HANDLE_VALUE) return;
   if (reg == 1) { reg_feat = data; return; }
   if (reg != 7) { regs[reg] = data; if (regs[8] & 4) reset(); return; }

   // execute command!
   if (((regs[6] ^ device_id) & 0x10) && data != 0x90) return;
   if (!(regs[7] & 0x40)) return; // !DRDY

   regs[1] = 0; intrq = 0;

//{printf(" [");for (int q=1;q<9;q++) printf("-%02X",regs[q]);printf("]\n");}

   if ((data & 0xFE) == 0x20) { read_sectors(); return; } // cmd #21 obsolette, rqd for is-dos

   if ((data & 0xFE) == 0x30 && !readonly) { // cmd #31 obsolette
      if (seek()) state = S_WRITE_SECTORS, regs[7] = 0x18, transptr = 0;
      return;
   }

   if (data == 0xEC) { prepare_id(); return; }

   if (data == 0xE7) { // FLUSH CACHE
      if (FlushFileBuffers(hDevice)) command_ok(), intrq = 1;
      else regs[7] = 0x71; // DF,DRDY,ERR
      return;
   }

   if (data == 0x10) { // ATA-5 drives should not implement recalibration command, but programs expect it
      command_ok();
      intrq = 1;
      return;
   }

   if (data == 0x70) { // ATA-5 drives should not implement seek command, but ATM CP/M expect it
      if (!seek()) return;
      command_ok();
      intrq = 1;
      return;
   }

//printf(" *** unknown cmd %02X *** ", data);
   regs[7] = 0x51; regs[1] = 0x04; // ERR: ABORT
   state = S_IDLE; intrq = 1;
}

void ATA_DEVICE::write_data(unsigned data)
{
   if (hDevice == INVALID_HANDLE_VALUE) return;
   if ((regs[6] ^ device_id) & 0x10) return;
   if (/* (regs[7] & 0x88) != 0x08 ||*/ transptr >= 0x100) return;
   *(unsigned short*)(transbf + transptr*2) = (unsigned short)data; transptr++;
   if (transptr < 0x100) return;
   // look to state, prepare next block
   if (state == S_WRITE_SECTORS) { write_sectors(); return; }

}

char ATA_DEVICE::seek()
{
   unsigned pos;
   if (regs[6] & 0x40) {
      pos = *(unsigned*)(regs+3) & 0x0FFFFFFF;
      if (pos >= lba) { seek_err: regs[7] = 0x41; regs[1] = 0x10; intrq = 1; return 0; }
   } else {
      if (*(unsigned short*)(regs+4) >= c || (unsigned)(regs[6] & 0x0F) >= h || regs[3] > s) goto seek_err;
      pos = (*(unsigned short*)(regs+4) * h + (regs[6] & 0x0F)) * s + regs[3] - 1;
//printf("\nchs %4d/%02d/%02d: %8d ", *(unsigned short*)(regs+4), (regs[6] & 0x0F), regs[3], pos);
   };
   LARGE_INTEGER offset; offset.QuadPart = ((__int64)pos) << 9;
//printf("[seek %I64d]", offset.QuadPart);
   DWORD code = SetFilePointer(hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
   if (code == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) { intrq = 1; regs[7] = 0x61; regs[1] = 0x10; return 0; }
   return 1;
}

void ATA_DEVICE::write_sectors()
{
   intrq = 1; DWORD sz;
//printf(" [write] ");
   if (!WriteFile(hDevice, transbf, 512, &sz, 0) || sz != 512) {
      regs[7] = 0x51; regs[1] = 0x40;
      state = S_IDLE;
      return;
   }
   if (!--regs[2]) { command_ok(); return; }
   next_sector(); if (!seek()) return;
   transptr = 0; state = S_WRITE_SECTORS;
   regs[1] = 0, regs[7] = 0x18;
}

void ATA_DEVICE::read_sectors()
{
   intrq = 1;
   if (!seek()) return;
   DWORD sz = 0;
   if (!ReadFile(hDevice, transbf, 512, &sz, 0)) {
      regs[7] = 0x51; regs[1] = 0x40; state = S_IDLE;
      return;
   }
   if (sz != 512) memset(transbf+sz, 0, 512-sz); // on EOF, or truncated file, read 00
   transptr = 0; state = S_READ_SECTORS;
   regs[1] = 0, regs[7] = 0x18;
}

void ATA_DEVICE::next_sector()
{
   if (regs[6] & 0x40) { *(unsigned*)(regs+3) = (*(unsigned*)(regs+3) & 0xF0000000) + ((*(unsigned*)(regs+3)+1) & 0x0FFFFFFF); return; }
   // need to recalc CHS for every sector, coz ATA registers should contain current position on failure
   if (regs[3] < s) { regs[3]++; return; }
   regs[3] = 1; unsigned char head = (regs[6] & 0x0F) + 1;
   if (head < h) { regs[6] = (regs[6] & 0xF0)+head; return; }
   regs[6] &= 0xF0; (*(unsigned short*)(regs+4))++;
}

void make_ata_string(unsigned char *dst, unsigned n_words, char *src)
{
   for (unsigned i = 0; i < n_words*2 && src[i]; i++) dst[i] = src[i];
   while (i < n_words*2) dst[i++] = ' ';
   unsigned char tmp;
   for (i = 0; i < n_words*2; i += 2)
      tmp = dst[i], dst[i] = dst[i+1], dst[i+1] = tmp;
}

void ATA_DEVICE::prepare_id()
{
   if (phys_hdd == -1) {
      memset(transbf, 0, 512);
      make_ata_string(transbf+54, 20, "UNREAL SPECCY HARD DRIVE IMAGE");
      make_ata_string(transbf+20, 10, "0000");
      make_ata_string(transbf+46,  4, VERS_STRING);
      *(unsigned short*)transbf = 0x045A;
      ((unsigned short*)transbf)[1] = (unsigned short)c;
      ((unsigned short*)transbf)[3] = (unsigned short)h;
      ((unsigned short*)transbf)[6] = (unsigned short)s;
      *(unsigned*)(transbf+61*2) = lba;
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
      memcpy(transbf, hdd_id[phys_hdd], 512);
   }

   state = S_READ_ID;
   transptr = 0; intrq = 1;
   regs[7] = 0x18; regs[1] = 0;
}
