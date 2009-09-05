
void GSHLE::runBASS()
{
   if (hBass) return;
   hBass = LoadLibrary("bass.dll");
   if (!hBass) { printf("can't load bass.dll"); exit(); }

   BASS_GetVersion = (tBASS_GetVersion)GetProcAddress(hBass, "BASS_GetVersion");
   unsigned v1, v2; char ln[64];
   GetPrivateProfileString("*","BASS",nil,ln,sizeof ln, ininame);
   if (!BASS_GetVersion || (sscanf(ln,"%d.%d", &v1, &v2) == 2 && (int)BASS_GetVersion() != MAKELONG(v1,v2)))
   { printf("error: unexpected BASS version"); exit(); }

   BASS_Init = (tBASS_Init)GetProcAddress(hBass, "BASS_Init");
   BASS_Free = (tBASS_Free)GetProcAddress(hBass, "BASS_Free");
   BASS_Pause = (tBASS_Pause)GetProcAddress(hBass, "BASS_Pause");
   BASS_SetGlobalVolumes = (tBASS_SetGlobalVolumes)GetProcAddress(hBass, "BASS_SetGlobalVolumes");
   BASS_Start = (tBASS_Start)GetProcAddress(hBass, "BASS_Start");
   BASS_Stop = (tBASS_Start)GetProcAddress(hBass, "BASS_Stop");
   BASS_MusicFree = (tBASS_MusicFree)GetProcAddress(hBass, "BASS_MusicFree");
   BASS_MusicLoad = (tBASS_MusicLoad)GetProcAddress(hBass, "BASS_MusicLoad");
   BASS_MusicPlayEx = (tBASS_MusicPlayEx)GetProcAddress(hBass, "BASS_MusicPlayEx");
   BASS_ChannelGetPosition = (tBASS_ChannelGetPosition)GetProcAddress(hBass, "BASS_ChannelGetPosition");
   BASS_ChannelSetPosition = (tBASS_ChannelSetPosition)GetProcAddress(hBass, "BASS_ChannelSetPosition");
   BASS_ChannelGetLevel = (tBASS_ChannelGetLevel)GetProcAddress(hBass, "BASS_ChannelGetLevel");
   BASS_ErrorGetCode = (tBASS_ErrorGetCode)GetProcAddress(hBass, "BASS_ErrorGetCode");

   if (!BASS_Init || !BASS_Free || !BASS_Pause || !BASS_Start ||
         !BASS_SetGlobalVolumes || !BASS_Stop || !BASS_MusicFree ||
         !BASS_MusicLoad || !BASS_MusicPlayEx ||
         !BASS_ChannelGetPosition || !BASS_ChannelSetPosition ||
         !BASS_ChannelGetLevel || !BASS_ErrorGetCode
   ) { printf("can't import BASS API"); exit(); }

   if (!BASS_Init(-1, conf.sound.fq, BASS_DEVICE_NOSYNC, wnd)) {
      printf("warning: can't use default BASS device, trying silence\n");
      if (!BASS_Init(-2, 11025, BASS_DEVICE_NOSYNC, wnd)) { printf("can't init BASS"); exit(); }
   }
   hmod = 0;
}

void GSHLE::setmodvol(unsigned vol)
{
   runBASS();
   BASS_SetGlobalVolumes(100 * vol * conf.sound.bass / (0x40*32768), 0, 0);
}

void GSHLE::init_mod()
{
   runBASS();
   if (hmod) BASS_MusicFree(hmod); hmod = 0;
   hmod = BASS_MusicLoad(1, mod, 0, modsize, BASS_MUSIC_LOOP | BASS_MUSIC_POSRESET | BASS_MUSIC_RAMPS);
   if (!hmod) { printf("BASS_MusicLoad(): %08X\n", BASS_ErrorGetCode()); exit(); }
}

void GSHLE::restart_mod(unsigned pos)
{
   if (!hmod) return;
   BASS_Start();
   if (!BASS_MusicPlayEx(hmod, pos, BASS_MUSIC_LOOP | BASS_MUSIC_POSRESET | BASS_MUSIC_RAMPS, 1))
   { printf("BASS_MusicPlayEx(): %08X\n", BASS_ErrorGetCode()); exit(); }
   /*else*/
   mod_playing = 1;
}

void GSHLE::resetmod()
{
   if (hmod) BASS_MusicFree(hmod); hmod = 0;
}

DWORD GSHLE::modgetpos()
{
   runBASS();
   return BASS_ChannelGetPosition(hmod);
}

void GSHLE::stop_mod()
{
   runBASS();
   BASS_Pause();
}

void GSHLE::cont_mod()
{
   runBASS();
   BASS_Start();
}
