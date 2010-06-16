#include "std.h"

#include "emul.h"
#include "vars.h"
#include "debug.h"
#include "memory.h"
#include "gsz80.h"
#include "z80.h"

namespace z80fast
{
#include "z80_main.h"
}

#include "sndrender/sndcounter.h"

//#include "sndrender/sndbuffer.cpp"

int fmsoundon0=4; //Alone Coder
int tfmstatuson0=2; //Alone Coder
char pressedit = 0; //Alone Coder

void __fastcall wmdbg(u32 addr, u8 val);
u8 __fastcall rmdbg(u32 addr);
u8 *__fastcall MemDbg(u32 addr);
void __cdecl BankNames(int i, char *Name);
namespace z80dbg
{
void __cdecl step();
__int64 __cdecl delta();
void __cdecl SetLastT();
}

#pragma pack(8)

void out(unsigned port, unsigned char val);
unsigned char in(unsigned port);

Z80INLINE unsigned char m1_cycle(Z80 *cpu)
{
   cpu->r_low++; cpu->t += 4;
   return cpu->MemIf->rm(cpu->pc++);
}

/*
unsigned char TMainZ80::rm(unsigned addr) { return z80fast::rm(addr); }

unsigned char TMainZ80::dbgrm(unsigned addr) { return ::rmdbg(addr); }

void TMainZ80::wm(unsigned addr, unsigned char val) { z80fast::wm(addr, val); }

void TMainZ80::dbgwm(unsigned addr, unsigned char val) { ::wmdbg(addr, val); }
*/
u8 *TMainZ80::DirectMem(unsigned addr) const
{
    return am_r(addr);
}

unsigned char TMainZ80::m1_cycle() { return ::m1_cycle(this); }

unsigned char TMainZ80::in(unsigned port) { return ::in(port); }

void TMainZ80::out(unsigned port, unsigned char val) { ::out(port, val); }

static const TMemIf FastMemIf = { Rm, Wm };
static const TMemIf DbgMemIf = { DbgRm, DbgWm };

TMainZ80 cpu(0, BankNames, z80dbg::step, z80dbg::delta, z80dbg::SetLastT, membits, &FastMemIf, &DbgMemIf);


#ifdef MOD_GSZ80
namespace z80gs
{
    u8 __fastcall dbgrm(u32 addr);
    void __fastcall dbgwm(u32 addr, u8 val);
    u8 *__fastcall MemDbg(u32 addr);
    void __cdecl BankNames(int i, char *Name);
    void __cdecl step();
    __int64 __cdecl delta();
    void __cdecl SetLastT();
    u8 membits[0x10000];

    SNDRENDER sound;
    static const TMemIf FastMemIf = { z80gs::Rm, z80gs::Wm };
    static const TMemIf DbgMemIf = { z80gs::DbgRm, z80gs::DbgWm };
}

TGsZ80 gscpu(1, z80gs::BankNames, z80gs::step, z80gs::delta,
    z80gs::SetLastT, z80gs::membits, &z80gs::FastMemIf, &z80gs::DbgMemIf);
#endif

TCpuMgr CpuMgr;

void TCpuMgr::SwitchCpu()
{
    CurrentCpu++;
    CurrentCpu %= Count;
}

Z80 *TCpuMgr::Cpus[] =
{
  &cpu,
#ifdef MOD_GSZ80
  &gscpu
#endif
};

const int TCpuMgr::Count = _countof(Cpus);
TZ80State TCpuMgr::PrevCpus[TCpuMgr::Count];
int TCpuMgr::CurrentCpu = 0;

#ifdef MOD_GSBASS
GSHLE gs;
#endif

unsigned char dbgbreak = 0;


CONFIG conf;
COMPUTER comp;
TEMP temp;
ATA_PORT hdd;   // not in `comp' - not cleared in reset()
K_INPUT input;
ISA_MODEM modem;

SNDRENDER sound;
SNDCHIP ay[2];
SNDCOUNTER sndcounter;


unsigned char *base_sos_rom, *base_dos_rom, *base_128_rom, *base_sys_rom;


