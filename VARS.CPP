#pragma pack(8)

Z80 cpu;

#ifdef MOD_GSZ80
Z80 gscpu;
namespace z80gs { SOUND_STREAM sound; }
#endif

#ifdef MOD_GSBASS
GSHLE gs;
#endif

CONFIG conf;
COMPUTER comp;
TEMP temp;
ATA_PORT hdd;   // not in `comp' - not cleared in reset()
K_INPUT input;
ISA_MODEM modem;

unsigned char *base_sos_rom, *base_dos_rom, *base_128_rom, *base_sys_rom;


__int64 memory__[PAGE*MAX_PAGES/sizeof(__int64)];
#define memory ((unsigned char*)memory__)

enum { MEMBITS_R = 1, MEMBITS_W = 2, MEMBITS_BPR = 0x10, MEMBITS_BPW = 0x20, MEMBITS_BPX = 0x40 };
unsigned char membits[0x10000];
unsigned char *bankr[4], *bankw[4];
unsigned char cmos[0x100];
unsigned char nvram[0x800];

#define SNDBUFSIZE 16384
unsigned sndbuf[SNDBUFSIZE];
unsigned sb_readpos, sb_start_frame;

#define PLAYBUFSIZE 16384
unsigned sndplaybuf[PLAYBUFSIZE]; unsigned spbsize;

FILE *savesnd; unsigned char savesndtype; // 0-none,1-wave,2-vtx
unsigned char *vtxbuf; unsigned vtxbufsize, vtxbuffilled;

#define ROMLED_TIME 16
unsigned char trdos_load, trdos_save, trdos_format; // for leds
unsigned char needclr; // clear screenbuffer before rendering

HWND wnd; HINSTANCE hIn;

enum
{
   VK_LMB = 0x101,
   VK_RMB,
   VK_MMB,
   VK_MWU,
   VK_MWD,
   VK_JLEFT,
   VK_JRIGHT,
   VK_JUP,
   VK_JDOWN,
   VK_JFIRE,

   VK_MAX
};

unsigned char kbdpc[VK_MAX]; // add cells for mouse & joystick
keyports inports[VK_MAX];

char statusline[128]; unsigned statcnt;

char arcbuffer[0x2000]; // extensions and command lines for archivers
char skiparc[0x400]; // ignore this files in archive

unsigned char exitflag = 0; // avoid call exit() twice

extern action ac_main[];
extern action ac_main_xt[];
extern action ac_mon[];
extern action ac_regs[];
extern action ac_trace[];
extern action ac_mem[];
extern RENDER renders[];
extern VOID_FUNC prebuffers[];

// beta128 vars
unsigned trd_toload = 0; // next drive to load
char ininame[0x200];
char helpname[0x200];
unsigned snapsize;

// conditional breakpoints support
unsigned brk_port_in, brk_port_out;
unsigned char brk_port_val;
unsigned char dbgbreak = 0, dbgchk = 0;
unsigned dbg_stophere = -1, dbg_stopsp = -1;

virtkeyt pckeys[] =
{
   { "ESC", VK_ESCAPE },
   { "F1", VK_F1 }, { "F2", VK_F2 }, { "F3", VK_F3 },
   { "F4", VK_F4 }, { "F5", VK_F5 }, { "F6", VK_F6 },
   { "F7", VK_F7 }, { "F8", VK_F8 }, { "F9", VK_F9 },
   { "F10", VK_F10 }, { "F11", VK_F11 }, { "F12", VK_F12 },
   { "PRSCR", VK_SNAPSHOT }, { "SCLOCK", VK_SCROLL }, { "PAUSE", VK_PAUSE },

   { "1", '1' }, { "2", '2' }, { "3", '3' }, { "4", '4' }, { "5", '5' },
   { "6", '6' }, { "7", '7' }, { "8", '8' }, { "9", '9' }, { "0", '0' },

   { "Q", 'Q' }, { "W", 'W' }, { "E", 'E' }, { "R", 'R' }, { "T", 'T' },
   { "Y", 'Y' }, { "U", 'U' }, { "I", 'I' }, { "O", 'O' }, { "P", 'P' },
   { "A", 'A' }, { "S", 'S' }, { "D", 'D' }, { "F", 'F' }, { "G", 'G' },
   { "H", 'H' }, { "J", 'J' }, { "K", 'K' }, { "L", 'L' },
   { "Z", 'Z' }, { "X", 'X' }, { "C", 'C' }, { "V", 'V' }, { "B", 'B' },
   { "N", 'N' }, { "M", 'M' },

   { "MINUS", 0xBD }, { "PLUS", 0xBB }, { "BACK", VK_BACK },
   { "TAB", VK_TAB }, { "LB", 0xDB }, { "RB", 0xDD },
   { "CAPS", VK_CAPITAL }, { "TIL", 0xC0 }, { "SPACE", VK_SPACE },
   { "COL", 0xBA }, { "QUOTE", 0xDE }, { "ENTER", VK_RETURN },
   { "COMMA", 0xBC }, { "POINT", 0xBE }, { "SLASH", 0xBF }, { "BACKSL", 0xDC },
   { "SHIFT", VK_SHIFT }, { "ALT", VK_MENU }, { "CONTROL", VK_CONTROL },
   { "LSHIFT", VK_LSHIFT }, { "LALT", VK_LMENU }, { "LCONTROL", VK_LCONTROL },
   { "RSHIFT", VK_RSHIFT }, { "RALT", VK_RMENU }, { "RCONTROL", VK_RCONTROL },

   { "INS", VK_INSERT }, { "HOME", VK_HOME }, { "PGUP", VK_PRIOR },
   { "DEL", VK_DELETE }, { "END", VK_END },   { "PGDN", VK_NEXT },

   { "UP", VK_UP }, { "DOWN", VK_DOWN }, { "LEFT", VK_LEFT }, { "RIGHT", VK_RIGHT },

   { "NUMLOCK", VK_NUMLOCK }, { "GRDIV", VK_DIVIDE },
   { "GRMUL", VK_MULTIPLY }, { "GRSUB", VK_SUBTRACT }, { "GRADD", VK_ADD },

   { "N0", VK_NUMPAD0 }, { "N1", VK_NUMPAD1 }, { "N2", VK_NUMPAD2 },
   { "N3", VK_NUMPAD3 }, { "N4", VK_NUMPAD4 }, { "N5", VK_NUMPAD5 },
   { "N6", VK_NUMPAD6 }, { "N7", VK_NUMPAD7 }, { "N8", VK_NUMPAD8 },
   { "N9", VK_NUMPAD9 }, { "NP", VK_DECIMAL },

   { "LMB", VK_LMB }, { "RMB", VK_RMB }, { "MMB", VK_MMB },
   { "MWU", VK_MWU }, { "MWD", VK_MWD },

   { "JLEFT", VK_JLEFT }, { "JRIGHT", VK_JRIGHT },
   { "JUP", VK_JUP }, { "JDOWN", VK_JDOWN }, { "JFIRE", VK_JFIRE },

};

