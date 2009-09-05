
#define TICK_FF 6
#define TICK_F (1<<TICK_FF)
#define MULT_C  14      // ay/cpu/gscpu tick -> sound tick
#define MULT_C_1 16     // cpu tick -> ay tick

// b=1+ln2(48000/10)=1+ln2(max_sndfq/min_intfq); assert(b+TICK_FF+MULT_C <= 32)

double filter_coeff[TICK_F*2] =
{
   // filter designed with Matlab's DSP toolbox
   0.000797243121022152, 0.000815206499600866, 0.000844792477531490, 0.000886460636664257,
   0.000940630171246217, 0.001007677515787512, 0.001087934129054332, 0.001181684445143001,
   0.001289164001921830, 0.001410557756409498, 0.001545998595893740, 0.001695566052785407,
   0.001859285230354019, 0.002037125945605404, 0.002229002094643918, 0.002434771244914945,
   0.002654234457752337, 0.002887136343664226, 0.003133165351783907, 0.003391954293894633,
   0.003663081102412781, 0.003946069820687711, 0.004240391822953223, 0.004545467260249598,
   0.004860666727631453, 0.005185313146989532, 0.005518683858848785, 0.005860012915564928,
   0.006208493567431684, 0.006563280932335042, 0.006923494838753613, 0.007288222831108771,
   0.007656523325719262, 0.008027428904915214, 0.008399949736219575, 0.008773077102914008,
   0.009145787031773989, 0.009517044003286715, 0.009885804729257883, 0.010251021982371376,
   0.010611648461991030, 0.010966640680287394, 0.011314962852635887, 0.011655590776166550,
   0.011987515680350414, 0.012309748033583185, 0.012621321289873522, 0.012921295559959939,
   0.013208761191466523, 0.013482842243062109, 0.013742699838008606, 0.013987535382970279,
   0.014216593638504731, 0.014429165628265581, 0.014624591374614174, 0.014802262449059521,
   0.014961624326719471, 0.015102178534818147, 0.015223484586101132, 0.015325161688957322,
   0.015406890226980602, 0.015468413001680802, 0.015509536233058410, 0.015530130313785910,
   0.015530130313785910, 0.015509536233058410, 0.015468413001680802, 0.015406890226980602,
   0.015325161688957322, 0.015223484586101132, 0.015102178534818147, 0.014961624326719471,
   0.014802262449059521, 0.014624591374614174, 0.014429165628265581, 0.014216593638504731,
   0.013987535382970279, 0.013742699838008606, 0.013482842243062109, 0.013208761191466523,
   0.012921295559959939, 0.012621321289873522, 0.012309748033583185, 0.011987515680350414,
   0.011655590776166550, 0.011314962852635887, 0.010966640680287394, 0.010611648461991030,
   0.010251021982371376, 0.009885804729257883, 0.009517044003286715, 0.009145787031773989,
   0.008773077102914008, 0.008399949736219575, 0.008027428904915214, 0.007656523325719262,
   0.007288222831108771, 0.006923494838753613, 0.006563280932335042, 0.006208493567431684,
   0.005860012915564928, 0.005518683858848785, 0.005185313146989532, 0.004860666727631453,
   0.004545467260249598, 0.004240391822953223, 0.003946069820687711, 0.003663081102412781,
   0.003391954293894633, 0.003133165351783907, 0.002887136343664226, 0.002654234457752337,
   0.002434771244914945, 0.002229002094643918, 0.002037125945605404, 0.001859285230354019,
   0.001695566052785407, 0.001545998595893740, 0.001410557756409498, 0.001289164001921830,
   0.001181684445143001, 0.001087934129054332, 0.001007677515787512, 0.000940630171246217,
   0.000886460636664257, 0.000844792477531490, 0.000815206499600866, 0.000797243121022152
};
unsigned filter_diff[TICK_F*2];
const double filter_sum_full = 1.0, filter_sum_half = 0.5;
const unsigned filter_sum_full_u = (unsigned)(filter_sum_full * 0x10000),
               filter_sum_half_u = (unsigned)(filter_sum_half * 0x10000);

