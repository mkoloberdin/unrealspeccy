
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

void rend_copy16d1_nf(unsigned char *dst, unsigned pitch) {
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
