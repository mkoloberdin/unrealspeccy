#include "std.h"

#include "emul.h"
#include "vars.h"
#include "sdcard.h"

#include "util.h"

void TSdCard::Reset()
{
//    printf(__FUNCTION__"\n");

    CurrState = ST_IDLE;
    ArgCnt = 0;
    Cmd = CMD_INVALID;
    DataBlockLen = 512;
    DataCnt = 0;

    CsdCnt = 0;
    memset(Csd, 0, sizeof(Csd));
    Csd[1] = (8 << 3); // TACC F=8.0 | 1ns
    Csd[3] = 0x32; // TRAN_SPEED
    Csd[4] = 0x5B;
    Csd[5] = 0x59;
    Csd[6] = (1 << 7);
    Csd[15] = 1;

    CidCnt = 0;
    memset(Cid, 0, sizeof(Cid));

    Cid[1] = 'U';
    Cid[2] = 'S';
    Cid[3] = 'U';
    Cid[4] = 'S';
    Cid[5] = '3';
    Cid[6] = '7';
    Cid[7] = '3';
    Cid[8] = 0x37;
    Cid[14] = 0x04;
    Cid[15] = 1;

    Ocr = 0x80200000;
    OcrCnt = 0;
    AppCmd = false;
}

void TSdCard::Wr(u8 Val)
{
    static u32 WrPos = -1;
    TState NextState = ST_IDLE;
//    printf(__FUNCTION__" Val = %X\n", Val);

    if(!Image)
        return;

    switch(CurrState)
    {
        case ST_IDLE:
        case ST_WR_DATA_SIG:
        {
            if((Val & 0xC0) != 0x40) // start=0, transm=1
               break;

            Cmd = TCmd(Val & 0x3F);
            if(!AppCmd)
            {
                switch(Cmd) // Check commands
                {
                case CMD_GO_IDLE_STATE:
//                    printf(__FUNCTION__" CMD_GO_IDLE_STATE, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_SEND_OP_COND:
//                    printf(__FUNCTION__" CMD_SEND_OP_COND, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_SET_BLOCKLEN:
//                    printf(__FUNCTION__" CMD_SET_BLOCKLEN, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_READ_SINGLE_BLOCK:
//                    printf(__FUNCTION__" CMD_READ_SINGLE_BLOCK, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_READ_MULTIPLE_BLOCK:
//                    printf(__FUNCTION__" CMD_READ_MULTIPLE_BLOCK, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_WRITE_BLOCK:
//                    printf(__FUNCTION__" CMD_WRITE_BLOCK, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_WRITE_MULTIPLE_BLOCK:
//                    printf(__FUNCTION__" CMD_WRITE_MULTIPLE_BLOCK, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_STOP_TRANSMISSION:
//                    printf(__FUNCTION__" CMD_STOP_TRANSMISSION, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_SEND_IF_COND:
//                    printf(__FUNCTION__" CMD_SEND_IF_COND, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_SEND_CSD:
//                    printf(__FUNCTION__" CMD_SEND_CSD, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                    CsdCnt = 0;
                break;

                case CMD_SEND_CID:
//                    printf(__FUNCTION__" CMD_SEND_CID, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                    CidCnt = 0;
                break;

                case CMD_CRC_ON_OFF:
//                    printf(__FUNCTION__" CMD_CRC_ON_OFF, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;

                case CMD_READ_OCR:
//                    printf(__FUNCTION__" CMD_READ_OCR, Val = %X\n", Val);
//                    __debugbreak();
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                    OcrCnt = 0;
                break;

                case CMD_APP_CMD:
//                    printf(__FUNCTION__" CMD_APP_CMD, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                    AppCmd = true;
                break;

                default:
//                    printf(__FUNCTION__" Unknown CMD, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                }
            }
            else // AppCmd
            {
                switch(Cmd)
                {
                case CMD_SD_SEND_OP_COND:
//                    printf(__FUNCTION__" CMD_SD_SEND_OP_COND, Val = %X\n", Val);
                    NextState = ST_RD_ARG;
                    ArgCnt = 0;
                break;
                default:
//                    printf(__FUNCTION__" Unknown ACMD, Val = %X\n", Val);
                    ;
                }
            }
        }
        break;

        case ST_RD_ARG:
            NextState = ST_RD_ARG;
            ArgArr[3 - ArgCnt++] = Val;

//            printf(__FUNCTION__" ST_RD_ARG val=0x%X\n", Val);
            if(ArgCnt == 4)
            {
                if(!AppCmd)
                {
                    switch(Cmd)
                    {
                    case CMD_READ_SINGLE_BLOCK:
//                        printf(__FUNCTION__" CMD_READ_SINGLE_BLOCK, Addr = 0x%X\n", Arg);
                        fseek(Image, Arg, SEEK_SET);
                        fread(Buf, 512, 1, Image);
                    break;

                    case CMD_READ_MULTIPLE_BLOCK:
//                        printf(__FUNCTION__" CMD_READ_MULTIPLE_BLOCK, Addr = 0x%X\n", Arg);
                        fseek(Image, Arg, SEEK_SET);
                        fread(Buf, 512, 1, Image);
                    break;

                    case CMD_WRITE_BLOCK:
//                        printf(__FUNCTION__" CMD_WRITE_BLOCK, Addr = 0x%X\n", Arg);
                    break;

                    case CMD_WRITE_MULTIPLE_BLOCK:
                        WrPos = Arg;
//                        printf(__FUNCTION__" CMD_WRITE_MULTIPLE_BLOCK, Addr = 0x%X\n", Arg);
                    break;
                    }
                }

                NextState = ST_RD_CRC;
                ArgCnt = 0;
            }
        break;

        case ST_RD_CRC:
//            printf(__FUNCTION__" ST_RD_CRC val=0x%X\n", Val);
            NextState = GetRespondType();
        break;

        case ST_RD_DATA_SIG:
            NextState = ST_RD_DATA;
/*
            if(Val != 0xFE) // Проверка сигнатуры данных
                __debugbreak();
*/
            DataCnt = 0;
        break;

        case ST_RD_DATA_SIG_MUL:
            switch(Val)
            {
            case 0xFC: // Проверка сигнатуры данных
//                printf(__FUNCTION__" ST_RD_DATA_SIG_MUL, Start\n");
                DataCnt = 0;
                NextState = ST_RD_DATA_MUL;
            break;
            case 0xFD: // Окончание передачи
//                printf(__FUNCTION__" ST_RD_DATA_SIG_MUL, Stop\n");
                NextState = ST_IDLE;
            break;
            default:
                NextState = ST_RD_DATA_SIG_MUL;
            }
        break;

        case ST_RD_DATA: // Прием данных в буфер
        {
//            printf(__FUNCTION__" ST_RD_DATA, Addr = 0x%X, Idx=%d\n", Arg, DataCnt);
            Buf[DataCnt++] = Val;
            NextState = ST_RD_DATA;
            if(DataCnt == DataBlockLen) // Запись данных в SD карту
            {
                DataCnt = 0;
//                printf(__FUNCTION__" ST_RD_DATA, Addr = 0x%X, write to disk\n", Arg);
                fseek(Image, Arg, SEEK_SET);
                fwrite(Buf, 512, 1, Image);
                NextState = ST_WR_DATA_RESP;
            }
        }
        break;

        case ST_RD_DATA_MUL: // Прием данных в буфер
        {
//            printf(__FUNCTION__" ST_RD_DATA_MUL, Addr = 0x%X, Idx=%d\n", WrPos, DataCnt);
            Buf[DataCnt++] = Val;
            NextState = ST_RD_DATA_MUL;
            if(DataCnt == DataBlockLen) // Запись данных в SD карту
            {
                DataCnt = 0;
//                printf(__FUNCTION__" ST_RD_DATA_MUL, Addr = 0x%X, write to disk\n", WrPos);
                fseek(Image, WrPos, SEEK_SET);
                fwrite(Buf, 512, 1, Image);
                WrPos += 512;
                NextState = ST_RD_DATA_SIG_MUL;
            }
        }
        break;

        case ST_RD_CRC16_1: // Чтение старшего байта CRC16
            NextState = ST_RD_CRC16_2;
        break;

        case ST_RD_CRC16_2: // Чтение младшего байта CRC16
            NextState = ST_WR_DATA_RESP;
        break;

        default:
//            printf(__FUNCTION__" St=0x%X,  val=0x%X\n", CurrState, Val);
            return;
    }

    CurrState = NextState;
}

u8 TSdCard::Rd()
{
//    printf(__FUNCTION__" cmd=0x%X, St=0x%X\n", Cmd, CurrState);
    if(!Image)
        return 0xFF;

    switch(Cmd)
    {
    case CMD_GO_IDLE_STATE:
        if(CurrState == ST_R1)
        {
//            Cmd = CMD_INVALID;
            CurrState = ST_IDLE;
            return 1;
        }
    break;
    case CMD_SEND_OP_COND:
        if(CurrState == ST_R1)
        {
//            Cmd = CMD_INVALID;
            CurrState = ST_IDLE;
            return 0;
        }
    break;
    case CMD_SET_BLOCKLEN:
        if(CurrState == ST_R1)
        {
//            Cmd = CMD_INVALID;
            CurrState = ST_IDLE;
            return 0;
        }
    break;
    case CMD_SEND_IF_COND:
        if(CurrState == ST_R7)
        {
            CurrState = ST_IDLE;
            return 5; // invalid command | idle state
        }
    break;

    case CMD_READ_OCR:
        if(CurrState == ST_R1)
        {
            CurrState = ST_R3;
            return 0;
        }
    break;

    case CMD_APP_CMD:
        if(CurrState == ST_R1)
        {
            CurrState = ST_IDLE;
            return 0;
        }
    break;

    case CMD_SD_SEND_OP_COND:
        if(CurrState == ST_R1)
        {
            CurrState = ST_IDLE;
            return 0;
        }
    break;

    case CMD_CRC_ON_OFF:
        if(CurrState == ST_R1)
        {
            CurrState = ST_IDLE;
            return 0;
        }
    break;

    case CMD_STOP_TRANSMISSION:
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_R1b;
            return 0;
        case ST_R1b:
            CurrState = ST_IDLE;
            return 0xFF;
        }
    break;

    case CMD_READ_SINGLE_BLOCK:
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_WR_DATA;
            return 0xFE;
        case ST_WR_DATA_SIG:
            CurrState = ST_WR_DATA;
            return 0xFE;
        case ST_WR_DATA:
        {
            u8 Val = Buf[DataCnt++];
            if(DataCnt == DataBlockLen)
            {
                DataCnt = 0;
                CurrState = ST_IDLE;
                Cmd = CMD_INVALID;
            }
            return Val;
        }
        }
//        Cmd = CMD_INVALID;
    break;

    case CMD_READ_MULTIPLE_BLOCK:
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_IDLE;
            return 0xFE;
        case ST_WR_DATA_SIG:
            CurrState = ST_IDLE;
            return 0xFE;
        case ST_IDLE:
        {
            u8 Val = Buf[DataCnt++];
            if(DataCnt == DataBlockLen)
            {
                DataCnt = 0;
                fread(Buf, 512, 1, Image);
                CurrState = ST_WR_CRC16_1;
            }
            return Val;
        }
        case ST_WR_CRC16_1:
            CurrState = ST_WR_CRC16_2;
            return 0xFF;
        case ST_WR_CRC16_2:
            CurrState = ST_WR_DATA_SIG;
            return 0xFF;
        }
    break;

    case CMD_SEND_CSD:
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_WR_DATA;
            return 0xFE;
        case ST_WR_DATA_SIG:
            CurrState = ST_WR_DATA;
            return 0xFE;
        case ST_WR_DATA:
        {
            u8 Val = Csd[CsdCnt++];
            if(CsdCnt == 16)
            {
                CsdCnt = 0;
                CurrState = ST_IDLE;
                Cmd = CMD_INVALID;
            }
            return Val;
        }
        }
