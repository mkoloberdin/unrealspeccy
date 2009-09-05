
struct GSHLE
{
   unsigned char gsstat;// command and data bits
   unsigned char gscmd;
   unsigned char busy;  // don't play fx
   unsigned char mod_playing;

   unsigned modvol, fxvol; // module and FX master volumes
   unsigned used;       // used memory
   unsigned char *mod; unsigned modsize; // for modplayer
   unsigned total_fx;      // samples loaded
   unsigned cur_fx;        // selected sample

   unsigned char data_in[8];// data register
   unsigned char gstmp[8], *resptr; unsigned resmode; // from GS
   unsigned char *to_ptr; unsigned resmod2; // to GS

   unsigned char *streamstart; unsigned streamsize;
   unsigned char load_stream;

   unsigned char loadmod, loadfx; // leds flags

   unsigned char badgs[0x100]; // unrecognized commands
   unsigned note2rate[0x100];

   struct SAMPLE {
      unsigned char *start;
      unsigned loop, end;
      unsigned char volume, note;
   } sample[64];

   struct CHANNEL {
      unsigned char *start;
      unsigned loop, end, ptr;
      unsigned volume, freq;
      HSTREAM bass_ch;
      unsigned char busy;
   } chan[4];

   unsigned char in(unsigned char port);
   void out(unsigned char port, unsigned char byte);
   void reset();
   void reset_sound();
   void applyconfig();

   void set_busy(unsigned char newval);
   void start_fx(unsigned fx, unsigned chan, unsigned char vol, unsigned char note);
   void flush_gs_frame(); // calc volume values for leds

   /* BASS-specific functions and variables */

   tBASS_GetVersion        BASS_GetVersion;
   tBASS_Init              BASS_Init;
   tBASS_Free              BASS_Free;
   tBASS_Pause             BASS_Pause;
   tBASS_Start             BASS_Start;
   tBASS_Stop              BASS_Stop;
   tBASS_GetConfig         BASS_GetConfig;
   tBASS_SetConfig         BASS_SetConfig;
   tBASS_GetInfo           BASS_GetInfo;

   tBASS_MusicFree         BASS_MusicFree;
   tBASS_MusicLoad         BASS_MusicLoad;
   tBASS_MusicGetOrderPosition BASS_MusicGetOrderPosition;
   tBASS_ChannelPause      BASS_ChannelPause;
   tBASS_ChannelPlay       BASS_ChannelPlay;

   tBASS_ChannelGetPosition BASS_ChannelGetPosition;
   tBASS_ChannelSetPosition BASS_ChannelSetPosition;
   tBASS_ChannelSetAttributes BASS_ChannelSetAttributes;
   tBASS_ChannelGetLevel   BASS_ChannelGetLevel;
   tBASS_ErrorGetCode      BASS_ErrorGetCode;
   tBASS_ChannelSetFlags   BASS_ChannelSetFlags;
   tBASS_StreamCreate      BASS_StreamCreate;
   tBASS_StreamFree        BASS_StreamFree;

   HMODULE hBass; HMUSIC hmod;

   void runBASS();
   void initChannels();
   void setmodvol(unsigned vol);
   void init_mod();
   void restart_mod(unsigned order, unsigned row);
   void startfx(CHANNEL *ch, unsigned pan);
   __inline void resetmod();
   __inline void resetfx();
   __inline DWORD modgetpos();
   __inline void stop_mod();
   __inline void cont_mod();
   void debug_note(unsigned i);

   void reportError(char *err);
};
