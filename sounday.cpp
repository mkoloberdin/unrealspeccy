
unsigned voltab[32], stereo[6];
unsigned vols[6][32];

__inline void flush_ay_sndbuf(AY *ay)
{
   unsigned stream_tick = ay->t*temp.mult_const3 >> MULT_C; //(t*temp.snd_frame_ticks*conf.intfq)/(conf.sound.ayfq/8);
   ay->sound.flush(stream_tick);
}

void flush_ay_snd(AY *ay, unsigned cputick)
{
   if (temp.sndblock || !conf.sound.ay) return;
   unsigned ts = (cputick*temp.mult_const2) >> MULT_C_1; //cputick*(conf.sound.ayfq/(8*conf.intfq))/conf.frame;
   if (ay->t >= ts) return;

   unsigned ea = (ay->r_vA & 0x10)? -1 : 0, va = ((ay->r_vA & 0x0F)*2+1) & ~ea;
   unsigned eb = (ay->r_vB & 0x10)? -1 : 0, vb = ((ay->r_vB & 0x0F)*2+1) & ~eb;
   unsigned ec = (ay->r_vC & 0x10)? -1 : 0, vc = ((ay->r_vC & 0x0F)*2+1) & ~ec;

   while (ay->t < ts) {
      ay->t++;
      if (++ay->ta >= ay->fa) ay->ta = 0, ay->bitA ^= -1;
      if (++ay->tb >= ay->fb) ay->tb = 0, ay->bitB ^= -1;
      if (++ay->tc >= ay->fc) ay->tc = 0, ay->bitC ^= -1;
      if (++ay->tn >= ay->fn)
         ay->tn = 0,
         ay->ns = (ay->ns*2+1) ^ (((ay->ns>>16)^(ay->ns>>13)) & 1),
         ay->bitN = 0 - ((ay->ns >> 16) & 1);
      if (++ay->te >= ay->fe) {
         ay->te = 0, ay->env += ay->denv;
         if (ay->env & ~31) {
            unsigned mask = (1<<ay->r_env);
            if (mask & ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<15)))
               ay->env = ay->denv = 0;
            else if (mask & ((1<<8)|(1<<12)))
               ay->env &= 31;
            else if (mask & ((1<<10)|(1<<14)))
               ay->denv = -(int)ay->denv, ay->env = ay->env + ay->denv;
            else ay->env = 31, ay->denv = 0; //11,13
         }
      }

      unsigned en, mix_l, mix_r;

      en = ((ea & ay->env) | va) & ((ay->bitA | ay->bit0) & (ay->bitN | ay->bit3));
      mix_l  = vols[0][en]; mix_r  = vols[1][en];

      en = ((eb & ay->env) | vb) & ((ay->bitB | ay->bit1) & (ay->bitN | ay->bit4));
      mix_l += vols[2][en]; mix_r += vols[3][en];

      en = ((ec & ay->env) | vc) & ((ay->bitC | ay->bit2) & (ay->bitN | ay->bit5));
      mix_l += vols[4][en]; mix_r += vols[5][en];

      if ((mix_l ^ ay->sound.mix_l) | (mix_r ^ ay->sound.mix_r)) {
         flush_ay_sndbuf(ay);
         ay->sound.mix_l = mix_l, ay->sound.mix_r = mix_r;
      }
   }
}

// todo: pass AY* as param, don't use comp.ay
void set_ay_reg(unsigned char reg, unsigned char val)
{
   if (reg >= 0x10) return;

   if ((1 << reg) & ((1<<1)|(1<<3)|(1<<5)|(1<<13))) val &= 0x0F;
   if ((1 << reg) & ((1<<6)|(1<<8)|(1<<9)|(1<<10))) val &= 0x1F;

   if (reg != 13 && comp.ay.reg[reg] == val) return;

   comp.ay.reg[reg] = val;

   if (conf.sound.ay_mix < CHIP_MIX_DIGITAL) flush_ay_snd(&comp.ay, cpu.t);

   switch (reg) {
      case 1:
      case 0:
         comp.ay.fa = (conf.sound.ay_mix < CHIP_MIX_DIGITAL)? comp.ay.r_fA :
                      temp.ay_div/(comp.ay.r_fA+1);
         break;
      case 3:
      case 2:
         comp.ay.fb = (conf.sound.ay_mix < CHIP_MIX_DIGITAL)? comp.ay.r_fB :
                      temp.ay_div/(comp.ay.r_fB+1);
         break;
      case 5:
      case 4:
         comp.ay.fc = (conf.sound.ay_mix < CHIP_MIX_DIGITAL)? comp.ay.r_fC :
                      temp.ay_div/(comp.ay.r_fC+1);
         break;
      case 6:
         comp.ay.fn = (conf.sound.ay_mix < CHIP_MIX_DIGITAL) ?
            val*2 : temp.ay_div/(val+1);
         break;
      case 7:
         comp.ay.bit0 = 0 - ((val>>0) & 1);
         comp.ay.bit1 = 0 - ((val>>1) & 1);
         comp.ay.bit2 = 0 - ((val>>2) & 1);
         comp.ay.bit3 = 0 - ((val>>3) & 1);
         comp.ay.bit4 = 0 - ((val>>4) & 1);
         comp.ay.bit5 = 0 - ((val>>5) & 1);
         break;
      case 11:
      case 12:
         comp.ay.fe = (conf.sound.ay_mix < CHIP_MIX_DIGITAL) ? comp.ay.r_envT
            : (comp.ay.r_envT? temp.ay_div2/(unsigned)comp.ay.r_envT : temp.ay_div2);
         break;
      case 13:
         comp.ay.r13_reloaded = 1;
         comp.ay.te = 0;
         if (comp.ay.r_env & 4)
            comp.ay.env = 0, comp.ay.denv = 1; // attack
         else comp.ay.env = 31, comp.ay.denv = -1; // decay
         break;
   }
}

void swap_vols()
{
   unsigned t[32], sz = sizeof t;
   for (int i = 0; i < 3; i++) {
      memcpy(t, vols[2*i], sz);
      memcpy(vols[2*i], vols[2*i+1], sz);
      memcpy(vols[2*i+1], t, sz);
   }
}
