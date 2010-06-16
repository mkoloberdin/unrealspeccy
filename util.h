#pragma once

#define VK_ALT VK_MENU

#define WORD4(a,b,c,d) (((unsigned)(a)) | (((unsigned)(b)) << 8) | (((unsigned)(c)) << 16) | (((unsigned)(d)) << 24))
#define WORD2(a,b) ((a) | ((b)<<8))
#define align_by(a,b) (((ULONG_PTR)(a) + ((b)-1)) & ~((b)-1))
#define hexdigit(a) ((a) < 'A' ? (a)-'0' : toupper(a)-'A'+10)

extern const char nop;
extern const char * const nil;

void eat();
void trim(char *dst);
void errmsg(const char *err, const char *str = 0);
void err_win32(DWORD errcode = 0xFFFFFFFF);
void color(int ink = CONSCLR_DEFAULT);
int ishex(char c);
unsigned char hex(char p);
unsigned char hex(const char *p);

void __declspec(noreturn) errexit(const char *err, const char *str = 0);
unsigned process_msgs();
char dispatch(action *table);
char dispatch_more(action *table);

#ifdef _M_IX86
void __cdecl fillCpuString(char *dst);
unsigned cpuid(unsigned _eax, int ext);
#endif
unsigned __int64 GetCPUFrequency();

forceinline unsigned __int64 rdtsc()
{
#ifdef __INTEL_COMPILER
    return __rdtsc();
#else
    __asm rdtsc
#endif
}

bool wcmatch(char *string, char *wc);