#ifdef CACHE_ALIGNED
CACHE_ALIGNED unsigned char memory[PAGE*MAX_PAGES];
#else // __declspec(align) not available, force QWORD align with old method
__int64 memory__[PAGE*MAX_PAGES/sizeof(__int64)];
unsigned char * const memory = (unsigned char*)memory__;
#endif

#ifdef MOD_VID_VD
CACHE_ALIGNED unsigned char vdmem[4][0x2000];
#endif

unsigned char membits[0x10000];
unsigned char *bankr[4];
unsigned char *bankw[4];
unsigned char cmos[0x100];
unsigned char nvram[0x800];

unsigned sndplaybuf[PLAYBUFSIZE];
unsigned spbsize;

FILE *savesnd;
unsigned char savesndtype; // 0-none,1-wave,2-vtx
unsigned char *vtxbuf; unsigned vtxbufsize, vtxbuffilled;

unsigned char trdos_load, trdos_save, trdos_format, trdos_seek; // for leds
unsigned char needclr; // clear screenbuffer before rendering

HWND wnd; HINSTANCE hIn;

char droppedFile[512];

const TMemModel mem_model[N_MM_MODELS] =
{
    { "PENTAGON", "PENTAGON",                       128,  RAM_128 | RAM_256 | RAM_512 | RAM_1024 },
    { "ZS SCORPION", "SCORPION",                    256,  RAM_256 | RAM_1024 },
    { "ZS SCORPION + PROF ROM", "PROFSCORP",        256,  RAM_256 | RAM_1024 },
    { "PROFI", "PROFI",                             1024, RAM_1024 },
    { "ATM-TURBO v4.50", "ATM450",                  512,  RAM_512 | RAM_1024 },
    { "ATM-TURBO 2+ v7.10", "ATM710",               1024, RAM_128 | RAM_256 | RAM_512 | RAM_1024 },
    { "Nemo's KAY", "KAY",                          256,  RAM_256 | RAM_1024 },
    { "ZX-Spectrum +3", "PLUS3",                    128,  RAM_128 },
};

unsigned char kbdpc[VK_MAX]; // add cells for mouse & joystick
unsigned char kbdpcEX[6]; //Dexus
keyports inports[VK_MAX];

char statusline[128];
unsigned statcnt;

char arcbuffer[0x2000]; // extensions and command lines for archivers
char skiparc[0x400]; // ignore this files in archive

unsigned char exitflag = 0; // avoid call exit() twice

// beta128 vars
unsigned trd_toload = 0; // drive to load
char trd_loaded[4]; // used to get first free drive with no account of autoloaded images
char ininame[0x200];
char helpname[0x200];
unsigned snapsize;

// conditional breakpoints support
unsigned brk_port_in, brk_port_out;
unsigned char brk_port_val;

