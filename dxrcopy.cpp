
// #define QUAD_BUFFER  // tests show that this variant is slower, even in noflic mode

void line32_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*4; x += 32) {
      unsigned char byte = *src;
      unsigned *t1 = tab + src[1];
      unsigned char byt1 = src[rb2_offs];
      unsigned *t2 = tab + src[rb2_offs+1];
      src += 2;

      *(unsigned*)(dst+x)    = t1[(byte << 1) & 0x100] +
                               t2[(byt1 << 1) & 0x100];
      *(unsigned*)(dst+x+4)  = t1[(byte << 2) & 0x100] +
                               t2[(byt1 << 2) & 0x100];
      *(unsigned*)(dst+x+8)  = t1[(byte << 3) & 0x100] +
                               t2[(byt1 << 3) & 0x100];
      *(unsigned*)(dst+x+12) = t1[(byte << 4) & 0x100] +
                               t2[(byt1 << 4) & 0x100];
      *(unsigned*)(dst+x+16) = t1[(byte << 5) & 0x100] +
                               t2[(byt1 << 5) & 0x100];
      *(unsigned*)(dst+x+20) = t1[(byte << 6) & 0x100] +
                               t2[(byt1 << 6) & 0x100];
      *(unsigned*)(dst+x+24) = t1[(byte << 7) & 0x100] +
                               t2[(byt1 << 7) & 0x100];
      *(unsigned*)(dst+x+28) = t1[(byte << 8) & 0x100] +
                               t2[(byt1 << 8) & 0x100];
   }
}

void line32d_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*8; x += 64) {
      unsigned char byte = *src;
      unsigned *t1 = tab + src[1];
      unsigned char byt1 = src[rb2_offs];
      unsigned *t2 = tab + src[rb2_offs+1];
      src += 2;

      *(unsigned*)(dst+x)    =
      *(unsigned*)(dst+x+4)  =
                               t1[(byte << 1) & 0x100] +
                               t2[(byt1 << 1) & 0x100];
      *(unsigned*)(dst+x+8)  =
      *(unsigned*)(dst+x+12) =
                               t1[(byte << 2) & 0x100] +
                               t2[(byt1 << 2) & 0x100];
      *(unsigned*)(dst+x+16)  =
      *(unsigned*)(dst+x+20)  =
                               t1[(byte << 3) & 0x100] +
                               t2[(byt1 << 3) & 0x100];
      *(unsigned*)(dst+x+24) =
      *(unsigned*)(dst+x+28) =
                               t1[(byte << 4) & 0x100] +
                               t2[(byt1 << 4) & 0x100];
      *(unsigned*)(dst+x+32) =
      *(unsigned*)(dst+x+36) =
                               t1[(byte << 5) & 0x100] +
                               t2[(byt1 << 5) & 0x100];
      *(unsigned*)(dst+x+40) =
      *(unsigned*)(dst+x+44) =
                               t1[(byte << 6) & 0x100] +
                               t2[(byt1 << 6) & 0x100];
      *(unsigned*)(dst+x+48) =
      *(unsigned*)(dst+x+52) =
                               t1[(byte << 7) & 0x100] +
                               t2[(byt1 << 7) & 0x100];
      *(unsigned*)(dst+x+56) =
      *(unsigned*)(dst+x+60) =
                               t1[(byte << 8) & 0x100] +
                               t2[(byt1 << 8) & 0x100];
   }
}

