#define EMUL_DEBUG
#define TRASH_PAGE

struct Z80
{
   unsigned t;
   /*------------------------------*/
   union {
      unsigned pc;
      struct {
         unsigned char pcl;
         unsigned char pch;
      };
   };
   union {
      unsigned sp;
      struct {
         unsigned char spl;
         unsigned char sph;
      };
   };
   union {
      unsigned ir_;
      struct {
         unsigned char r_low;
         unsigned char i;
      };
   };
   union {
      unsigned int_flags;
      struct {
         unsigned char r_hi;
         unsigned char iff1;
         unsigned char iff2;
         unsigned char halted;
      };
   };
   /*------------------------------*/
   union {
      unsigned bc;
      unsigned short bc16;
      struct {
         unsigned char c;
         unsigned char b;
      };
   };
   union {
      unsigned de;
      struct {
         unsigned char e;
         unsigned char d;
      };
   };
   union {
      unsigned hl;
      struct {
         unsigned char l;
         unsigned char h;
      };
   };
   union {
      unsigned af;
      struct {
         unsigned char f;
         unsigned char a;
      };
   };
   /*------------------------------*/
   union {
      unsigned ix;
      struct {
         unsigned char xl;
         unsigned char xh;
      };
   };
   union {
      unsigned iy;
      struct {
         unsigned char yl;
         unsigned char yh;
      };
   };
   /*------------------------------*/
   struct {
      union {
         unsigned bc;
         struct {
            unsigned char c;
            unsigned char b;
         };
      };
      union {
         unsigned de;
         struct {
            unsigned char e;
            unsigned char d;
         };
      };
      union {
         unsigned hl;
         struct {
            unsigned char l;
            unsigned char h;
         };
      };
      union {
         unsigned af;
         struct {
            unsigned char f;
            unsigned char a;
         };
      };
   } alt;
   unsigned eipos, haltpos;
   /*------------------------------*/
   unsigned char im;
   unsigned char tmp0, tmp1, tmp3;
   void reset() { int_flags = ir_ = pc = 0; im = 0; }
};
typedef void (__fastcall *STEPFUNC)(Z80*);

#define CF 0x01
#define NF 0x02
#define PV 0x04
#define F3 0x08
#define HF 0x10
#define F5 0x20
#define ZF 0x40
#define SF 0x80

#define PAGE 0x4000
#define MAX_RAM_PAGES 64        // 1Mb RAM
#define MAX_CACHE_PAGES 2       // 32K cache
#define MAX_MISC_PAGES 1        // trash page
#define MAX_ROM_PAGES 64        // 1Mb

#ifdef MOD_GSZ80
#define MAX_GSROM_PAGES 2       // 32K
#define MAX_GSRAM_PAGES 30      // for gs512 (last 32k unusable)
#else
#define MAX_GSROM_PAGES 0
#define MAX_GSRAM_PAGES 0
#endif

#define MAX_PAGES (MAX_RAM_PAGES + MAX_CACHE_PAGES + MAX_MISC_PAGES + MAX_ROM_PAGES + MAX_GSROM_PAGES + MAX_GSRAM_PAGES)

#define RAM_BASE_M  memory
#define CACHE_M     (RAM_BASE_M + MAX_RAM_PAGES*PAGE)
#define MISC_BASE_M (CACHE_M + MAX_CACHE_PAGES*PAGE)
#define ROM_BASE_M  (MISC_BASE_M + MAX_MISC_PAGES*PAGE)

#ifdef MOD_GSZ80
#define ROM_GS_M    (ROM_BASE_M + MAX_ROM_PAGES*PAGE)
#define GSRAM_M     (ROM_GS_M + MAX_GSROM_PAGES*PAGE)
#endif

#define TRASH_M     (MISC_BASE_M+0*PAGE)


enum IDE_SCHEME { IDE_NONE = 0, IDE_ATM, IDE_NEMO, IDE_NEMO_A8, IDE_SMUC };

enum MEM_MODEL
{
   MM_PENTAGON = 0, MM_MYPENT,
   MM_SCORP, MM_PROFSCORP,
   MM_PROFI,
   MM_ATM450, MM_ATM710,
   MM_KAY,

   N_MM_MODELS
};

enum ROM_MODE { RM_NOCHANGE=0, RM_SOS, RM_DOS, RM_SYS, RM_128, RM_CACHE };

struct
{
   char *fullname, *shortname;
} mem_model[N_MM_MODELS] =
{
                { "PENTAGON", "PENTAGON" },
                { "PENTAGON + GRM SERVICE ROM", "PENTSR" },
                { "ZS SCORPION", "SCORPION" },
                { "ZS SCORPION + PROF ROM", "PROFSCORP" },
                { "PROFI", "PROFI" },
                { "ATM 4.50 (himem only)", "ATM450" },
                { "ATM-TURBO 2+ v7.10", "ATM710" },
                { "Nemo's KAY", "KAY" }
};

