#ifndef SDCARD_H
#define SDCARD_H

const u8 _SDNCS = 1;
const u8 _SDDET = 2;
const u8 _SDWP  = 4;

class TSdCard
{
    enum TCmd
    {
        CMD_INVALID = -1,
        CMD0 = 0,
        CMD_GO_IDLE_STATE = CMD0,
        CMD1 = 1,
        CMD_SEND_OP_COND = CMD1,
        CMD8 = 8,
        CMD_SEND_IF_COND = CMD8,
        CMD12 = 12,
        CMD_STOP_TRANSMISSION = CMD12,
        CMD16 = 16,
        CMD_SET_BLOCKLEN = CMD16,
        CMD17 = 17,
        CMD_READ_SINGLE_BLOCK = CMD17,
        CMD18 = 18,
        CMD_READ_MULTIPLE_BLOCK = CMD18,
        CMD55 = 55,
        CMD_APP_CMD = CMD55,
        CMD58 = 58,
        CMD_READ_OCR = CMD58,
        CMD59 = 59,
        CMD_CRC_ON_OFF = CMD59,
        ACMD41 = 41,
        CMD_SD_SEND_OP_COND = ACMD41,
    };
    enum TState
    {
        ST_IDLE, ST_RD_ARG, ST_RD_CRC, ST_R1, ST_R1b, ST_R2, ST_R3, ST_R7,
        ST_WR_DATA_SIG, ST_WR_DATA, ST_WR_CRC16_1, ST_WR_CRC16_2
    };
    TState CurrState;
    union
    {
    	u8 ArgArr[4];
    	u32 Arg;
    };
    u32 ArgCnt;

    union
    {
    	u8 OcrArr[4];
    	u32 Ocr;
    };
    u32 OcrCnt;

    bool AppCmd;
    TCmd Cmd;
    u32 DataBlockLen;
    u32 DataCnt;

    u8 Buf[512];

    FILE *Image;
public:
    TSdCard() { Image = 0; Reset(); }
    void Reset();
    void Open(const char *Name);
    void Close();
    void Wr(u8 Val);
    u8 Rd();
private:
    TState GetRespondType();
};
extern TSdCard SdCard;
#endif
