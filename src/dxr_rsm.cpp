
struct RSM_DATA
{
   __m64 *colortab; // tables for current frame

   enum { MAX_MIX_FRAMES = 8, FRAME_TABLES_SIZE = MAX_MIX_FRAMES*0x100*4*8 };
   enum { max_line_dwords = 48*8 };
   unsigned mix_frames, frame_table_size;
   union {
      __m64 line_buffer[max_line_dwords*2];
      unsigned line_buffer_d[max_line_dwords];
   };

   unsigned period, frame, rbuf_dst;

   unsigned char *data;
   unsigned char *needframes; // new spectrum frames in next pc frame
   __m64 *tables;

   RSM_DATA() { data = 0; }
   ~RSM_DATA() { free(data); }

} rsm;

void line_rsm_32(unsigned char *dst, unsigned char *src0)
{
   unsigned line_size_d = temp.scx;
   for (unsigned i = 0; i < line_size_d; i++)
      rsm.line_buffer_d[i] = 0;

   unsigned line_size = line_size_d / 2, frame = 0;
   __m64 *tab = rsm.colortab;
   for (;;) {
      unsigned char *src = src0;
      for (i = 0; i < line_size; ) {
         unsigned s = *(unsigned*)src, attr = (s >> 6) & 0x3FC;
         rsm.line_buffer[i+0] = _mm_adds_pi8(rsm.line_buffer[i+0], tab[((s >> 6) & 3) + attr]);
         rsm.line_buffer[i+1] = _mm_adds_pi8(rsm.line_buffer[i+1], tab[((s >> 4) & 3) + attr]);
         rsm.line_buffer[i+2] = _mm_adds_pi8(rsm.line_buffer[i+2], tab[((s >> 2) & 3) + attr]);
         rsm.line_buffer[i+3] = _mm_adds_pi8(rsm.line_buffer[i+3], tab[((s >> 0) & 3) + attr]);
         attr = (s >> 22) & 0x3FC;
         rsm.line_buffer[i+4] = _mm_adds_pi8(rsm.line_buffer[i+4], tab[((s >>22) & 3) + attr]);
         rsm.line_buffer[i+5] = _mm_adds_pi8(rsm.line_buffer[i+5], tab[((s >>20) & 3) + attr]);
         rsm.line_buffer[i+6] = _mm_adds_pi8(rsm.line_buffer[i+6], tab[((s >>18) & 3) + attr]);
         rsm.line_buffer[i+7] = _mm_adds_pi8(rsm.line_buffer[i+7], tab[((s >>16) & 3) + attr]);
         i += 8, src += 4;
      }
      if (++frame == rsm.mix_frames) break;
      src0 += rb2_offs;
      if (src0 >= rbuf_s + rb2_offs * rsm.mix_frames) src0 -= rb2_offs * rsm.mix_frames;
      tab += 0x100*4;
   }
   for (i = 0; i < line_size_d; i++)
      *(unsigned*)(dst + i*4) = (rsm.line_buffer_d[i] + 0x40404040);
   _mm_empty(); // EMMS
}

void __fastcall render_rsm(unsigned char *dst, unsigned pitch)
{
   rsm.colortab = (__m64*)((int)rsm.tables + rsm.frame * rsm.frame_table_size);
   unsigned char *src = rbuf_s + rb2_offs * rsm.rbuf_dst;
   unsigned delta = temp.scx/4;
   for (unsigned y = 0; y < temp.scy; y++) {
      line_rsm_32(dst, src);
      dst += pitch; src += delta;
   }
}

void rsmloop()
{
   unsigned char skipped = 0;
   for (;;)
   {
      if (skipped < temp.frameskip) skipped++, temp.vidblock = 1;
      else skipped = temp.vidblock = 0;

      if (!temp.vidblock) flip();
      for (unsigned f = rsm.needframes[rsm.frame]; f; f--) {
         temp.sndblock = !conf.sound.enabled;
         temp.inputblock = temp.vidblock;
         spectrum_frame();
         if (!temp.inputblock) dispatch(conf.atm.xt_kbd? ac_main_xt : ac_main);
         if (needrestart) return;
         if (!temp.sndblock) do_sound();
         memcpy(rbuf_s + rsm.rbuf_dst * rb2_offs, rbuf, temp.scx*temp.scy/2);
         if (++rsm.rbuf_dst == rsm.mix_frames) rsm.rbuf_dst = 0;
      }

      if (++rsm.frame == rsm.period) rsm.frame = 0;
   }
}

