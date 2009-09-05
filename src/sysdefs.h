
#ifndef __SYSDEFS_H_INCLUDED
#define __SYSDEFS_H_INCLUDED

#define inline __inline
#define forceinline __forceinline
#define fastcall __fastcall             // parameters in registers

#ifdef _MSC_VER
typedef unsigned __int64 uint64_t;
#endif

typedef uint64_t QWORD;

#ifdef __GNUC__
#include <stdint.h>
#define HANDLE_PRAGMA_PACK_PUSH_POP
#endif


#endif // __SYSDEFS_H_INCLUDED