void SOUND_STREAM::flush(unsigned endtick)
{
   endtick += sb_start_frame;

   unsigned scale;
   if (!((endtick ^ tick) & ~(TICK_F-1))) {

      scale = filter_diff[(endtick & (TICK_F-1)) + TICK_F] - filter_diff[(tick & (TICK_F-1)) + TICK_F];
      s2_l += mix_l * scale;
      s2_r += mix_r * scale;

      scale = filter_diff[endtick & (TICK_F-1)] - filter_diff[tick & (TICK_F-1)];
      tick = endtick;
      s1_l += mix_l * scale;
      s1_r += mix_r * scale;

      tick = endtick;

   } else {

      scale = filter_sum_full_u - filter_diff[(tick & (TICK_F-1)) + TICK_F];

      unsigned p = (tick >> TICK_FF) & (SNDBUFSIZE-1);
      sndbuf[p] += ((mix_l*scale + s2_l) >> 16) +
                     ((mix_r*scale + s2_r) & 0xFFFF0000);


      scale = filter_sum_half_u - filter_diff[tick & (TICK_F-1)];
      s2_l = s1_l + mix_l * scale;
      s2_r = s1_r + mix_r * scale;

      tick = (tick | (TICK_F-1))+1; p++;

      if ((endtick ^ tick) & ~(TICK_F-1)) {
         // assume filter_coeff is symmetric
         unsigned val_l = mix_l * filter_sum_half_u;
         unsigned val_r = mix_r * filter_sum_half_u;
         do {
            sndbuf[p] += ((s2_l + val_l) >> 16) +
                           ((s2_r + val_r) & 0xFFFF0000); // save s2+val

            tick += TICK_F; p = (p+1) & (SNDBUFSIZE-1);
            s2_l = val_l, s2_r = val_r; // s2=s1, s1=0;

         } while ((endtick ^ tick) & ~(TICK_F-1));
      }

      tick = endtick;

      scale = filter_diff[(endtick & (TICK_F-1)) + TICK_F] - filter_sum_half_u;
      s2_l += mix_l * scale;
      s2_r += mix_r * scale;

      scale = filter_diff[endtick & (TICK_F-1)];
      s1_l = mix_l * scale;
      s1_r = mix_r * scale;
   }
}

__inline unsigned SOUND_STREAM::ready_samples()
{
   unsigned sz = tick - sb_readpos;
   return sz;
}

void mixChip();
void mixChip2();
#define SAMPLE_SIZE (1024*3)
#define SAMPLE_T    256         // don't change: used for AY/YM too
int waveA[SAMPLE_SIZE], waveB[SAMPLE_SIZE], waveC[SAMPLE_SIZE];

int spkr_dig, mic_dig, covFB_vol, covDD_vol, sd_l, sd_r;
struct SOUND_STREAM sound;

#include "sounday.cpp"

void copy_to_sndplaybuf(unsigned n_samples)
{
   if (n_samples > PLAYBUFSIZE) n_samples = PLAYBUFSIZE;
   spbsize = n_samples * 4;
   unsigned ptr = sb_readpos >> TICK_FF;
   sb_readpos += (n_samples << TICK_FF);
   for (unsigned k = 0; k < n_samples; k++, ptr++) {
      sndplaybuf[k] = sndbuf[ptr & (SNDBUFSIZE-1)];
      sndbuf[ptr & (SNDBUFSIZE-1)] = 0;
   }
}

void flush_dig_snd()
{
   if (temp.sndblock) return;

   unsigned mono = (spkr_dig+mic_dig+covFB_vol+covDD_vol);
   sound.mix_l = mono + sd_l, sound.mix_r = mono + sd_r;

   unsigned endtick = (cpu.t*temp.mult_const1) >> MULT_C; //(cpu.t*temp.snd_frame_ticks)/conf.frame
   sound.flush(endtick);
}

__inline void init_snd_frame()
{
   comp.ay.t = 0;
   #ifdef MOD_GS
   init_gs_frame();
   #endif
}