virtkeyt pckeys[] =
{
   { "ESC", DIK_ESCAPE },
   { "F1", DIK_F1 }, { "F2", DIK_F2 }, { "F3", DIK_F3 },
   { "F4", DIK_F4 }, { "F5", DIK_F5 }, { "F6", DIK_F6 },
   { "F7", DIK_F7 }, { "F8", DIK_F8 }, { "F9", DIK_F9 },
   { "F10", DIK_F10 }, { "F11", DIK_F11 }, { "F12", DIK_F12 },
   { "PRSCR", DIK_SYSRQ }, { "SCLOCK", DIK_SCROLL }, { "PAUSE", DIK_PAUSE },

   { "1", DIK_1 }, { "2", DIK_2 }, { "3", DIK_3 }, { "4", DIK_4 }, { "5", DIK_5 },
   { "6", DIK_6 }, { "7", DIK_7 }, { "8", DIK_8 }, { "9", DIK_9 }, { "0", DIK_0 },

   { "Q", DIK_Q }, { "W", DIK_W }, { "E", DIK_E }, { "R", DIK_R }, { "T", DIK_T },
   { "Y", DIK_Y }, { "U", DIK_U }, { "I", DIK_I }, { "O", DIK_O }, { "P", DIK_P },
   { "A", DIK_A }, { "S", DIK_S }, { "D", DIK_D }, { "F", DIK_F }, { "G", DIK_G },
   { "H", DIK_H }, { "J", DIK_J }, { "K", DIK_K }, { "L", DIK_L },
   { "Z", DIK_Z }, { "X", DIK_X }, { "C", DIK_C }, { "V", DIK_V }, { "B", DIK_B },
   { "N", DIK_N }, { "M", DIK_M },

   { "MINUS", DIK_MINUS }, { "PLUS", DIK_EQUALS }, { "BACK", DIK_BACK },
   { "TAB", DIK_TAB }, { "LB", DIK_LBRACKET }, { "RB", DIK_RBRACKET },
   { "CAPS", DIK_CAPITAL }, { "TIL", DIK_GRAVE }, { "SPACE", DIK_SPACE },
   { "COL", DIK_SEMICOLON }, { "QUOTE", DIK_APOSTROPHE }, { "ENTER", DIK_RETURN },
   { "COMMA", DIK_COMMA }, { "POINT", DIK_PERIOD }, { "SLASH", DIK_SLASH }, { "BACKSL", DIK_BACKSLASH },
   { "SHIFT", DIK_SHIFT }, { "ALT", DIK_MENU }, { "CONTROL", DIK_CONTROL },
   { "LSHIFT", DIK_LSHIFT }, { "LALT", DIK_LMENU }, { "LCONTROL", DIK_LCONTROL },
   { "RSHIFT", DIK_RSHIFT }, { "RALT", DIK_RMENU }, { "RCONTROL", DIK_RCONTROL },

   { "INS", DIK_INSERT }, { "HOME", DIK_HOME }, { "PGUP", DIK_PRIOR },
   { "DEL", DIK_DELETE }, { "END", DIK_END },   { "PGDN", DIK_NEXT },

   { "UP", DIK_UP }, { "DOWN", DIK_DOWN }, { "LEFT", DIK_LEFT }, { "RIGHT", DIK_RIGHT },

   { "NUMLOCK", DIK_NUMLOCK }, { "GRDIV", DIK_DIVIDE },
   { "GRMUL", DIK_MULTIPLY }, { "GRSUB", DIK_SUBTRACT }, { "GRADD", DIK_ADD },

   { "N0", DIK_NUMPAD0 }, { "N1", DIK_NUMPAD1 }, { "N2", DIK_NUMPAD2 },
   { "N3", DIK_NUMPAD3 }, { "N4", DIK_NUMPAD4 }, { "N5", DIK_NUMPAD5 },
   { "N6", DIK_NUMPAD6 }, { "N7", DIK_NUMPAD7 }, { "N8", DIK_NUMPAD8 },
   { "N9", DIK_NUMPAD9 }, { "NP", DIK_DECIMAL },

   { "LMB", VK_LMB }, { "RMB", VK_RMB }, { "MMB", VK_MMB },
   { "MWU", VK_MWU }, { "MWD", VK_MWD },

   { "JLEFT", VK_JLEFT }, { "JRIGHT", VK_JRIGHT },
   { "JUP", VK_JUP }, { "JDOWN", VK_JDOWN }, { "JFIRE", VK_JFIRE },

};
const size_t pckeys_count = _countof(pckeys);

zxkey zxk_default[] =
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

