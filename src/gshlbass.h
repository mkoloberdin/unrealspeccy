

typedef DWORD HMUSIC;           // MOD music handle
typedef DWORD HSTREAM;
typedef DWORD (CALLBACK *STREAMPROC)(HSTREAM,void*,DWORD,DWORD);

typedef DWORD (WINAPI *tBASS_GetVersion)();
typedef int   (WINAPI *tBASS_ErrorGetCode)();
typedef BOOL  (WINAPI *tBASS_Init)(int,DWORD,DWORD,HWND,GUID*);
typedef VOID  (WINAPI *tBASS_Free)();
typedef BOOL  (WINAPI *tBASS_Pause)();
typedef BOOL  (WINAPI *tBASS_Start)();
typedef BOOL  (WINAPI *tBASS_Stop)();
typedef DWORD (WINAPI *tBASS_SetConfig)(DWORD,DWORD);
typedef DWORD (WINAPI *tBASS_GetConfig)(DWORD);

typedef BOOL  (WINAPI *tBASS_MusicFree)(HMUSIC);
typedef HMUSIC(WINAPI *tBASS_MusicLoad)(BOOL,void*,DWORD,DWORD,DWORD,DWORD);
typedef DWORD (WINAPI *tBASS_MusicGetOrderPosition)(HMUSIC);
typedef BOOL  (WINAPI *tBASS_ChannelPause)(DWORD);
typedef BOOL  (WINAPI *tBASS_ChannelPlay)(DWORD,BOOL);

typedef QWORD (WINAPI *tBASS_ChannelGetPosition)(DWORD);
typedef BOOL  (WINAPI *tBASS_ChannelSetPosition)(DWORD,QWORD);
typedef DWORD (WINAPI *tBASS_ChannelGetLevel)(DWORD);
typedef BOOL  (WINAPI *tBASS_ChannelSetFlags)(DWORD,DWORD);
typedef HSTREAM (WINAPI *tBASS_StreamCreate)(DWORD,DWORD,DWORD,STREAMPROC,DWORD);
typedef BOOL  (WINAPI *tBASS_StreamFree)(HSTREAM);
typedef BOOL  (WINAPI *tBASS_ChannelSetAttributes)(DWORD,int,int,int);

#define BASS_SAMPLE_8BITS               1       // 8 bit
#define BASS_SAMPLE_LOOP                4       // looped

#define BASS_MUSIC_RAMP                 0x200   // normal ramping
#define BASS_MUSIC_POSRESET             0x8000  // stop all notes when moving position
#define BASS_MUSIC_LOOP                 BASS_SAMPLE_LOOP // loop music

#define BASS_STREAMPROC_END             0x80000000      // end of user stream flag
#define BASS_DEVICE_LATENCY             256     // calculate device latency (BASS_INFO struct)

#define BASS_CONFIG_BUFFER              0
#define BASS_CONFIG_UPDATEPERIOD        1

#define MAKEMUSICPOS(order,row) (0x80000000|MAKELONG(order,row))

typedef struct
{
        DWORD flags;    // device capabilities (DSCAPS_xxx flags)
        DWORD hwsize;   // size of total device hardware memory
        DWORD hwfree;   // size of free device hardware memory
        DWORD freesam;  // number of free sample slots in the hardware
        DWORD free3d;   // number of free 3D sample slots in the hardware
        DWORD minrate;  // min sample rate supported by the hardware
        DWORD maxrate;  // max sample rate supported by the hardware
        BOOL eax;               // device supports EAX? (always FALSE if BASS_DEVICE_3D was not used)
        DWORD minbuf;   // recommended minimum buffer length in ms (requires BASS_DEVICE_LATENCY)
        DWORD dsver;    // DirectSound version
        DWORD latency;  // delay (in ms) before start of playback (requires BASS_DEVICE_LATENCY)
        DWORD initflags;// "flags" parameter of BASS_Init call
        DWORD speakers; // number of speakers available
        const char *driver;     // driver
        DWORD freq;             // current output rate (OSX only)
} BASS_INFO;

typedef BOOL  (WINAPI *tBASS_GetInfo)(BASS_INFO*);
