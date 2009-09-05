
const atmh0_ofs = 0;
const atmh1_ofs = 0x2000;
const atmh2_ofs = -4*PAGE;
const atmh3_ofs = -4*PAGE+0x2000;


void line_atm2_8(unsigned char *dst, unsigned char *src, unsigned *tab0)
{
   for (unsigned x = 0; x < 640; x += 0x40) {
      unsigned s  = *(unsigned*)(src+atmh0_ofs);
      unsigned t  = *(unsigned*)(src+atmh1_ofs);
      unsigned as = *(unsigned*)(src+atmh2_ofs);
      unsigned at = *(unsigned*)(src+atmh3_ofs);
      unsigned *tab = tab0 + ((as << 4) & 0xFF0);
      *(unsigned*)(dst+x+0x00) = tab[((s >> 4)  & 0xF)];
      *(unsigned*)(dst+x+0x04) = tab[((s >> 0)  & 0xF)];
      tab = tab0 + ((at << 4) & 0xFF0);
      *(unsigned*)(dst+x+0x08) = tab[((t >> 4)  & 0xF)];
      *(unsigned*)(dst+x+0x0C) = tab[((t >> 0)  & 0xF)];
      tab = tab0 + ((as >> 4) & 0xFF0);
      *(unsigned*)(dst+x+0x10) = tab[((s >> 12) & 0xF)];
      *(unsigned*)(dst+x+0x14) = tab[((s >> 8)  & 0xF)];
      tab = tab0 + ((at >> 4) & 0xFF0);
      *(unsigned*)(dst+x+0x18) = tab[((t >> 12) & 0xF)];
      *(unsigned*)(dst+x+0x1C) = tab[((t >> 8)  & 0xF)];
      tab = tab0 + ((as >> 12) & 0xFF0);
      *(unsigned*)(dst+x+0x20) = tab[((s >> 20) & 0xF)];
      *(unsigned*)(dst+x+0x24) = tab[((s >> 16) & 0xF)];
      tab = tab0 + ((at >> 12) & 0xFF0);
      *(unsigned*)(dst+x+0x28) = tab[((t >> 20) & 0xF)];
      *(unsigned*)(dst+x+0x2C) = tab[((t >> 16) & 0xF)];
      tab = tab0 + ((as >> 20) & 0xFF0);
      *(unsigned*)(dst+x+0x30) = tab[((s >> 28) & 0xF)];
      *(unsigned*)(dst+x+0x34) = tab[((s >> 24) & 0xF)];
      tab = tab0 + ((at >> 20) & 0xFF0);
      *(unsigned*)(dst+x+0x38) = tab[((t >> 28) & 0xF)];
      *(unsigned*)(dst+x+0x3C) = tab[((t >> 24) & 0xF)];
      src += 4;
   }
}

void r_atm2_8s(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm2_8(dst, temp.base + y*40, t.zctab8[0]);
      dst += pitch;
   }
}

void r_atm2_8d(unsigned char *dst, unsigned pitch)
{
   for (unsigned y = 0; y < 200; y++) {
      line_atm2_8(dst, temp.base + y*40, t.zctab8[0]); dst += pitch;
      line_atm2_8(dst, temp.base + y*40, t.zctab8[1]); dst += pitch;
   }
}


void rend_atm2(unsigned char *dst, unsigned pitch)
{
   unsigned char *dst2 = dst + (temp.ox-640)*temp.obpp/16;
   if (temp.scy > 200) dst2 += (temp.scy-200)/2*pitch * ((temp.oy > temp.scy)?2:1);
   if (temp.oy > temp.scy && conf.fast_sl) pitch *= 2;

   if (temp.obpp == 8)  { if (conf.fast_sl) rend_atmframe_x2_8s (dst, pitch), r_atm2_8s (dst2, pitch); else rend_atmframe_x2_8d (dst, pitch), r_atm2_8d (dst2, pitch); return; }
//   if (temp.obpp == 16) { if (conf.fast_sl) rend_atmframe_x2_16s(dst, pitch), r_atm6_16s(dst2, pitch); else rend_atmframe_x2_16d(dst, pitch), r_atm6_16d(dst2, pitch); return; }
//   if (temp.obpp == 32) { if (conf.fast_sl) rend_atmframe_x2_32s(dst, pitch), r_atm6_32s(dst2, pitch); else rend_atmframe_x2_32d(dst, pitch), r_atm6_32d(dst2, pitch); return; }
}

