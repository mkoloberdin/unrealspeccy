#define Z80FQ 3500000 // !! todo: #define Z80FQ = (conf.frame*conf.intfq)
#define FDD_RPS 5     // rotation speed

const MAX_TRACK_LEN = 6912;
const MAX_CYLS = 86;            // don't load images with so many tracks
const MAX_PHYS_CYL = 86;        // don't seek over it
const MAX_SEC = 256;

struct SECHDR {
   unsigned char c,s,n,l;
   unsigned short crc;
   unsigned char c1, c2; // correct CRCs in address and data
   unsigned char *data, *id;
   unsigned datlen;
   unsigned crcd;
};

struct DISK {
   unsigned char *rawdata;              // used in VirtualAlloc/VirtualFree
   unsigned rawsize;
   unsigned cyls, sides;
   unsigned optype; // bits: 0-not modified, 1-write sector, 2-format track
   unsigned trklen[MAX_CYLS][2];
   unsigned char *trkd[MAX_CYLS][2];
   unsigned char *trki[MAX_CYLS][2];
   char name[0x200];
   char dsc[0x200];
} disk[4];

struct {
   unsigned drive, cyl, side;
   unsigned cyls, sides, trklen;
   unsigned char *trkd, *trki;
   unsigned ts_byte, s, sf;
   SECHDR hdr[MAX_SEC];
} trkcache;

__declspec(naked) unsigned __fastcall wordswap(unsigned x) { // shit for intel
   __asm bswap ecx
   __asm shr ecx,16
   __asm mov eax,ecx
   __asm ret
}

unsigned wd93_crc(unsigned char *ptr, unsigned size) {
   unsigned crc = 0xCDB4;
   while (size--) {
      crc ^= (*ptr++) << 8;
      for (int j = 8; j; j--) // todo: rewrite with pre-calc'ed table
         if ((crc *= 2) & 0x10000) crc ^= 0x1021; // bit representation of x^12+x^5+1
   }
   return wordswap(crc); // return crc & 0xFFFF;
}

void seek_track(unsigned n, unsigned cyl, unsigned side, unsigned fs=0)
{
   if (!((n-trkcache.drive)|(trkcache.sf-fs)|(cyl-trkcache.cyl)|(side-trkcache.side))) return;
   DISK *d = &disk[n]; trkcache.sf = fs;
   trkcache.cyls = d->cyls, trkcache.sides = d->sides;
   trkcache.cyl = cyl, trkcache.side = side, trkcache.drive = n;
   if (cyl >= d->cyls) { trkcache.trkd = 0; return; }
   trkcache.trkd = d->trkd[cyl][side];
   trkcache.trki = d->trki[cyl][side];
   trkcache.trklen = d->trklen[cyl][side];
   if (!trkcache.trklen) { trkcache.trkd = 0; return; }
   trkcache.ts_byte = Z80FQ/(trkcache.trklen*FDD_RPS);
   if (!fs) return; // else find sectors
   trkcache.s = 0;
   if (!d->rawdata) { trkcache.trkd = 0; return; }
   for (unsigned i = 0; i < trkcache.trklen; i++) {
      if (trkcache.trkd[i] == 0xA1 && trkcache.trkd[i+1] == 0xFE && (trkcache.trki[i/8] & (1<<(i&7)))) {
         if (trkcache.s == MAX_SEC) { printf("too many sectors"); exit(); }
         SECHDR *h = &trkcache.hdr[trkcache.s++];
         h->id = trkcache.trkd+i+2; *(unsigned*)h = *(unsigned*)h->id;
         h->crc = *(unsigned short*)(trkcache.trkd+i+6);
         h->c1 = (wd93_crc(trkcache.trkd+i+1, 5) == h->crc);
         h->data = 0; h->datlen = 0;
         if (h->l > 5) continue;
         for (unsigned j = i+8; j < trkcache.trklen; j++) {
            if (trkcache.trkd[j] == 0xA1 && (trkcache.trki[j/8] & (1<<(j&7))) && !(trkcache.trki[(j+1)/8] & (1<<((j+1)&7)))) {
               if (trkcache.trkd[j+1] == 0xF8 || trkcache.trkd[j+1] == 0xFB) {
                  h->datlen = 128 << h->l;
                  h->data = trkcache.trkd+j+2;
                  h->c2 = (wd93_crc(h->data-1, h->datlen+1) == *(unsigned short*)(h->data+h->datlen));
               }
               break;
            }
         }
      }
   }
}

