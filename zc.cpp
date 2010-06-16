// Z-Controller by KOE
// Only SD-card
#include "std.h"

#include "emul.h"
#include "vars.h"
#include "sdcard.h"
#include "zc.h"

void TZc::Reset()
{
    SdCard.Reset();
    Cfg = 0;
    Status = 0;
}

void TZc::Wr(u32 Port, u8 Val)
{
    switch(Port & 0xFF)
    {
    case 0x77: // config
        Cfg = Val & 3;
    break;
    case 0x57: // data
        SdCard.Wr(Val);
    break;
    }
}

u8 TZc::Rd(u32 Port)
{
    switch(Port & 0xFF)
    {
    case 0x77: // status
    return Status;
    case 0x57: // data
    return SdCard.Rd();
    }
    return 0xFF;
}

TZc Zc;
