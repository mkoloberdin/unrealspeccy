#include "std.h"

#include "emul.h"
#include "vars.h"
#include "draw.h"
#include "dxrend.h"
#include "dxr_atm0.h"
#include "dxr_atm2.h"
#include "dxr_atm6.h"
#include "dxr_atm7.h"
#include "dxr_atmf.h"

/*
#include "dxr_atmf.cpp"
#include "dxr_atm0.cpp"
#include "dxr_atm2.cpp"
#include "dxr_atm6.cpp"
#include "dxr_atm7.cpp"
*/

void rend_atm_frame(unsigned char *dst, unsigned pitch)
{
    if (conf.fast_sl) {
        switch(temp.obpp)
        {
        case 8:
            rend_atmframe_x2_8s (dst, pitch);
            break;
        case 16:
            rend_atmframe_x2_16s (dst, pitch);
            break;
        case 32:
            rend_atmframe_x2_32s(dst, pitch);
            break;
        }
    } else {
        switch(temp.obpp)
        {
        case 8:
            rend_atmframe_x2_8d (dst, pitch);
            break;
        case 16:
            rend_atmframe_x2_16d(dst, pitch);
            break;
        case 32:
            rend_atmframe_x2_32d(dst, pitch);
            break;
        }
    }

}

void rend_atm_2(unsigned char *dst, unsigned pitch)
{
    if (temp.comp_pal_changed) {
        pixel_tables();
        temp.comp_pal_changed = 0;
    }

    if ( 3 == (comp.pFF77 & 7) ) //< Sinclair VideoMode
    {
        rend_dbl(dst, pitch);
        return;
    }

    if ( 7 == (comp.pFF77 & 7) ) //< Undocumented Sinclair Textmode VideoMode
    {
        rend_atm7(dst, pitch);
        return;
    }
    
    if (temp.oy > temp.scy && conf.fast_sl) 
        pitch *= 2;
    rend_atm_frame(dst, pitch);

    for (int y=0; y<200; y++)
    {
        const AtmVideoController::ScanLine& Scanline = AtmVideoCtrl.Scanlines[y+56];
        switch (Scanline.VideoMode)
        {
        case 0: //< EGA 320x200
            rend_atm0(dst, pitch, y, Scanline.Offset);
            break;
        case 2: // Hardware Multicolor
            rend_atm2(dst, pitch, y, Scanline.Offset);
            break;
        case 6: //< Textmode
            rend_atm6(dst, pitch, y, Scanline.Offset);
            break;
        }
    }
}

void rend_atm_1(unsigned char *dst, unsigned pitch)
{
   if (temp.comp_pal_changed) {
      pixel_tables();
      temp.comp_pal_changed = 0;
   }

   int VideoMode = (comp.aFE >> 5) & 3;
   if ( 3 == VideoMode ) //< Sinclair VideoMode
   {
       rend_dbl(dst, pitch);
       return;
   }

   if (temp.oy > temp.scy && conf.fast_sl) 
       pitch *= 2;
   rend_atm_frame(dst, pitch);

   for (int y=0; y<200; y++)
   {
       const AtmVideoController::ScanLine& Scanline = AtmVideoCtrl.Scanlines[y+56];
       switch (Scanline.VideoMode)
       {
       case 0: //< EGA 320x200
           rend_atm0(dst, pitch, y, Scanline.Offset);
           break;
       case 1: // Hardware Multicolor
           rend_atm2(dst, pitch, y, Scanline.Offset);
           break;
       }
   }
}
