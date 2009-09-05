namespace z80gs
{
unsigned char *gsbankr[4] = { ROM_GS_M, GSRAM_M + PAGE, ROM_GS_M, ROM_GS_M+PAGE };
unsigned char *gsbankw[4] = { TRASH_M, GSRAM_M + PAGE, TRASH_M, TRASH_M };

unsigned gs_v[4];
unsigned char gsvol[4], gsbyte[4];
unsigned led_gssum[4], led_gscnt[4];
unsigned char gsdata_in, gsdata_out, gspage = 0;
unsigned char gscmd, gsstat;

unsigned mult_gs, mult_gs2;

unsigned __int64 gs_t_states; // inc'ed with GSCPUINT every gs int
unsigned __int64 gscpu_t_at_frame_start; // gs_t_states+gscpu.t when spectrum frame begins

const GSCPUFQ = 12000000; // hz
const GSINTFQ = 37500; // hz
const GSCPUFQI = GSCPUFQ/50;
const GSCPUINT = GSCPUFQ/GSINTFQ;
const MULT_GS_SHIFT = 12; // cpu tick -> gscpu tick precision
void flush_gs_z80();

void apply_gs()
{
   mult_gs = (temp.snd_frame_ticks << MULT_C)/GSCPUFQI;
   mult_gs2 = (GSCPUFQI<<MULT_GS_SHIFT)/conf.frame;
}

inline void flush_gs_sound()
{
   if (temp.sndblock) return;
   sound.update(gscpu.t + (unsigned) (gs_t_states - gscpu_t_at_frame_start), gs_v[0] + gs_v[1], gs_v[2] + gs_v[3]);
   for (int ch = 0; ch < 4; ch++) {
      gsleds[ch].level = led_gssum[ch] * gsvol[ch] / (led_gscnt[ch]*(0x100*0x40/16)+1);
      led_gssum[ch] = led_gscnt[ch] = 0;
      gsleds[ch].attrib = 0x0F;
   }
}

inline void init_gs_frame()
{
   gscpu_t_at_frame_start = gs_t_states + gscpu.t;
   sound.start_frame();
}

inline void flush_gs_frame()
{
   flush_gs_z80();
   sound.end_frame(gscpu.t + (unsigned) (gs_t_states - gscpu_t_at_frame_start));
}

__inline void out_gs(unsigned port, unsigned char val)
{
   flush_gs_z80();
   if ((unsigned char)port == 0xBB) gscmd = val, gsstat |= 0x01;
   else gsdata_out = val, gsstat |= 0x80;
}

__inline unsigned char in_gs(unsigned port)
{
   flush_gs_z80();
   if ((unsigned char)port == 0xBB) return gsstat | 0x7E;
   gsstat &= 0x7F; return gsdata_in;
}

void gs_byte_to_dac(unsigned addr, unsigned char byte)
{
   flush_gs_sound();
   unsigned chan = (addr>>8) & 3;
   gsbyte[chan] = byte;
   gs_v[chan] = (gsbyte[chan] * gs_vfx[gsvol[chan]]) >> 8;
   led_gssum[chan] += byte;
   led_gscnt[chan]++;
}

Z80INLINE unsigned char rm(unsigned addr)
{
   unsigned char byte = gsbankr[(addr >> 14) & 3][addr & (PAGE-1)];
   if ((addr & 0xE000) == 0x6000) gs_byte_to_dac(addr, byte);
   return byte;
}

Z80INLINE void wm(unsigned addr, unsigned char val)
{
   unsigned char *bank = gsbankw[(addr >> 14) & 3];
#ifndef TRASH_PAGE
   if (!bank) return;
#endif
   bank[addr & (PAGE-1)] = val;
}

Z80INLINE unsigned char m1_cycle(Z80 *cpu)
{
   cpu->r_low++; cpu->t += 4;
   return rm(cpu->pc++);
}

void out(unsigned port, unsigned char val)
{
   switch (port & 0x0F) {
      case 0x00:
         gspage = (val &= 0x0F);
         if (val) {
            gsbankr[2] = gsbankw[2] = GSRAM_M + PAGE*2*(val-1);
            gsbankr[3] = gsbankw[3] = GSRAM_M + PAGE*2*(val-1) + PAGE;
         } else {
            gsbankr[2] = ROM_GS_M;
            gsbankr[3] = ROM_GS_M+PAGE;
            gsbankw[2] = gsbankw[3] = TRASH_M;
         }
         return;
      case 0x02: gsstat &= 0x7F; return;
      case 0x03: gsstat |= 0x80; gsdata_in = val; return;
      case 0x05: gsstat &= 0xFE; return;
      case 0x06: case 0x07: case 0x08: case 0x09:
      {
         flush_gs_sound();
         unsigned chan = (port & 0x0F)-6; val &= 0x3F;
         gsvol[chan] = val;
         gs_v[chan] = (gsbyte[chan] * gs_vfx[gsvol[chan]]) >> 8;
         return;
      }
      case 0x0A: gsstat = (gsstat & 0x7F) | (gspage << 7); return;
      case 0x0B: gsstat = (gsstat & 0xFE) | ((gsvol[0] >> 5) & 1); return;
   }
}

unsigned char in(unsigned port)
{
   switch (port & 0x0F) {
      case 0x01: return gscmd;
      case 0x02: gsstat &= 0x7F; return gsdata_out;
      case 0x03: gsstat |= 0x80; gsdata_in = 0xFF; return 0xFF;
      case 0x04: return gsstat;
      case 0x05: gsstat &= 0xFE; return 0xFF;
      case 0x0A: gsstat = (gsstat & 0x7F) | (gspage << 7); return 0xFF;
      case 0x0B: gsstat = (gsstat & 0xFE) | (gsvol[0] >> 5); return 0xFF;
   }
   return 0xFF;
}

#include "z80/cmd.cpp"

void flush_gs_z80()
{
   unsigned __int64 end = ((cpu.t * mult_gs2) >> MULT_GS_SHIFT); // t*GSCPUFQI/conf.frame;
   end += gscpu_t_at_frame_start;
   for (;;) { // while (gs_t_states+gscpu.t < end)
      int max_gscpu_t = min(GSCPUINT, (int)(end - gs_t_states));
      while ((int)gscpu.t < max_gscpu_t) {
         unsigned char opcode = m1_cycle(&gscpu); // step()
         (normal_opcode[opcode])(&gscpu);
         if (gscpu.halted) {
            unsigned st = (GSCPUINT-gscpu.t-1)/4+1;
            gscpu.t += 4*st;
            gscpu.r_low += st;
            break;
         }
      }
      if (gscpu.t < GSCPUINT) break;
      if (gscpu.iff1 && gscpu.t != gscpu.eipos) // interrupt, but not after EI
         handle_int(&gscpu, 0xFF);
      gscpu.t -= GSCPUINT;
      gs_t_states += GSCPUINT;
   }
}

void reset()
{
   gscpu.reset();
   gscpu.t = 0;
   gs_t_states = 0;
}

} // end of z80gs namespace