zxkey zxk_bk08[] =
{
   { "KRIGHT", &input.kjoy, ~1 },
   { "KLEFT",  &input.kjoy, ~2 },
   { "KDOWN",  &input.kjoy, ~4 },
   { "KUP",    &input.kjoy, ~8 },
   { "KFIRE",  &input.kjoy, ~16},

   { "ALT", input.kbd+0, ~0x01 },
   { "Z",   input.kbd+0, ~0x02 },
   { "X",   input.kbd+0, ~0x04 },
   { "C",   input.kbd+0, ~0x08 },
   { "V",   input.kbd+0, ~0x10 },
   { "RUS", input.kbd+0, ~0x20 },
   { "SHF", input.kbd+0,  0x7F },

   { "A",   input.kbd+1, ~0x01 },
   { "S",   input.kbd+1, ~0x02 },
   { "D",   input.kbd+1, ~0x04 },
   { "F",   input.kbd+1, ~0x08 },
   { "G",   input.kbd+1, ~0x10 },
   { "BSL", input.kbd+1, ~0x20 },
   { "SL",  input.kbd+1,  0x7F },

   { "Q",   input.kbd+2, ~0x01 },
   { "W",   input.kbd+2, ~0x02 },
   { "E",   input.kbd+2, ~0x04 },
   { "R",   input.kbd+2, ~0x08 },
   { "T",   input.kbd+2, ~0x10 },
   { "CMA", input.kbd+2, ~0x20 },
   { "PNT", input.kbd+2,  0x7F },

   { "1",   input.kbd+3, ~0x01 },
   { "2",   input.kbd+3, ~0x02 },
   { "3",   input.kbd+3, ~0x04 },
   { "4",   input.kbd+3, ~0x08 },
   { "5",   input.kbd+3, ~0x10 },
   { "TIL", input.kbd+3, ~0x20 },
   { "TAB", input.kbd+3,  0x7F },

   { "0",   input.kbd+4, ~0x01 },
   { "9",   input.kbd+4, ~0x02 },
   { "8",   input.kbd+4, ~0x04 },
   { "7",   input.kbd+4, ~0x08 },
   { "6",   input.kbd+4, ~0x10 },
   { "MNS", input.kbd+4, ~0x20 },
   { "PLS", input.kbd+4,  0x7F },

   { "P",   input.kbd+5, ~0x01 },
   { "O",   input.kbd+5, ~0x02 },
   { "I",   input.kbd+5, ~0x04 },
   { "U",   input.kbd+5, ~0x08 },
   { "Y",   input.kbd+5, ~0x10 },
   { "LB",  input.kbd+5, ~0x20 },
   { "RB",  input.kbd+5,  0x7F },

   { "ENT", input.kbd+6, ~0x01 },
   { "L",   input.kbd+6, ~0x02 },
   { "K",   input.kbd+6, ~0x04 },
   { "J",   input.kbd+6, ~0x08 },
   { "H",   input.kbd+6, ~0x10 },
   { "COL", input.kbd+6, ~0x20 },
   { "QUO", input.kbd+6,  0x7F },

   { "SPC", input.kbd+7, ~0x01 },
   { "CTL", input.kbd+7, ~0x02 },
   { "M",   input.kbd+7, ~0x04 },
   { "N",   input.kbd+7, ~0x08 },
   { "B",   input.kbd+7, ~0x10 },
   { "R/A", input.kbd+7, ~0x20 },
   { "CPS", input.kbd+7,  0x7F }
};

zxkeymap zxk_maps[] =
{
   { "default", zxk_default, _countof(zxk_default) },
   { "BK08", zxk_bk08, _countof(zxk_bk08) }
} ;

const size_t zxk_maps_count = _countof(zxk_maps);

PALETTEENTRY syspalette[0x100];

GDIBMP gdibmp = { { { sizeof BITMAPINFOHEADER, 320, -240, 1, 8, BI_RGB, 0 } } };

PALETTE_OPTIONS pals[32] = {{"default",0x00,0x80,0xC0,0xE0,0xFF,0xC8,0xFF,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF}};

#pragma pack()

unsigned char snbuf[2*1048576]; // large temporary buffer (for reading snapshots)
unsigned char gdibuf[2*1048576];

// on-screen watches block
unsigned watch_script[4][64];
unsigned char watch_enabled[4];
unsigned char used_banks[MAX_PAGES];
unsigned char trace_rom=1, trace_ram=1;

DWORD WinVerMajor;
DWORD WinVerMinor;
HWND dlg; // used in setcheck/getcheck: gui settings, monitor dialogs

HBITMAP hbm;  // bitmap for repaint background
DWORD bm_dx, bm_dy;
DWORD mousepos;  // left-clicked point in monitor
unsigned nowait; // don't close console window after error if started from GUI
bool normal_exit = false; // true on correct exit, false on failure exit

char *ayvols[64]; unsigned num_ayvols;
char *aystereo[64]; unsigned num_aystereo;
char *ulapreset[64]; unsigned num_ula;
char presetbuf[0x4000], *setptr = presetbuf;

/*
#include "fontdata.cpp"
#include "font8.cpp"
#include "font14.cpp"
#include "font16.cpp"
#include "fontatm2.cpp"
*/

const char * const ay_schemes[] = { "no soundchip", "single chip", "pseudo-turbo", "quadro-AY", "turbo-AY // POS", "turbo-sound // NedoPC" };
