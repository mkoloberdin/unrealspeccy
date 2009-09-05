
void FDD::format_trd()
{
   static const unsigned char lv[3][16] =
    { { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 },
      { 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 },
      { 1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6 } };

   newdisk(MAX_CYLS, 2);

   for (unsigned c = 0; c < cyls; c++) {
      for (unsigned side = 0; side < 2; side++) {
         t.seek(this, c, side, JUST_SEEK); t.s = 16;
         for (unsigned sn = 0; sn < 16; sn++) {
            unsigned s = lv[conf.trdos_interleave][sn];
            t.hdr[sn].n = s, t.hdr[sn].l = 1;
            t.hdr[sn].c = c, t.hdr[sn].s = 0;
            t.hdr[sn].c1 = t.hdr[sn].c2 = 0;
            t.hdr[sn].data = (unsigned char*)1;
         }
         t.format();
      }
   }
}

void FDD::emptydisk()
{
   format_trd();
   t.seek(this, 0,0,LOAD_SECTORS);
   SECHDR *s8 = t.get_sector(9);
   if (!s8) return;
   s8->data[0xE2] = 1;                 // first free track
   s8->data[0xE3] = 0x16;              // 80T,DS
   *(short*)(s8->data+0xE5) = 2544;    // free sec
   s8->data[0xE7] = 0x10;              // trdos flag
   t.write_sector(9,s8->data);         // update sector CRC
}

int FDD::addfile(unsigned char *hdr, unsigned char *data)
{
   t.seek(this, 0,0,LOAD_SECTORS);
   SECHDR *s8 = t.get_sector(9);
   if (!s8) return 0;
   unsigned len = hdr[13];
   unsigned pos = s8->data[0xE4]*0x10;
   SECHDR *dir = t.get_sector(1+pos/0x100);
   if (!dir) return 0;
   if (*(unsigned short*)(s8->data+0xE5) < len) return 0; // disk full
   memcpy(dir->data + (pos & 0xFF), hdr, 14);
   *(short*)(dir->data + (pos & 0xFF) + 14) = *(short*)(s8->data+0xE1);
   t.write_sector(1+pos/0x100,dir->data);

   pos = s8->data[0xE1] + 16*s8->data[0xE2];
   s8->data[0xE1] = (pos+len) & 0x0F, s8->data[0xE2] = (pos+len) >> 4;
   s8->data[0xE4]++;
   *(unsigned short*)(s8->data+0xE5) -= len;
   t.write_sector(9,s8->data);

   // goto next track. s8 become invalid
   for (unsigned i = 0; i < len; i++, pos++) {
      t.seek(this, pos/32, (pos/16) & 1, LOAD_SECTORS);
      if (!t.trkd) return 0;
      if (!t.write_sector((pos&0x0F)+1,data+i*0x100)) return 0;
   }
   return 1;
}

// destroys snbuf - use after loading all files
void FDD::addboot()
{
   t.seek(this, 0,0,LOAD_SECTORS);
   for (unsigned s = 0; s < 8; s++) {
      SECHDR *sc = t.get_sector(s+1);
      if (!sc) return;
      for (unsigned p = 0; p < 0x100; p += 0x10)
         if (!memcmp(sc->data + p, "boot    B", 9)) return;
   }
   FILE *f = fopen(conf.appendboot, "rb");
   if (!f) return;
   fread(snbuf, 1, sizeof snbuf, f);
   fclose(f);
   snbuf[13] = snbuf[14]; // copy length
   addfile(snbuf, snbuf+0x11);
}

int FDD::read_scl()
{
   emptydisk();
   unsigned size, i;
   for (i = size = 0; i < snbuf[8]; i++) size += snbuf[9 + 14*i + 13];
   if (size > 2544) {
      t.seek(this, 0,0,LOAD_SECTORS);
      SECHDR *s8 = t.get_sector(9);
      *(short*)(s8->data+0xE5) = size;    // free sec
      t.write_sector(9,s8->data);         // update sector CRC
   }
   unsigned char *data = snbuf + 9 + 14*snbuf[8];
   for (i = 0; i < snbuf[8]; i++) {
      if (!addfile(snbuf + 9 + 14*i, data)) return 0;
      data += snbuf[9 + 14*i + 13]*0x100;
   }
   return 1;
}

int FDD::read_hob()
{
   if (!rawdata) emptydisk();
   snbuf[13] = snbuf[14];
   int r = addfile(snbuf, snbuf+0x11);
   return r;
}

int FDD::read_trd()
{
   format_trd();
   for (unsigned i = 0; i < snapsize; i += 0x100) {
      t.seek(this, i>>13, (i>>12) & 1, LOAD_SECTORS);
      t.write_sector(((i>>8) & 0x0F)+1, snbuf+i);
   }
   return 1;
}

int FDD::write_trd(FILE *ff)
{
   unsigned char zerosec[256] = { 0 };
   for (unsigned i = 0; i < 2560; i++) {
      t.seek(this, i>>5, (i>>4) & 1, LOAD_SECTORS);
      SECHDR *hdr = t.get_sector((i & 0x0F)+1);
      unsigned char *ptr = zerosec;
      if (hdr) ptr = hdr->data;
      if (!ptr) ptr = zerosec;
      if (fwrite(ptr, 1, 256, ff) != 256) return 0;
   }
   return 1;
}
