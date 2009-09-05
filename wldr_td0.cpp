unsigned short crc16(unsigned char *buf, unsigned size);

int write_td0(unsigned n, FILE *ff) {
   memset(snbuf, 0, 0x2000);
   *(unsigned short*)snbuf = WORD2('T','D');
   snbuf[4] = 21; snbuf[6] = 2; snbuf[9] = (unsigned char)disk[n].sides;
   if (*disk[n].dsc) snbuf[7] = 0x80;
   *(unsigned short*)(snbuf + 10) = crc16(snbuf, 10);
   fwrite(snbuf, 1, 12, ff);
   if (snbuf[7]) {
      unsigned char inf[0x200] = { 0 };
      strcpy((char*)inf+10, disk[n].dsc);
      unsigned len = strlen(disk[n].dsc)+1;
      *(unsigned*)(inf+2) = len;
      *(unsigned short*)inf = crc16(inf+2, len+8);
      fwrite(inf, 1, len+10, ff);
   }

   for (unsigned c = 0; c < disk[n].cyls; c++)
      for (unsigned s = 0; s < disk[n].sides; s++) {
         unsigned char bf[16];
         seek_track(n,c,s,1); *bf = trkcache.s;
         bf[1] = c, bf[2] = s;
         bf[3] = (unsigned char)crc16(bf, 3);
         fwrite(bf, 1, 4, ff);
         for (unsigned sec = 0; sec < trkcache.s; sec++) {
            if (!trkcache.hdr[sec].data) { trkcache.hdr[sec].data = snbuf+0x1000, trkcache.hdr[sec].datlen = 128, trkcache.hdr[sec].l = 1; }
            *(unsigned*)bf = *(unsigned*)&trkcache.hdr[sec];
            bf[4] = 0; // flags
            bf[5] = (unsigned char)crc16(trkcache.hdr[sec].data, trkcache.hdr[sec].datlen);
            *(unsigned short*)(bf+6) = trkcache.hdr[sec].datlen + 1;
            bf[8] = 0; // compression type = none
            fwrite(bf, 1, 9, ff);
            if (fwrite(trkcache.hdr[sec].data, 1, trkcache.hdr[sec].datlen, ff) != trkcache.hdr[sec].datlen) return 0;
         }
      }
   c = WORD4(0xFF,0,0,0);
   if (fwrite(&c, 1, 4, ff) != 4) return 0;
   return 1;
}


unsigned unpack_lzh(unsigned char *src, unsigned size, unsigned char *buf);

int read_td0(unsigned n) {

   if (*(short*)snbuf == WORD2('t','d')) { // packed disk
      unsigned char *tmp = (unsigned char*)malloc(snapsize);
      memcpy(tmp, snbuf+12, snapsize-12);
      snapsize = 12+unpack_lzh(tmp, snapsize-12, snbuf+12);
      free(tmp);
      //*(short*)snbuf = WORD2('T','D');
   }

   unsigned char *start = snbuf+12;
   char dsc[1+sizeof disk[n].dsc];
   if (snbuf[7] & 0x80) {
      start += 10;
      strncpy(dsc, (char*)start, sizeof dsc);
      unsigned len = *(unsigned short*)(snbuf+14);
      dsc[len] = 0; start += len;
   }
   unsigned char *td0_src = start;

   unsigned max_cyl = 0, max_head = 0;
   for (;;) {
      unsigned char s = *td0_src;
      if (s == 0xFF) break;
      max_cyl = max(max_cyl, td0_src[1]);
      max_head = max(max_head, td0_src[2]);
      td0_src += 4;
      for (; s; s--) {
         td0_src += 6;
         if (td0_src > snbuf + snapsize) return 0;
         td0_src += *(unsigned short*)td0_src + 2;
      }
   }
   new_disk(n, max_cyl+1, max_head+1);
   strcpy(disk[n].dsc, dsc);

   td0_src = start;
   for (;;) {
      SECHDR hdr[MAX_SEC];
      unsigned char t0[16384], *dst = t0;
      unsigned char *trkh = td0_src; td0_src += 4;
      if (*trkh == 0xFF) break;
      for (unsigned se = 0; se < trkh[0]; se++) {
         unsigned size = 0x80 << td0_src[3];
         *(unsigned*)&hdr[se] = *(unsigned*)td0_src;
         hdr[se].c1 = hdr[se].c2 = 0;
         hdr[se].data = dst; td0_src += 6;
         unsigned src_size = *(unsigned short*)td0_src; td0_src += 2;
         unsigned char *end_packed_data = td0_src + src_size;
         memset(dst, 0, size);
         switch (*td0_src++) {
            case 0:
               memcpy(dst, td0_src, src_size-1); break;
            case 1:
            {
               unsigned n = *(unsigned short*)td0_src; td0_src += 2;
               unsigned short data = *(unsigned short*)td0_src;
               for (unsigned i = 0; i < n; i--)
                  *(unsigned short*)(dst+2*i) = data;
               break;
            }
            case 2:
            {
               unsigned short data; unsigned char s, *d0 = dst;
               do {
                  switch (*td0_src++) {
                     case 0:
                        for (s = *td0_src++; s; s--)
                           *dst++ = *td0_src++;
                        break;
                     case 1:
                        s = *td0_src++;
                        data = *(unsigned short*)td0_src; td0_src += 2;
                        for ( ; s; s--) *(unsigned short*)dst = data, dst += 2;
                        break;
                     default: goto shit;
                  }
               } while (td0_src < end_packed_data);
               dst = d0;
               break;
            }
            default: shit: printf("bad TD0 file"); exit();
         }
         dst += size; td0_src = end_packed_data;
      }
      format_track(n, trkh[1], trkh[2], trkh[0], hdr);
   }
   test_addboot(n);
   return 1;
}