void line32q_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*16; x += 128) {
      unsigned char byte = *src;
      unsigned *t1 = tab + src[1];
      unsigned char byt1 = src[rb2_offs];
      unsigned *t2 = tab + src[rb2_offs+1];
      src += 2;

      *(unsigned*)(dst+x+0x00) =
      *(unsigned*)(dst+x+0x04) =
      *(unsigned*)(dst+x+0x08) =
      *(unsigned*)(dst+x+0x0C) =
                               t1[(byte << 1) & 0x100] +
                               t2[(byt1 << 1) & 0x100];
      *(unsigned*)(dst+x+0x10) =
      *(unsigned*)(dst+x+0x14) =
      *(unsigned*)(dst+x+0x18) =
      *(unsigned*)(dst+x+0x1C) =
                               t1[(byte << 2) & 0x100] +
                               t2[(byt1 << 2) & 0x100];
      *(unsigned*)(dst+x+0x20) =
      *(unsigned*)(dst+x+0x24) =
      *(unsigned*)(dst+x+0x28) =
      *(unsigned*)(dst+x+0x2C) =
                               t1[(byte << 3) & 0x100] +
                               t2[(byt1 << 3) & 0x100];
      *(unsigned*)(dst+x+0x30) =
      *(unsigned*)(dst+x+0x34) =
      *(unsigned*)(dst+x+0x38) =
      *(unsigned*)(dst+x+0x3C) =
                               t1[(byte << 4) & 0x100] +
                               t2[(byt1 << 4) & 0x100];
      *(unsigned*)(dst+x+0x40) =
      *(unsigned*)(dst+x+0x44) =
      *(unsigned*)(dst+x+0x48) =
      *(unsigned*)(dst+x+0x4C) =
                               t1[(byte << 5) & 0x100] +
                               t2[(byt1 << 5) & 0x100];
      *(unsigned*)(dst+x+0x50) =
      *(unsigned*)(dst+x+0x54) =
      *(unsigned*)(dst+x+0x58) =
      *(unsigned*)(dst+x+0x5C) =
                               t1[(byte << 6) & 0x100] +
                               t2[(byt1 << 6) & 0x100];
      *(unsigned*)(dst+x+0x60) =
      *(unsigned*)(dst+x+0x64) =
      *(unsigned*)(dst+x+0x68) =
      *(unsigned*)(dst+x+0x6C) =
                               t1[(byte << 7) & 0x100] +
                               t2[(byt1 << 7) & 0x100];
      *(unsigned*)(dst+x+0x70) =
      *(unsigned*)(dst+x+0x74) =
      *(unsigned*)(dst+x+0x78) =
      *(unsigned*)(dst+x+0x7C) =
                               t1[(byte << 8) & 0x100] +
                               t2[(byt1 << 8) & 0x100];
   }
}

void line32(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*4; x += 32) {
      unsigned char byte = *src++;
      unsigned *t = tab + *src++;
      *(unsigned*)(dst+x)    = t[(byte << 1) & 0x100];
      *(unsigned*)(dst+x+4)  = t[(byte << 2) & 0x100];
      *(unsigned*)(dst+x+8)  = t[(byte << 3) & 0x100];
      *(unsigned*)(dst+x+12) = t[(byte << 4) & 0x100];
      *(unsigned*)(dst+x+16) = t[(byte << 5) & 0x100];
      *(unsigned*)(dst+x+20) = t[(byte << 6) & 0x100];
      *(unsigned*)(dst+x+24) = t[(byte << 7) & 0x100];
      *(unsigned*)(dst+x+28) = t[(byte << 8) & 0x100];
   }
}

void line32d(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*8; x += 64) {
      unsigned char byte = *src++;
      unsigned *t = tab + *src++;
      *(unsigned*)(dst+x)    =
      *(unsigned*)(dst+x+4)  =
                               t[(byte << 1) & 0x100];
      *(unsigned*)(dst+x+8)  =
      *(unsigned*)(dst+x+12) =
                               t[(byte << 2) & 0x100];
      *(unsigned*)(dst+x+16) =
      *(unsigned*)(dst+x+20) =
                               t[(byte << 3) & 0x100];
      *(unsigned*)(dst+x+24) =
      *(unsigned*)(dst+x+28) =
                               t[(byte << 4) & 0x100];
      *(unsigned*)(dst+x+32) =
      *(unsigned*)(dst+x+36) =
                               t[(byte << 5) & 0x100];
      *(unsigned*)(dst+x+40) =
      *(unsigned*)(dst+x+44) =
                               t[(byte << 6) & 0x100];
      *(unsigned*)(dst+x+48) =
      *(unsigned*)(dst+x+52) =
                               t[(byte << 7) & 0x100];
      *(unsigned*)(dst+x+56) =
      *(unsigned*)(dst+x+60) =
                               t[(byte << 8) & 0x100];
   }
}

