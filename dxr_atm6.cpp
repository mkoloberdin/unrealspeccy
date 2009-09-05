
const text0_ofs = 0x01C0;
const text1_ofs = 0x21C0;
const text2_ofs = -4*PAGE+0x01C1;
const text3_ofs = -4*PAGE+0x21C0;

void line_atm6_8(unsigned char *dst, unsigned char *src, unsigned *tab0, unsigned char *font)
{
   for (unsigned x = 0; x < 640; x += 0x40) {
      unsigned p0 = *(unsigned*)(src + text0_ofs),
               p1 = *(unsigned*)(src + text1_ofs),
               a0 = *(unsigned*)(src + text2_ofs),
               a1 = *(unsigned*)(src + text3_ofs);
      unsigned c, *tab;
      tab = tab0 + ((a1 << 4) & 0xFF0), c = font[p0 & 0xFF];
      *(unsigned*)(dst+x+0x00) = tab[((c >> 4)  & 0xF)];
      *(unsigned*)(dst+x+0x04) = tab[c & 0xF];
      tab = tab0 + ((a0 << 4) & 0xFF0); c = font[p1 & 0xFF];
      *(unsigned*)(dst+x+0x08) = tab[((c >> 4)  & 0xF)];
      *(unsigned*)(dst+x+0x0C) = tab[c & 0xF];
      tab = tab0 + ((a1 >> 4) & 0xFF0); c = font[(p0 >> 8) & 0xFF];
      *(unsigned*)(dst+x+0x10) = tab[((c >> 4) & 0xF)];
      *(unsigned*)(dst+x+0x14) = tab[c & 0xF];
      tab = tab0 + ((a0 >> 4) & 0xFF0); c = font[(p1 >> 8) & 0xFF];
      *(unsigned*)(dst+x+0x18) = tab[((c >> 4) & 0xF)];
      *(unsigned*)(dst+x+0x1C) = tab[c & 0xF];
      tab = tab0 + ((a1 >> 12) & 0xFF0); c = font[(p0 >> 16) & 0xFF];
      *(unsigned*)(dst+x+0x20) = tab[((c >> 4) & 0xF)];
      *(unsigned*)(dst+x+0x24) = tab[c & 0xF];
      tab = tab0 + ((a0 >> 12) & 0xFF0); c = font[(p1 >> 16) & 0xFF];
      *(unsigned*)(dst+x+0x28) = tab[((c >> 4) & 0xF)];
      *(unsigned*)(dst+x+0x2C) = tab[c & 0xF];
      tab = tab0 + ((a1 >> 20) & 0xFF0); c = font[p0 >> 24];
      *(unsigned*)(dst+x+0x30) = tab[((c >> 4) & 0xF)];
      *(unsigned*)(dst+x+0x34) = tab[c & 0xF];
      tab = tab0 + ((a0 >> 20) & 0xFF0); c = font[p1 >> 24];
      *(unsigned*)(dst+x+0x38) = tab[((c >> 4) & 0xF)];
      *(unsigned*)(dst+x+0x3C) = tab[c & 0xF];
      src += 4;
   }
}

