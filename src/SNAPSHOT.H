#pragma pack(1)
struct hdrSNA128 {
   unsigned char i;
   unsigned short althl, altde, altbc, altaf;
   unsigned short hl, de, bc, iy, ix;
   unsigned char iff1; /* 00 - reset, FF - set */
   unsigned char r;
   unsigned short af, sp;
   unsigned char im,pFE;
   unsigned char page5[PAGE]; // 4000-7FFF
   unsigned char page2[PAGE]; // 8000-BFFF
   unsigned char active_page[PAGE]; // C000-FFFF
   /* 128k extension */
   unsigned short pc;
   unsigned char p7FFD;
   unsigned char trdos;
//   unsigned char pages[PAGE]; // all pages, except already saved
                                // (m.b. 5 or 6 pages)
};
struct hdrZ80 {
   unsigned char a,f;
   unsigned short bc,hl,pc,sp;
   unsigned char i,r,flags;
   unsigned short de,bc1,de1,hl1;
   unsigned char a1,f1;
   unsigned short iy,ix;
   unsigned char iff1, iff2, im;
   /* 2.01 extension */
   unsigned short len, newpc;
   unsigned char model, p7FFD;
   unsigned char r1,r2, p7FFD_1;
   unsigned char AY[16];
   /* 3.0 extension */
};
struct hdrSP {
   unsigned short sig;
   unsigned short len;
   unsigned short start;
   unsigned short bc,de,hl,af;
   unsigned short ix,iy;
   unsigned short altbc,altde,althl,altaf;
   unsigned char r, i;
   unsigned short sp, pc;
   unsigned short reserved;
   unsigned char pFE;
   unsigned char reserved2;
   unsigned short flags;
};
#pragma pack()