unsigned short crcTab[256] =
   { 0x0000, 0x97A0, 0xB9E1, 0x2E41, 0xE563, 0x72C3, 0x5C82, 0xCB22,
     0xCAC7, 0x5D67, 0x7326, 0xE486, 0x2FA4, 0xB804, 0x9645, 0x01E5,
     0x032F, 0x948F, 0xBACE, 0x2D6E, 0xE64C, 0x71EC, 0x5FAD, 0xC80D,
     0xC9E8, 0x5E48, 0x7009, 0xE7A9, 0x2C8B, 0xBB2B, 0x956A, 0x02CA,
     0x065E, 0x91FE, 0xBFBF, 0x281F, 0xE33D, 0x749D, 0x5ADC, 0xCD7C,
     0xCC99, 0x5B39, 0x7578, 0xE2D8, 0x29FA, 0xBE5A, 0x901B, 0x07BB,
     0x0571, 0x92D1, 0xBC90, 0x2B30, 0xE012, 0x77B2, 0x59F3, 0xCE53,
     0xCFB6, 0x5816, 0x7657, 0xE1F7, 0x2AD5, 0xBD75, 0x9334, 0x0494,
     0x0CBC, 0x9B1C, 0xB55D, 0x22FD, 0xE9DF, 0x7E7F, 0x503E, 0xC79E,
     0xC67B, 0x51DB, 0x7F9A, 0xE83A, 0x2318, 0xB4B8, 0x9AF9, 0x0D59,
     0x0F93, 0x9833, 0xB672, 0x21D2, 0xEAF0, 0x7D50, 0x5311, 0xC4B1,
     0xC554, 0x52F4, 0x7CB5, 0xEB15, 0x2037, 0xB797, 0x99D6, 0x0E76,
     0x0AE2, 0x9D42, 0xB303, 0x24A3, 0xEF81, 0x7821, 0x5660, 0xC1C0,
     0xC025, 0x5785, 0x79C4, 0xEE64, 0x2546, 0xB2E6, 0x9CA7, 0x0B07,
     0x09CD, 0x9E6D, 0xB02C, 0x278C, 0xECAE, 0x7B0E, 0x554F, 0xC2EF,
     0xC30A, 0x54AA, 0x7AEB, 0xED4B, 0x2669, 0xB1C9, 0x9F88, 0x0828,
     0x8FD8, 0x1878, 0x3639, 0xA199, 0x6ABB, 0xFD1B, 0xD35A, 0x44FA,
     0x451F, 0xD2BF, 0xFCFE, 0x6B5E, 0xA07C, 0x37DC, 0x199D, 0x8E3D,
     0x8CF7, 0x1B57, 0x3516, 0xA2B6, 0x6994, 0xFE34, 0xD075, 0x47D5,
     0x4630, 0xD190, 0xFFD1, 0x6871, 0xA353, 0x34F3, 0x1AB2, 0x8D12,
     0x8986, 0x1E26, 0x3067, 0xA7C7, 0x6CE5, 0xFB45, 0xD504, 0x42A4,
     0x4341, 0xD4E1, 0xFAA0, 0x6D00, 0xA622, 0x3182, 0x1FC3, 0x8863,
     0x8AA9, 0x1D09, 0x3348, 0xA4E8, 0x6FCA, 0xF86A, 0xD62B, 0x418B,
     0x406E, 0xD7CE, 0xF98F, 0x6E2F, 0xA50D, 0x32AD, 0x1CEC, 0x8B4C,
     0x8364, 0x14C4, 0x3A85, 0xAD25, 0x6607, 0xF1A7, 0xDFE6, 0x4846,
     0x49A3, 0xDE03, 0xF042, 0x67E2, 0xACC0, 0x3B60, 0x1521, 0x8281,
     0x804B, 0x17EB, 0x39AA, 0xAE0A, 0x6528, 0xF288, 0xDCC9, 0x4B69,
     0x4A8C, 0xDD2C, 0xF36D, 0x64CD, 0xAFEF, 0x384F, 0x160E, 0x81AE,
     0x853A, 0x129A, 0x3CDB, 0xAB7B, 0x6059, 0xF7F9, 0xD9B8, 0x4E18,
     0x4FFD, 0xD85D, 0xF61C, 0x61BC, 0xAA9E, 0x3D3E, 0x137F, 0x84DF,
     0x8615, 0x11B5, 0x3FF4, 0xA854, 0x6376, 0xF4D6, 0xDA97, 0x4D37,
     0x4CD2, 0xDB72, 0xF533, 0x6293, 0xA9B1, 0x3E11, 0x1050, 0x87F0 };