__inline void flush_snd_frame()
{
   #ifdef MOD_GS
   flush_gs_frame();
   #endif
   if (temp.sndblock) return;
   flush_dig_snd();

   char ay_emulated = 0;
   if (conf.ay_regs) {
      if (conf.sound.ay) {
         if (conf.sound.ay_mix < CHIP_MIX_DIGITAL) {
            flush_ay_snd(&comp.ay, cpu.t);
            flush_ay_sndbuf(&comp.ay);
            if (conf.sound.ay_mix == CHIP_MIX_TURBO) {
               comp.ay2.t = 0;
               swap_vols(); flush_ay_snd(&comp.ay2, cpu.t); swap_vols();
               flush_ay_sndbuf(&comp.ay2);
               memcpy(&comp.ay2, &comp.ay, (int)&comp.ay2.sound - (int)&comp.ay2);
            }
         } else mixChip();
         ay_emulated = 1;
      }
      if (savesndtype == 2) {
         if (!vtxbuf)
            vtxbuf = (unsigned char*)malloc(32768),
            vtxbufsize = 32768, vtxbuffilled = 0;
         if (vtxbuffilled + 14 >= vtxbufsize)
            vtxbufsize += 32768,
            vtxbuf = (unsigned char*)realloc(vtxbuf, vtxbufsize);
         memcpy(vtxbuf+vtxbuffilled, comp.ay.reg, 14);
         if (!comp.ay.r13_reloaded) vtxbuf[vtxbuffilled+13] = 0xFF;
         vtxbuffilled += 14;
      }
      comp.ay.r13_reloaded = 0;
   }

   if (!ay_emulated) comp.ay.sound.flush_empty();

   if (comp.tape.play_pointer) { // play tape pulses
      tape_bit(), comp.tape.sound.mix_l = comp.tape.sound.mix_r = conf.sound.micin & comp.tape.tape_bit;
      comp.tape.sound.flush(temp.snd_frame_ticks);
   } else comp.tape.sound.flush_empty();

   // find size and copy sound buffer

   unsigned save_ticks = temp.snd_frame_ticks; // sound output limit = 1 frame
   save_ticks = min(save_ticks, sound.ready_samples());
   save_ticks = min(save_ticks, comp.ay.sound.ready_samples());
   save_ticks = min(save_ticks, comp.tape.sound.ready_samples());
   #ifdef MOD_GSZ80
   if (conf.gs_type == 1)
      save_ticks = min(save_ticks, z80gs::sound.ready_samples());
   #endif
   sb_start_frame += temp.snd_frame_ticks;
   copy_to_sndplaybuf(save_ticks >> TICK_FF);
}

void mix_dig(AY *ay)
{
   unsigned base = sb_start_frame >> TICK_FF;
   for (unsigned i = 0; i < temp.snd_frame_samples; i++) {

      ay->ta += ay->fa; while (ay->ta >= SAMPLE_SIZE*0x100) ay->ta -= SAMPLE_SIZE*0x100;
      ay->tb += ay->fb; while (ay->tb >= SAMPLE_SIZE*0x100) ay->tb -= SAMPLE_SIZE*0x100;
      ay->tc += ay->fc; while (ay->tc >= SAMPLE_SIZE*0x100) ay->tc -= SAMPLE_SIZE*0x100;
      ay->tn += ay->fn;
      while (ay->tn >= 0x10000) {
         ay->ns = (ay->ns*2+1) ^ (((ay->ns>>16)^(ay->ns>>13)) & 1);
         ay->bitN = 0 - ((ay->ns >> 16) & 1);
         ay->tn -= 0x10000;
      }
      ay->te += ay->fe;
      while (ay->te >= 0x10000) {
         ay->env += ay->denv;
         if (ay->env & ~31) {
            unsigned mask = (1<<comp.ay.r_env);
            if (mask & ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<15)))
               ay->env = ay->denv = 0;
            else if (mask & ((1<<8)|(1<<12)))
               ay->env &= 31;
            else if (mask & ((1<<10)|(1<<14)))
               ay->denv = -(int)ay->denv, ay->env = ay->env + ay->denv;
            else ay->env = 31, ay->denv = 0; //11,13
         }
         ay->te -= 0x10000;
      }

      unsigned left = 0, right = 0, en, vol;

      en = (ay->r_vA & 0x10) ? ay->env : (ay->r_vA & 0x0F)*2+1;
      vol = (ay->bitN | ay->bit3) & (waveA[ay->ta/0x100] | ay->bit0) & 0xFFFF;
      left += vol*vols[0][en], right += vol*vols[1][en];

      en = (ay->r_vB & 0x10) ? ay->env : (ay->r_vB & 0x0F)*2+1;
      vol = (ay->bitN | ay->bit4) & (waveB[ay->tb/0x100] | ay->bit1) & 0xFFFF;
      left += vol*vols[2][en], right += vol*vols[3][en];

      en = (ay->r_vC & 0x10) ? ay->env : (ay->r_vC & 0x0F)*2+1;
      vol = (ay->bitN | ay->bit5) & (waveC[ay->tc/0x100] | ay->bit2) & 0xFFFF;
      left += vol*vols[4][en], right += vol*vols[5][en];

      *(unsigned*)&sndbuf[(i+base) & (SNDBUFSIZE-1)] += (left >> 16) + (right & 0xFFFF0000);
   }
   ay->sound.flush_empty();
}

