
void format_trd(unsigned n, unsigned cyls)
{
   new_disk(n, cyls, 2);
   SECHDR hdr[16];
   for (unsigned c = 0; c < trkcache.cyls; c++) {
      for (unsigned side = 0; side < 2; side++) {
         for (unsigned sn = 0; sn < 16; sn++) {
            static unsigned char lv[3][16] =
             { { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 },
               { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 },
               { 1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6 } };
            unsigned s = lv[conf.trdos_interleave][sn];
            hdr[sn].n = s, hdr[sn].l = 1;
            hdr[sn].c = c, hdr[sn].s = 0;
            hdr[sn].c1 = hdr[sn].c2 = 0;
            hdr[sn].data = (unsigned char*)1;
         }
         format_track(n, c, side, 16, hdr);
      }
   }
}

int write_sector(unsigned n, unsigned c, unsigned s, unsigned sec, unsigned char *data, unsigned crc=-1)
{
   seek_track(n,c,s,1);
   for (unsigned i = 0; i < trkcache.s; i++)
      if (trkcache.hdr[i].n == sec) break;
   if (i == trkcache.s) return 0;
   if (trkcache.hdr[i].c != c || trkcache.hdr[i].l != 1) return 0;
   memcpy(trkcache.hdr[i].data, data, 256);
   if (crc == -1) crc = wd93_crc(trkcache.hdr[i].data-1, 257);
   *(unsigned short*)(trkcache.hdr[i].data+256) = (unsigned short)crc;
   return 1;
}

SECHDR *get_sector(unsigned n, unsigned c, unsigned s, unsigned sec)
{
   seek_track(n,c,s,1);
   for (unsigned i = 0; i < trkcache.s; i++)
      if (trkcache.hdr[i].n == sec) break;
   if (i == trkcache.s) return 0;
   if (trkcache.hdr[i].l != 1 || trkcache.hdr[i].c != c) return 0;
   return &trkcache.hdr[i];
}

void emptydisk(unsigned n)
{
   format_trd(n, MAX_CYLS);
   SECHDR *s8 = get_sector(n,0,0,9);
   if (!s8) return;
   s8->data[0xE2] = 1;                 // first free track
   s8->data[0xE3] = 0x16;              // 80T,DS
   *(short*)(s8->data+0xE5) = 2544;    // free sec
   s8->data[0xE7] = 0x10;              // trdos flag
   write_sector(n,0,0,9,s8->data);     // update sector CRC
}

int addfile(unsigned n, unsigned char *hdr, unsigned char *data)
{
   SECHDR *s8 = get_sector(n,0,0,9);
   if (!s8) return 0;
   unsigned len = hdr[13];
   unsigned pos = s8->data[0xE4]*0x10;
   SECHDR *dir = get_sector(n,0,0,1+pos/0x100);
   if (!dir) return 0;
   if (*(unsigned short*)(s8->data+0xE5) < len) return 0; // disk full
   memcpy(dir->data + (pos & 0xFF), hdr, 14);
   *(short*)(dir->data + (pos & 0xFF) + 14) = *(short*)(s8->data+0xE1);
   write_sector(n,0,0,1+pos/0x100,dir->data);

   pos = s8->data[0xE1] + 16*s8->data[0xE2];
   s8->data[0xE1] = (pos+len) & 0x0F, s8->data[0xE2] = (pos+len) >> 4;
   s8->data[0xE4]++;
   *(unsigned short*)(s8->data+0xE5) -= len;
   write_sector(n,0,0,9,s8->data);

   // goto next track. s8 become invalid
   for (unsigned i = 0; i < len; i++, pos++)
      write_sector(n,pos/32,(pos/16)&1,(pos&0x0F)+1,data+i*0x100);
   return 1;
}

void test_addboot(unsigned n)
{ // destroys snbuf - use after loading all files
   if (!*conf.appendboot) return;
   for (unsigned s = 0; s < 8; s++) {
      SECHDR *sc = get_sector(n,0,0,s+1);
      if (!sc) return;
      for (unsigned p = 0; p < 0x100; p += 0x10)
         if (!memcmp(sc->data + p, "boot    B", 9)) return;
   }
   FILE *f = fopen(conf.appendboot, "rb");
   if (f) {
      fread(snbuf, 1, sizeof snbuf, f);
      fclose(f);
      snbuf[13] = snbuf[14]; // copy length
      addfile(n, snbuf, snbuf+0x11);
   }
}

int read_scl(unsigned n)
{
   emptydisk(n);
   unsigned size, i;
   for (i = size = 0; i < snbuf[8]; i++) size += snbuf[9 + 14*i + 13];
   if (size > 2544) {
      SECHDR *s8 = get_sector(n,0,0,9);
      *(short*)(s8->data+0xE5) = size;    // free sec
      write_sector(n,0,0,9,s8->data);     // update sector CRC
   }
   unsigned char *data = snbuf + 9 + 14*snbuf[8];
   for (i = 0; i < snbuf[8]; i++) {
      if (!addfile(n, snbuf + 9 + 14*i, data)) return 0;
      data += snbuf[9 + 14*i + 13]*0x100;
   }
   test_addboot(n);
   return 1;
}

int read_hob(unsigned n)
{
   if (!disk[n].rawdata) emptydisk(n);
   snbuf[13] = snbuf[14];
   int r = addfile(n, snbuf, snbuf+0x11);
   test_addboot(n);
   return r;
}

int read_trd(unsigned n)
{
   format_trd(n, MAX_CYLS /* snapsize/0x2000 + ((snapsize & 0x1FFF)?1:0) */);
   for (unsigned i = 0; i < snapsize; i += 0x100)
      write_sector(n, i>>13, (i>>12) & 1, ((i>>8) & 0x0F)+1, snbuf+i);
   test_addboot(n);
   return 1;
}

int write_trd(unsigned n, FILE *ff)
{
   memset(snbuf, 0, 256);
   for (unsigned i = 0; i < /*disk[n].cyls*16*disk[n].sides*/ 2560; i++) {
      SECHDR *hdr = get_sector(n, i>>5, (i>>4) & 1, (i & 0x0F)+1);
      unsigned char *ptr = snbuf;
      if (hdr) ptr = hdr->data;
      if (!ptr) ptr = snbuf;
      if (fwrite(ptr, 1, 256, ff) != 256) return 0;
   }
   return 1;
}
