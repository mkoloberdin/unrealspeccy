
struct ATA_DEVICE
{
   HANDLE hDevice;
   unsigned c,h,s,lba;
   unsigned char regs[9]; // reg8 - control (CS1,DA=6)
   unsigned char reg_cmd, reg_feat;
   unsigned char intrq, readonly;
   unsigned char device_id; // 0x00 - master, 0x10 - slave
   unsigned char r1, r2; // reserved

   unsigned char read(unsigned reg);
   void write(unsigned reg, unsigned char data);
   unsigned read_data();
   void write_data(unsigned data);
   unsigned char read_intrq();

   char seek();
   void configure(IDE_CONFIG *cfg);
   void reset();
   void prepare_id();
   void command_ok();
   void next_sector();
   void read_sectors();
   void write_sectors();

   enum HD_STATE
   {
      S_IDLE = 0, S_READ_ID,
      S_READ_SECTORS, S_WRITE_SECTORS
   };

   HD_STATE state;
   unsigned transptr;
   unsigned phys_hdd;

   unsigned char transbf[512];

   ATA_DEVICE() { hDevice = INVALID_HANDLE_VALUE;  reset(); }
   ~ATA_DEVICE() { if (hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice); }
};

struct ATA_PORT
{
   ATA_DEVICE dev[2];
   unsigned char read_high, write_high;

   ATA_PORT() { dev[0].device_id = 0, dev[1].device_id = 0x10; }

   unsigned char read(unsigned reg);
   void write(unsigned reg, unsigned char data);
   unsigned read_data();
   void write_data(unsigned data);
   unsigned char read_intrq();

   void reset();
};
