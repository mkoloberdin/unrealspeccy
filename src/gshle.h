
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

   struct {
      unsigned char *start;
      unsigned loop, end;
      unsigned char volume, note;
   } sample[64];

   struct CHANNEL {
      unsigned char *start;
      unsigned loop, end, ptr;
      unsigned volume, delta;
      SOUND_STREAM sound_state;
   } chan[4];

   unsigned char in(unsigned char port);
   void out(unsigned char port, unsigned char byte);
   void reset();
   void applyconfig();

   void start_fx(unsigned fx, unsigned chan, unsigned char vol, unsigned char note);
   void mixchannel(CHANNEL *ch, unsigned left_ch);
   __inline void mix_fx();

   /* bass */
   typedef DWORD HMUSIC;           // MOD music handle
   typedef DWORD (WINAPI *tBASS_GetVersion)();
   typedef BOOL  (WINAPI *tBASS_Init)(int,DWORD,DWORD,HWND);
   typedef VOID  (WINAPI *tBASS_Free)();
   typedef BOOL  (WINAPI *tBASS_Pause)();
   typedef VOID  (WINAPI *tBASS_SetGlobalVolumes)(int,int,int);
   typedef BOOL  (WINAPI *tBASS_Start)();
   typedef BOOL  (WINAPI *tBASS_Stop)();
   typedef BOOL  (WINAPI *tBASS_MusicFree)(HMUSIC);
   typedef HMUSIC(WINAPI *tBASS_MusicLoad)(BOOL,void*,DWORD,DWORD,DWORD);
   typedef BOOL  (WINAPI *tBASS_MusicPlayEx)(HMUSIC,DWORD,int,BOOL);
   typedef DWORD (WINAPI *tBASS_ChannelGetPosition)(DWORD);
   typedef DWORD (WINAPI *tBASS_ChannelSetPosition)(DWORD,DWORD);
   typedef DWORD (WINAPI *tBASS_ChannelGetLevel)(DWORD);
   typedef DWORD (WINAPI *tBASS_ErrorGetCode)();

   tBASS_GetVersion        BASS_GetVersion;
   tBASS_Init              BASS_Init;
   tBASS_Free              BASS_Free;
   tBASS_Pause             BASS_Pause;
   tBASS_SetGlobalVolumes  BASS_SetGlobalVolumes;
   tBASS_Start             BASS_Start;
   tBASS_Stop              BASS_Stop;
   tBASS_MusicFree         BASS_MusicFree;
   tBASS_MusicLoad         BASS_MusicLoad;
   tBASS_MusicPlayEx       BASS_MusicPlayEx;
   tBASS_ChannelGetPosition BASS_ChannelGetPosition;
   tBASS_ChannelSetPosition BASS_ChannelSetPosition;
   tBASS_ChannelGetLevel   BASS_ChannelGetLevel;
   tBASS_ErrorGetCode      BASS_ErrorGetCode;

   HMODULE hBass; HMUSIC hmod;

   void runBASS();
   void setmodvol(unsigned vol);
   void init_mod();
   void restart_mod(unsigned pos);
   __inline void resetmod();
   __inline DWORD modgetpos();
   __inline void stop_mod();
   __inline void cont_mod();

   void reportError(char *err);
};

#define BASS_MUSIC_RAMP         1       // normal ramping
#define BASS_MUSIC_RAMPS        2       // sensitive ramping
#define BASS_MUSIC_LOOP         4       // loop music
#define BASS_MUSIC_FT2MOD       16      // play .MOD as FastTracker 2 does
#define BASS_MUSIC_PT1MOD       32      // play .MOD as ProTracker 1 does
#define BASS_MUSIC_MONO         64      // force mono mixing (less CPU usage)
#define BASS_MUSIC_3D           128     // enable 3D functionality
#define BASS_MUSIC_POSRESET     256     // stop all notes when moving position
#define BASS_MUSIC_SURROUND     512     // surround sound
#define BASS_MUSIC_SURROUND2    1024    // surround sound (mode 2)
#define BASS_MUSIC_STOPBACK     2048    // stop the music on a backwards jump effect
#define BASS_MUSIC_FX           4096    // enable DX8 effects
#define BASS_MUSIC_CALCLEN      8192    // calculate playback length

#define BASS_DEVICE_8BITS       1       // use 8 bit resolution, else 16 bit
#define BASS_DEVICE_MONO        2       // use mono, else stereo
#define BASS_DEVICE_3D          4       // enable 3D functionality
#define BASS_DEVICE_A3D         8       // enable A3D functionality
#define BASS_DEVICE_NOSYNC      16      // disable synchronizers
#define BASS_DEVICE_LEAVEVOL    32      // leave the volume as it is
#define BASS_DEVICE_OGG         64      // enable OGG support (requires OGG.DLL & VORBIS.DLL)
#define BASS_DEVICE_NOTHREAD    128     // update buffers manually (using BASS_Update)
#define BASS_DEVICE_LATENCY     256     // calculate device latency (BASS_INFO struct)