zxkey zxk[] =
{
   { "KRIGHT", &input.kjoy, ~1 },
   { "KLEFT",  &input.kjoy, ~2 },
   { "KDOWN",  &input.kjoy, ~4 },
   { "KUP",    &input.kjoy, ~8 },
   { "KFIRE",  &input.kjoy, ~16},

   { "ENT", input.kbd+6, ~0x01 },
   { "SPC", input.kbd+7, ~0x01 },
   { "SYM", input.kbd+7, ~0x02 },

   { "CAP", input.kbd+0, ~0x01 },
   { "Z",   input.kbd+0, ~0x02 },
   { "X",   input.kbd+0, ~0x04 },
   { "C",   input.kbd+0, ~0x08 },
   { "V",   input.kbd+0, ~0x10 },

   { "A",   input.kbd+1, ~0x01 },
   { "S",   input.kbd+1, ~0x02 },
   { "D",   input.kbd+1, ~0x04 },
   { "F",   input.kbd+1, ~0x08 },
   { "G",   input.kbd+1, ~0x10 },

   { "Q",   input.kbd+2, ~0x01 },
   { "W",   input.kbd+2, ~0x02 },
   { "E",   input.kbd+2, ~0x04 },
   { "R",   input.kbd+2, ~0x08 },
   { "T",   input.kbd+2, ~0x10 },

   { "1",   input.kbd+3, ~0x01 },
   { "2",   input.kbd+3, ~0x02 },
   { "3",   input.kbd+3, ~0x04 },
   { "4",   input.kbd+3, ~0x08 },
   { "5",   input.kbd+3, ~0x10 },

   { "0",   input.kbd+4, ~0x01 },
   { "9",   input.kbd+4, ~0x02 },
   { "8",   input.kbd+4, ~0x04 },
   { "7",   input.kbd+4, ~0x08 },
   { "6",   input.kbd+4, ~0x10 },

   { "P",   input.kbd+5, ~0x01 },
   { "O",   input.kbd+5, ~0x02 },
   { "I",   input.kbd+5, ~0x04 },
   { "U",   input.kbd+5, ~0x08 },
   { "Y",   input.kbd+5, ~0x10 },

   { "L",   input.kbd+6, ~0x02 },
   { "K",   input.kbd+6, ~0x04 },
   { "J",   input.kbd+6, ~0x08 },
   { "H",   input.kbd+6, ~0x10 },

   { "M",   input.kbd+7, ~0x04 },
   { "N",   input.kbd+7, ~0x08 },
   { "B",   input.kbd+7, ~0x10 },

};

PALETTEENTRY syspalette[0x100];

struct {
   BITMAPINFO header;
   RGBQUAD waste[0x100];
} gdibmp = { { { sizeof BITMAPINFOHEADER, 320, -240, 1, 8, BI_RGB, 0 } } };

struct PALETTE_OPTIONS { // custom palettes
   char name[33];
   unsigned char ZZ,ZN,NN,NB,BB,ZB;
   unsigned char r11,r12,r13,r21,r22,r23,r31,r32,r33;
} pals[32] = {{"default",0x00,0x80,0xC0,0xE0,0xFF,0xC8,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF}};

#pragma pack()

unsigned char snbuf[2*1048576]; // large temporary buffer
unsigned char gdibuf[2*1048576];

// on-screen watches block
unsigned watch_script[4][64];
unsigned char watch_enabled[4];
unsigned char used_banks[MAX_PAGES];
unsigned char trace_rom=1, trace_ram=1;

HWND dlg; // used in setcheck/getcheck: gui settings, monitor dialogs

HBITMAP hbm;  // bitmap for repaint background
DWORD bm_dx, bm_dy;
DWORD mousepos;  // left-clicked point in monitor
unsigned nowait; // don't close console window after error if started from GUI

char *ayvols[64]; unsigned num_ayvols;
char *aystereo[64]; unsigned num_aystereo;
char *ulapreset[64]; unsigned num_ula;
char presetbuf[0x4000], *setptr = presetbuf;

#include "fontdata.cpp"
#include "font8.cpp"
#include "font14.cpp"
#include "font16.cpp"
#include "fontatm2.cpp"
