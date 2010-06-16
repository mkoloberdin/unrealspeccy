
#ifndef __SYSDEFS_H_INCLUDED
#define __SYSDEFS_H_INCLUDED

#define inline __inline
#define forceinline __forceinline
#define fastcall __fastcall             // parameters in registers

#ifdef _MSC_VER
typedef unsigned __int64 uint64_t;
#endif

typedef uint64_t QWORD;

typedef unsigned long u32;
typedef unsigned short u16;
typedef signed  short i16;
typedef unsigned char u8;

#if __ICL >= 1000
static inline u8 rol8(u8 val, u8 shift)
{
    __asm__ volatile ("rolb %1,%0" : "=r"(val) : "cI"(shift), "0"(val));
    return val;
}

static inline u8 ror8(u8 val, u8 shift)
{
    __asm__ volatile ("rorb %1,%0" : "=r"(val) : "cI"(shift), "0"(val));
    return val;
}
static inline void asm_pause() { __asm__("pause"); }
#else
static inline u8 rol8(u8 val, u8 shift) { return _rotl8(val, shift); }
static inline u8 ror8(u8 val, u8 shift) { return _rotr8(val, shift); }
static inline void asm_pause() { __asm {rep nop} }
#endif

#if _MSC_VER < 1300
static inline u16 _byteswap_ushort(u16 i){return (i>>8)|(i<<8);}
static inline u32 _byteswap_ulong(u32 i){return _byteswap_ushort((u16)(i>>16))|(_byteswap_ushort((u16)i)<<16);};
#endif

#ifdef __GNUC__
#include <stdint.h>
#define HANDLE_PRAGMA_PACK_PUSH_POP
#endif


#endif // __SYSDEFS_H_INCLUDED