void line32q(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*16; x += 128) {
      unsigned char byte = *src++;
      unsigned *t = tab + *src++;
      *(unsigned*)(dst+x+0x00) =
      *(unsigned*)(dst+x+0x04) =
      *(unsigned*)(dst+x+0x08) =
      *(unsigned*)(dst+x+0x0C) =
                               t[(byte << 1) & 0x100];
      *(unsigned*)(dst+x+0x10) =
      *(unsigned*)(dst+x+0x14) =
      *(unsigned*)(dst+x+0x18) =
      *(unsigned*)(dst+x+0x1C) =
                               t[(byte << 2) & 0x100];
      *(unsigned*)(dst+x+0x20) =
      *(unsigned*)(dst+x+0x24) =
      *(unsigned*)(dst+x+0x28) =
      *(unsigned*)(dst+x+0x2C) =
                               t[(byte << 3) & 0x100];
      *(unsigned*)(dst+x+0x30) =
      *(unsigned*)(dst+x+0x34) =
      *(unsigned*)(dst+x+0x38) =
      *(unsigned*)(dst+x+0x3C) =
                               t[(byte << 4) & 0x100];
      *(unsigned*)(dst+x+0x40) =
      *(unsigned*)(dst+x+0x44) =
      *(unsigned*)(dst+x+0x48) =
      *(unsigned*)(dst+x+0x4C) =
                               t[(byte << 5) & 0x100];
      *(unsigned*)(dst+x+0x50) =
      *(unsigned*)(dst+x+0x54) =
      *(unsigned*)(dst+x+0x58) =
      *(unsigned*)(dst+x+0x5C) =
                               t[(byte << 6) & 0x100];
      *(unsigned*)(dst+x+0x60) =
      *(unsigned*)(dst+x+0x64) =
      *(unsigned*)(dst+x+0x68) =
      *(unsigned*)(dst+x+0x6C) =
                               t[(byte << 7) & 0x100];
      *(unsigned*)(dst+x+0x70) =
      *(unsigned*)(dst+x+0x74) =
      *(unsigned*)(dst+x+0x78) =
      *(unsigned*)(dst+x+0x7C) =
                               t[(byte << 8) & 0x100];
   }
}

#define line16 line8d

void line16_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*2; x += 32) {
      unsigned s = *(unsigned*)src, attr = (s >> 6) & 0x3FC;
      unsigned r = *(unsigned*)(src + rb2_offs), atr2 = (r >> 6) & 0x3FC;
      *(unsigned*)(dst+x)   = (tab[((s >> 6) & 3) + attr]) +
                              (tab[((r >> 6) & 3) + atr2]);
      *(unsigned*)(dst+x+4) = (tab[((s >> 4) & 3) + attr]) +
                              (tab[((r >> 4) & 3) + atr2]);
      *(unsigned*)(dst+x+8) = (tab[((s >> 2) & 3) + attr]) +
                              (tab[((r >> 2) & 3) + atr2]);
      *(unsigned*)(dst+x+12)= (tab[((s >> 0) & 3) + attr]) +
                              (tab[((r >> 0) & 3) + atr2]);
      attr = (s >> 22) & 0x3FC; atr2 = (r >> 22) & 0x3FC;
      *(unsigned*)(dst+x+16)= (tab[((s >>22) & 3) + attr]) +
                              (tab[((r >>22) & 3) + atr2]);
      *(unsigned*)(dst+x+20)= (tab[((s >>20) & 3) + attr]) +
                              (tab[((r >>20) & 3) + atr2]);
      *(unsigned*)(dst+x+24)= (tab[((s >>18) & 3) + attr]) +
                              (tab[((r >>18) & 3) + atr2]);
      *(unsigned*)(dst+x+28)= (tab[((s >>16) & 3) + attr]) +
                              (tab[((r >>16) & 3) + atr2]);
      src += 4;
   }
}

#define line16d line32
#define line16d_nf line32_nf

#define line16q line32d
#define line16q_nf line32d_nf

void line8(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx; x += 32) {
      unsigned src0 = *(unsigned*)src, attr = (src0 >> 4) & 0xFF0;
      *(unsigned*)(dst+x)    = tab[((src0 >> 4)  & 0xF) + attr];
      *(unsigned*)(dst+x+4)  = tab[((src0 >> 0)  & 0xF) + attr];
      attr = (src0 >> 20) & 0xFF0;
      *(unsigned*)(dst+x+8)  = tab[((src0 >> 20) & 0xF) + attr];
      *(unsigned*)(dst+x+12) = tab[((src0 >> 16) & 0xF) + attr];
      src0 = *(unsigned*)(src+4), attr = (src0 >> 4) & 0xFF0;
      *(unsigned*)(dst+x+16) = tab[((src0 >> 4)  & 0xF) + attr];
      *(unsigned*)(dst+x+20) = tab[((src0 >> 0)  & 0xF) + attr];
      attr = (src0 >> 20) & 0xFF0;
      *(unsigned*)(dst+x+24) = tab[((src0 >> 20) & 0xF) + attr];
      *(unsigned*)(dst+x+28) = tab[((src0 >> 16) & 0xF) + attr];
      src += 8;
   }
}

