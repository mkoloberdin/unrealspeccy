
unsigned voltab[32], stereo[6];
unsigned vols[6][32];

__inline void AY::flush_sndbuf()
{
   unsigned stream_tick = t*temp.mult_const3 >> MULT_C; //(t*temp.snd_frame_ticks*conf.intfq)/(conf.sound.ayfq/8);
   sound.flush(stream_tick);
}

void AY::flush(unsigned cputick)
{
   if (temp.sndblock || !conf.sound.ay) return;
   unsigned ts = (cputick*temp.mult_const2) >> MULT_C_1; //cputick*(conf.sound.ayfq/(8*conf.intfq))/conf.frame;
   if (t >= ts) return;

   unsigned ea = (r_vA & 0x10)? -1 : 0, va = ((r_vA & 0x0F)*2+1) & ~ea;
   unsigned eb = (r_vB & 0x10)? -1 : 0, vb = ((r_vB & 0x0F)*2+1) & ~eb;
   unsigned ec = (r_vC & 0x10)? -1 : 0, vc = ((r_vC & 0x0F)*2+1) & ~ec;

   while (t < ts) {
      t++;
      if (++ta >= fa) ta = 0, bitA ^= -1;
      if (++tb >= fb) tb = 0, bitB ^= -1;
      if (++tc >= fc) tc = 0, bitC ^= -1;
      if (++tn >= fn)
         tn = 0,
         ns = (ns*2+1) ^ (((ns>>16)^(ns>>13)) & 1),
         bitN = 0 - ((ns >> 16) & 1);
      if (++te >= fe) {
         te = 0, env += denv;
         if (env & ~31) {
            unsigned mask = (1<<r_env);
            if (mask & ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<15)))
               env = denv = 0;
            else if (mask & ((1<<8)|(1<<12)))
               env &= 31;
            else if (mask & ((1<<10)|(1<<14)))
               denv = -(int)denv, env = env + denv;
            else env = 31, denv = 0; //11,13
         }
      }

      unsigned en, mix_l, mix_r;

      en = ((ea & env) | va) & ((bitA | bit0) & (bitN | bit3));
      mix_l  = vols[0][en]; mix_r  = vols[1][en];

      en = ((eb & env) | vb) & ((bitB | bit1) & (bitN | bit4));
      mix_l += vols[2][en]; mix_r += vols[3][en];

      en = ((ec & env) | vc) & ((bitC | bit2) & (bitN | bit5));
      mix_l += vols[4][en]; mix_r += vols[5][en];

      if ((mix_l ^ sound.mix_l) | (mix_r ^ sound.mix_r)) {
         flush_sndbuf();
         sound.mix_l = mix_l, sound.mix_r = mix_r;
      }
   }
}

void AY::select(unsigned char nreg)
{
   if (conf.sound.ay_chip == CHIP_AY) nreg &= 0x0F;
   activereg = nreg;
}

void AY::write(unsigned char val)
{
   if (activereg >= 0x10) return;

   if ((1 << activereg) & ((1<<1)|(1<<3)|(1<<5)|(1<<13))) val &= 0x0F;
   if ((1 << activereg) & ((1<<6)|(1<<8)|(1<<9)|(1<<10))) val &= 0x1F;

   if (activereg != 13 && reg[activereg] == val) return;

   reg[activereg] = val;

   if (!conf.sound.ay_samples) flush(cpu.t);

   switch (activereg) {
      case 1:
      case 0:
         fa = (!conf.sound.ay_samples)? r_fA : temp.ay_div/(r_fA+1);
         break;
      case 3:
      case 2:
         fb = (!conf.sound.ay_samples)? r_fB : temp.ay_div/(r_fB+1);
         break;
      case 5:
      case 4:
         fc = (!conf.sound.ay_samples)? r_fC : temp.ay_div/(r_fC+1);
         break;
      case 6:
         fn = (!conf.sound.ay_samples)? val*2 : temp.ay_div/(val+1);
         break;
      case 7:
         bit0 = 0 - ((val>>0) & 1);
         bit1 = 0 - ((val>>1) & 1);
         bit2 = 0 - ((val>>2) & 1);
         bit3 = 0 - ((val>>3) & 1);
         bit4 = 0 - ((val>>4) & 1);
         bit5 = 0 - ((val>>5) & 1);
         break;
      case 11:
      case 12:
         fe = (!conf.sound.ay_samples) ? r_envT
            : (r_envT? temp.ay_div2/(unsigned)r_envT : temp.ay_div2);
         break;
      case 13:
         r13_reloaded = 1;
         te = 0;
         if (r_env & 4) env = 0, denv = 1; // attack
         else env = 31, denv = -1; // decay
         break;
   }
}

void AY::reset()
{
   for (unsigned char r = 0; r < 16; r++)
      select(r), write(0);
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
