
int read_fdi(unsigned n) {
   new_disk(n, snbuf[4], snbuf[6]);
   strncpy(disk[n].dsc, (char*)snbuf + *(unsigned short*)(snbuf+8), sizeof disk[n].dsc);

   int res = 1;
   unsigned char *trk = snbuf + 0x0E + *(unsigned short*)(snbuf+0x0C);
   unsigned char *dat = snbuf + *(unsigned short*)(snbuf+0x0A);
   SECHDR hdr[MAX_SEC];
   for (unsigned c = 0; c < snbuf[4]; c++)
      for (unsigned s = 0; s < snbuf[6]; s++) {
         unsigned char *t0 = dat + *(unsigned*)trk;
         unsigned ns = trk[6];
         trk += 7;
         for (unsigned sec = 0; sec < ns; sec++) {
            *(unsigned*)&hdr[sec] = *(unsigned*)trk;
            hdr[sec].c1 = 0;
            if (trk[4] & 0x40) hdr[sec].data = 0;
            else {
               hdr[sec].data = t0 + *(unsigned short*)(trk+5);
               if (hdr[sec].data+128 > snbuf+snapsize) return 0;
               hdr[sec].c2 = (trk[4] & (1<<trk[3])) ? 0:2;
            }
            if (hdr[sec].l>5) { hdr[sec].data = 0; if (!(trk[4] & 0x40)) res = 0; }
            trk += 7;
         }
         format_track(n, c, s, ns, hdr);
      }
   test_addboot(n);
   return res;
}

int write_fdi(unsigned n, FILE *ff)
{
   unsigned c,s, total_s = 0;
   for (c = 0; c < disk[n].cyls; c++)
      for (s = 0; s < disk[n].sides; s++) {
         seek_track(n,c,s,1);
         total_s += trkcache.s;
      }
   unsigned tlen = strlen(disk[n].dsc)+1;
   unsigned hsize = 14+(total_s+disk[n].cyls*disk[n].sides)*7;
   *(unsigned*)snbuf = WORD4('F','D','I',0);
   *(unsigned short*)(snbuf+4) = disk[n].cyls;
   *(unsigned short*)(snbuf+6) = disk[n].sides;
   *(unsigned short*)(snbuf+8) = hsize;
   *(unsigned short*)(snbuf+0x0A) = hsize + tlen;
   *(unsigned short*)(snbuf+0x0C) = 0;
   fwrite(snbuf, 1, 14, ff);
   unsigned trkoffs = 0;
   for (c = 0; c < disk[n].cyls; c++)
      for (s = 0; s < disk[n].sides; s++) {
         seek_track(n,c,s,1);
         unsigned secoffs = 0;
         *(unsigned*)snbuf = trkoffs;
         *(unsigned*)(snbuf+4) = 0;
         snbuf[6] = trkcache.s;
         fwrite(snbuf, 1, 7, ff);
         for (unsigned se = 0; se < trkcache.s; se++) {
            *(unsigned*)snbuf = *(unsigned*)&trkcache.hdr[se];
            snbuf[4] = trkcache.hdr[se].c2 ? (1<<trkcache.hdr[se].l) : 0;
            if (trkcache.hdr[se].data && trkcache.hdr[se].data[-1] == 0xF8) snbuf[4] |= 0x80;
            if (!trkcache.hdr[se].data) snbuf[4] |= 0x40;
            *(unsigned*)(snbuf+5) = secoffs;
            fwrite(snbuf, 1, 7, ff);
            secoffs += trkcache.hdr[se].datlen;
         }
         trkoffs += secoffs;
      }
   fseek(ff, hsize, SEEK_SET);
   fwrite(disk[n].dsc, 1, tlen, ff);
   for (c = 0; c < disk[n].cyls; c++)
      for (s = 0; s < disk[n].sides; s++) {
         seek_track(n,c,s,1);
         for (unsigned se = 0; se < trkcache.s; se++)
            if (trkcache.hdr[se].data)
               if (fwrite(trkcache.hdr[se].data, 1, trkcache.hdr[se].datlen, ff) != trkcache.hdr[se].datlen) return 0;
      }
   return 1;
}
