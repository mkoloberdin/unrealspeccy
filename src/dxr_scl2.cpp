
void line_8_any(unsigned char *dst, unsigned char *src)
{
   if (conf.noflic) line8(dst, src, t.sctab8[0]);
   else line8_nf(dst, src, t.sctab8[0]);
}

#if 1   // switch between vectorized and branched code

void lines_scale2(unsigned upos, unsigned char *dst1, unsigned char *dst2)
{
   unsigned char *u = t.scale2buf[upos & 3],
                 *l = t.scale2buf[(upos+2) & 3],
                 *m = t.scale2buf[(upos+1) & 3];

   for (unsigned i = 0; i < temp.scx; i += 4) {

      if (*(unsigned*)(u+i) ^ *(unsigned*)(l+i)) {

         __m64 mm = *(__m64*)(m+i-2);
         __m64 uu = *(__m64*)(u+i-2);
         __m64 ll = *(__m64*)(l+i-2);
         __m64 md = _mm_slli_si64(mm,8);
         __m64 mf = _mm_srli_si64(mm,8);
         __m64 maskall = _mm_or_si64(_mm_cmpeq_pi8(md,mf), _mm_cmpeq_pi8(uu,ll));

         __m64 e0, e1, v1, v2;

         e0 = _mm_cmpeq_pi8(md,uu);
         e0 = _mm_andnot_si64(maskall, e0);
         e0 = _mm_srli_si64(e0,16);
         e0 = _mm_unpacklo_pi8(e0, _mm_setzero_si64());

         e1 = _mm_cmpeq_pi8(mf,uu);
         e1 = _mm_andnot_si64(maskall, e1);
         e1 = _mm_srli_si64(e1,16);
         e1 = _mm_unpacklo_pi8(_mm_setzero_si64(), e1);

         e0 = _mm_or_si64(e0, e1);

         v1 = _m_from_int(*(unsigned*)(m+i));
         v2 = _m_from_int(*(unsigned*)(u+i));
         v1 = _mm_unpacklo_pi8(v1,v1);
         v2 = _mm_unpacklo_pi8(v2,v2);

         *(__m64*)(dst1 + 2*i) = _mm_or_si64( _mm_and_si64(e0,v2), _mm_andnot_si64(e0,v1) );

         e0 = _mm_cmpeq_pi8(md,ll);
         e0 = _mm_andnot_si64(maskall, e0);
         e0 = _mm_srli_si64(e0,16);
         e0 = _mm_unpacklo_pi8(e0, _mm_setzero_si64());

         e1 = _mm_cmpeq_pi8(mf,ll);
         e1 = _mm_andnot_si64(maskall, e1);
         e1 = _mm_srli_si64(e1,16);
         e1 = _mm_unpacklo_pi8(_mm_setzero_si64(), e1);

         e0 = _mm_or_si64(e0, e1);

         v1 = _m_from_int(*(unsigned*)(m+i));
         v2 = _m_from_int(*(unsigned*)(l+i));
         v1 = _mm_unpacklo_pi8(v1,v1);
         v2 = _mm_unpacklo_pi8(v2,v2);

         *(__m64*)(dst2 + 2*i) = _mm_or_si64( _mm_and_si64(e0,v2), _mm_andnot_si64(e0,v1) );

      } else {

         __m64 v1 = _m_from_int(*(unsigned*)(m+i));
         v1 = _mm_unpacklo_pi8(v1,v1);
         *(__m64*)(dst1 + 2*i) = v1;
         *(__m64*)(dst2 + 2*i) = v1;

      }

   }
}

#else

void lines_scale2(unsigned upos, unsigned char *dst1, unsigned char *dst2)
{
   unsigned char *u = t.scale2buf[upos & 3],
                 *l = t.scale2buf[(upos+2) & 3],
                 *m = t.scale2buf[(upos+1) & 3];

   for (unsigned i = 0; i < temp.scx; i += 4) {
      unsigned dw = *(unsigned*)(m+i);
      __m64 v1 = _m_from_int(dw);
      v1 = _mm_unpacklo_pi8(v1,v1);
      *(__m64*)(dst1 + 2*i) = v1;
      *(__m64*)(dst2 + 2*i) = v1;

      dw = *(unsigned*)(u+i) ^ *(unsigned*)(l+i);
      if (!dw) continue;

   #define process_pix(n)                                       \
      if ((dw & (0xFF << (8*n))) && m[i+n-1] != m[i+n+1]) {     \
         if (u[i+n] == m[i+n-1]) dst1[2*(i+n)] = u[i+n];        \
         if (u[i+n] == m[i+n+1]) dst1[2*(i+n)+1] = u[i+n];      \
         if (l[i+n] == m[i+n-1]) dst2[2*(i+n)] = l[i+n];        \
         if (l[i+n] == m[i+n+1]) dst2[2*(i+n)+1] = l[i+n];      \
      }

      process_pix(0);
      process_pix(1);
      process_pix(2);
      process_pix(3);
   #undef process_pix

   }

}

#endif


