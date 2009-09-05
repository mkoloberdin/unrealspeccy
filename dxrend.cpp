
void __fastcall render_small(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {
      if (temp.obpp == 8)  { rend_copy8_nf (dst, pitch); }
      if (temp.obpp == 16) { rend_copy16_nf(dst, pitch); }
      if (temp.obpp == 32) { rend_copy32_nf(dst, pitch); }
      memcpy(rbuf_s, rbuf, temp.scy*temp.scx/4);
   } else {
      if (temp.obpp == 8)  { rend_copy8 (dst, pitch); return; }
      if (temp.obpp == 16) { rend_copy16(dst, pitch); return; }
      if (temp.obpp == 32) { rend_copy32(dst, pitch); return; }
   }
}

void rend_dbl(unsigned char *dst, unsigned pitch)
{
   if (temp.oy > temp.scy && conf.fast_sl)
       pitch *= 2;
   if (conf.noflic)
   {
      if (temp.obpp == 8)
      {
          if (conf.fast_sl)
              rend_copy8d1_nf (dst, pitch);
          else
              rend_copy8d_nf (dst, pitch);
      }
      else if (temp.obpp == 16)
      {
          if (conf.fast_sl)
              rend_copy16d1_nf(dst, pitch);
          else
              rend_copy16d_nf(dst, pitch);
      }
      else if (temp.obpp == 32)
      {
          if (conf.fast_sl)
              rend_copy32d1_nf(dst, pitch);
          else
              rend_copy32d_nf(dst, pitch);
      }

      memcpy(rbuf_s, rbuf, temp.scy * temp.scx / 4);
   }
   else
   {
      if (temp.obpp == 8)
      {
          if (conf.fast_sl)
              rend_copy8d1 (dst, pitch);
          else
              rend_copy8d (dst, pitch);
          return;
      }
      if (temp.obpp == 16)
      {
          if (conf.fast_sl)
              rend_copy16d1(dst, pitch);
          else
              rend_copy16d(dst, pitch);
          return;
      }
      if (temp.obpp == 32)
      {
          if (conf.fast_sl)
              rend_copy32d1(dst, pitch);
          else
              rend_copy32d(dst, pitch);
          return;
      }
   }
}

void __fastcall render_dbl(unsigned char *dst, unsigned pitch)
{
   #ifdef MOD_VID_VD
   if ((comp.pVD & 8) && temp.obpp == 8) { rend_vd8dbl(dst, pitch); return; }
   #endif

   // todo: add ini option to show zx-screen with palette or with MC
   if (comp.pEFF7 & EFF7_512) { rend_512(dst, pitch); return; }
   if (comp.pEFF7 & EFF7_4BPP) { rend_p4bpp(dst, pitch); return; }
   if ((comp.pDFFD & 0x80) && conf.mem_model == MM_PROFI) { rend_profi(dst, pitch); return; }
   if (conf.mem_model == MM_ATM450) { rend_atm_1(dst, pitch); return; }
   if (conf.mem_model == MM_ATM710) { rend_atm_2(dst, pitch); return; }

   rend_dbl(dst, pitch);
}

void __fastcall render_quad(unsigned char *dst, unsigned pitch)
{
   if (conf.noflic) {
      if (temp.obpp == 8)  rend_copy8q_nf (dst, pitch);
      if (temp.obpp == 16) rend_copy16q_nf(dst, pitch);
      if (temp.obpp == 32) rend_copy32q_nf(dst, pitch);
      memcpy(rbuf_s, rbuf, temp.scy*temp.scx/4);
   } else {
      if (temp.obpp == 8)  { rend_copy8q (dst, pitch); return; }
      if (temp.obpp == 16) { rend_copy16q(dst, pitch); return; }
      if (temp.obpp == 32) { rend_copy32q(dst, pitch); return; }
   }
}


