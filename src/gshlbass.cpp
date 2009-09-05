
void GSHLE::reportError(char *err)
{
   color(CONSCLR_ERROR);
   printf("BASS library reports error in %s\n", err);
   color(CONSCLR_ERRCODE);
   printf("error code is 0x%04X\n", BASS_ErrorGetCode());
   exit();
}

void GSHLE::runBASS()
{
   if (hBass) return;
   hBass = LoadLibrary("bass.dll");
   if (!hBass) errexit("can't load bass.dll");

   BASS_GetVersion = (tBASS_GetVersion)GetProcAddress(hBass, "BASS_GetVersion");
   if (!BASS_GetVersion || (HIWORD(BASS_GetVersion()) != 0x0203))
      errexit("unexpected BASS version. unreal requires BASS 2.3");

   BASS_ErrorGetCode = (tBASS_ErrorGetCode)GetProcAddress(hBass, "BASS_ErrorGetCode");
   BASS_Init = (tBASS_Init)GetProcAddress(hBass, "BASS_Init");
   BASS_Free = (tBASS_Free)GetProcAddress(hBass, "BASS_Free");
   BASS_Pause = (tBASS_Pause)GetProcAddress(hBass, "BASS_Pause");
   BASS_Start = (tBASS_Start)GetProcAddress(hBass, "BASS_Start");
   BASS_Stop = (tBASS_Start)GetProcAddress(hBass, "BASS_Stop");
   BASS_SetConfig = (tBASS_SetConfig)GetProcAddress(hBass, "BASS_SetConfig");
   BASS_GetConfig = (tBASS_GetConfig)GetProcAddress(hBass, "BASS_GetConfig");
   BASS_GetInfo = (tBASS_GetInfo)GetProcAddress(hBass, "BASS_GetInfo");

   BASS_MusicFree = (tBASS_MusicFree)GetProcAddress(hBass, "BASS_MusicFree");
   BASS_MusicLoad = (tBASS_MusicLoad)GetProcAddress(hBass, "BASS_MusicLoad");
   BASS_MusicGetOrderPosition = (tBASS_MusicGetOrderPosition)GetProcAddress(hBass, "BASS_MusicGetOrderPosition");
   BASS_ChannelPlay = (tBASS_ChannelPlay)GetProcAddress(hBass, "BASS_ChannelPlay");
   BASS_ChannelPause = (tBASS_ChannelPause)GetProcAddress(hBass, "BASS_ChannelPause");

   BASS_ChannelGetPosition = (tBASS_ChannelGetPosition)GetProcAddress(hBass, "BASS_ChannelGetPosition");
   BASS_ChannelSetPosition = (tBASS_ChannelSetPosition)GetProcAddress(hBass, "BASS_ChannelSetPosition");
   BASS_ChannelSetAttributes = (tBASS_ChannelSetAttributes)GetProcAddress(hBass, "BASS_ChannelSetAttributes");
   BASS_ChannelSetFlags = (tBASS_ChannelSetFlags)GetProcAddress(hBass, "BASS_ChannelSetFlags");
   BASS_ChannelGetLevel = (tBASS_ChannelGetLevel)GetProcAddress(hBass, "BASS_ChannelGetLevel");
   BASS_StreamCreate = (tBASS_StreamCreate)GetProcAddress(hBass, "BASS_StreamCreate");
   BASS_StreamFree = (tBASS_StreamFree)GetProcAddress(hBass, "BASS_StreamFree");

   if (!BASS_Init || !BASS_Free || !BASS_Pause ||
         !BASS_Start || !BASS_Stop || !BASS_MusicFree ||
         !BASS_MusicLoad || !BASS_MusicGetOrderPosition ||
         !BASS_GetConfig || !BASS_SetConfig || !BASS_GetInfo ||
         !BASS_ChannelSetFlags || !BASS_ChannelSetAttributes ||
         !BASS_ChannelGetPosition || !BASS_ChannelSetPosition ||
         !BASS_ChannelGetLevel || !BASS_ErrorGetCode ||
         !BASS_StreamCreate || !BASS_StreamFree ||
         !BASS_ChannelPlay || !BASS_ChannelPause
   ) errexit("can't import BASS API");

   if (BASS_Init(-1, conf.sound.fq, BASS_DEVICE_LATENCY, wnd, 0)) {
      DWORD len = BASS_GetConfig(BASS_CONFIG_UPDATEPERIOD);
      BASS_INFO info; BASS_GetInfo(&info);
      BASS_SetConfig(BASS_CONFIG_BUFFER, len + info.minbuf);
      color(CONSCLR_HARDITEM); printf("BASS device latency is "); color(CONSCLR_HARDINFO); printf("%dms\n", len + info.minbuf);
   } else {
      color(CONSCLR_WARNING); printf("warning: can't use default BASS device, trying silence\n");
      if (!BASS_Init(-2, 11025, 0, wnd, 0)) errexit("can't init BASS");
   }

   hmod = 0;
   for (int ch = 0; ch < 4; ch++) chan[ch].bass_ch = 0;
}