void line_atm6_16(unsigned char *dst, unsigned char *src, unsigned *tab0, unsigned char *font)
{
   for (unsigned x = 0; x < 640*2; x += 0x80) {
      unsigned p0 = *(unsigned*)(src + text0_ofs),
               p1 = *(unsigned*)(src + text1_ofs),
               a0 = *(unsigned*)(src + text2_ofs),
               a1 = *(unsigned*)(src + text3_ofs);
      unsigned c, *tab;
      tab = tab0 + ((a1 << 2) & 0x3FC), c = font[p0 & 0xFF];
      *(unsigned*)(dst+x+0x00) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x04) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x08) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x0C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a0 << 2) & 0x3FC); c = font[p1 & 0xFF];
      *(unsigned*)(dst+x+0x10) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x14) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x18) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x1C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a1 >> 6) & 0x3FC); c = font[(p0 >> 8) & 0xFF];
      *(unsigned*)(dst+x+0x20) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x24) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x28) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x2C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a0 >> 6) & 0x3FC); c = font[(p1 >> 8) & 0xFF];
      *(unsigned*)(dst+x+0x30) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x34) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x38) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x3C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a1 >> 14) & 0x3FC); c = font[(p0 >> 16) & 0xFF];
      *(unsigned*)(dst+x+0x40) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x44) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x48) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x4C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a0 >> 14) & 0x3FC); c = font[(p1 >> 16) & 0xFF];
      *(unsigned*)(dst+x+0x50) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x54) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x58) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x5C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a1 >> 22) & 0x3FC); c = font[p0 >> 24];
      *(unsigned*)(dst+x+0x60) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x64) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x68) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x6C) = tab[((c >> 0)  & 0x03)];
      tab = tab0 + ((a0 >> 22) & 0x3FC); c = font[p1 >> 24];
      *(unsigned*)(dst+x+0x70) = tab[((c >> 6)  & 0x03)];
      *(unsigned*)(dst+x+0x74) = tab[((c >> 4)  & 0x03)];
      *(unsigned*)(dst+x+0x78) = tab[((c >> 2)  & 0x03)];
      *(unsigned*)(dst+x+0x7C) = tab[((c >> 0)  & 0x03)];
      src += 4;
   }
}

void line_atm6_32(unsigned char *dst, unsigned char *src, unsigned *tab0, unsigned char *font)
{
   for (unsigned x = 0; x < 640*4; x += 0x80) {
      unsigned c, *tab;
      tab = tab0 + src[text3_ofs]; c = font[src[text0_ofs]];
      *(unsigned*)(dst+x+0x00) = tab[((c << 1) & 0x100)];
      *(unsigned*)(dst+x+0x04) = tab[((c << 2) & 0x100)];
      *(unsigned*)(dst+x+0x08) = tab[((c << 3) & 0x100)];
      *(unsigned*)(dst+x+0x0C) = tab[((c << 4) & 0x100)];
      *(unsigned*)(dst+x+0x10) = tab[((c << 5) & 0x100)];
      *(unsigned*)(dst+x+0x14) = tab[((c << 6) & 0x100)];
      *(unsigned*)(dst+x+0x18) = tab[((c << 7) & 0x100)];
      *(unsigned*)(dst+x+0x1C) = tab[((c << 8) & 0x100)];

      tab = tab0 + src[text2_ofs]; c = font[src[text1_ofs]];
      *(unsigned*)(dst+x+0x20) = tab[((c << 1) & 0x100)];
      *(unsigned*)(dst+x+0x24) = tab[((c << 2) & 0x100)];
      *(unsigned*)(dst+x+0x28) = tab[((c << 3) & 0x100)];
      *(unsigned*)(dst+x+0x2C) = tab[((c << 4) & 0x100)];
      *(unsigned*)(dst+x+0x30) = tab[((c << 5) & 0x100)];
      *(unsigned*)(dst+x+0x34) = tab[((c << 6) & 0x100)];
      *(unsigned*)(dst+x+0x38) = tab[((c << 7) & 0x100)];
      *(unsigned*)(dst+x+0x3C) = tab[((c << 8) & 0x100)];

      tab = tab0 + src[text3_ofs+1]; c = font[src[text0_ofs+1]];
      *(unsigned*)(dst+x+0x40) = tab[((c << 1) & 0x100)];
      *(unsigned*)(dst+x+0x44) = tab[((c << 2) & 0x100)];
      *(unsigned*)(dst+x+0x48) = tab[((c << 3) & 0x100)];
      *(unsigned*)(dst+x+0x4C) = tab[((c << 4) & 0x100)];
      *(unsigned*)(dst+x+0x50) = tab[((c << 5) & 0x100)];
      *(unsigned*)(dst+x+0x54) = tab[((c << 6) & 0x100)];
      *(unsigned*)(dst+x+0x58) = tab[((c << 7) & 0x100)];
      *(unsigned*)(dst+x+0x5C) = tab[((c << 8) & 0x100)];

      tab = tab0 + src[text2_ofs+1]; c = font[src[text1_ofs+1]];
      *(unsigned*)(dst+x+0x60) = tab[((c << 1) & 0x100)];
      *(unsigned*)(dst+x+0x64) = tab[((c << 2) & 0x100)];
      *(unsigned*)(dst+x+0x68) = tab[((c << 3) & 0x100)];
      *(unsigned*)(dst+x+0x6C) = tab[((c << 4) & 0x100)];
      *(unsigned*)(dst+x+0x70) = tab[((c << 5) & 0x100)];
      *(unsigned*)(dst+x+0x74) = tab[((c << 6) & 0x100)];
      *(unsigned*)(dst+x+0x78) = tab[((c << 7) & 0x100)];
      *(unsigned*)(dst+x+0x7C) = tab[((c << 8) & 0x100)];

      src += 2;
   }
}

