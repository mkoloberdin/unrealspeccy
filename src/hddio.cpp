typedef int (__cdecl *GetASPI32SupportInfo_t)();
typedef int (__cdecl *SendASPI32Command_t)(void *SRB);
const ATAPI_CDB_SIZE = 12; // sizeof(CDB) == 16
const MAX_INFO_LEN = 48;

GetASPI32SupportInfo_t _GetASPI32SupportInfo = 0;
SendASPI32Command_t _SendASPI32Command = 0;
HMODULE hAspiDll = 0;
HANDLE hASPICompletionEvent;


DWORD ATA_PASSER::open(PHYS_DEVICE *dev)
{
   close();

   hDevice = CreateFile(dev->filename,
                GENERIC_READ | GENERIC_WRITE, // R/W required!
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                0, (dev->type == ATA_FILEHDD? OPEN_ALWAYS : OPEN_EXISTING), 0, 0);

   if (hDevice != INVALID_HANDLE_VALUE) return NO_ERROR;
   return GetLastError();;
}

void ATA_PASSER::close()
{
   if (hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice);
   hDevice = INVALID_HANDLE_VALUE;
}

unsigned ATA_PASSER::identify(PHYS_DEVICE *outlist, int max)
{
   int res = 0;
   ATA_PASSER ata;
   for (int drive = 0; drive < MAX_PHYS_HD_DRIVES && res < max; drive++) {

      PHYS_DEVICE *dev = outlist + res;
      dev->type = ATA_NTHDD;
      dev->spti_id = drive;
      sprintf(dev->filename, "\\\\.\\PhysicalDrive%d", dev->spti_id);

      DWORD errcode = ata.open(dev);
      if (errcode == ERROR_FILE_NOT_FOUND) continue;

      color(CONSCLR_HARDITEM); printf("hd%d: ", drive);
      if (errcode != NO_ERROR) { color(CONSCLR_ERROR), printf("access failed\n"); err_win32(errcode); continue; }

      SENDCMDINPARAMS in = { 512 };
      in.irDriveRegs.bCommandReg = ID_CMD;
      struct { SENDCMDOUTPARAMS out; char xx[512]; } res_buffer;
      res_buffer.out.cBufferSize = 512; DWORD sz;

      DISK_GEOMETRY geo;
      int res1 = DeviceIoControl(ata.hDevice, SMART_RCV_DRIVE_DATA, &in, sizeof in, &res_buffer, sizeof res_buffer, &sz, 0);
      int res2 = DeviceIoControl(ata.hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, 0, 0, &geo, sizeof geo, &sz, 0);
      if (geo.BytesPerSector != 512) { color(CONSCLR_ERROR); printf("unsupported sector size (%d bytes)\n", geo.BytesPerSector); continue; }

      ata.close();

      if (!res1) { color(CONSCLR_ERROR), printf("identification failed\n"); continue; }

      memcpy(dev->idsector, res_buffer.out.bBuffer, 512);
      char model[42], serial[22];
      swap_bytes(model, res_buffer.out.bBuffer+54, 20);
      swap_bytes(serial, res_buffer.out.bBuffer+20, 10);

      dev->hdd_size = geo.Cylinders.LowPart * geo.SectorsPerTrack * geo.TracksPerCylinder;
      unsigned shortsize = dev->hdd_size / 2; char mult = 'K';
      if (shortsize >= 100000) {
         shortsize /= 1024, mult = 'M';
         if (shortsize >= 100000) shortsize /= 1024, mult = 'G';
      }

      color(CONSCLR_HARDINFO);
      printf("%-40s %-20s ", model, serial);
      color(CONSCLR_HARDITEM);
      printf("%8d %cb\n", shortsize, mult);
      if (dev->hdd_size > 0xFFFFFFF) color(CONSCLR_WARNING), printf("     warning! LBA48 is not supported. only first 128GB visible\n", drive);

      print_device_name(dev->viewname, dev);
      res++;
   }

   return res;
}