//        Cmd = CMD_INVALID;
    break;

    case CMD_SEND_CID:
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_WR_DATA;
            return 0xFE;
        case ST_WR_DATA_SIG:
            CurrState = ST_WR_DATA;
            return 0xFE;
        case ST_WR_DATA:
        {
            u8 Val = Cid[CidCnt++];
            if(CidCnt == 16)
            {
                CidCnt = 0;
                CurrState = ST_IDLE;
                Cmd = CMD_INVALID;
            }
            return Val;
        }
        }
//        Cmd = CMD_INVALID;
    break;

    case CMD_WRITE_BLOCK:
//        printf(__FUNCTION__" cmd=0x%X, St=0x%X\n", Cmd, CurrState);
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_RD_DATA_SIG;
            return 0xFE;

        case ST_WR_DATA_RESP:
        {
            CurrState = ST_IDLE;
            u8 Resp = ((STAT_DATA_ACCEPTED) << 1) | 1;
            return Resp;
        }
        }
    break;

    case CMD_WRITE_MULTIPLE_BLOCK:
        switch(CurrState)
        {
        case ST_R1:
            CurrState = ST_RD_DATA_SIG_MUL;
            return 0xFE;
        case ST_WR_DATA_RESP:
        {
            CurrState = ST_RD_DATA_SIG_MUL;
            u8 Resp = ((STAT_DATA_ACCEPTED) << 1) | 1;
            return Resp;
        }
        }
    break;
    }

    if(CurrState == ST_R3)
    {
        u8 Val = OcrArr[3 - OcrCnt++];

        if(OcrCnt == 4)
        {
            CurrState = ST_IDLE;
            OcrCnt = 0;
        }
        return Val;
    }
    return 0xFF;
}