void __fastcall render_scale(unsigned char *dst, unsigned pitch)
{
   unsigned char *src = rbuf;
   unsigned dx = temp.scx / 4;
   unsigned char buf[MAX_WIDTH*2];
   unsigned x; //Alone Coder 0.36.7
   for (unsigned y = 0; y < temp.scy-1; y++)
   {
      for (x = 0; x < dx; x += 2)
      {
         unsigned xx = (t.dbl[src[x]] << 16) + t.dbl[src[x+2]];
         unsigned yy = (t.dbl[src[x+dx]] << 16) + t.dbl[src[x+dx+2]];
         unsigned x1 = xx | (yy & ((xx>>1) | (xx<<1)));
         unsigned *tab0 = t.sctab8[0] + (src[x+1] << 4);
         *(unsigned*)(dst+x*8+ 0)   = tab0[(x1>>28) & 0x0F];
         *(unsigned*)(dst+x*8+ 4)   = tab0[(x1>>24) & 0x0F];
         *(unsigned*)(dst+x*8+ 8)   = tab0[(x1>>20) & 0x0F];
         *(unsigned*)(dst+x*8+12)   = tab0[(x1>>16) & 0x0F];
         unsigned *tab1 = t.sctab8[0] + src[x+3];
         *(unsigned*)(dst+x*8+16)   = tab1[(x1>>12) & 0x0F];
         *(unsigned*)(dst+x*8+20)   = tab1[(x1>> 8) & 0x0F];
         *(unsigned*)(dst+x*8+24)   = tab1[(x1>> 4) & 0x0F];
         *(unsigned*)(dst+x*8+28)   = tab1[(x1>> 0) & 0x0F];
         x1 = yy | (xx & ((yy>>1) | (yy<<1)));
         *(unsigned*)(buf+x*8+ 0)   = tab0[(x1>>28) & 0x0F];
         *(unsigned*)(buf+x*8+ 4)   = tab0[(x1>>24) & 0x0F];
         *(unsigned*)(buf+x*8+ 8)   = tab0[(x1>>20) & 0x0F];
         *(unsigned*)(buf+x*8+12)   = tab0[(x1>>16) & 0x0F];
         *(unsigned*)(buf+x*8+16)   = tab1[(x1>>12) & 0x0F];
         *(unsigned*)(buf+x*8+20)   = tab1[(x1>> 8) & 0x0F];
         *(unsigned*)(buf+x*8+24)   = tab1[(x1>> 4) & 0x0F];
         *(unsigned*)(buf+x*8+28)   = tab1[(x1>> 0) & 0x0F];
      }
      dst += pitch;
      for (x = 0; x < temp.ox; x += 4)
          *(unsigned*)(dst+x) = *(unsigned*)(buf+x);
      src += dx; dst += pitch;
   }
}

unsigned __int64 mask49 = 0x4949494949494949;
unsigned __int64 mask92 = 0x9292929292929292;
#ifdef _M_IX86
void __declspec(naked) __fastcall _bil_line1(unsigned char *dst, unsigned char *src)
{
//      for (j = 0; j < temp.scx; j++, src++)
//         *dst++ = *src, *dst++ = ((*src + src[1]) >> 1);
   __asm {

      push ebx
      push edi
      push ebp

      mov  ebp, [temp.scx]
      xor  eax, eax
      xor  ebx, ebx // ebx - prev. pixel
      shr ebp,1

l1:
      mov  al, [edx]
      xadd eax, ebx
      shr  eax, 1
      mov  [ecx+1], bl
      mov  [ecx], al
      mov  al, [edx+1]
      add  ecx, 4
      xadd eax, ebx
      add  edx, 2
      shr  eax, 1
      mov  [ecx-1], bl
      dec  ebp
      mov  [ecx-2], al
      jnz l1

      pop ebp
      pop edi
      pop ebx
      retn
   }
}

void __declspec(naked) __fastcall _bil_line2(unsigned char *dst, unsigned char *s1)
{
//      for (j = 0; j < temp.ox; j+=4) {
//         unsigned a = *(unsigned*)(s1+j),
//                  b = *(unsigned*)(s1+j+2*MAX_WIDTH);
//         *(unsigned*)(dst+j) = (0x49494949 & ((a&b)^((a^b)>>1))) |
//                               (0x92929292 & ((a&b)|((a|b)&((a&b)<<1))));
//      }
   __asm {

      mov  eax, [temp.ox]
      movq mm2, [mask49]
      movq mm3, [mask92]
      shr  eax, 3

m2:   movq  mm0, [edx]
      movq  mm1, [edx+MAX_WIDTH*2]
      movq  mm4, mm0
      movq  mm5, mm0
      pand  mm4, mm1    // mm4 = a & b
      pxor  mm5, mm1    // mm5 = a ^ b
      movq  mm6, mm0
      psrlq mm5, 1      // mm5 = (a ^ b) >> 1
      por   mm6, mm1    // mm6 = a | b
      movq  mm7, mm4
      pxor  mm5, mm4    // mm5 = (a & b) ^ ((a ^ b) >> 1)
      psllq mm7, 1      // mm7 = (a & b) << 1
      pand  mm5, mm2    // mm5 = 0x49494949 & ((a & b) ^ ((a ^ b) >> 1))
      pand  mm7, mm6    // mm7 = (a|b) & ((a & b) << 1)
      por   mm7, mm4    // mm7 = (a&b) | ((a|b)&((a&b)<<1))
      add   ecx, 8
      pand  mm7, mm3    // mm7 &= 0x92929292
      add  edx, 8
      por   mm7, mm5
      dec  eax
      movq [ecx-8], mm7
      jnz  m2

      retn
   }
}

