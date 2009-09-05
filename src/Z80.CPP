
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
   unsigned char oldEFF7 = comp.pEFF7;
   unsigned char oldFE = comp.pFE, oldboder = comp.border_attr;
   cpu.int_flags = cpu.ir_ = cpu.pc = 0; cpu.im = 0;

   memset(&comp, 0, sizeof comp);
   comp.tape.edge_change = 0x7FFFFFFFFFFFFFFF;
   #ifdef MOD_GS
   if (conf.sound.gsreset) reset_gs();
   #endif
   if (conf.mem_model == MM_ATM710) reset_atm();
   hdd.reset();
   input.atm51.reset();

   comp.pEFF7 = oldEFF7 & conf.EFF7_mask;
   comp.pFE = oldFE, comp.border_attr = oldboder;

   if ((!conf.trdos_present && mode == RM_DOS) ||
        (!conf.cache && mode == RM_CACHE)) mode = RM_SOS;
   set_mode(mode);
}