void line8_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx; x += 32) {
      unsigned s = *(unsigned*)src, attr = (s >> 4) & 0xFF0;
      unsigned r = *(unsigned*)(src + rb2_offs), atr2 = (r >> 4) & 0xFF0;
      *(unsigned*)(dst+x)    = (tab[((s >> 4)  & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 4)  & 0xF) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+4)  = (tab[((s >> 0)  & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 0)  & 0xF) + atr2] & 0xF0F0F0F0);
      attr = (s >> 20) & 0xFF0; atr2 = (r >> 20) & 0xFF0;
      *(unsigned*)(dst+x+8)  = (tab[((s >> 20) & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 20) & 0xF) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+12) = (tab[((s >> 16) & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 16) & 0xF) + atr2] & 0xF0F0F0F0);
      s = *(unsigned*)(src+4), attr = (s >> 4) & 0xFF0;
      r = *(unsigned*)(src+rb2_offs+4), atr2 = (r >> 4) & 0xFF0;
      *(unsigned*)(dst+x+16) = (tab[((s >> 4)  & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 4)  & 0xF) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+20) = (tab[((s >> 0)  & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 0)  & 0xF) + atr2] & 0xF0F0F0F0);
      attr = (s >> 20) & 0xFF0; atr2 = (r >> 20) & 0xFF0;
      *(unsigned*)(dst+x+24) = (tab[((s >> 20) & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 20) & 0xF) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+28) = (tab[((s >> 16) & 0xF) + attr] & 0x0F0F0F0F) +
                               (tab[((r >> 16) & 0xF) + atr2] & 0xF0F0F0F0);
      src += 8;
   }
}

void line8d(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*2; x += 32) {
      unsigned s = *(unsigned*)src, attr = (s >> 6) & 0x3FC;
      *(unsigned*)(dst+x)   = tab[((s >> 6) & 3) + attr];
      *(unsigned*)(dst+x+4) = tab[((s >> 4) & 3) + attr];
      *(unsigned*)(dst+x+8) = tab[((s >> 2) & 3) + attr];
      *(unsigned*)(dst+x+12)= tab[((s >> 0) & 3) + attr];
      attr = (s >> 22) & 0x3FC;
      *(unsigned*)(dst+x+16)= tab[((s >>22) & 3) + attr];
      *(unsigned*)(dst+x+20)= tab[((s >>20) & 3) + attr];
      *(unsigned*)(dst+x+24)= tab[((s >>18) & 3) + attr];
      *(unsigned*)(dst+x+28)= tab[((s >>16) & 3) + attr];
      src += 4;
   }
}

void line8q(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*4; x += 32) {
      unsigned char byte = *src++;
      unsigned *t = tab + *src++;
      *(unsigned*)(dst+x+0x00) = t[(byte << 1) & 0x100];
      *(unsigned*)(dst+x+0x04) = t[(byte << 2) & 0x100];
      *(unsigned*)(dst+x+0x08) = t[(byte << 3) & 0x100];
      *(unsigned*)(dst+x+0x0C) = t[(byte << 4) & 0x100];
      *(unsigned*)(dst+x+0x10) = t[(byte << 5) & 0x100];
      *(unsigned*)(dst+x+0x14) = t[(byte << 6) & 0x100];
      *(unsigned*)(dst+x+0x18) = t[(byte << 7) & 0x100];
      *(unsigned*)(dst+x+0x1C) = t[(byte << 8) & 0x100];
   }
}