typedef void (__fastcall *VOID_FUNC)(void);
typedef void (__fastcall *RENDER_FUNC)(unsigned char*,unsigned);
typedef void (__fastcall *STEPFUNC)(Z80*);

struct RENDER
{
   char *name;
   RENDER_FUNC func;
   char *nick;
   unsigned flags;
};

struct IDE_CONFIG
{
   char image[512];
   unsigned c,h,s,lba;
   unsigned char readonly;
};

enum RSM_MODE { RSM_SIMPLE, RSM_FIR0, RSM_FIR1, RSM_FIR2 };

struct CONFIG
{
   unsigned paper;  // start of paper
   unsigned t_line; // t-states per line
   unsigned frame;  // t-states per frame
   unsigned intfq;  // usually 50Hz
   unsigned nopaper;// hide paper
   __int64 cpufq;
   unsigned ticks_frame; // x86 t-states in frame

   unsigned render, driver, fontsize;

   unsigned soundbuffer, refresh;

   unsigned char flashcolor, noflic, fast_sl, alt_nf;
   unsigned char frameskip, frameskipmax;
   unsigned char flip, fullscr;

   unsigned char lockmouse;
   unsigned char detect_video;
   unsigned char tape_autostart;
   unsigned char bmpshot;

   unsigned char ch_size;
   unsigned char EFF7_mask;
   unsigned char reset_rom;
   unsigned char use_romset;

   unsigned char updateb, bordersize;
   unsigned char even_M1, border_4T;

   unsigned char floatbus, floatdos;
   unsigned char modem_port;
   unsigned char r1; // reserved for modem_scheme

   unsigned char trdos_present, trdos_interleave;
   unsigned char trdos_traps, wd93_nodelay;
   unsigned char trdos_wp[4];

   unsigned char cache;
   unsigned char cmos;
   unsigned char smuc;
   unsigned char ula_preset;

   unsigned char gs_type;
   unsigned char pixelscroll;
   unsigned char ide_skip_real;
   unsigned char sleepidle;

   unsigned char highpriority;
   unsigned char videoscale;

   MEM_MODEL mem_model;
   unsigned ramsize, romsize;

   IDE_SCHEME ide_scheme;
   IDE_CONFIG ide[2];
   struct {
      unsigned fq, ayfq;
      int beeper, micout, micin, ay, aydig,
          covoxFB, covoxDD, sd, gs, bass;
      VOID_FUNC do_sound;
      unsigned char enabled, gsreset, dsprimary;
      unsigned char ay_chip, ay_scheme, ay_stereo, ay_vols, ay_samples;
   } sound;
   struct {
      unsigned firenum;
      unsigned char altlock, fire, firedelay;
      unsigned char paste_hold, paste_release, paste_newline;
      unsigned char mouse, mouseswap, kjoy, keymatrix, joymouse;
      signed char mousescale;
   } input;
   struct {
      unsigned char enabled;
      unsigned char flash_ay_kbd;
      unsigned char perf_t;
      unsigned char reserved1;
      unsigned bandBpp;
      #define NUM_LEDS 7
      unsigned ay;
      unsigned perf;
      unsigned load;
      unsigned input;
      unsigned time;
      unsigned osw;
      unsigned memband;
   } led;
   struct {
      unsigned char mem_swap;
      unsigned char use_pal;
      unsigned char xt_kbd;
      unsigned char reserved1;
   } atm;
   unsigned pal, num_pals;      // selected palette and total number of pals
   unsigned minres;             // min. screen x-resolution
   unsigned scanbright;         // scanlines intensity

   struct {
      unsigned char mix_frames;
      unsigned char mode; // RSM_MODE
   } rsm;

   char sos_rom_path[0x200];
   char dos_rom_path[0x200];
   char zx128_rom_path[0x200];
   char sys_rom_path[0x200];
   char atm_rom_path[0x200];
   char scorp_rom_path[0x200];
   char prof_rom_path[0x200];
   char profi_rom_path[0x200];
   char kay_rom_path[0x200];

   #ifdef MOD_GSZ80
   char gs_rom_path[0x200];
   #endif

   #ifdef MOD_MONITOR
   char sos_labels_path[0x200];
   #endif

   char keyset[64]; // short name of keyboard layout
   char appendboot[0x200];
   char workdir[0x200];
};