TSdCard::TState TSdCard::GetRespondType()
{
    if(!AppCmd)
    {
        switch(Cmd)
        {
        case CMD_GO_IDLE_STATE:
        case CMD_SEND_OP_COND:
        case CMD_SET_BLOCKLEN:
        case CMD_READ_SINGLE_BLOCK:
        case CMD_READ_MULTIPLE_BLOCK:
        case CMD_APP_CMD:
        case CMD_CRC_ON_OFF:
        case CMD_STOP_TRANSMISSION:
        case CMD_SEND_CSD:
        case CMD_SEND_CID:
            return ST_R1;
        case CMD_READ_OCR:
            return ST_R1; // R3
        case CMD_SEND_IF_COND:
            return ST_R7;

        case CMD_WRITE_BLOCK:
            return ST_RD_DATA_SIG;
        case CMD_WRITE_MULTIPLE_BLOCK:
            return ST_RD_DATA_SIG_MUL;
        }
    }
    else
    {
        switch(Cmd)
        {
            case CMD_SD_SEND_OP_COND:
                AppCmd = false;
                return ST_R1;
        }
    }

    return ST_IDLE;
}

void TSdCard::Open(const char *Name)
{
//    printf(__FUNCTION__"\n");
    assert(!Image);
    Image = fopen(Name, "r+b");
    if(!Image && Name[0])
    {
        errmsg("can't find SD card image `%s'", Name);
    }
}

void TSdCard::Close()
{
    if(Image)
    {
        fclose(Image);
        Image = 0;
    }
}

TSdCard SdCard;
