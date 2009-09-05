#define _WIN32_WINNT        0x0500   // mouse wheel since win2k
// #define _WIN32_IE           0x0300   // for property sheet in win95. without this will not start in 9x
#define DIRECTINPUT_VERSION 0x0500   // joystick since dx 5.0 (for NT4, need 3.0)
#define DIRECTSOUND_VERSION 0x0500
#define DIRECTDRAW_VERSION  0x0500

#include <windows.h>
#include <commctrl.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <urlmon.h>
#include <mshtmhst.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <math.h>
#include <emmintrin.h>

#pragma warning(disable:4288)  // loop control variable declared in the for-loop is used outside the for-loop scope;
                               // it conflicts with the declaration in the outer scope


#pragma comment(lib, "dinput.lib")
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")
//#pragma comment(linker, "settings.res")


#define CACHE_LINE 64
#define CACHE_ALIGNED __declspec(align(CACHE_LINE))

#define INLINE __forceinline
