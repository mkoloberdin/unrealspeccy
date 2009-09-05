
void TRKCACHE::seek(FDD *d, unsigned cyl, unsigned side, SEEK_MODE fs)
{
   if (!(((int)d-(int)drive)|(sf-fs)|(cyl-TRKCACHE::cyl)|(side-TRKCACHE::side))) return;

   drive = d; sf = fs; s = 0;
   TRKCACHE::cyl = cyl; TRKCACHE::side = side;
   if (cyl >= d->cyls || !d->rawdata) { trkd = 0; return; }

   trkd = d->trkd[cyl][side];
   trki = d->trki[cyl][side];
   trklen = d->trklen[cyl][side];
   if (!trklen) { trkd = 0; return; }

   ts_byte = Z80FQ/(trklen*FDD_RPS);
   if (fs == JUST_SEEK) return; // else find sectors

   for (unsigned i = 0; i < trklen - 8; i++) {
      if (trkd[i] != 0xA1 || trkd[i+1] != 0xFE || !test_i(i)) continue;

      if (s == MAX_SEC) errexit("too many sectors");
      SECHDR *h = &hdr[s++];
      h->id = trkd+i+2; *(unsigned*)h = *(unsigned*)h->id;
      h->crc = *(unsigned short*)(trkd+i+6);
      h->c1 = (wd93_crc(trkd+i+1, 5) == h->crc);
      h->data = 0; h->datlen = 0;
      if (h->l > 5) continue;

      unsigned end = min(trklen-8, i+8+43); // 43-DD, 30-SD
      for (unsigned j = i+8; j < end; j++) {
         if (trkd[j] != 0xA1 || !test_i(j) || test_i(j+1)) continue;

         if (trkd[j+1] == 0xF8 || trkd[j+1] == 0xFB) {
            h->datlen = 128 << h->l;
            h->data = trkd+j+2;
            h->c2 = (wd93_crc(h->data-1, h->datlen+1) == *(unsigned short*)(h->data+h->datlen));
         }
         break;
      }
   }
}

void TRKCACHE::format()
{
   memset(trkd, 0, trklen);
   memset(trki, 0, trklen/8 + ((trklen&7) ? 1:0));

   unsigned char *dst = trkd;

   for (unsigned i = 0; i < 80; i++) *dst++ = 0x4E; // 1st gap
   for (i = 0; i < 12; i++) *dst++ = 0;
   for (i = 0; i < 3; i++) write(dst++ - trkd, 0xC2, 1);
   *dst++ = 0xFC; // index

   for (unsigned is = 0; is < s; is++) {
      for (i = 0; i < 50; i++) *dst++ = 0x4E;
      for (i = 0; i < 12; i++) *dst++ = 0;
      for (i = 0; i < 3; i++) write(dst++ - trkd, 0xA1, 1);
      *dst++ = 0xFE; // marker

      SECHDR *sechdr = hdr + is;
      *(unsigned*)dst = *(unsigned*)sechdr; dst += 4;
      unsigned crc = wd93_crc(dst-5, 5);
      if (sechdr->c1==1) crc = sechdr->crc;
      if (sechdr->c1==2) crc ^= 0xFFFF;
      *(unsigned*)dst = crc; dst += 2;
      if (sechdr->data) {
         for (i = 0; i < 22; i++) *dst++ = 0x4E;
         for (i = 0; i < 12; i++) *dst++ = 0;
         for (i = 0; i < 3; i++) write(dst++ - trkd, 0xA1, 1);
         *dst++ = 0xFB; // sector
         if (sechdr->l > 5) errexit("strange sector");
         unsigned len = 128 << sechdr->l;
         if (sechdr->data != (unsigned char*)1) memcpy(dst, sechdr->data, len);
         else memset(dst, 0, len);
         crc = wd93_crc(dst-1, len+1);
         if (sechdr->c2==1) crc = sechdr->crcd;
         if (sechdr->c2==2) crc ^= 0xFFFF;
         *(unsigned*)(dst+len) = crc; dst += len+2;
      }
   }
   if (dst > trklen + trkd) errexit("track too long");
   while (dst < trkd + trklen) *dst++ = 0x4E;
}

#if 0
void TRKCACHE::dump()
{
   printf("\n%d/%d:", cyl, side);
   if (!trkd) { printf("<e>"); return; }
   if (!sf) { printf("<n>"); return; }
   for (unsigned i = 0; i < s; i++)
      printf("%c%02X-%02X-%02X-%02X,%c%c%c", i?' ':'<', hdr[i].c,hdr[i].s,hdr[i].n,hdr[i].l, hdr[i].c1?'+':'-', hdr[i].c2?'+':'-', hdr[i].data?'d':'h');
   printf(">");
}
#endif

int TRKCACHE::write_sector(unsigned sec, unsigned char *data)
{
   SECHDR *h = get_sector(sec);
   if (!h || !h->data) return 0;
   unsigned sz = h->datlen;
   memcpy(h->data, data, sz);
   *(unsigned short*)(h->data+sz) = (unsigned short)wd93_crc(h->data-1, sz+1);
   return sz;
}

SECHDR *TRKCACHE::get_sector(unsigned sec)
{
   for (unsigned i = 0; i < s; i++)
      if (hdr[i].n == sec) break;
   if (i == s) return 0;
   if (hdr[i].l != 1 || hdr[i].c != cyl) return 0;
   return &hdr[i];
}