struct TEMP
{
   unsigned rflags;    // render_func flags
   unsigned border_add, border_and;   // for scorpion 4T border update
   unsigned char *base, *base_2;  // pointers to Spectrum screen memory
   unsigned char rom_mask, ram_mask;
   unsigned char evenM1_C0; // C0 for scorpion, 00 for pentagon
   unsigned char hi15; // 0 - 16bit color, 1 - 15bit color, 2 - YUY2 colorspace
   unsigned mult_const1;               // for sound mixer
   unsigned mult_const2, mult_const3;           // for ay engine
   unsigned mult_gs, mult_gs2;                  // for GS mixer
   unsigned snd_frame_samples;  // samples / frame
   unsigned snd_frame_ticks;    // sound ticks / frame
   unsigned ay_div, ay_div2; // for digital_ay

   unsigned gx, gy, gdx, gdy; // updating rectangle (used by GDI renderer)
   RECT client;               // updating rectangle (used by DD_blt renderer)
   HDC gdidc;
   unsigned ox, oy, obpp, ofq; // destination video format
   unsigned scx, scy; // multicolor area (320x240 or 384x300), used in MCR renderer
   unsigned odx, ody; // offset to border in surface, used only in flip()
   unsigned rsx, rsy; // screen resolution
   unsigned b_top, b_left, b_right, b_bottom; // border frame used to setup MCR

   struct { // led coords
      unsigned char *ay;
      unsigned char *perf;
      unsigned char *load;
      unsigned char *input;
      unsigned char *time;
      unsigned char *osw;
      unsigned char *memband;
      unsigned char *fdd;

      __int64 tape_started;
      unsigned gssum[4], gscnt[4];
   } led;
   unsigned char profrom_mask;
   unsigned char atm_pal_changed;
   unsigned char mmx; // cpu has MMX instructions
   unsigned char reserved1;

   unsigned char vidblock, sndblock, inputblock, frameskip;
};

extern TEMP temp;
extern unsigned sb_start_frame;

struct SOUND_STREAM
{
   unsigned tick;
   unsigned mix_l, mix_r;
   unsigned s1_l, s1_r;
   unsigned s2_l, s2_r;
   void flush(unsigned endtick);
   __inline unsigned ready_samples();
   __inline void flush_empty() { tick = sb_start_frame + temp.snd_frame_ticks; }
   void clear() { tick = 0; }
   static void copy_to_sndplaybuf(unsigned save_ticks);
};

#pragma pack(push)
#pragma pack(1)
struct AY
{
   union {
      unsigned char reg[16];
      struct {
         unsigned short r_fA, r_fB, r_fC;
         unsigned char r_noise, r_mix;
         unsigned char r_vA, r_vB, r_vC;
         unsigned short r_envT;
         unsigned char r_env;
         unsigned char r_portA, r_portB;
      };
   };
   unsigned fa, fb, fc, fn, fe;
   unsigned bit0, bit1, bit2, bit3, bit4, bit5;
   unsigned t, ta, tb, tc, tn, te, env, denv;
   unsigned bitA, bitB, bitC, bitN, ns;
   struct SOUND_STREAM sound;
   unsigned char r13_reloaded, activereg;

   __inline void flush_sndbuf();
   void flush(unsigned cputick);
   void select(unsigned char nreg);
   void write(unsigned char val);
   void mix_dig();
   void reset();

   enum { CHIP_AY, CHIP_YM, CHIP_MAX };
   enum { SCHEME_NONE = 0, SCHEME_SINGLE, SCHEME_PSEUDO, SCHEME_QUADRO, SCHEME_POS, SCHEME_CHRV, SCHEME_MAX };
};
#pragma pack(pop)

#include "wd93.h"
#include "hdd.h"
#include "input.h"
#include "modem.h"

#ifdef MOD_GSBASS
#include "gshle.h"
#endif

#define EFF7_HWMC       0x01
#define EFF7_512        0x02
#define EFF7_LOCKMEM    0x04
#define EFF7_ROCACHE    0x08
#define EFF7_GIGASCREEN 0x10
#define EFF7_SB         0x20
#define EFF7_384        0x40
#define EFF7_CMOS       0x80

enum SNAP
{
   snNOFILE, snUNKNOWN, snTOOLARGE,
   snSP, snZ80, snSNA_48, snSNA_128,
   snTAP, snTZX, snCSW,
   snHOB, snSCL, snTRD, snFDI, snTD0, snUDI,
};

struct NVRAM
{
   enum EEPROM_STATE { IDLE = 0, RCV_CMD, RCV_ADDR, RCV_DATA, SEND_DATA, RD_ACK };
   unsigned address;
   unsigned char datain, dataout, bitsin, bitsout;
   unsigned char state;
   unsigned char prev;
   unsigned char out;
   unsigned char out_z;