void line8d_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*2; x += 32) {
      unsigned s = *(unsigned*)src, attr = (s >> 6) & 0x3FC;
      unsigned r = *(unsigned*)(src + rb2_offs), atr2 = (r >> 6) & 0x3FC;
      *(unsigned*)(dst+x)   = (tab[((s >> 6) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >> 6) & 3) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+4) = (tab[((s >> 4) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >> 4) & 3) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+8) = (tab[((s >> 2) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >> 2) & 3) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+12)= (tab[((s >> 0) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >> 0) & 3) + atr2] & 0xF0F0F0F0);
      attr = (s >> 22) & 0x3FC; atr2 = (r >> 22) & 0x3FC;
      *(unsigned*)(dst+x+16)= (tab[((s >>22) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >>22) & 3) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+20)= (tab[((s >>20) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >>20) & 3) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+24)= (tab[((s >>18) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >>18) & 3) + atr2] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+28)= (tab[((s >>16) & 3) + attr] & 0x0F0F0F0F) +
                              (tab[((r >>16) & 3) + atr2] & 0xF0F0F0F0);
      src += 4;
   }
}

void line8q_nf(unsigned char *dst, unsigned char *src, unsigned *tab)
{
   for (unsigned x = 0; x < temp.scx*4; x += 32) {
      unsigned char byte1 = src[0], byte2 = src[rb2_offs+0];
      unsigned *t1 = tab + src[1], *t2 = tab + src[rb2_offs+1];
      src += 2;

      *(unsigned*)(dst+x+0x00) = (t1[(byte1 << 1) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 1) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x04) = (t1[(byte1 << 2) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 2) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x08) = (t1[(byte1 << 3) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 3) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x0C) = (t1[(byte1 << 4) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 4) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x10) = (t1[(byte1 << 5) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 5) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x14) = (t1[(byte1 << 6) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 6) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x18) = (t1[(byte1 << 7) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 7) & 0x100] & 0xF0F0F0F0);
      *(unsigned*)(dst+x+0x1C) = (t1[(byte1 << 8) & 0x100] & 0x0F0F0F0F) + (t2[(byte2 << 8) & 0x100] & 0xF0F0F0F0);
   }
}


void rend_copy32_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line32_nf(dst, src, t.sctab32_nf[0]);
      dst += pitch; src += delta;
   }
}

void rend_copy32(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line32(dst, src, t.sctab32[0]);
      dst += pitch; src += delta;
   }
}

void rend_copy32d1_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line32d_nf(dst, src, t.sctab32_nf[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy32d_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   if (conf.alt_nf) {
      int offset = rb2_offs;
      if (comp.frame_counter & 1) src += rb2_offs, offset = -offset;
      for (unsigned y = 0; y < temp.scy; y++) {
         line32d(dst, src, t.sctab32[0]); dst += pitch;
         line32d(dst, src+offset, t.sctab32[0]); dst += pitch;
         src += delta;
      }
   } else {
      for (unsigned y = 0; y < temp.scy; y++) {
         line32d_nf(dst, src, t.sctab32_nf[0]); dst += pitch;
         line32d_nf(dst, src, t.sctab32_nf[1]); dst += pitch;
         src += delta;
      }
   }
}

void rend_copy32q_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
#ifdef QUAD_BUFFER
      unsigned char buffer[MAX_WIDTH*4*sizeof(DWORD)];
      line32q_nf(buffer, src, t.sctab32_nf[0]);
      for (int i = 0; i < 4; i++) {
         memcpy(dst, buffer, temp.scx*16);
         dst += pitch;
      }
#else
      line32q_nf(dst, src, t.sctab32_nf[0]); dst += pitch;
      line32q_nf(dst, src, t.sctab32_nf[0]); dst += pitch;
      line32q_nf(dst, src, t.sctab32_nf[0]); dst += pitch;
      line32q_nf(dst, src, t.sctab32_nf[0]); dst += pitch;
#endif
      src += delta;
   }
}

void rend_copy32d1(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line32d(dst, src, t.sctab32[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy32d(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line32d(dst, src, t.sctab32[0]); dst += pitch;
      line32d(dst, src, t.sctab32[1]); dst += pitch;
      src += delta;
   }
}

void rend_copy32q(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
#ifdef QUAD_BUFFER
      unsigned char buffer[MAX_WIDTH*4*sizeof(DWORD)];
      line32q(buffer, src, t.sctab32[0]);
      for (int i = 0; i < 4; i++) {
         memcpy(dst, buffer, temp.scx*16);
         dst += pitch;
      }
#else
      line32q(dst, src, t.sctab32[0]); dst += pitch;
      line32q(dst, src, t.sctab32[0]); dst += pitch;
      line32q(dst, src, t.sctab32[0]); dst += pitch;
      line32q(dst, src, t.sctab32[0]); dst += pitch;
#endif
      src += delta;
   }
}

void rend_copy16(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16(dst, src, t.sctab16[0]);
      dst += pitch, src += delta;
   }
}

