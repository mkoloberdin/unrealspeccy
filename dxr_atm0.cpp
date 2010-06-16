#include "std.h"

#include "emul.h"
#include "vars.h"
#include "draw.h"
#include "dxr_atmf.h"
#include "dxr_atm0.h"

const int ega0_ofs = -4*PAGE;
const int ega1_ofs = 0;
const int ega2_ofs = -4*PAGE+0x2000;
const int ega3_ofs = 0x2000;

void line_atm0_8(unsigned char *dst, unsigned char *src, unsigned *tab, int src_offset)
{
    for (unsigned x = 0; x < 640; x += 0x10) {
        src_offset &= 0x1FFF;
        *(unsigned*)(dst+x+0x00) = tab[src[ega0_ofs + src_offset]];
        *(unsigned*)(dst+x+0x04) = tab[src[ega1_ofs + src_offset]];
        *(unsigned*)(dst+x+0x08) = tab[src[ega2_ofs + src_offset]];
        *(unsigned*)(dst+x+0x0C) = tab[src[ega3_ofs + src_offset]];
        ++src_offset;
    }
}

void line_atm0_16(unsigned char *dst, unsigned char *src, unsigned *tab, int src_offset)
{
    for (unsigned x = 0; x < 640*2; x += 0x20) {
        src_offset &= 0x1FFF;
        *(unsigned*)(dst+x+0x00) = tab[0+2*src[ega0_ofs + src_offset]];
        *(unsigned*)(dst+x+0x04) = tab[1+2*src[ega0_ofs + src_offset]];
        *(unsigned*)(dst+x+0x08) = tab[0+2*src[ega1_ofs + src_offset]];
        *(unsigned*)(dst+x+0x0C) = tab[1+2*src[ega1_ofs + src_offset]];
        *(unsigned*)(dst+x+0x10) = tab[0+2*src[ega2_ofs + src_offset]];
        *(unsigned*)(dst+x+0x14) = tab[1+2*src[ega2_ofs + src_offset]];
        *(unsigned*)(dst+x+0x18) = tab[0+2*src[ega3_ofs + src_offset]];
        *(unsigned*)(dst+x+0x1C) = tab[1+2*src[ega3_ofs + src_offset]];
        ++src_offset;
    }
}

void line_atm0_32(unsigned char *dst, unsigned char *src, unsigned *tab, int src_offset)
{
    for (unsigned x = 0; x < 640*4; x += 0x40) {
        src_offset &= 0x1FFF;
        *(unsigned*)(dst+x+0x00) = *(unsigned*)(dst+x+0x04) = tab[0+2*src[ega0_ofs + src_offset]];
        *(unsigned*)(dst+x+0x08) = *(unsigned*)(dst+x+0x0C) = tab[1+2*src[ega0_ofs + src_offset]];
        *(unsigned*)(dst+x+0x10) = *(unsigned*)(dst+x+0x14) = tab[0+2*src[ega1_ofs + src_offset]];
        *(unsigned*)(dst+x+0x18) = *(unsigned*)(dst+x+0x1C) = tab[1+2*src[ega1_ofs + src_offset]];
        *(unsigned*)(dst+x+0x20) = *(unsigned*)(dst+x+0x24) = tab[0+2*src[ega2_ofs + src_offset]];
        *(unsigned*)(dst+x+0x28) = *(unsigned*)(dst+x+0x2C) = tab[1+2*src[ega2_ofs + src_offset]];
        *(unsigned*)(dst+x+0x30) = *(unsigned*)(dst+x+0x34) = tab[0+2*src[ega3_ofs + src_offset]];
        *(unsigned*)(dst+x+0x38) = *(unsigned*)(dst+x+0x3C) = tab[1+2*src[ega3_ofs + src_offset]];
        ++src_offset;
    }
}

void rend_atm0(unsigned char *dst, unsigned pitch, int y, int Offset)
{
    unsigned char *dst2 = dst + (temp.ox-640)*temp.obpp/16;
    if (temp.scy > 200) 
        dst2 += (temp.scy-200)/2*pitch * ((temp.oy > temp.scy)?2:1);

    if (conf.fast_sl) {
        dst2 += y*pitch;
        switch(temp.obpp)
        {
        case 8:
            line_atm0_8 (dst2, temp.base, t.p4bpp8[0], Offset);
            break;
        case 16:
            line_atm0_16(dst2, temp.base, t.p4bpp16[0], Offset);
            break;
        case 32:
            line_atm0_32(dst2, temp.base, t.p4bpp32[0], Offset);
            break;
        }
    } else {
        dst2 += 2*y*pitch;
        switch(temp.obpp)
        {
        case 8:
            line_atm0_8 (dst2, temp.base, t.p4bpp8[0], Offset);
            dst2 += pitch;
            line_atm0_8 (dst2, temp.base, t.p4bpp8[1], Offset);
            break;
        case 16:
            line_atm0_16(dst2, temp.base, t.p4bpp16[0], Offset);
            dst2 += pitch;
            line_atm0_16(dst2, temp.base, t.p4bpp16[1], Offset);
            break;
        case 32:
            line_atm0_32(dst2, temp.base, t.p4bpp32[0], Offset);
            dst2 += pitch;
            line_atm0_32(dst2, temp.base, t.p4bpp32[1], Offset);
            break;
        }
    }
}