   void write(unsigned char val);
};

struct COMPUTER
{
   unsigned char p7FFD, pFE, pEFF7, pXXXX;
   unsigned char pDFFD, pFDFD, p1FFD, pFF77;
   __int64 t_states; // inc with conf.frame by each frame
   unsigned frame_counter; // inc each frame
   unsigned char pFFF7[8];
   unsigned aFF77;
   unsigned active_ay;

   unsigned char flags;
   unsigned char border_attr;
   unsigned char cmos_addr;
   unsigned char resv1;

   unsigned char pFFBA, p7FBA; // SMUC
   unsigned char res1, res2;

   unsigned char p0F, p1F, p4F, p5F; // soundrive
   struct AY ay[2];
   struct WD1793 wd;
   struct NVRAM nvram;
   struct {
      __int64 edge_change;
      unsigned char *play_pointer; // or NULL if tape stopped
      unsigned char *end_of_tape;  // where to stop tape
      unsigned index;    // current tape block
      unsigned tape_bit;
      struct SOUND_STREAM sound;
   } tape;
   unsigned char atm_pal[0x10];
   unsigned char ide_read, ide_write; // high byte in IDE i/o
   unsigned char profrom_bank;
};

// bits for COMPUTER::flags
#define CF_DOSPORTS  0x01       // tr-dos ports are accessible
#define CF_TRDOS     0x02       // DOSEN trigger
#define CF_SETDOSROM 0x04       // tr-dos ROM become active at #3Dxx
#define CF_LEAVEROM  0x08       // ROM will be closed at pc>#4000 (executing RAM in ATM)
#define CF_CACHEON   0x10       // cache active
#define CF_SYSTEMROM 0x20       // service ROM active (pentagon grm)
#define CF_PROFROM   0x40       // PROF-ROM active

#define TAPE_QUANTUM 64
struct tzx_block
{
   unsigned char *data;
   unsigned datasize;    // data size, in bytes
   unsigned pause;
   union {
      struct {
         unsigned pilot_t, s1_t, s2_t, zero_t, one_t;
         unsigned pilot_len;
      };
      struct {
         unsigned numblocks, numpulses;
      };
      unsigned param;
   };
   unsigned char type; // 0-playable, 1-pulses, 10-20 - info, etc...
   unsigned char crc; // xor of all bytes
   char desc[128];
};

struct SNDVAL {
   union {
      unsigned data;
      struct {
         short left, right;
      };
   };
};

struct virtkeyt {
   const char *name;
   unsigned short virtkey;
};

struct keyports {
  volatile unsigned char *port1, *port2;
  unsigned char mask1, mask2;
};

struct zxkey {
  const char *name;
  volatile unsigned char * const port;
  const unsigned char mask;
};

struct action {
   const char *name;
   void (*func)();
   unsigned short k1, k2, k3, k4;
};


// flags for video filters
                                // misc options
#define RF_BORDER   0x00000002   // no multicolor painter, read directly from spectrum memory
#define RF_MON      0x00000004   // not-flippable surface, show mouse cursor
#define RF_DRIVER   0x00000008   // use options from driver
//#define RF_VSYNC    0x00000010   // force VSYNC
#define RF_GDI      0x00000020   // output to window
#define RF_CLIP     0x00000040   // use DirectDraw clipper for windowed mode
#define RF_OVR      0x00000080   // output to overlay

                                // main area size
#define RF_1X       0x00000000   // 256x192,320x240 with border (default)
#define RF_2X       0x00000100   // default x2
#define RF_3X       0x00000001   // default x3
#define RF_4X       0x00000200   // default x4
#define RF_64x48    0x00000400   // 64x48  (for chunky 4x4)
#define RF_128x96   0x00000800   // 128x96 (for chunky 2x2)

#define RF_8        0x00000000   // 8 bit (default)
#define RF_8BPCH    0x00001000   // 8-bit per color channel. GDI mode => 32-bit surface. 8-bit mode => grayscale palette
#define RF_YUY2     0x00002000   // pixel format: 16bit, YUY2
#define RF_16       0x00004000   // hicolor mode (RGB555/RGB565)
#define RF_32       0x00008000   // true color

#define RF_USEC32   0x00010000  // use c32tab
#define RF_USE32AS16 0x0020000  // c32tab contain hi-color WORDS
#define RF_USEFONT  0x00040000  // use font_tables
#define RF_PALB     0x00080000  // palette for bilinear filter
#define RF_ATMPAL   0x00100000  // use palette from ATM palette registers
#define RF_GRAY     0x00200000  // grayscale palette

#define RF_MONITOR (RF_MON | RF_GDI | RF_2X)
