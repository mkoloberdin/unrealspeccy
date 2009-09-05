
void rend_atmframe_x2_8s(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   unsigned char *src = rbuf; unsigned scx = temp.scx, delta = scx/4;
   unsigned top = (temp.scy-200)/2;
   unsigned y; //Alone Coder 0.36.7
   for (/*unsigned*/ y = 0; y < top; y++) {
      line8d(dst, src, t.sctab8d[0]); dst += pitch;
      src += delta;
   }
   temp.scx = (scx-320)/2;
   unsigned r_start = scx - temp.scx,
            scr_offs = r_start*2,
            atr_offs = r_start/4;
   for (y = 0; y < 200; y++) {
      line8d(dst, src, t.sctab8d[0]);
      line8d(dst+scr_offs, src + atr_offs, t.sctab8d[0]);
      dst += pitch; src += delta;
   }
   temp.scx = scx;
   for (y = 0; y < top; y++) {
      line8d(dst, src, t.sctab8d[0]); dst += pitch;
      src += delta;
   }
}

void rend_atmframe_x2_8d(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   unsigned char *src = rbuf; unsigned scx = temp.scx, delta = scx/4;
   unsigned top = (temp.scy-200)/2;
   unsigned y; //Alone Coder 0.36.7
   for (/*unsigned*/ y = 0; y < top; y++) {
      line8d(dst, src, t.sctab8d[0]); dst += pitch;
      line8d(dst, src, t.sctab8d[1]); dst += pitch;
      src += delta;
   }
   temp.scx = (scx-320)/2;
   unsigned r_start = scx - temp.scx,
            scr_offs = r_start*2,
            atr_offs = r_start/4;
   for (y = 0; y < 200; y++) {
      line8d(dst, src, t.sctab8d[0]);
      line8d(dst+scr_offs, src + atr_offs, t.sctab8d[0]);
      dst += pitch,
      line8d(dst, src, t.sctab8d[1]);
      line8d(dst+scr_offs, src + atr_offs, t.sctab8d[1]);
      dst += pitch,
      src += delta;
   }
   temp.scx = scx;
   for (y = 0; y < top; y++) {
      line8d(dst, src, t.sctab8d[0]); dst += pitch;
      line8d(dst, src, t.sctab8d[1]); dst += pitch;
      src += delta;
   }
}

void rend_atmframe_x2_16s(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   unsigned char *src = rbuf; unsigned scx = temp.scx, delta = scx/4;
   unsigned top = (temp.scy-200)/2;
   unsigned y; //Alone Coder 0.36.7
   for (/*unsigned*/ y = 0; y < top; y++) {
      line16d(dst, src, t.sctab16d[0]); dst += pitch;
      src += delta;
   }
   temp.scx = (scx-320)/2;
   unsigned r_start = scx - temp.scx,
            scr_offs = r_start*4,
            atr_offs = r_start/4;
   for (y = 0; y < 200; y++) {
      line16d(dst, src, t.sctab16d[0]);
      line16d(dst+scr_offs, src + atr_offs, t.sctab16d[0]);
      dst += pitch; src += delta;
   }
   temp.scx = scx;
   for (y = 0; y < top; y++) {
      line16d(dst, src, t.sctab16d[0]); dst += pitch;
      src += delta;
   }
}

void rend_atmframe_x2_16d(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   unsigned char *src = rbuf; unsigned scx = temp.scx, delta = scx/4;
   unsigned top = (temp.scy-200)/2;
   unsigned y; //Alone Coder 0.36.7
   for (/*unsigned*/ y = 0; y < top; y++) {
      line16d(dst, src, t.sctab16d[0]); dst += pitch;
      line16d(dst, src, t.sctab16d[1]); dst += pitch;
      src += delta;
   }
   temp.scx = (scx-320)/2;
   unsigned r_start = scx - temp.scx,
            scr_offs = r_start*4,
            atr_offs = r_start/4;
   for (y = 0; y < 200; y++) {
      line16d(dst, src, t.sctab16d[0]);
      line16d(dst+scr_offs, src + atr_offs, t.sctab16d[0]);
      dst += pitch,
      line16d(dst, src, t.sctab16d[1]);
      line16d(dst+scr_offs, src + atr_offs, t.sctab16d[1]);
      dst += pitch,
      src += delta;
   }
   temp.scx = scx;
   for (y = 0; y < top; y++) {
      line16d(dst, src, t.sctab16d[0]); dst += pitch;
      line16d(dst, src, t.sctab16d[1]); dst += pitch;
      src += delta;
   }
}

/*
void rend_atmframe_x2_16(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   if (!conf.fast_sl) rend_atmframe_x2_16d(dst, pitch);
   else rend_atmframe_x2_16s(dst, pitch*2);
}
*/

void rend_atmframe_x2_32s(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   unsigned char *src = rbuf; unsigned scx = temp.scx, delta = scx/4;
   unsigned top = (temp.scy-200)/2;
   unsigned y; //Alone Coder 0.36.7
   for (/*unsigned*/ y = 0; y < top; y++) {
      line32d(dst, src, t.sctab32[0]); dst += pitch;
      src += delta;
   }
   temp.scx = (scx-320)/2;
   unsigned r_start = scx - temp.scx,
            scr_offs = r_start*8,
            atr_offs = r_start/4;
   for (y = 0; y < 200; y++) {
      line32d(dst, src, t.sctab32[0]);
      line32d(dst+scr_offs, src + atr_offs, t.sctab32[0]);
      dst += pitch; src += delta;
   }
   temp.scx = scx;
   for (y = 0; y < top; y++) {
      line32d(dst, src, t.sctab32[0]); dst += pitch;
      src += delta;
   }
}

void rend_atmframe_x2_32d(unsigned char *dst, unsigned pitch)
{
   if (!conf.updateb) return;
   unsigned char *src = rbuf; unsigned scx = temp.scx, delta = scx/4;
   unsigned top = (temp.scy-200)/2;
   unsigned y; //Alone Coder 0.36.7
   for (/*unsigned*/ y = 0; y < top; y++) {
      line32d(dst, src, t.sctab32[0]); dst += pitch;
      line32d(dst, src, t.sctab32[1]); dst += pitch;
      src += delta;
   }
   temp.scx = (scx-320)/2;
   unsigned r_start = scx - temp.scx,
            scr_offs = r_start*8,
            atr_offs = r_start/4;
   for (y = 0; y < 200; y++) {
      line32d(dst, src, t.sctab32[0]);
      line32d(dst+scr_offs, src + atr_offs, t.sctab32[0]);
      dst += pitch,
      line32d(dst, src, t.sctab32[1]);
      line32d(dst+scr_offs, src + atr_offs, t.sctab32[1]);
      dst += pitch,
      src += delta;
   }
   temp.scx = scx;
   for (y = 0; y < top; y++) {
      line32d(dst, src, t.sctab32[0]); dst += pitch;
      line32d(dst, src, t.sctab32[1]); dst += pitch;
      src += delta;
   }
}

/*
void rend_atmframe_x2_32(unsigned char *dst, unsigned pitch)
{
   if (!conf.fast_sl) rend_atmframe_x2_32d(dst, pitch);
   else rend_atmframe_x2_32s(dst, pitch*2);
}
*/
