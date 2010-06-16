#pragma once
#define PNG_NO_STDIO
#define PNG_NO_GLOBAL_ARRAYS

#include "png/zlib.h"
#include "png/pngconf.h"
#include "png/png.h"

bool PngInit();
void PngDone();
BOOL PngSaveImage (FILE *pfFile, png_byte *pDiData,
                   int iWidth, int iHeight, png_color bkgColor);