// return: 0 - CANCEL, 1 - YES/SAVED, 2 - NOT SAVED
char testdisk(unsigned n)
{
   if (!disk[n].optype) return 1;

   static char changed[] = "Disk A: changed. Save ?"; changed[5] = n+'A';
   int r = MessageBox(GetForegroundWindow(), changed, "Save disk", exitflag? MB_ICONQUESTION | MB_YESNO : MB_ICONQUESTION | MB_YESNOCANCEL);
   if (r == IDCANCEL) return 0;
   if (r == IDNO) return 2;
   // r == IDYES
   unsigned char *image = (unsigned char*)malloc(sizeof snbuf);
   memcpy(image, snbuf, sizeof snbuf);
   savesnap(n);
   memcpy(snbuf, image, sizeof snbuf);
   free(image);
   if (disk[n].optype) return 0;
   return 1;
}

void freedisk(unsigned n)
{
   DISK *d = &disk[n];
   if (d->rawdata) VirtualFree(d->rawdata, d->rawsize, MEM_RELEASE);
   memset(d, 0, sizeof *d); conf.trdos_wp[n] = 0;
   trkcache.drive=-1; seek_track(n,0,0);
}

void new_disk(unsigned n, unsigned cyls, unsigned sides)
{
   DISK *d = &disk[n]; freedisk(n);
   unsigned trklen = MAX_TRACK_LEN;
   unsigned trklen2 = trklen + (trklen/8) + ((trklen & 7) ? 1 : 0);
   d->rawsize = align(cyls * sides * trklen2, 4096);
   d->rawdata = (unsigned char*)VirtualAlloc(0, d->rawsize, MEM_COMMIT, PAGE_READWRITE);
   // ZeroMemory(d->rawdata, d->rawsize); // already done by VirtualAlloc
   d->cyls = cyls; d->sides = sides;
   for (unsigned i = 0; i < cyls; i++)
      for (unsigned j = 0; j < sides; j++) {
         d->trklen[i][j] = trklen;
         d->trkd[i][j] = d->rawdata + trklen2*(i*sides + j);
         d->trki[i][j] = d->trkd[i][j] + trklen;
      }
   trkcache.drive = -1; seek_track(n,0,0);
}

void format_track(unsigned n, unsigned cyl, unsigned side, unsigned numsec, SECHDR *sechdr) {
   trkcache.drive = -1; seek_track(n, cyl, side);
   memset(trkcache.trkd, 0, trkcache.trklen);
   memset(trkcache.trki, 0, trkcache.trklen/8 + ((trkcache.trklen&7) ? 1:0));

   unsigned char *p = trkcache.trkd;

   for (unsigned i = 0; i < 80; i++) *p++ = 0x4E; // 1st gap
   for (i = 0; i < 12; i++) *p++ = 0;
   for (i = 0; i < 3; i++) {
      unsigned n = p-trkcache.trkd;
      trkcache.trki[n/8] |= (1<<(n&7));
      *p++ = 0xC2;
   }
   *p++ = 0xFC; // index
   for (unsigned s = 0; s < numsec; s++, sechdr++) {
      for (i = 0; i < 50; i++) *p++ = 0x4E;
      for (i = 0; i < 12; i++) *p++ = 0;
      for (i = 0; i < 3; i++) {
         unsigned n = p-trkcache.trkd;
         trkcache.trki[n/8] |= (1<<(n&7));
         *p++ = 0xA1;
      }
      *p++ = 0xFE; // marker
      *(unsigned*)p = *(unsigned*)sechdr; p += 4;
      unsigned crc = wd93_crc(p-5, 5);
      if (sechdr->c1==1) crc = sechdr->crc;
      if (sechdr->c1==2) crc ^= 0xFFFF;
      *(unsigned*)p = crc; p += 2;
      if (sechdr->data) {
         for (i = 0; i < 22; i++) *p++ = 0x4E;
         for (i = 0; i < 12; i++) *p++ = 0;
         for (i = 0; i < 3; i++) {
            unsigned n = p-trkcache.trkd;
            trkcache.trki[n/8] |= (1<<(n&7));
            *p++ = 0xA1;
         }
         *p++ = 0xFB; // sector
         if (sechdr->l > 5) { printf("strange sector"); exit(); }
         unsigned len = 128 << sechdr->l;
         if (sechdr->data != (unsigned char*)1) memcpy(p, sechdr->data, len);
         else memset(p, 0, len);
         crc = wd93_crc(p-1, len+1);
         if (sechdr->c2==1) crc = sechdr->crcd;
         if (sechdr->c2==2) crc ^= 0xFFFF;
         *(unsigned*)(p+len) = crc; p += len+2;
      }
   }
//printf("c%d,s%d l=%d\n", cyl,side,p- trkcache.trkd);
   if (p > trkcache.trklen + trkcache.trkd) { printf("error: track too long"); exit(); }
   while (p < trkcache.trkd + trkcache.trklen) *p++ = 0x4E;
}

#if 0
void new_trdos_disk(unsigned n) {
   new_disk(n, 80, 2);
}
#endif

#include "wldr_trd.cpp"
#include "wldr_udi.cpp"
#include "wldr_fdi.cpp"
#include "wldr_td0.cpp"
//#include "wldr_rd.cpp"