unsigned gcd(unsigned x, unsigned y)
{
   while (x != y) if (x > y) x -= y; else y -= x;
   return x;
}

unsigned lcm(unsigned x, unsigned y)
{
   return x*y / gcd(x,y);
}

void calc_rsm_tables()
{
   rsm.mix_frames = (conf.rsm.mode == RSM_SIMPLE)? 2 : conf.rsm.mix_frames;
   rsm.frame_table_size = rsm.mix_frames * 0x100 * 4 * sizeof(__m64);

   unsigned fmax = lcm(conf.intfq, temp.ofq);
   rsm.period = fmax / conf.intfq;
   unsigned step = fmax / temp.ofq;

   rsm.data = (unsigned char*)realloc(rsm.data, rsm.period * (rsm.frame_table_size + 1));
   rsm.tables = (__m64*)rsm.data;
   rsm.needframes = rsm.data + rsm.frame_table_size * rsm.period;

   double *weights = (double*)alloca(rsm.period * rsm.mix_frames * sizeof(double));
   double *dst = weights;

   if (conf.rsm.mode != RSM_SIMPLE) {
      unsigned fsize = rsm.period * rsm.mix_frames;
      double *flt = (double*)alloca((fsize+1) * sizeof(double));

      double cutoff = 0.9;
      if (conf.rsm.mode == RSM_FIR1) cutoff = 0.5;
      if (conf.rsm.mode == RSM_FIR2) cutoff = 0.33333333;

      double pi = 4*atan(1.0);
      cutoff *= 1 / (double)rsm.period; // cutoff scale = inftq/maxfq = 1/rsm.period
      double c1 = 0.54 / pi, c2 = 0.46 / pi;
      for (unsigned i = 0; i <= fsize; i++) {
         if (i == fsize/2) flt[i] = cutoff;
         else flt[i] = sin(pi*cutoff*((double)i - fsize/2)) * (c1 - c2*cos(2*pi*(double)i/fsize)) / ((double)i - fsize/2);
      }

      for (unsigned frame = 0; frame < rsm.period; frame++)
      {
         unsigned pos = frame * step, srcframe = pos / rsm.period;
         if (frame) srcframe++; // (pos % rsm.period) != 0

         unsigned nextpos = pos + step, nextframe = nextpos / rsm.period;
         if (frame+1 != rsm.period) nextframe++; // (nextpos % rsm.period) != 0

         rsm.needframes[frame] = (nextframe - srcframe);
         unsigned offset = (srcframe * rsm.period) - pos;
         for (unsigned ch = 0; ch < rsm.mix_frames; ch++) {
            *dst++ = flt[offset] * rsm.period;
            offset += rsm.period;
         }
      }
   } else { // RSM_SIMPLE

      double div = 1 / (double)step;
      for (unsigned frame = 0; frame < rsm.period; frame++)
      {
         unsigned pos = frame * step, srcframe = pos / rsm.period;
         unsigned nextpos = pos + step, nextframe = nextpos / rsm.period;
         unsigned offset = (srcframe == nextframe)? step : (nextpos - nextframe*rsm.period);
         rsm.needframes[frame] = (nextframe - srcframe);
         *dst++ = offset * div;
         *dst++ = (step - offset) * div;
      }
   }

   unsigned char *dst32 = (unsigned char*)rsm.tables;
   for (unsigned frame = 0; frame < rsm.period; frame++)
   {
      for (unsigned ch = 0; ch < rsm.mix_frames; ch++) {
         double scale = *weights++;
         for (unsigned a = 0; a < 0x100; a++)
            for (unsigned pix = 0; pix < 4; pix++) {
               unsigned data[2];
               data[0] = t.sctab32[0][(pix >>1)*0x100 + a];
               data[1] = t.sctab32[0][(pix & 1)*0x100 + a];
               unsigned char *src = (unsigned char*)data;
               for (unsigned k = 0; k < 8; k++) {
                  double color = scale * (src[k]/2);
                  if (color > 255) color = 255;
//                  if (color < 0) color = 0; // += 256;
                  if (color < 0) color += 256;
                  *dst32++ = (unsigned char)color;
               }
            }
      }
   }
   rsm.rbuf_dst = rsm.frame = 0;
}
