
#include "z80/init.cpp"

#define Z80_COMMON
   #include "z80/cmd.cpp"
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
   comp.pEFF7 &= conf.EFF7_mask;
   comp.t_states = 0; comp.frame_counter = 0;
   comp.p7FFD = comp.pDFFD = comp.pFDFD = comp.p1FFD = 0;
   comp.aFB = 0x80;

   if (conf.mem_model == MM_ATM710) set_atm_FF77(0,0);
   if (conf.mem_model == MM_ATM450) set_atm_aFE(0x80);

   comp.flags = 0;
   comp.active_ay = 0;

   cpu.reset();
   reset_tape();
   ay[0].reset();
   ay[1].reset();

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
