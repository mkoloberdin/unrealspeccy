
#include "z80t.cpp"

#define Z80_COMMON
   #include "z80n.cpp"
   #include "z80e.cpp"
   #include "z80l.cpp"
   #include "z80x.cpp"
   #include "z80y.cpp"
   #include "z80xl.cpp"
#undef Z80_COMMON

#ifdef MOD_FASTCORE
namespace z80fast
{
   #include "z80_main.cpp"
}
#else
   #define z80fast z80dbg
#endif

#ifdef MOD_DEBUGCORE
namespace z80dbg
{
   #define Z80_DBG
   #include "z80_main.cpp"
   #undef Z80_DBG
}
#else
   #define z80dbg z80fast
#endif

void reset(ROM_MODE mode)
{
   comp.p7FFD = comp.pDFFD = comp.pFDFD = comp.p1FFD = comp.pFF77 = 0;
   comp.t_states = 0; comp.frame_counter = 0;
   comp.aFF77 = 0;
   comp.pEFF7 &= conf.EFF7_mask;

   comp.flags = 0;
   comp.active_ay = 0;

   cpu.reset();
   reset_tape();
   comp.ay[0].reset();
   comp.ay[1].reset();

   #ifdef MOD_GS
   if (conf.sound.gsreset) reset_gs();
   #endif

   #ifdef MOD_VID_VD
   comp.vdbase = 0; comp.pVD = 0;
   #endif

   if (conf.mem_model == MM_ATM710) reset_atm();
   hdd.reset();
   input.atm51.reset();

   if ((!conf.trdos_present && mode == RM_DOS) ||
        (!conf.cache && mode == RM_CACHE)) mode = RM_SOS;
   set_mode(mode);
}