void __fastcall render_bil(unsigned char *dst, unsigned pitch)
{
   render_small(snbuf, MAX_WIDTH);

   unsigned char *src = snbuf;
   unsigned char l1[MAX_WIDTH*4];
   #define l2 (l1+MAX_WIDTH*2)
   _bil_line1(l1, src); src += MAX_WIDTH;
   unsigned j; //Alone Coder 0.36.7
   for (/*unsigned*/ j = 0; j < temp.ox; j+=4)
      *(unsigned*)(dst+j) = *(unsigned*)(l1+j);
   dst += pitch;

   for (unsigned i = temp.scy/2-1; i; i--) {
      _bil_line1(l2, src); src += MAX_WIDTH;
      _bil_line2(dst, l1); dst += pitch;
      for (j = 0; j < temp.ox; j+=4)
         *(unsigned*)(dst+j) = *(unsigned*)(l2+j);
      dst += pitch;

      _bil_line1(l1, src); src += MAX_WIDTH;
      _bil_line2(dst, l1); dst += pitch;
      for (j = 0; j < temp.ox; j+=4)
         *(unsigned*)(dst+j) = *(unsigned*)(l1+j);
      dst += pitch;
   }
   _bil_line1(l2, src); src += MAX_WIDTH;
   _bil_line2(dst, l1); dst += pitch;
   for (j = 0; j < temp.ox; j+=4)
      *(unsigned*)(dst+j) = *(unsigned*)(l2+j);
   dst += pitch;
   for (j = 0; j < temp.ox; j+=4)
      *(unsigned*)(dst+j) = *(unsigned*)(l2+j);
   #undef l2
   __asm emms
}
#endif

void __fastcall render_tv(unsigned char *dst, unsigned pitch)
{
// ripped from ccs and *highly* simplified and optimized

   unsigned char midbuf[MAX_WIDTH*2];
   unsigned char line[MAX_WIDTH*2+4*2], line2[MAX_WIDTH*2];

   unsigned j; //Alone Coder 0.36.7
   for (/*unsigned*/ j = 0; j < MAX_WIDTH/2; j++)
      *(unsigned*)(midbuf+j*4) = WORD4(0,0x80,0,0x80);

   unsigned char *src = rbuf; unsigned delta = temp.scx/4;

   for (unsigned i = temp.scy; i; i--) {
      *(unsigned*)line = *(unsigned*)(line+4) = WORD4(0,0x80,0,0x80);

      if (conf.noflic) line16_nf(line+8, src, t.sctab16_nf[0]);
      else line16(line+8, src, t.sctab16[0]);

      src += delta;

      for (j = 0; j < temp.scx; j++) {

         unsigned Y = line[j*2+8]*9+
                      line[j*2-2+8]*4+
                      line[j*2-4+8]*2+
                      line[j*2-8+8];
/*
         unsigned U = line[j*2+8+1]*12 +
                      line[j*2-2+8+1]*2+
                      line[j*2-4+8+1]+
                      line[j*2-8+8+1];
*/
         line2[j*2] = Y>>4;
//         line2[j*2+1] = U>>4;
         line2[j*2+1] = line[j*2+9];
      }
      // there must be only fixed length fader buffer
      for (j = 0; j < temp.scx/2; j++) {
         *(unsigned*)(midbuf+j*4) = *(unsigned*)(dst + j*4) =
         ((*(unsigned*)(midbuf+j*4) & 0xFEFEFEFE)/2 + (*(unsigned*)(line2+j*4) & 0xFEFEFEFE)/2);
      }
      dst += pitch;
   }
   if (conf.noflic) memcpy(rbuf_s, rbuf, temp.scy*temp.scx/4);
}