void GSHLE::reset_sound()
{
  // runBASS();
  // BASS_Stop(); // todo: move to silent state?
}


DWORD CALLBACK gs_render(HSTREAM handle, void *buffer, DWORD length, DWORD user);

void GSHLE::initChannels()
{
   if (chan->bass_ch) return;
   for (int ch = 0; ch < 4; ch++) {
      chan[ch].bass_ch = BASS_StreamCreate(11025, 1, BASS_SAMPLE_8BITS, gs_render, (DWORD)&chan[ch]);
      if (!chan[ch].bass_ch) reportError("BASS_StreamCreate()");
   }
}

void GSHLE::setmodvol(unsigned vol)
{
   if (!hmod) return;
   runBASS();
   vol = (vol * conf.sound.bass * 100) / (8000 * 64);
   if (!BASS_ChannelSetAttributes(hmod, -1, vol, -101)) reportError("BASS_ChannelSetAttributes() [music volume]");
}

void GSHLE::init_mod()
{
   runBASS();
   if (hmod) BASS_MusicFree(hmod); hmod = 0;
   hmod = BASS_MusicLoad(1, mod, 0, modsize, BASS_MUSIC_LOOP | BASS_MUSIC_POSRESET | BASS_MUSIC_RAMP, 0);
   if (!hmod) reportError("BASS_MusicLoad()");
}

void GSHLE::restart_mod(unsigned order, unsigned row)
{
   if (!hmod) return;
   if (!BASS_ChannelSetPosition(hmod, MAKEMUSICPOS(order,row))) reportError("BASS_ChannelSetPosition() [music]");
   if (!BASS_ChannelSetFlags(hmod, BASS_MUSIC_LOOP | BASS_MUSIC_POSRESET | BASS_MUSIC_RAMP)) reportError("BASS_ChannelSetFlags() [music]");
   BASS_Start();
   if (!BASS_ChannelPlay(hmod, TRUE)) reportError("BASS_ChannelPlay() [music]");

   mod_playing = 1;
}

void GSHLE::resetmod()
{
   if (hmod) BASS_MusicFree(hmod); hmod = 0;
}

void GSHLE::resetfx()
{
   runBASS();
   for (int i = 0; i < 4; i++)
     if (chan[i].bass_ch)
       BASS_StreamFree(chan[i].bass_ch), chan[i].bass_ch = 0;
}

DWORD GSHLE::modgetpos()
{
   runBASS();
   //return (DWORD)BASS_ChannelGetPosition(hmod);
   return BASS_MusicGetOrderPosition(hmod);
}

void GSHLE::stop_mod()
{
   runBASS();
   if (!hmod) return;
   if (!BASS_ChannelPause(hmod)) reportError("BASS_ChannelPause() [music]");
}

void GSHLE::cont_mod()
{
   runBASS();
   if (!hmod) return;
   if (!BASS_ChannelPlay(hmod, TRUE)) reportError("BASS_ChannelPlay() [music]");
}

void GSHLE::startfx(CHANNEL *ch, unsigned pan)
{
   initChannels();

   unsigned vol = (ch->volume * conf.sound.gs * 100) / (8000*64);
   if (!BASS_ChannelSetAttributes(ch->bass_ch, ch->freq, vol, pan)) reportError("BASS_ChannelSetAttributes()");
   if (!BASS_ChannelPlay(ch->bass_ch, FALSE)) reportError("BASS_ChannelPlay()");
}

void GSHLE::flush_gs_frame()
{
   unsigned lvl;
   if (!hmod || (lvl = BASS_ChannelGetLevel(hmod)) == -1) lvl = 0;

   gsleds[0].level = LOWORD(lvl) >> (15-4);
   gsleds[0].attrib = 0x0D;
   gsleds[1].level = HIWORD(lvl) >> (15-4);
   gsleds[1].attrib = 0x0D;

   for (int ch = 0; ch < 4; ch++) {
      if (chan[ch].bass_ch && (lvl = BASS_ChannelGetLevel(chan[ch].bass_ch)) != -1) {
         lvl = max(HIWORD(lvl), LOWORD(lvl));
         lvl >>= (15-4);
      } else lvl = 0;
      gsleds[ch+2].level = lvl;
      gsleds[ch+2].attrib = 0x0F;
   }
}

void GSHLE::debug_note(unsigned i)
{
   GSHLE::CHANNEL ch = { 0 };
   ch.volume = 64; ch.ptr = 0;
   ch.start = sample[i].start;
   ch.loop = sample[i].loop;
   ch.end = sample[i].end;
   unsigned note = sample[i].note; if (note == 60) note = 50;
   ch.freq = note2rate[note];
   ch.bass_ch = BASS_StreamCreate(11025, 1, BASS_SAMPLE_8BITS, gs_render, (DWORD)&ch);
   startfx(&ch, 0);
   unsigned mx = (sample[i].loop < sample[i].end)? 5000 : 10000;
   for (unsigned j = 0; j < mx/256; j++) {
      if (!ch.start) break;
      Sleep(256);
   }
   BASS_StreamFree(ch.bass_ch);
}