bool ATA_PASSER::seek(unsigned nsector)
{
   LARGE_INTEGER offset; offset.QuadPart = ((__int64)nsector) << 9;
   DWORD code = SetFilePointer(hDevice, offset.LowPart, &offset.HighPart, FILE_BEGIN);
   return (code != INVALID_SET_FILE_POINTER || GetLastError() == NO_ERROR);
}

bool ATA_PASSER::read_sector(unsigned char *dst)
{
   DWORD sz = 0;
   if (!ReadFile(hDevice, dst, 512, &sz, 0)) return false;
   if (sz < 512) memset(dst+sz, 0, 512-sz); // on EOF, or truncated file, read 00
   return true;
}

bool ATA_PASSER::write_sector(unsigned char *src)
{
   DWORD sz = 0;
   return (WriteFile(hDevice, src, 512, &sz, 0) && sz == 512);
}

DWORD ATAPI_PASSER::open(PHYS_DEVICE *dev)
{
   close();
   this->dev = dev;
   if (dev->type == ATA_ASPI_CD) return NO_ERROR;

   hDevice = CreateFile(dev->filename,
                GENERIC_READ | GENERIC_WRITE, // R/W required!
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                0, OPEN_EXISTING, 0, 0);

   if (hDevice != INVALID_HANDLE_VALUE) return NO_ERROR;
   return GetLastError();;
}

void ATAPI_PASSER::close()
{
   if (!dev || dev->type == ATA_ASPI_CD) return;
   if (hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice);
   hDevice = INVALID_HANDLE_VALUE;
}

unsigned ATAPI_PASSER::identify(PHYS_DEVICE *outlist, int max)
{
   int res = 0;
   ATAPI_PASSER atapi;

   if (conf.cd_aspi) {

      init_aspi();


      for (int adapterid = 0; ; adapterid++) {

         SRB_HAInquiry SRB = { 0 };
         SRB.SRB_Cmd        = SC_HA_INQUIRY;
         SRB.SRB_HaId       = (unsigned char)adapterid;
         DWORD ASPIStatus = _SendASPI32Command(&SRB);

         if (ASPIStatus != SS_COMP) break;

         char b1[20], b2[20];
         memcpy(b1, SRB.HA_ManagerId, 16); b1[16] = 0;
         memcpy(b2, SRB.HA_Identifier, 16); b2[16] = 0;

         if (adapterid == 0) {
            color(CONSCLR_HARDITEM); printf("using ");
            color(CONSCLR_WARNING); printf("%s", b1);
            color(CONSCLR_HARDITEM); printf(" %s\n", b2);
         }
         if (adapterid >= (int)SRB.HA_Count) break;
         // int maxTargets = (int)SRB.HA_Unique[3]; // always 8 (?)

         for (int targetid = 0; targetid < 8; targetid++) {

            PHYS_DEVICE *dev = outlist + res;
            dev->type = ATA_ASPI_CD;
            dev->adapterid = adapterid; // (int)SRB.HA_SCSI_ID; // does not work with Nero ASPI
            dev->targetid = targetid;

            DWORD errcode = atapi.open(dev);
            if (errcode != NO_ERROR) continue;

            int ok = atapi.read_atapi_id(dev->idsector, 1);
            atapi.close();
            if (ok != 2) continue; // not a CD-ROM

            print_device_name(dev->viewname, dev);
            res++;
         }
      }


   } else {

      for (int drive = 0; drive < MAX_PHYS_CD_DRIVES && res < max; drive++) {

         PHYS_DEVICE *dev = outlist + res;
         dev->type = ATA_SPTI_CD;
         dev->spti_id = drive;
         sprintf(dev->filename, "\\\\.\\CdRom%d", dev->spti_id);

         DWORD errcode = atapi.open(dev);
         if (errcode == ERROR_FILE_NOT_FOUND) continue;

         color(CONSCLR_HARDITEM); printf("cd%d: ", drive);
         if (errcode != NO_ERROR) { color(CONSCLR_ERROR), printf("access failed\n"); err_win32(errcode); continue; }


         int ok = atapi.read_atapi_id(dev->idsector, 0);
         atapi.close();
         if (!ok) { color(CONSCLR_ERROR), printf("identification failed\n"); continue; }
         if (ok < 2) continue; // not a CD-ROM

         print_device_name(dev->viewname, dev);
         res++;
      }
   }

   return res;
}

