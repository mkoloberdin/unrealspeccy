#define _WIN32_WINNT        0x0500   // mouse wheel since win2k
#define _WIN32_IE           0x0500   // for property sheet in win95. without this will not start in 9x
#define DIRECTINPUT_VERSION 0x05b2   // joystick since dx 5.0 (for NT4, need 3.0)
#define DIRECTSOUND_VERSION 0x0800
#define DIRECTDRAW_VERSION  0x0500
//#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include <windows.h>
#include <windowsx.h>
#include <setupapi.h>
#include <commctrl.h>
#include "ddraw.h"

#ifdef DEBUG
#define D3D_DEBUG_INFO 1
#endif

#include <d3d9.h>
#include "dinput.h"
#include "dsound.h"
#include "urlmon.h"
#include "mshtmhst.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <malloc.h>
#include <conio.h>
#include <math.h>
#include <process.h>

#ifdef _M_IX86
#include <assert.h>
#else
#define assert(x)
#endif

#if _MSC_VER >= 1300
#include <intrin.h>
#include <emmintrin.h>
#endif

#ifdef __GNUC__
#include <x86intrin.h>
#include <cpuid.h>
#include <mcx.h>
#include <winioctl.h>
#include "mingww32bugs.h"
#endif

#include "ddk.h"

#include "mods.h"

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
#pragma comment(lib, "dxerr8.lib")
#pragma comment(lib, "setupapi.lib")
//#pragma comment(linker, "settings.res")


#define CACHE_LINE 64

#if _MSC_VER >= 1300
#define CACHE_ALIGNED __declspec(align(CACHE_LINE))
#else
#define CACHE_ALIGNED /*Alone Coder*/
#endif