void rend_copy16_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16_nf(dst, src, t.sctab16_nf[0]);
      dst += pitch, src += delta;
   }
}

void rend_copy16d1(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16d(dst, src, t.sctab16d[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy16d(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16d(dst, src, t.sctab16d[0]); dst += pitch;
      line16d(dst, src, t.sctab16d[1]); dst += pitch;
      src += delta;
   }
}

void rend_copy16q(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16q(dst, src, t.sctab16d[0]); dst += pitch;
      line16q(dst, src, t.sctab16d[0]); dst += pitch;
      line16q(dst, src, t.sctab16d[0]); dst += pitch;
      line16q(dst, src, t.sctab16d[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy16d1_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16d_nf(dst, src, t.sctab16d_nf[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy16d_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   if (conf.alt_nf) {
      int offset = rb2_offs;
      if (comp.frame_counter & 1) src += rb2_offs, offset = -offset;
      for (unsigned y = 0; y < temp.scy; y++) {
         line16d(dst, src, t.sctab16d[0]); dst += pitch;
         line16d(dst, src+offset, t.sctab16d[0]); dst += pitch;
         src += delta;
      }
   } else {
      unsigned char *src = rbuf; unsigned delta = temp.scx/4;
      for (unsigned y = 0; y < temp.scy; y++) {
         line16d_nf(dst, src, t.sctab16d_nf[0]); dst += pitch;
         line16d_nf(dst, src, t.sctab16d_nf[1]); dst += pitch;
         src += delta;
      }
   }
}

void rend_copy16q_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line16q_nf(dst, src, t.sctab16d_nf[0]); dst += pitch;
      line16q_nf(dst, src, t.sctab16d_nf[0]); dst += pitch;
      line16q_nf(dst, src, t.sctab16d_nf[0]); dst += pitch;
      line16q_nf(dst, src, t.sctab16d_nf[0]); dst += pitch;
      src += delta;
   }
}

void __fastcall rend_copy8(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8(dst, src, t.sctab8[0]);
      dst += pitch, src += delta;
   }
}

void __fastcall rend_copy8_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8_nf(dst, src, t.sctab8[0]);
      dst += pitch, src += delta;
   }
}

void rend_copy8d1(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8d(dst, src, t.sctab8d[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy8d(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8d(dst, src, t.sctab8d[0]); dst += pitch;
      line8d(dst, src, t.sctab8d[1]); dst += pitch;
      src += delta;
   }
}

void rend_copy8q(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8q(dst, src, t.sctab8q); dst += pitch;
      line8q(dst, src, t.sctab8q); dst += pitch;
      line8q(dst, src, t.sctab8q); dst += pitch;
      line8q(dst, src, t.sctab8q); dst += pitch;
      src += delta;
   }
}

void rend_copy8d1_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8d_nf(dst, src, t.sctab8d[0]); dst += pitch;
      src += delta;
   }
}

void rend_copy8d_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   if (conf.alt_nf) {
      int offset = rb2_offs;
      if (comp.frame_counter & 1) src += rb2_offs, offset = -offset;
      for (unsigned y = 0; y < temp.scy; y++) {
         line8d(dst, src, t.sctab8d[0]); dst += pitch;
         line8d(dst, src+offset, t.sctab8d[0]); dst += pitch;
         src += delta;
      }
   } else {
      for (unsigned y = 0; y < temp.scy; y++) {
         line8d_nf(dst, src, t.sctab8d[0]); dst += pitch;
         line8d_nf(dst, src, t.sctab8d[1]); dst += pitch;
         src += delta;
      }
   }
}

void rend_copy8q_nf(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line8q_nf(dst, src, t.sctab8q); dst += pitch;
      line8q_nf(dst, src, t.sctab8q); dst += pitch;
      line8q_nf(dst, src, t.sctab8q); dst += pitch;
      line8q_nf(dst, src, t.sctab8q); dst += pitch;
      src += delta;
   }
}

