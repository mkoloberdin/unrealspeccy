#ifndef _Z80_DEFS_H_INCLUDED
#define _Z80_DEFS_H_INCLUDED

#include "../sysdefs.h"

struct Z80;
#define Z80FAST fastcall
#define Z80INLINE forceinline // time-critical inlines

typedef void (Z80FAST *STEPFUNC)(Z80*);
#define Z80OPCODE void Z80FAST
typedef unsigned char (Z80FAST *LOGICFUNC)(Z80*, unsigned char byte);
#define Z80LOGIC unsigned char Z80FAST

struct Z80
{
   unsigned t;
   /*------------------------------*/
   union {
      unsigned pc;
      struct {
         unsigned char pcl;
         unsigned char pch;
      };
   };
   union {
      unsigned sp;
      struct {
         unsigned char spl;
         unsigned char sph;
      };
   };
   union {
      unsigned ir_;
      struct {
         unsigned char r_low;
         unsigned char i;
      };
   };
   union {
      unsigned int_flags;
      struct {
         unsigned char r_hi;
         unsigned char iff1;
         unsigned char iff2;
         unsigned char halted;
      };
   };
   /*------------------------------*/
   union {
      unsigned bc;
      unsigned short bc16;
      struct {
         unsigned char c;
         unsigned char b;
      };
   };
   union {
      unsigned de;
      struct {
         unsigned char e;
         unsigned char d;
      };
   };
   union {
      unsigned hl;
      struct {
         unsigned char l;
         unsigned char h;
      };
   };
   union {
      unsigned af;
      struct {
         unsigned char f;
         unsigned char a;
      };
   };
   /*------------------------------*/
   union {
      unsigned ix;
      struct {
         unsigned char xl;
         unsigned char xh;
      };
   };
   union {
      unsigned iy;
      struct {
         unsigned char yl;
         unsigned char yh;
      };
   };
   /*------------------------------*/
   struct {
      union {
         unsigned bc;
         struct {
            unsigned char c;
            unsigned char b;
         };
      };
      union {
         unsigned de;
         struct {
            unsigned char e;
            unsigned char d;
         };
      };
      union {
         unsigned hl;
         struct {
            unsigned char l;
            unsigned char h;
         };
      };
      union {
         unsigned af;
         struct {
            unsigned char f;
            unsigned char a;
         };
      };
   } alt;
   union {
      unsigned memptr; // undocumented register
      struct {
        unsigned char meml;
        unsigned char memh;
      };
   };
   unsigned eipos, haltpos;
   /*------------------------------*/
   unsigned char im;
   unsigned char tmp0, tmp1, tmp3;
   unsigned short last_branch;
   unsigned trace_curs, trace_top, trace_mode;
   unsigned mem_curs, mem_top, mem_second;
   unsigned pc_trflags;
   unsigned nextpc;
   unsigned dbg_stophere;
   unsigned dbg_stopsp;
   unsigned dbg_loop_r1;
   unsigned dbg_loop_r2;
   u32 trpc[40];
   typedef u8 (__fastcall * TRmDbg)(u32 addr);
   typedef u8 *(__fastcall * TMemDbg)(u32 addr);
   typedef void ( *TBankNames)(int i, char *Name);
   typedef void (* TStep)();
   TRmDbg RmDbg; // read memory in debuger
   TMemDbg MemDbg; // get direct memory pointer in debuger
   TBankNames BankNames;
   TStep Step;
   void reset() { int_flags = ir_ = pc = 0; im = 0; last_branch = 0; }
   Z80(TRmDbg RmDbg = 0, TMemDbg MemDbg = 0, TBankNames BankNames = 0, TStep Step = 0) :
       RmDbg(RmDbg), MemDbg(MemDbg), BankNames(BankNames), Step(Step)
   {
       trace_curs = trace_top = -1; trace_mode = 0;
       mem_curs = mem_top = 0;
       pc_trflags = nextpc = 0;
       dbg_stophere = dbg_stopsp = -1;
       dbg_loop_r1 = 0;
       dbg_loop_r2 = 0xFFFF;
   }
};

#define CF 0x01
#define NF 0x02
#define PV 0x04
#define F3 0x08
#define HF 0x10
#define F5 0x20
#define ZF 0x40
#define SF 0x80

#endif // _Z80_DEFS_H_INCLUDED