int ATAPI_PASSER::pass_through(void *databuf, int bufsize)
{
   int res = (conf.cd_aspi)? SEND_ASPI_CMD(databuf, bufsize) : SEND_SPTI_CMD(databuf, bufsize);
   return res;
}

int ATAPI_PASSER::SEND_SPTI_CMD(void *databuf, int bufsize)
{
   memset(databuf, 0, bufsize);

   struct {
      SCSI_PASS_THROUGH_DIRECT p;
      unsigned char sense[MAX_SENSE_LEN];
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
printf("sense=%d, data=%d, srbsz=%d/%d, dir=%d. ok%d\n", senselen, passed_length, outsize, sizeof(srb.p), dst.p.DataIn, res);
printf("srb:"); dump1((BYTE*)&dst, outsize);
printf("data:"); dump1((BYTE*)databuf, 0x40);
#endif

   return 1;
}

int ATAPI_PASSER::read_atapi_id(unsigned char *idsector, char prefix)
{
   memset(&cdb, 0, sizeof(CDB));
   memset(idsector, 0, 512);

   INQUIRYDATA inq;
   cdb.CDB6INQUIRY.OperationCode = 0x12; // INQUIRY
   cdb.CDB6INQUIRY.AllocationLength = sizeof(inq);
   if (!pass_through(&inq, sizeof(inq))) return 0;

   char vendor[10], product[18], revision[6], id[22], ata_name[26];

   memcpy(vendor, inq.VendorId, sizeof(inq.VendorId)); vendor[sizeof(inq.VendorId)] = 0;
   memcpy(product, inq.ProductId, sizeof(inq.ProductId)); product[sizeof(inq.ProductId)] = 0;
   memcpy(revision, inq.ProductRevisionLevel, sizeof(inq.ProductRevisionLevel)); revision[sizeof(inq.ProductRevisionLevel)] = 0;
   memcpy(id, inq.VendorSpecific, sizeof(inq.VendorSpecific)); id[sizeof(inq.VendorSpecific)] = 0;

   if (prefix) {
      color(CONSCLR_HARDITEM);
      if (dev->type == ATA_ASPI_CD) printf("%d.%d: ", dev->adapterid, dev->targetid);
      if (dev->type == ATA_SPTI_CD) printf("cd%d: ", dev->spti_id);
   }

   trim(vendor); trim(product); trim(revision); trim(id);
   sprintf(ata_name, "%s %s", vendor, product);

   idsector[0] = 0xC0; // removable, accelerated DRQ, 12-byte packet
   idsector[1] = 0x85; // protocol: ATAPI, device type: CD-ROM

   make_ata_string(idsector+54, 20, ata_name);
   make_ata_string(idsector+20, 10, id);
   make_ata_string(idsector+46,  4, revision);

   idsector[0x63] = 0x0B; // caps: IORDY,LBA,DMA
   idsector[0x67] = 4; // PIO timing
   idsector[0x69] = 2; // DMA timing

   if (inq.DeviceType == 5) color(CONSCLR_HARDINFO);
   printf("%-40s %-20s  ", ata_name, id);
   color(CONSCLR_HARDITEM);
   printf("rev.%-4s\n", revision);

   return 1 + (inq.DeviceType == 5);
}

void make_ata_string(unsigned char *dst, unsigned n_words, char *src)
{
   for (unsigned i = 0; i < n_words*2 && src[i]; i++) dst[i] = src[i];
   while (i < n_words*2) dst[i++] = ' ';
   unsigned char tmp;
   for (i = 0; i < n_words*2; i += 2)
      tmp = dst[i], dst[i] = dst[i+1], dst[i+1] = tmp;
}

void swap_bytes(char *dst, BYTE *src, unsigned n_words)
{
   for (unsigned i = 0; i < n_words; i++) {
      char c1 = src[2*i], c2 = src[2*i+1];
      dst[2*i] = c2, dst[2*i+1] = c1;
   }
   dst[2*i] = 0; trim(dst);
}

void print_device_name(char *dst, PHYS_DEVICE *dev)
{
   char model[42], serial[22];
   swap_bytes(model, dev->idsector + 54, 20);
   swap_bytes(serial, dev->idsector + 20, 10);
   sprintf(dst, "<%s,%s>", model, serial);
}

void init_aspi()
{
   if (_SendASPI32Command) return;
   hAspiDll = LoadLibrary("WNASPI32.DLL");
   if (!hAspiDll) { errmsg("failed to load WNASPI32.DLL"); err_win32(); exit(); }
   _GetASPI32SupportInfo = (GetASPI32SupportInfo_t)GetProcAddress(hAspiDll, "GetASPI32SupportInfo");
   _SendASPI32Command = (SendASPI32Command_t)GetProcAddress(hAspiDll, "SendASPI32Command");
   if (!_GetASPI32SupportInfo || !_SendASPI32Command) errexit("invalid ASPI32 library");
   DWORD init = _GetASPI32SupportInfo();
   if (((init >> 8) & 0xFF) != SS_COMP) errexit("error in ASPI32 initialization");
   hASPICompletionEvent = CreateEvent(0,0,0,0);
}

int ATAPI_PASSER::SEND_ASPI_CMD(void *buf, int buf_sz)
{
   SRB_ExecSCSICmd SRB = { 0 };
   SRB.SRB_Cmd        = SC_EXEC_SCSI_CMD;
   SRB.SRB_HaId       = (unsigned char)dev->adapterid;
   SRB.SRB_Flags      = SRB_DIR_IN | SRB_EVENT_NOTIFY | SRB_ENABLE_RESIDUAL_COUNT;
   SRB.SRB_Target     = (unsigned char)dev->targetid;
   SRB.SRB_BufPointer = (unsigned char*)buf;
   SRB.SRB_BufLen     = buf_sz;
   SRB.SRB_SenseLen   = sizeof(SRB.SenseArea);
   SRB.SRB_CDBLen     = ATAPI_CDB_SIZE;
   SRB.SRB_PostProc   = hASPICompletionEvent;
   memcpy(SRB.CDBByte, &cdb, ATAPI_CDB_SIZE);

   /* DWORD ASPIStatus = */ _SendASPI32Command(&SRB);
   passed_length = SRB.SRB_BufLen;

   if (SRB.SRB_Status == SS_PENDING) {
      DWORD ASPIEventStatus = WaitForSingleObject(hASPICompletionEvent, 10000); // timeout 10sec
      if (ASPIEventStatus == WAIT_OBJECT_0) ResetEvent(hASPICompletionEvent);
   }
   if (senselen = SRB.SRB_SenseLen) memcpy(sense, SRB.SenseArea, senselen);

#ifdef DUMP_HDD_IO
printf("sense=%d, data=%d/%d, ok%d\n", senselen, passed_length, buf_sz, SRB.SRB_Status);
printf("srb:"); dump1((BYTE*)&SRB, sizeof(SRB));
printf("data:"); dump1((BYTE*)buf, 0x40);
#endif

   return (SRB.SRB_Status == SS_COMP);
}

void done_aspi()
{
   if (!hAspiDll) return;
   FreeLibrary(hAspiDll);
   CloseHandle(hASPICompletionEvent);
}