unsigned short crc16(unsigned char *buf, unsigned size) {
   unsigned crc = 0;
   while (size--) crc = (crc>>8) ^ crcTab[(crc&0xff) ^ *buf++];
   return wordswap(crc);
}



// ------------------------------------------------------ LZH unpacker


unsigned char *packed_ptr, *packed_end;

int readChar(void) {
  if (packed_ptr < packed_end) return *packed_ptr++;
  else return -1;
}

unsigned char d_code[256] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
        0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
        0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
        0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
        0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
        0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
        0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
        0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
        0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
        0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
        0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
        0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
        0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
        0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
        0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

unsigned char d_len[256] = {
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};


const N = 4096;     // buffer size
const F = 60;       // lookahead buffer size
const THRESHOLD =   2;
const NIL = N;      // leaf of tree

unsigned char text_buf[N + F - 1];

const N_CHAR = (256 - THRESHOLD + F);       // kinds of characters (character code = 0..N_CHAR-1)
const T =   (N_CHAR * 2 - 1);       // size of table
const R = (T - 1);                  // position of root
const MAX_FREQ = 0x8000;            // updates tree when the
                                    // root frequency comes to this value.

unsigned short freq[T + 1];        // frequency table

short prnt[T + N_CHAR]; // pointers to parent nodes, except for the
                        // elements [T..T + N_CHAR - 1] which are used to get
                        // the positions of leaves corresponding to the codes.
short son[T];           // pointers to child nodes (son[], son[] + 1)


int r;

unsigned getbuf;
unsigned char getlen;

int GetBit(void)      /* get one bit */
{
  int i;

  while (getlen <= 8)
  {
    if((i = readChar()) == -1) i = 0;
    getbuf |= i << (8 - getlen);
    getlen += 8;
  }
  i = getbuf;
  getbuf <<= 1;
  getlen--;
  return ((i>>15) & 1);
}

int GetByte(void)     /* get one byte */
{
  unsigned i;

  while (getlen <= 8)
  {
    if((i = readChar()) == -1) i = 0;
    getbuf |= i << (8 - getlen);
    getlen += 8;
  }
  i = getbuf;
  getbuf <<= 8;
  getlen -= 8;
  return (i >> 8) & 0xFF;
}

