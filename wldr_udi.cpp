struct UDI {
   char label[3];
   unsigned char cyls;
   unsigned char sides;
};

int read_udi(unsigned n) {
   freedisk(n);
   DISK *d = &disk[n]; unsigned c,s;
   unsigned mem = 0; unsigned char *ptr = snbuf + 0x10;
   for (c = 0; c <= snbuf[9]; c++)
      for (s = 0; s <= snbuf[10]; s++) {
         if (*ptr) return 0;
         unsigned sz = *(unsigned short*)(ptr+1);
         sz += sz/8 + ((sz & 7)? 1 : 0);
         mem += sz; ptr += 3 + sz;
         if (ptr > snbuf+snapsize) return 0;
      }
   d->cyls = snbuf[9]+1, d->sides = snbuf[10]+1;
   d->rawsize = align(mem, 4096);
   d->rawdata = (unsigned char*)VirtualAlloc(0, d->rawsize, MEM_COMMIT, PAGE_READWRITE);
   ptr = snbuf+0x10; unsigned char *dst = d->rawdata;
   for (c = 0; c < d->cyls; c++)
      for (s = 0; s < d->sides; s++) {
         unsigned sz = *(unsigned short*)(ptr+1);
         d->trklen[c][s] = sz;
         d->trkd[c][s] = dst;
         d->trki[c][s] = dst+sz;
         sz += sz/8 + ((sz & 7)? 1 : 0);
         memcpy(dst, ptr+3, sz);
         ptr += 3 + sz; dst += sz;
      }
   if (snbuf[11] & 1) strcpy(d->dsc, (char*)ptr);
   trkcache.drive = -1; seek_track(n,0,0);
   test_addboot(n);
   return 1;
}

void crc32(int &crc, unsigned char *buf, unsigned len) {
   while (len--) {
      crc ^= -1 ^ *buf++;
      for(int k = 8; k--; )
         { int temp = -(crc & 1); crc >>= 1, crc ^= 0xEDB88320 & temp; }
      crc ^= -1;
   }
}

int write_udi(unsigned n, FILE *ff)
{
   memset(snbuf, 0, 0x10);
   *(unsigned*)snbuf = WORD4('U','D','I','!');
   snbuf[8] = snbuf[11] = 0;
   snbuf[9] = disk[n].cyls-1;
   snbuf[10] = disk[n].sides-1;
   *(unsigned*)(snbuf+12) = 0;

   unsigned char *dst = snbuf+0x10;
   for (unsigned c = 0; c < disk[n].cyls; c++)
      for (unsigned s = 0; s < disk[n].sides; s++) {
         *dst++ = 0;
         unsigned len = disk[n].trklen[c][s];
         *(unsigned short*)dst = len; dst += 2;
         memcpy(dst, disk[n].trkd[c][s], len); dst += len;
         len = (len+7)/8;
         memcpy(dst, disk[n].trki[c][s], len); dst += len;
      }
   if (*disk[n].dsc)
      strcpy((char*)dst, disk[n].dsc),
      dst += strlen(disk[n].dsc)+1, snbuf[11] = 1;
   *(unsigned*)(snbuf+4) = dst-snbuf;
   int crc = -1; crc32(crc, snbuf, dst-snbuf);
   *(unsigned*)dst = crc; dst += 4;
   if (fwrite(snbuf, 1, dst-snbuf, ff) != (unsigned)(dst-snbuf)) return 0;
   return 1;
}