void r_atm6_8s(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 25; y++)
      for (unsigned v = 0; v < 8; v++) {
         line_atm6_8(dst, temp.base + y*64, t.zctab8[0], fontatm2 + v*0x100); dst += pitch;
      }
}

void r_atm6_8d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 25; y++)
      for (unsigned v = 0; v < 8; v++) {
         line_atm6_8(dst, temp.base + y*64, t.zctab8[0], fontatm2 + v*0x100); dst += pitch;
         line_atm6_8(dst, temp.base + y*64, t.zctab8[1], fontatm2 + v*0x100); dst += pitch;
      }
}

void r_atm6_16s(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 25; y++)
      for (unsigned v = 0; v < 8; v++) {
         line_atm6_16(dst, temp.base + y*64, t.zctab16[0], fontatm2 + v*0x100); dst += pitch;
      }
}

void r_atm6_16d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 25; y++)
      for (unsigned v = 0; v < 8; v++) {
         line_atm6_16(dst, temp.base + y*64, t.zctab16[0], fontatm2 + v*0x100); dst += pitch;
         line_atm6_16(dst, temp.base + y*64, t.zctab16[1], fontatm2 + v*0x100); dst += pitch;
      }
}

void r_atm6_32s(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 25; y++)
      for (unsigned v = 0; v < 8; v++) {
         line_atm6_32(dst, temp.base + y*64, t.zctab32[0], fontatm2 + v*0x100); dst += pitch;
      }
}

void r_atm6_32d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 25; y++)
      for (unsigned v = 0; v < 8; v++) {
         line_atm6_32(dst, temp.base + y*64, t.zctab32[0], fontatm2 + v*0x100); dst += pitch;
         line_atm6_32(dst, temp.base + y*64, t.zctab32[1], fontatm2 + v*0x100); dst += pitch;
      }
}

void rend_atm6(unsigned char *dst, unsigned pitch)
{
   unsigned char *dst2 = dst + (temp.ox-640)*temp.obpp/16;
   if (temp.scy > 200) dst2 += (temp.scy-200)/2*pitch * ((temp.oy > temp.scy)?2:1);
   if (temp.oy > temp.scy && conf.fast_sl) pitch *= 2;

   if (temp.obpp == 8)  { if (conf.fast_sl) rend_atmframe_x2_8s (dst, pitch), r_atm6_8s (dst2, pitch); else rend_atmframe_x2_8d (dst, pitch), r_atm6_8d (dst2, pitch); return; }
   if (temp.obpp == 16) { if (conf.fast_sl) rend_atmframe_x2_16s(dst, pitch), r_atm6_16s(dst2, pitch); else rend_atmframe_x2_16d(dst, pitch), r_atm6_16d(dst2, pitch); return; }
   if (temp.obpp == 32) { if (conf.fast_sl) rend_atmframe_x2_32s(dst, pitch), r_atm6_32s(dst2, pitch); else rend_atmframe_x2_32d(dst, pitch), r_atm6_32d(dst2, pitch); return; }
}
