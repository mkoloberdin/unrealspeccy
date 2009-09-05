
struct ATA_DEVICE
{
   HANDLE hDevice;
   unsigned c,h,s,lba;
   union {
      unsigned char regs[12];
      struct {
         unsigned char data;
         unsigned char err;             // for write, features
         union {
            unsigned char count;
            unsigned char intreason;
         };
         unsigned char sec;
         union {
            unsigned short cyl;
            unsigned short atapi_count;
            struct {
               unsigned char cyl_l;
               unsigned char cyl_h;
            };
         };
         unsigned char devhead;
         unsigned char status;          // for write, cmd
         /*                  */
         unsigned char control;         // reg8 - control (CS1,DA=6)
         unsigned char feat;
         unsigned char cmd;
         unsigned char reserved;        // reserved
      } reg;
   };
   unsigned char intrq;
   unsigned char readonly;
   unsigned char device_id;             // 0x00 - master, 0x10 - slave
   unsigned char atapi;                 // flag for CD-ROM device

   unsigned char read(unsigned n_reg);
   void write(unsigned n_reg, unsigned char data);
   unsigned read_data();
   void write_data(unsigned data);
   unsigned char read_intrq();

   char exec_ata_cmd(unsigned char cmd);
   char exec_atapi_cmd(unsigned char cmd);

   enum RESET_TYPE { RESET_HARD, RESET_SOFT, RESET_SRST };
   void reset_signature(RESET_TYPE mode = RESET_SOFT);

   void reset(RESET_TYPE mode);
   char seek();
   void configure(IDE_CONFIG *cfg);
   void prepare_id();
   void command_ok();
   void next_sector();
   void read_sectors();
   void write_sectors();

   enum ATAPI_INT_REASON
   {
      INT_COD       = 0x01,
      INT_IO        = 0x02,
      INT_RELEASE   = 0x04
   };

   enum HD_STATUS
   {
      STATUS_BSY   = 0x80,
      STATUS_DRDY  = 0x40,
      STATUS_DWF   = 0x20,
      STATUS_DSC   = 0x10,
      STATUS_DRQ   = 0x08,
      STATUS_CORR  = 0x04,
      STATUS_IDX   = 0x02,
      STATUS_ERR   = 0x01
   };

   enum HD_ERROR
   {
      ERR_BBK   = 0x80,
      ERR_UNC   = 0x40,
      ERR_MC    = 0x20,
      ERR_IDNF  = 0x10,
      ERR_MCR   = 0x08,
      ERR_ABRT  = 0x04,
      ERR_TK0NF = 0x02,
      ERR_AMNF  = 0x01
   };

   enum HD_CONTROL
   {
      CONTROL_SRST = 0x04,
      CONTROL_nIEN = 0x02
   };

   enum HD_STATE
   {
      S_IDLE = 0, S_READ_ID,
      S_READ_SECTORS, S_WRITE_SECTORS,
      S_RECV_PACKET, S_READ_ATAPI,
      S_MODE_SELECT
   };

   HD_STATE state;
   unsigned transptr, transcount;
   unsigned phys_dev;

   CDB cdb;
   unsigned passed_length;

   int pass_through(void *buf2, int bufsize);
   int read_atapi_id();
   void handle_atapi_packet();
   void exec_mode_select();

   unsigned char sense[0x40]; unsigned senselen;
   unsigned char transbf[65535]; // ATAPI is able to tranfer so much data

   ATA_DEVICE() { hDevice = INVALID_HANDLE_VALUE; }
   ~ATA_DEVICE() { if (hDevice != INVALID_HANDLE_VALUE) CloseHandle(hDevice); }
};

struct ATA_PORT
{
   ATA_DEVICE dev[2];
   unsigned char read_high, write_high;

   ATA_PORT() { dev[0].device_id = 0, dev[1].device_id = 0x10; reset(); }

   unsigned char read(unsigned n_reg);
   void write(unsigned n_reg, unsigned char data);
   unsigned read_data();
   void write_data(unsigned data);
   unsigned char read_intrq();

   void reset();
};
