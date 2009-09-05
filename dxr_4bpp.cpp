
// AlCo 4bpp mode

const p4bpp0_ofs = -PAGE;
const p4bpp1_ofs = 0;
const p4bpp2_ofs = -PAGE+0x2000;
const p4bpp3_ofs = 0x2000;

#define p4bpp8_nf p4bpp8

int buf4bpp_shift = 0;

void line_p4bpp_8(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < 512; x += 0x10) {
      *(unsigned*)(dst+x+0x00) = tab[src[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x04) = tab[src[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x08) = tab[src[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x0C) = tab[src[p4bpp3_ofs]];
      src++;
   }
}

void line_p4bpp_8_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   unsigned char *src1 = src, *src2 = src + buf4bpp_shift;
   for (unsigned x = 0; x < 512; x += 0x10) {
      *(unsigned*)(dst+x+0x00) = (tab[src1[p4bpp0_ofs]] & 0x0F0F0F0F) + (tab[src2[p4bpp0_ofs]] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x04) = (tab[src1[p4bpp1_ofs]] & 0x0F0F0F0F) + (tab[src2[p4bpp1_ofs]] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x08) = (tab[src1[p4bpp2_ofs]] & 0x0F0F0F0F) + (tab[src2[p4bpp2_ofs]] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x0C) = (tab[src1[p4bpp3_ofs]] & 0x0F0F0F0F) + (tab[src2[p4bpp3_ofs]] & 0xF0F0F0F0);
      src1++; src2++;
   }
}

void line_p4bpp_16(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < 512*2; x += 0x20) {
      *(unsigned*)(dst+x+0x00) = tab[0+2*src[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x04) = tab[1+2*src[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x08) = tab[0+2*src[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x0C) = tab[1+2*src[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x10) = tab[0+2*src[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x14) = tab[1+2*src[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x18) = tab[0+2*src[p4bpp3_ofs]];
      *(unsigned*)(dst+x+0x1C) = tab[1+2*src[p4bpp3_ofs]];
      src++;
   }
}

void line_p4bpp_16_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   unsigned char *src1 = src, *src2 = src + buf4bpp_shift;
   for (unsigned x = 0; x < 512*2; x += 0x20) {
      *(unsigned*)(dst+x+0x00) = tab[0+2*src1[p4bpp0_ofs]] + tab[0+2*src2[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x04) = tab[1+2*src1[p4bpp0_ofs]] + tab[1+2*src2[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x08) = tab[0+2*src1[p4bpp1_ofs]] + tab[0+2*src2[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x0C) = tab[1+2*src1[p4bpp1_ofs]] + tab[1+2*src2[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x10) = tab[0+2*src1[p4bpp2_ofs]] + tab[0+2*src2[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x14) = tab[1+2*src1[p4bpp2_ofs]] + tab[1+2*src2[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x18) = tab[0+2*src1[p4bpp3_ofs]] + tab[0+2*src2[p4bpp3_ofs]];
      *(unsigned*)(dst+x+0x1C) = tab[1+2*src1[p4bpp3_ofs]] + tab[1+2*src2[p4bpp3_ofs]];
      src1++; src2++;
   }
}

void line_p4bpp_32(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < 512*4; x += 0x40) {
      *(unsigned*)(dst+x+0x00) = *(unsigned*)(dst+x+0x04) = tab[0+2*src[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x08) = *(unsigned*)(dst+x+0x0C) = tab[1+2*src[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x10) = *(unsigned*)(dst+x+0x14) = tab[0+2*src[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x18) = *(unsigned*)(dst+x+0x1C) = tab[1+2*src[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x20) = *(unsigned*)(dst+x+0x24) = tab[0+2*src[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x28) = *(unsigned*)(dst+x+0x2C) = tab[1+2*src[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x30) = *(unsigned*)(dst+x+0x34) = tab[0+2*src[p4bpp3_ofs]];
      *(unsigned*)(dst+x+0x38) = *(unsigned*)(dst+x+0x3C) = tab[1+2*src[p4bpp3_ofs]];
      src++;
   }
}

void line_p4bpp_32_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   unsigned char *src1 = src, *src2 = src + buf4bpp_shift;
   for (unsigned x = 0; x < 512*4; x += 0x40) {
      *(unsigned*)(dst+x+0x00) = *(unsigned*)(dst+x+0x04) = tab[0+2*src1[p4bpp0_ofs]] + tab[0+2*src2[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x08) = *(unsigned*)(dst+x+0x0C) = tab[1+2*src1[p4bpp0_ofs]] + tab[1+2*src2[p4bpp0_ofs]];
      *(unsigned*)(dst+x+0x10) = *(unsigned*)(dst+x+0x14) = tab[0+2*src1[p4bpp1_ofs]] + tab[0+2*src2[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x18) = *(unsigned*)(dst+x+0x1C) = tab[1+2*src1[p4bpp1_ofs]] + tab[1+2*src2[p4bpp1_ofs]];
      *(unsigned*)(dst+x+0x20) = *(unsigned*)(dst+x+0x24) = tab[0+2*src1[p4bpp2_ofs]] + tab[0+2*src2[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x28) = *(unsigned*)(dst+x+0x2C) = tab[1+2*src1[p4bpp2_ofs]] + tab[1+2*src2[p4bpp2_ofs]];
      *(unsigned*)(dst+x+0x30) = *(unsigned*)(dst+x+0x34) = tab[0+2*src1[p4bpp3_ofs]] + tab[0+2*src2[p4bpp3_ofs]];
      *(unsigned*)(dst+x+0x38) = *(unsigned*)(dst+x+0x3C) = tab[1+2*src1[p4bpp3_ofs]] + tab[1+2*src2[p4bpp3_ofs]];
      src1++; src2++;
   }
}

void r_p4bpp_8(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_8_nf(dst, src, t.p4bpp8_nf[0]); dst += pitch;
      }

   } else {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_8(dst, src, t.p4bpp8[0]); dst += pitch;
      }

   }
}

void r_p4bpp_8d(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_8_nf(dst, src, t.p4bpp8_nf[0]); dst += pitch;
         line_p4bpp_8_nf(dst, src, t.p4bpp8_nf[1]); dst += pitch;
      }

   } else {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_8(dst, src, t.p4bpp8[0]); dst += pitch;
         line_p4bpp_8(dst, src, t.p4bpp8[1]); dst += pitch;
      }

   }
}

void r_p4bpp_16(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_16_nf(dst, src, t.p4bpp16_nf[0]); dst += pitch;
      }

   } else {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_16(dst, src, t.p4bpp16[0]); dst += pitch;
      }

   }
}

void r_p4bpp_16d(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_16_nf(dst, src, t.p4bpp16_nf[0]); dst += pitch;
         line_p4bpp_16_nf(dst, src, t.p4bpp16_nf[1]); dst += pitch;
      }

   } else {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_16(dst, src, t.p4bpp16[0]); dst += pitch;
         line_p4bpp_16(dst, src, t.p4bpp16[1]); dst += pitch;
      }

   }
}

void r_p4bpp_32(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_32_nf(dst, src, t.p4bpp32_nf[0]); dst += pitch;
      }

   } else {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_32(dst, src, t.p4bpp32[0]); dst += pitch;
      }

   }
}

void r_p4bpp_32d(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_32_nf(dst, src, t.p4bpp32_nf[0]); dst += pitch;
         line_p4bpp_32_nf(dst, src, t.p4bpp32_nf[1]); dst += pitch;
      }

   } else {

      for (unsigned y = 0; y < 192; y++) {
         unsigned char *src = temp.base + t.scrtab[y];
         line_p4bpp_32(dst, src, t.p4bpp32[0]); dst += pitch;
         line_p4bpp_32(dst, src, t.p4bpp32[1]); dst += pitch;
      }

   }
}

void rend_p4bpp(unsigned char *dst, unsigned pitch)
{
   unsigned char *dst2 = dst + (temp.ox-512)*temp.obpp/16;
   dst2 += (temp.scy-192)/2*pitch * ((temp.oy > temp.scy)?2:1);
   if (temp.oy > temp.scy && conf.fast_sl) pitch *= 2;

   if (conf.noflic) buf4bpp_shift = (rbuf_s+PAGE) - temp.base;

   if (temp.obpp == 8)  { if (conf.fast_sl) rend_frame_x2_8s (dst, pitch), r_p4bpp_8 (dst2, pitch); else rend_frame_x2_8d (dst, pitch), r_p4bpp_8d (dst2, pitch); }
   if (temp.obpp == 16) { if (conf.fast_sl) rend_frame_x2_16s(dst, pitch), r_p4bpp_16(dst2, pitch); else rend_frame_x2_16d(dst, pitch), r_p4bpp_16d(dst2, pitch); }
   if (temp.obpp == 32) { if (conf.fast_sl) rend_frame_x2_32s(dst, pitch), r_p4bpp_32(dst2, pitch); else rend_frame_x2_32d(dst, pitch), r_p4bpp_32d(dst2, pitch); }

   if (conf.noflic) memcpy(rbuf_s, temp.base-PAGE, 2*PAGE);
}

