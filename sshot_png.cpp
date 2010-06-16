#include "std.h"

#include "emul.h"
#include "vars.h"
#include "sshot_png.h"

typedef void (PNGAPI *png_write_end_ptr) PNGARG((png_structp png_ptr,
   png_infop info_ptr));
typedef void (PNGAPI *png_write_image_ptr) PNGARG((png_structp png_ptr,
   png_bytepp image));
typedef void (PNGAPI *png_set_bgr_ptr) PNGARG((png_structp png_ptr));
typedef void (PNGAPI *png_write_info_ptr) PNGARG((png_structp png_ptr,
   png_infop info_ptr));
typedef void (PNGAPI *png_set_IHDR_ptr) PNGARG((png_structp png_ptr,
   png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int interlace_method, int compression_method,
   int filter_method));
typedef void (PNGAPI *png_set_write_fn_ptr) PNGARG((png_structp png_ptr,
   png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn));
typedef void (PNGAPI *png_destroy_write_struct_ptr)
   PNGARG((png_structpp png_ptr_ptr, png_infopp info_ptr_ptr));
typedef png_infop (PNGAPI *png_create_info_struct_ptr)
   PNGARG((png_structp png_ptr));
typedef void (PNGAPI *png_set_compression_level_ptr) PNGARG((png_structp png_ptr,
   int level));
typedef png_structp (PNGAPI *png_create_write_struct_ptr)
   PNGARG((png_const_charp user_png_ver, png_voidp error_ptr,
   png_error_ptr error_fn, png_error_ptr warn_fn));
typedef void (PNGAPI *png_set_filter_ptr) PNGARG((png_structp png_ptr, int method,
   int filters));

png_error_ptr png_error_p = 0;
png_write_end_ptr png_write_end_p = 0;
png_write_image_ptr png_write_image_p = 0;
png_set_bgr_ptr png_set_bgr_p = 0;
png_write_info_ptr png_write_info_p = 0;
png_set_IHDR_ptr png_set_IHDR_p = 0;
png_set_write_fn_ptr png_set_write_fn_p = 0;
png_destroy_write_struct_ptr png_destroy_write_struct_p = 0;
png_create_info_struct_ptr png_create_info_struct_p = 0;
png_set_compression_level_ptr png_set_compression_level_p = 0;
png_create_write_struct_ptr png_create_write_struct_p = 0;
png_set_filter_ptr png_set_filter_p = 0;

static HMODULE PngDll = 0;
bool PngInit()
{
    PngDll = LoadLibrary("libpng12.dll");
    if(!PngDll)
        return false;
    png_error_p = (png_error_ptr)GetProcAddress(PngDll, "png_error");
    if(!png_error_p)
        return false;
    png_write_end_p = (png_write_end_ptr)GetProcAddress(PngDll, "png_write_end");
    if(!png_write_end_p)
        return false;
    png_write_image_p = (png_write_image_ptr)GetProcAddress(PngDll, "png_write_image");
    if(!png_write_image_p)
        return false;
    png_set_bgr_p = (png_set_bgr_ptr)GetProcAddress(PngDll, "png_set_bgr");
    if(!png_set_bgr_p)
        return false;
    png_write_info_p = (png_write_info_ptr)GetProcAddress(PngDll, "png_write_info");
    if(!png_write_info_p)
        return false;
    png_set_IHDR_p = (png_set_IHDR_ptr)GetProcAddress(PngDll, "png_set_IHDR");
    if(!png_set_IHDR_p)
        return false;
    png_set_write_fn_p = (png_set_write_fn_ptr)GetProcAddress(PngDll, "png_set_write_fn");
    if(!png_set_write_fn_p)
        return false;
    png_destroy_write_struct_p = (png_destroy_write_struct_ptr)GetProcAddress(PngDll, "png_destroy_write_struct");
    if(!png_destroy_write_struct_p)
        return false;
    png_create_info_struct_p = (png_create_info_struct_ptr)GetProcAddress(PngDll, "png_create_info_struct");
    if(!png_create_info_struct_p)
        return false;
    png_set_compression_level_p = (png_set_compression_level_ptr)GetProcAddress(PngDll, "png_set_compression_level");
    if(!png_set_compression_level_p)
        return false;
    png_create_write_struct_p = (png_create_write_struct_ptr)GetProcAddress(PngDll, "png_create_write_struct");
    if(!png_create_write_struct_p)
        return false;
    png_set_filter_p = (png_set_filter_ptr)GetProcAddress(PngDll, "png_set_filter");
    if(!png_set_filter_p)
        return false;
    return true;
}

void PngDone()
{
    if(PngDll)
        FreeLibrary(PngDll);
}

static png_structp png_ptr = NULL;
static png_infop info_ptr = NULL;

static void
PNGAPI
png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_uint_32 check;

   check = fwrite(data, 1, length, (FILE *)(png_ptr->io_ptr));
   if (check != length)
   {
      png_error_p(png_ptr, "Write Error");
   }
}

static void
PNGAPI
png_flush(png_structp png_ptr)
{
   FILE *io_ptr;
   io_ptr = (FILE *)CVT_PTR((png_ptr->io_ptr));
   if (io_ptr != NULL)
      fflush(io_ptr);
}

BOOL PngSaveImage (FILE *pfFile, png_byte *pDiData,
                   int iWidth, int iHeight, png_color bkgColor)
{
    const int           ciBitDepth = 8;
    const int           ciChannels = 3;

    png_uint_32         ulRowBytes;
    static png_byte   **ppbRowPointers = NULL;
    int                 i;

    // prepare the standard PNG structures
    png_ptr = png_create_write_struct_p(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        return FALSE;
    }

    png_set_filter_p(png_ptr, 0, PNG_NO_FILTERS);

    png_set_compression_level_p(png_ptr, 4/*Z_BEST_COMPRESSION*/);

    info_ptr = png_create_info_struct_p(png_ptr);
    if (!info_ptr)
    {
        png_destroy_write_struct_p(&png_ptr, (png_infopp) NULL);
        return FALSE;
    }

    // initialize the png structure
    png_set_write_fn_p(png_ptr, (png_voidp)pfFile, png_write_data, png_flush);
    
    // we're going to write a very simple 3x8 bit RGB image
    png_set_IHDR_p(png_ptr, info_ptr, iWidth, iHeight, ciBitDepth,
        PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);
    
    // write the file header information
    png_write_info_p(png_ptr, info_ptr);
    
    // swap the BGR pixels in the DiData structure to RGB
    png_set_bgr_p(png_ptr);
    
    // row_bytes is the width x number of channels
    ulRowBytes = iWidth * ciChannels;
    
    // we can allocate memory for an array of row-pointers
    ppbRowPointers = (png_bytepp) malloc(iHeight * sizeof(png_bytep));
    
    // set the individual row-pointers to point at the correct offsets
    for (i = 0; i < iHeight; i++)
        ppbRowPointers[i] = pDiData + i * (((ulRowBytes + 3) >> 2) << 2);

    // write out the entire image data in one call
    png_write_image_p(png_ptr, ppbRowPointers);
    
    // write the additional chunks to the PNG file (not really needed)
    png_write_end_p(png_ptr, info_ptr);
    
    // and we're done
    free (ppbRowPointers);
    ppbRowPointers = NULL;
    
    // clean up after the write, and free any memory allocated
    png_destroy_write_struct_p(&png_ptr, (png_infopp) NULL);
    
    return TRUE;
}