void render_scale2(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   line_8_any(t.scale2buf[0], src);
   memcpy(t.scale2buf[1], t.scale2buf[0], temp.scx);
   for (unsigned y = 0; y < temp.scy; y++) {
      src += delta;
      line_8_any(t.scale2buf[(y+2) & 3], src);
      lines_scale2(y, dst, dst+pitch);
      dst += 2*pitch;
   }

   if (conf.noflic) memcpy(rbuf_s, rbuf, temp.scy*temp.scx/4);
   _mm_empty();
}

// MMX-vectorized version is not ready yet :(
void lines_scale3(unsigned upos, unsigned char *dst, unsigned pitch)
{
   unsigned char *u = t.scale2buf[upos & 3],
                 *l = t.scale2buf[(upos+2) & 3],
                 *m = t.scale2buf[(upos+1) & 3];

   for (unsigned i = 0; i < temp.scx; i += 4) {
      unsigned char c;

      c = m[i];
      dst[3*i+0+0*pitch+ 0] = dst[3*i+1+0*pitch+ 0] = dst[3*i+2+0*pitch+ 0] = c;
      dst[3*i+0+1*pitch+ 0] = dst[3*i+1+1*pitch+ 0] = dst[3*i+2+1*pitch+ 0] = c;
      dst[3*i+0+2*pitch+ 0] = dst[3*i+1+2*pitch+ 0] = dst[3*i+2+2*pitch+ 0] = c;

      c = m[i+1];
      dst[3*i+0+0*pitch+ 3] = dst[3*i+1+0*pitch+ 3] = dst[3*i+2+0*pitch+ 3] = c;
      dst[3*i+0+1*pitch+ 3] = dst[3*i+1+1*pitch+ 3] = dst[3*i+2+1*pitch+ 3] = c;
      dst[3*i+0+2*pitch+ 3] = dst[3*i+1+2*pitch+ 3] = dst[3*i+2+2*pitch+ 3] = c;

      c = m[i+2];
      dst[3*i+0+0*pitch+ 6] = dst[3*i+1+0*pitch+ 6] = dst[3*i+2+0*pitch+ 6] = c;
      dst[3*i+0+1*pitch+ 6] = dst[3*i+1+1*pitch+ 6] = dst[3*i+2+1*pitch+ 6] = c;
      dst[3*i+0+2*pitch+ 6] = dst[3*i+1+2*pitch+ 6] = dst[3*i+2+2*pitch+ 6] = c;

      c = m[i+3];
      dst[3*i+0+0*pitch+ 9] = dst[3*i+1+0*pitch+ 9] = dst[3*i+2+0*pitch+ 9] = c;
      dst[3*i+0+1*pitch+ 9] = dst[3*i+1+1*pitch+ 9] = dst[3*i+2+1*pitch+ 9] = c;
      dst[3*i+0+2*pitch+ 9] = dst[3*i+1+2*pitch+ 9] = dst[3*i+2+2*pitch+ 9] = c;

      unsigned dw = *(unsigned*)(u+i) ^ *(unsigned*)(l+i);
      if (!dw) continue;

   #define process_pix(n)                                               \
      if ((dw & (0xFF << (8*n))) && m[i+n-1] != m[i+n+1]) {             \
         if (u[i+n] == m[i+n-1]) dst[0*pitch+3*(i+n)] = u[i+n];         \
         if ((u[i+n] == m[i+n-1] && m[i+n] != u[i+n+1]) || (u[i+n] == m[i+n+1] && m[i+n] != u[i+n-1])) dst[0*pitch+3*(i+n)+1] = u[i+n];        \
         if (u[i+n] == m[i+n+1]) dst[0*pitch+3*(i+n)+2] = u[i+n];       \
         if ((u[i+n] == m[i+n-1] && m[i+n] != l[i+n-1]) || (l[i+n] == m[i+n-1] && m[i+n] != u[i+n-1])) dst[1*pitch+3*(i+n)+0] = m[i+n-1];      \
         if ((u[i+n] == m[i+n+1] && m[i+n] != l[i+n+1]) || (l[i+n] == m[i+n+1] && m[i+n] != u[i+n+1])) dst[1*pitch+3*(i+n)+2] = m[i+n+1];      \
         if (l[i+n] == m[i+n-1]) dst[2*pitch+3*(i+n)] = l[i+n];         \
         if ((l[i+n] == m[i+n-1] && m[i+n] != l[i+n+1]) || (l[i+n] == m[i+n+1] && m[i+n] != l[i+n-1])) dst[2*pitch+3*(i+n)+1] = l[i+n];        \
         if (l[i+n] == m[i+n+1]) dst[2*pitch+3*(i+n)+2] = l[i+n];       \
      }

      process_pix(0);
      process_pix(1);
      process_pix(2);
      process_pix(3);
   #undef process_pix
   }
}

void render_scale3(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf; unsigned delta = temp.scx/4;
   line_8_any(t.scale2buf[0], src);
   memcpy(t.scale2buf[1], t.scale2buf[0], temp.scx);
   for (unsigned y = 0; y < temp.scy; y++) {
      src += delta;
      line_8_any(t.scale2buf[(y+2) & 3], src);
      lines_scale3(y, dst, pitch);
      dst += 3*pitch;
   }

   if (conf.noflic) memcpy(rbuf_s, rbuf, temp.scy*temp.scx/4);
   _mm_empty();
}
