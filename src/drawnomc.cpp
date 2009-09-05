
void draw_border()
{
   unsigned br = comp.border_attr * 0x11001100;
   for (unsigned i = 0; i < temp.scx*temp.scy/4; i+=4)
      *(unsigned*)(rbuf + i) = br;
}

void draw_alco();
void draw_gigascreen_no_border();

void draw_screen()
{
   if (comp.pEFF7 & EFF7_384) { draw_alco(); return; }
   if (conf.nopaper) { draw_border(); return; }
   if (comp.pEFF7 & EFF7_GIGASCREEN) { draw_border(); draw_gigascreen_no_border(); return; }

   unsigned char *dst = rbuf;
   unsigned br = comp.border_attr * 0x11001100;

   for (unsigned i = temp.b_top*temp.scx/16; i; i--)
      *(unsigned*)dst = br, dst += 4;

   for (int y = 0; y < 192; y++) {
      *(volatile unsigned*)dst;
      for (unsigned x = temp.b_left; x; x-=16)
         *(unsigned*)dst = br, dst += 4;

      for (x = 0; x < 32; x++) {
         *(volatile unsigned char*)dst;
         *dst++ = temp.base[t.scrtab[y] + x];
         *dst++ = colortab[temp.base[atrtab[y] + x]];
      }

      *(volatile unsigned*)dst;
      for (x = temp.b_right; x; x-=16)
         *(unsigned*)dst = br, dst += 4;
   }
   for (i = temp.b_bottom*temp.scx/16; i; i--)
      *(unsigned*)dst = br, dst += 4;
}

void draw_gigascreen_no_border()
{
   unsigned char *dst = rbuf + (temp.b_top * temp.scx + temp.b_left) / 4;
   unsigned char * const screen = RAM_BASE_M + 5*PAGE;
   unsigned offset = (comp.frame_counter & 1)? 0 : 2*PAGE;
   for (int y = 0; y < 192; y++) {
      *(volatile unsigned char*)dst;
      for (unsigned x = 0; x < 32; x++) {
         dst[2*x+0] = screen[t.scrtab[y] + x + offset];
         dst[2*x+1] = colortab[screen[atrtab[y] + x + offset]];
      }
      offset ^= 2*PAGE;
      dst += temp.scx / 4;
   }
}

void draw_alco_384()
{
   unsigned ofs = (comp.p7FFD & 8) << 12;
   unsigned char *dst = rbuf;
   for (unsigned y = 0; y < temp.scy; y++) {
      for (unsigned x = 0; x < 6; x++) {

         unsigned char *data = t.alco[y][x].s+ofs, *attr = t.alco[y][x].a+ofs;
         unsigned d = *(unsigned*)data, a = *(unsigned*)attr;
         *(unsigned*)dst = (d & 0xFF) + colortab_s8[a & 0xFF] +
                           ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
         *(unsigned*)(dst+4) = ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                               ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
         d = *(unsigned*)(data+4), a = *(unsigned*)(attr+4);
         *(unsigned*)(dst+8) = (d & 0xFF) + colortab_s8[a & 0xFF] +
                               ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
         *(unsigned*)(dst+12)= ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                               ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
         dst += 16;
      }
   }
}

void draw_alco_320()
{
   unsigned ofs = (comp.p7FFD & 8) << 12;
   unsigned char *dst = rbuf;
   unsigned base = (304-temp.scy)/2;
   for (unsigned y = 0; y < temp.scy; y++) {

      unsigned char *data = t.alco[base+y][0].s+ofs+4, *attr = t.alco[base+y][0].a+ofs+4;
      unsigned d = *(unsigned*)data, a = *(unsigned*)attr;
      *(unsigned*)dst = (d & 0xFF) + colortab_s8[a & 0xFF] +
                        ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
      *(unsigned*)(dst+4) = ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                            ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
      dst += 8;

      for (unsigned x = 1; x < 5; x++) {
         data = t.alco[base+y][x].s+ofs, attr = t.alco[base+y][x].a+ofs;
         d = *(unsigned*)data, a = *(unsigned*)attr;
         *(unsigned*)dst = (d & 0xFF) + colortab_s8[a & 0xFF] +
                           ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
         *(unsigned*)(dst+4) = ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                               ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
         d = *(unsigned*)(data+4), a = *(unsigned*)(attr+4);
         *(unsigned*)(dst+8) = (d & 0xFF) + colortab_s8[a & 0xFF] +
                               ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
         *(unsigned*)(dst+12)= ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                               ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
         dst += 16;
      }

      data = t.alco[base+y][5].s+ofs, attr = t.alco[base+y][5].a+ofs;
      d = *(unsigned*)data, a = *(unsigned*)attr;
      *(unsigned*)dst = (d & 0xFF) + colortab_s8[a & 0xFF] +
                        ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
      *(unsigned*)(dst+4) = ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                            ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
      dst += 8;

   }
}

void draw_alco_256()
{
   unsigned ofs = (comp.p7FFD & 8) << 12;
   unsigned char *dst = rbuf;
   unsigned base = (304-temp.scy)/2;
   for (unsigned y = 0; y < temp.scy; y++) {
      for (unsigned x = 1; x < 5; x++) {

         unsigned char *data = t.alco[base+y][x].s+ofs, *attr = t.alco[base+y][x].a+ofs;
         unsigned d = *(unsigned*)data, a = *(unsigned*)attr;
         *(unsigned*)dst = (d & 0xFF) + colortab_s8[a & 0xFF] +
                           ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
         *(unsigned*)(dst+4) = ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                               ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
         d = *(unsigned*)(data+4), a = *(unsigned*)(attr+4);
         *(unsigned*)(dst+8) = (d & 0xFF) + colortab_s8[a & 0xFF] +
                               ((d << 8) & 0xFF0000) + colortab_s24[(a >> 8) & 0xFF];
         *(unsigned*)(dst+12)= ((d >> 16) & 0xFF) + colortab_s8[(a >> 16) & 0xFF] +
                               ((d >> 8) & 0xFF0000) + colortab_s24[(a >> 24) & 0xFF];
         dst += 16;
      }
   }
}

void draw_alco()
{
   if (temp.scx == 384) { draw_alco_384(); return; }
   if (temp.scx == 320) { draw_alco_320(); return; }
   draw_alco_256();
}
