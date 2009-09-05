
int FDD::index()
{
   return this - comp.wd.fdd;
}

// return: 0 - CANCEL, 1 - YES/SAVED, 2 - NOT SAVED
char FDD::test()
{
   if (!optype) return 1;

   static char changed[] = "Disk A: changed. Save ?"; changed[5] = index()+'A';
   int r = MessageBox(GetForegroundWindow(), changed, "Save disk", exitflag? MB_ICONQUESTION | MB_YESNO : MB_ICONQUESTION | MB_YESNOCANCEL);
   if (r == IDCANCEL) return 0;
   if (r == IDNO) return 2;
   // r == IDYES
   unsigned char *image = (unsigned char*)malloc(sizeof snbuf);
   memcpy(image, snbuf, sizeof snbuf);
   savesnap(index());
   memcpy(snbuf, image, sizeof snbuf);
   ::free(image);

   if (*(volatile char*)&optype) return 0;
   return 1;
}

void FDD::free()
{
   if (rawdata) VirtualFree(rawdata, rawsize, MEM_RELEASE);
   memset(this, 0, sizeof(FDD)); conf.trdos_wp[index()] = 0;
   comp.wd.trkcache.clear(); t.clear();
}

void FDD::newdisk(unsigned cyls, unsigned sides)
{
   free();

   FDD::cyls = cyls; FDD::sides = sides;
   unsigned len = MAX_TRACK_LEN;
   unsigned len2 = len + (len/8) + ((len & 7) ? 1 : 0);
   rawsize = align_by(cyls * sides * len2, 4096);
   rawdata = (unsigned char*)VirtualAlloc(0, rawsize, MEM_COMMIT, PAGE_READWRITE);
   // ZeroMemory(rawdata, rawsize); // already done by VirtualAlloc

   for (unsigned i = 0; i < cyls; i++)
      for (unsigned j = 0; j < sides; j++) {
         trklen[i][j] = len;
         trkd[i][j] = rawdata + len2*(i*sides + j);
         trki[i][j] = trkd[i][j] + len;
      }
   // comp.wd.trkcache.clear(); // already done in free()
}

int FDD::read(unsigned char type)
{
   int ok = 0;
   if (type == snTRD) ok = read_trd();
   if (type == snUDI) ok = read_udi();
   if (type == snHOB) ok = read_hob();
   if (type == snSCL) ok = read_scl();
   if (type == snFDI) ok = read_fdi();
   if (type == snTD0) ok = read_td0();
   return ok;
}

#include "wldr_trd.cpp"
#include "wldr_udi.cpp"
#include "wldr_fdi.cpp"
#include "wldr_td0.cpp"
