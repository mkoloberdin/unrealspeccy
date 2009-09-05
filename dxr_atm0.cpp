
const ega0_ofs = -4*PAGE;
const ega1_ofs = 0;
const ega2_ofs = -4*PAGE+0x2000;
const ega3_ofs = 0x2000;

void line_atm0_8(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < 640; x += 0x10) {
      *(unsigned*)(dst+x+0x00) = tab[src[ega0_ofs]];
      *(unsigned*)(dst+x+0x04) = tab[src[ega1_ofs]];
      *(unsigned*)(dst+x+0x08) = tab[src[ega2_ofs]];
      *(unsigned*)(dst+x+0x0C) = tab[src[ega3_ofs]];
      src++;
   }
}

void line_atm0_16(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < 640*2; x += 0x20) {
      *(unsigned*)(dst+x+0x00) = tab[0+2*src[ega0_ofs]];
      *(unsigned*)(dst+x+0x04) = tab[1+2*src[ega0_ofs]];
      *(unsigned*)(dst+x+0x08) = tab[0+2*src[ega1_ofs]];
      *(unsigned*)(dst+x+0x0C) = tab[1+2*src[ega1_ofs]];
      *(unsigned*)(dst+x+0x10) = tab[0+2*src[ega2_ofs]];
      *(unsigned*)(dst+x+0x14) = tab[1+2*src[ega2_ofs]];
      *(unsigned*)(dst+x+0x18) = tab[0+2*src[ega3_ofs]];
      *(unsigned*)(dst+x+0x1C) = tab[1+2*src[ega3_ofs]];
      src++;
   }
}

void line_atm0_32(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < 640*4; x += 0x40) {
      *(unsigned*)(dst+x+0x00) = *(unsigned*)(dst+x+0x04) = tab[0+2*src[ega0_ofs]];
      *(unsigned*)(dst+x+0x08) = *(unsigned*)(dst+x+0x0C) = tab[1+2*src[ega0_ofs]];
      *(unsigned*)(dst+x+0x10) = *(unsigned*)(dst+x+0x14) = tab[0+2*src[ega1_ofs]];
      *(unsigned*)(dst+x+0x18) = *(unsigned*)(dst+x+0x1C) = tab[1+2*src[ega1_ofs]];
      *(unsigned*)(dst+x+0x20) = *(unsigned*)(dst+x+0x24) = tab[0+2*src[ega2_ofs]];
      *(unsigned*)(dst+x+0x28) = *(unsigned*)(dst+x+0x2C) = tab[1+2*src[ega2_ofs]];
      *(unsigned*)(dst+x+0x30) = *(unsigned*)(dst+x+0x34) = tab[0+2*src[ega3_ofs]];
      *(unsigned*)(dst+x+0x38) = *(unsigned*)(dst+x+0x3C) = tab[1+2*src[ega3_ofs]];
      src++;
   }
}

void r_atm0_8(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm0_8(dst, temp.base + y*40, t.atm_ega8[0]); dst += pitch;
   }
}

void r_atm0_8d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm0_8(dst, temp.base + y*40, t.atm_ega8[0]); dst += pitch;
      line_atm0_8(dst, temp.base + y*40, t.atm_ega8[1]); dst += pitch;
   }
}

void r_atm0_16(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm0_16(dst, temp.base + y*40, t.atm_ega16[0]); dst += pitch;
   }
}

void r_atm0_16d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm0_16(dst, temp.base + y*40, t.atm_ega16[0]); dst += pitch;
      line_atm0_16(dst, temp.base + y*40, t.atm_ega16[1]); dst += pitch;
   }
}

void r_atm0_32(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm0_32(dst, temp.base + y*40, t.atm_ega32[0]); dst += pitch;
   }
}

void r_atm0_32d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm0_32(dst, temp.base + y*40, t.atm_ega32[0]); dst += pitch;
      line_atm0_32(dst, temp.base + y*40, t.atm_ega32[1]); dst += pitch;
   }
}

void rend_atm0(unsigned char *dst, unsigned pitch)
{
   unsigned char *dst2 = dst + (temp.ox-640)*temp.obpp/16;
   if (temp.scy > 200) dst2 += (temp.scy-200)/2*pitch * ((temp.oy > temp.scy)?2:1);
   if (temp.oy > temp.scy && conf.fast_sl) pitch *= 2;

   if (temp.obpp == 8)  { if (conf.fast_sl) rend_atmframe_x2_8s (dst, pitch), r_atm0_8 (dst2, pitch); else rend_atmframe_x2_8d (dst, pitch), r_atm0_8d (dst2, pitch); return; }
   if (temp.obpp == 16) { if (conf.fast_sl) rend_atmframe_x2_16s(dst, pitch), r_atm0_16(dst2, pitch); else rend_atmframe_x2_16d(dst, pitch), r_atm0_16d(dst2, pitch); return; }
   if (temp.obpp == 32) { if (conf.fast_sl) rend_atmframe_x2_32s(dst, pitch), r_atm0_32(dst2, pitch); else rend_atmframe_x2_32d(dst, pitch), r_atm0_32d(dst2, pitch); return; }
}