void StartHuff(void)
{
  int i, j;

  getbuf = 0, getlen = 0;
  for (i = 0; i < N_CHAR; i++) {
    freq[i] = 1;
    son[i] = i + T;
    prnt[i + T] = i;
  }
  i = 0; j = N_CHAR;
  while (j <= R) {
    freq[j] = freq[i] + freq[i + 1];
    son[j] = i;
    prnt[i] = prnt[i + 1] = j;
    i += 2; j++;
  }
  freq[T] = 0xffff;
  prnt[R] = 0;

  for (i = 0; i < N - F; i++) text_buf[i] = ' ';
  r = N - F;
}

/* reconstruction of tree */
void reconst(void)
{
  int i, j, k;
  int f, l;

  /* collect leaf nodes in the first half of the table */
  /* and replace the freq by (freq + 1) / 2. */
  j = 0;
  for(i = 0; i < T; i++)
  {
    if(son[i] >= T)
    {
      freq[j] = (freq[i] + 1) / 2;
      son[j] = son[i];
      j++;
    }
  }
  /* begin constructing tree by connecting sons */
  for(i = 0, j = N_CHAR; j < T; i += 2, j++)
  {
    k = i + 1;
    f = freq[j] = freq[i] + freq[k];
    for(k = j - 1; f < freq[k]; k--);
    k++;
    l = (j - k) * sizeof(*freq);
    MoveMemory(&freq[k + 1], &freq[k], l);
    freq[k] = f;
    MoveMemory(&son[k + 1], &son[k], l);
    son[k] = i;
  }
  /* connect prnt */
  for (i = 0; i < T; i++)
    if ((k = son[i]) >= T) prnt[k] = i;
    else prnt[k] = prnt[k + 1] = i;
}


/* increment frequency of given code by one, and update tree */

void update(int c)
{
  int i, j, k, l;

  if(freq[R] == MAX_FREQ) reconst();

  c = prnt[c + T];
  do {
    k = ++freq[c];

    /* if the order is disturbed, exchange nodes */
    if (k > freq[l = c + 1])
    {
      while (k > freq[++l]);
      l--;
      freq[c] = freq[l];
      freq[l] = k;

      i = son[c];
      prnt[i] = l;
      if (i < T) prnt[i + 1] = l;

      j = son[l];
      son[l] = i;

      prnt[j] = c;
      if (j < T) prnt[j + 1] = c;
      son[c] = j;

      c = l;
    }
  } while ((c = prnt[c]) != 0);  /* repeat up to root */
}

int DecodeChar(void)
{
  int c;

  c = son[R];

  /* travel from root to leaf, */
  /* choosing the smaller child node (son[]) if the read bit is 0, */
  /* the bigger (son[]+1} if 1 */
  while(c < T) c = son[c + GetBit()];
  c -= T;
  update(c);
  return c;
}

int DecodePosition(void)
{
  int i, j, c;

  /* recover upper 6 bits from table */
  i = GetByte();
  c = (int)d_code[i] << 6;
  j = d_len[i];
  /* read lower 6 bits verbatim */
  j -= 2;
  while (j--) i = (i << 1) + GetBit();
  return c | (i & 0x3f);
}

unsigned unpack_lzh(unsigned char *src, unsigned size, unsigned char *buf) {

  packed_ptr = src; packed_end = src+size;
  int  i, j, k, c;
  unsigned count = 0;
  StartHuff();

//  while (count < textsize)  // textsize - sizeof unpacked data
  while (packed_ptr < packed_end)
  {
    c = DecodeChar();
    if(c < 256)
    {
      *buf++ = c;
      text_buf[r++] = c;
      r &= (N - 1);
      count++;
    } else {
      i = (r - DecodePosition() - 1) & (N - 1);
      j = c - 255 + THRESHOLD;
      for (k = 0; k < j; k++)
      {
        c = text_buf[(i + k) & (N - 1)];
        *buf++ = c;
        text_buf[r++] = c;
        r &= (N - 1);
        count++;
      }
    }
  }
  return count;
}
