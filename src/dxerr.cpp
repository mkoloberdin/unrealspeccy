#ifdef EMUL_DEBUG
void printrdd(char *pr, HRESULT r)
{
   char buf[200]; sprintf(buf, "unknown error (%08X)", r);
   char *str = buf;
   switch (r) {
      case DD_OK: str = "ok"; break;
      case DDERR_GENERIC: str = "DDERR_GENERIC"; break;
      case DDERR_INCOMPATIBLEPRIMARY: str = "DDERR_INCOMPATIBLEPRIMARY"; break;
      case DDERR_INVALIDCAPS: str = "DDERR_INVALIDCAPS"; break;
      case DDERR_INVALIDMODE: str = "DDERR_INVALIDMODE"; break;
      case DDERR_INVALIDOBJECT: str = "DDERR_INVALIDOBJECT"; break;
      case DDERR_INVALIDPARAMS: str = "DDERR_INVALIDPARAMS"; break;
      case DDERR_INVALIDPIXELFORMAT: str = "DDERR_INVALIDPIXELFORMAT"; break;
      case DDERR_LOCKEDSURFACES: str = "DDERR_LOCKEDSURFACES"; break;
      case DDERR_NOALPHAHW: str = "DDERR_NOALPHAHW"; break;
      case DDERR_NOCOOPERATIVELEVELSET: str = "DDERR_NOCOOPERATIVELEVELSET"; break;
      case DDERR_NODIRECTDRAWHW: str = "DDERR_NODIRECTDRAWHW"; break;
      case DDERR_NOEMULATION: str = "DDERR_NOEMULATION"; break;
      case DDERR_NOTLOCKED: str = "DDERR_NOTLOCKED"; break;
      case DDERR_NOEXCLUSIVEMODE: str = "DDERR_NOEXCLUSIVEMODE"; break;
      case DDERR_NOFLIPHW: str = "DDERR_NOFLIPHW"; break;
      case DDERR_NOMIPMAPHW: str = "DDERR_NOMIPMAPHW"; break;
      case DDERR_NOTFLIPPABLE: str = "DDERR_NOTFLIPPABLE"; break;
      case DDERR_NOZBUFFERHW: str = "DDERR_NOZBUFFERHW"; break;
      case DDERR_OUTOFMEMORY: str = "DDERR_OUTOFMEMORY"; break;
      case DDERR_OUTOFVIDEOMEMORY: str = "DDERR_OUTOFVIDEOMEMORY"; break;
      case DDERR_PRIMARYSURFACEALREADYEXISTS: str = "DDERR_PRIMARYSURFACEALREADYEXISTS"; break;
      case DDERR_SURFACEBUSY: str = "DDERR_SURFACEBUSY"; break;
      case DDERR_SURFACELOST: str = "DDERR_SURFACELOST"; break;
      case DDERR_UNSUPPORTED: str = "DDERR_UNSUPPORTED"; break;
      case DDERR_UNSUPPORTEDMODE: str = "DDERR_UNSUPPORTEDMODE"; break;
      case DDERR_WASSTILLDRAWING: str = "DDERR_WASSTILLDRAWING"; break;
      case DDERR_WRONGMODE: str = "DDERR_WRONGMODE"; break;
      case DDERR_NOOVERLAYHW: str = "DDERR_NOOVERLAYHW"; break;
      case DDERR_NOBLTHW: str = "DDERR_NOBLTHW"; break;
      case DDERR_NOSTRETCHHW: str = "DDERR_NOSTRETCHHW"; break;
      case DDERR_INVALIDSURFACETYPE: str = "DDERR_INVALIDSURFACETYPE"; break;
      case DDERR_CANTLOCKSURFACE: str = "DDERR_CANTLOCKSURFACE"; break;
      case DDERR_NOPALETTEATTACHED: str = "DDERR_NOPALETTEATTACHED"; break;
      case DDERR_INVALIDRECT: str = "DDERR_INVALIDRECT"; break;
      case DDERR_NOTFOUND: str = "DDERR_NOTFOUND"; break;
   }
   color(CONSCLR_ERROR);
   printf("%s: %s\n", pr, str);
}

void printrdi(char *pr, HRESULT r)
{
   char buf[200]; sprintf(buf, "unknown error (%08X)", r);
   char *str = buf;
   switch (r) {
      case DI_OK: str = "ok"; break;
      case DI_BUFFEROVERFLOW: str = "DI_BUFFEROVERFLOW"; break;
      case DIERR_INVALIDPARAM: str = "DIERR_INVALIDPARAM"; break;
      case DIERR_NOTACQUIRED: str = "DIERR_NOTACQUIRED"; break;
      case DIERR_INPUTLOST: str = "DIERR_INPUTLOST"; break;
      case DIERR_NOTINITIALIZED: str = "DIERR_NOTINITIALIZED"; break;
      case DIERR_OTHERAPPHASPRIO: str = "DIERR_OTHERAPPHASPRIO"; break;
   }
   color(CONSCLR_ERROR);
   printf("%s: %s\n", pr, str);
}

void printrmm(char *pr, HRESULT r)
{
   char buf[200]; sprintf(buf, "unknown error (%08X)", r);
   char *str = buf;
   switch (r) {
      case MMSYSERR_NOERROR: str = "ok"; break;
      case MMSYSERR_INVALHANDLE: str = "MMSYSERR_INVALHANDLE"; break;
      case MMSYSERR_NODRIVER: str = "MMSYSERR_NODRIVER"; break;
      case WAVERR_UNPREPARED: str = "WAVERR_UNPREPARED"; break;
      case MMSYSERR_NOMEM: str = "MMSYSERR_NOMEM"; break;
      case MMSYSERR_ALLOCATED: str = "MMSYSERR_ALLOCATED"; break;
      case WAVERR_BADFORMAT: str = "WAVERR_BADFORMAT"; break;
      case WAVERR_SYNC: str = "WAVERR_SYNC"; break;
      case MMSYSERR_INVALFLAG: str = "MMSYSERR_INVALFLAG"; break;
   }
   color(CONSCLR_ERROR);
   printf("%s: %s\n", pr, str);
}

void printrds(char *pr, HRESULT r)
{
   char buf[200]; sprintf(buf, "unknown error (%08X)", r);
   char *str = buf;
   switch (r) {
      case DS_OK: str = "ok"; break;
      case DSERR_BADFORMAT: str = "DSERR_BADFORMAT"; break;
      case DSERR_BUFFERLOST: str = "DSERR_BUFFERLOST"; break;
      case DSERR_ALLOCATED: str = "DSERR_ALLOCATED"; break;
      case DSERR_UNINITIALIZED: str = "DSERR_UNINITIALIZED"; break;
      case DSERR_INVALIDPARAM: str = "DSERR_INVALIDPARAM"; break;
      case DSERR_INVALIDCALL: str = "DSERR_INVALIDCALL"; break;
      case DSERR_OTHERAPPHASPRIO: str = "DSERR_OTHERAPPHASPRIO"; break;
      case DSERR_NODRIVER: str = "DSERR_NODRIVER"; break;
   }
   color(CONSCLR_ERROR);
   printf("%s: %s\n", pr, str);
}
#else
#define printrdd(x,y)
#define printrdi(x,y)
#define printrds(x,y)
#define printrmm(x,y)
#endif
