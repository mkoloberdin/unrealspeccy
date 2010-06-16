#pragma once
#define DRIVER_DDRAW    0
#define DRIVER_GDI      1
#define DRIVER_OVR      2
#define DRIVER_BLT      3
#define DRIVER_DDRAW16  4
#define DRIVER_DDRAW32  5

#define MAXWQSIZE 32

extern const RENDER drivers[];
extern size_t renders_count;

extern unsigned char active;
extern unsigned char pause;

extern LPDIRECTDRAW7 dd;
extern LPDIRECTDRAWSURFACE7 sprim;
extern LPDIRECTDRAWSURFACE7 surf0;
extern LPDIRECTDRAWSURFACE7 surf1;

extern LPDIRECTINPUTDEVICE2 dijoyst;

void sound_play();
void sound_stop();
void __fastcall do_sound_none();
void __fastcall do_sound_wave();
void __fastcall do_sound_ds();
void do_sound();
void setpal(char system);
void set_priority();

void flip();
void set_vidmode();
void updatebitmap();
void adjust_mouse_cursor();
void start_dx();
void done_dx();

void readdevice(VOID *md, DWORD sz, LPDIRECTINPUTDEVICE dev);
void readmouse(DIMOUSESTATE *md);
void ReadKeyboard(PVOID KbdData);