void mixChip()
{
   mix_dig(&comp.ay);
   if (conf.sound.ay_mix == CHIP_MIX_DIGITAL) return;
   swap_vols(); mix_dig(&comp.ay2); swap_vols();
   memcpy(&comp.ay2, &comp.ay, (int)&comp.ay2.sound - (int)&comp.ay2);
}


#define PI 3.14159265359

double sin1(int i) {
   while (i > SAMPLE_SIZE) i -= SAMPLE_SIZE;
   if (i < SAMPLE_SIZE/2) return (double)i*2/SAMPLE_SIZE;
   return 2-(double)i*2/SAMPLE_SIZE;
}
double cos1(int i) {
   return 1-sin1(i);
}

int *wavs[3] = { waveA, waveB, waveC };
void make_samples() {
   #define cl (0.35)
   #define cl2 (0.25)
   #define clip(x) (((x>cl) ? cl : (x < cl) ? -cl : x)/cl)
   #define clip2(x) ((x < -cl2) ? 0 : (x+cl2))
   for (int i = 0; i < SAMPLE_SIZE; i++) {
      double p1 = 0.8+0.2*sin1(i*4);
      double p2 = 0.7+0.3*cos1(i*2);
      double p3 = 0.9+0.1*sin1(i);
      double t = (double)(i % SAMPLE_T)*2*PI/SAMPLE_T;
//      #define fabs(x) (x)
      waveA[i] = (unsigned)(fabs(p1*clip(1+sin(3*t/2))*0.7+p3*clip(sin(t))+p1*sin(4*t)*0.25+p2*clip2(cos(1+6*t)))*0x3FFF);
      waveB[i] = (unsigned)(fabs(p1*clip(2+sin(3*t/2))*0.7+p3*clip(sin(t))+p1*sin(1+7*t/2)*0.4+p2*clip2(cos(2+5*t)))*0x3FFF);
      waveC[i] = (unsigned)(fabs(p1*clip(0.5+sin(3*t/2))*0.7+p3*clip(sin(t))+p1*sin(0.2+9*t/2)*0.6+p2*clip2(cos(3+5*t)))*0x3FFF);
//      #undef fabs
   }
   #undef clip
   #undef cl
   #undef cl2
   #undef clip2
   for (int ind = 0; ind < 3; ind++) {
      int *arr = wavs[ind], max = -0x7FFFFFFF, min = 0x7FFFFFFF;
      for (int i1 = 0; i1 < SAMPLE_SIZE; i1++) {
         if (arr[i1] > max) max = arr[i1];
         if (arr[i1] < min) min = arr[i1];
      }
      for (i1 = 0; i1 < SAMPLE_SIZE; i1++)
         arr[i1] = (int)(((double)arr[i1] - min)*0x10000/(max-min));
   }
}

void restart_sound()
{
   sb_readpos = sb_start_frame = 0;
   sound.clear();
   comp.ay.sound.clear();
   comp.ay2.sound.clear();
   comp.tape.sound.clear();
   #ifdef MOD_GS
   reset_gs_sound();
   #endif
   memset(sndbuf, 0, sizeof sndbuf);
}

void apply_sound()
{
   load_ay_stereo();
   load_ay_vols();
   #ifdef MOD_GS
   apply_gs();
   #endif
   temp.mult_const1 = (temp.snd_frame_ticks << MULT_C)/conf.frame;
   temp.mult_const2 = ((conf.sound.ayfq/conf.intfq) << (MULT_C_1-3))/conf.frame;
   temp.mult_const3 = TICK_F/2+(unsigned)((__int64)temp.snd_frame_ticks*conf.intfq*(1<<(MULT_C+3))/conf.sound.ayfq);

   temp.ay_div = ((unsigned)((double)conf.sound.ayfq*0x10*(double)SAMPLE_T/(double)conf.sound.fq));
   temp.ay_div2 = (conf.sound.ayfq*0x100)/(conf.sound.fq/32);

   for (int j = 0; j < 6; j++)
      for (int i = 0; i < 32; i++)
         vols[j][i] = voltab[i]*conf.sound.ay/65535*stereo[j]/100;

   comp.ay.ns = 0xFFFF;
   for (int i = 0; i < 14; i++) {
      unsigned char prev_reg = comp.ay.reg[i];
      set_ay_reg(i, prev_reg^1);
      set_ay_reg(i, prev_reg);
   }

   double sum = 0;
   for (i = 0; i < TICK_F*2; i++) {
      filter_diff[i] = (int)(sum * 0x10000);
      sum += filter_coeff[i];
   }
   restart_sound();
}
