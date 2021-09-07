// SPDX-License-Identifier: GPL-2.0
#include <Arduino.h>
#include "ILI9341_t3n.h"

extern "C"
{
#include "printf.h"
#include "id_vl.h"
#include "id_vl_private.h"
#include "ck_cross.h"
}

typedef struct VL_T4_Surface
{
    VL_SurfaceUsage use;
    int width, height;
    uint8_t *pixels;
} VL_T4_Surface;

#define TFT_ROTATION 1 //0-3
#define TFT_DC 40
#define TFT_CS 41
#define TFT_MOSI 26
#define TFT_SCK 27
#define TFT_MISO 39
#define TFT_RST 255
ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCK, TFT_MISO);
static DMAMEM uint16_t tft_buffer[320 * 240];
static uint16_t palette[16];
EXTMEM uint8_t extmem_start;

static void VL_T4_SetVideoMode(int mode)
{
    if (mode == 0xD)
    {
        tft.begin();
        tft.setRotation(TFT_ROTATION); //0-3
        tft.setFrameBuffer(tft_buffer);
        tft.useFrameBuffer(true);
        tft.fillScreen(ILI9341_BLACK);
        tft.updateScreen();
    }
    else
    {
    }
}

static void VL_T4_Present(void *surface, int scrlX, int scrlY, bool singleBuffered)
{
    VL_T4_Surface *src = (VL_T4_Surface *)surface;
    uint16_t *dest = tft_buffer;
    uint8_t src_row[src->width];
    int y_dest = 0;
    for (int _y = scrlY; _y < src->height; _y++)
    {
        if (y_dest >= 240)
        {
            break;
        }

        //Read the whole row in then draw the row
        memcpy(src_row, &src->pixels[_y * src->width], src->width);
        int x_dest = 0;
        for (int _x = scrlX; _x < src->width; _x++)
        {
            if (x_dest >= 320)
            {
                break;
            }

            //Get the final colour from the palette
            uint16_t colour565 = palette[src_row[_x]];

            //Where is it going
            uint32_t dest_row = y_dest * VL_EGAVGA_GFX_WIDTH;
            dest[dest_row + x_dest] = colour565;
            if (_y % 5 == 0)
            {
                //Every 5th row, place the pixel on the next row too.
                dest[(dest_row + VL_EGAVGA_GFX_WIDTH) + x_dest] = colour565;
            }
            x_dest++;
        }
        //Every 5th row incread the y value by an extra one. Over 200 rows, this will scale to 240 pixels.
        //320x200 will get scaled to 320x240 which is the DOS aspect ratio.
        if (_y % 5 == 0)
        {
            y_dest++;
        }
        y_dest++;
    }
    tft.updateScreenAsync();
}

static void VL_T4_WaitVBLs(int vbls)
{
    while (tft.asyncUpdateActive())
        ;
}

static void *VL_T4_CreateSurface(int w, int h, VL_SurfaceUsage usage)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)malloc(sizeof(VL_T4_Surface));
    surf->width = w;
    surf->height = h;

    //Attempt in RAM2
    surf->pixels = (uint8_t *)malloc(w * h);
    if (surf->pixels == NULL)
    {
        printf("Warning: Could not malloc surface internally. Attempting EXTMEM\n");
        surf->pixels = (uint8_t *)extmem_malloc(w * h);
    }
    if (surf->pixels == NULL)
    {
        printf("Could not malloc surface %d bytes\n", w * h);
        while (1)
            yield();
    }
    return surf;
}

static void VL_T4_DestroySurface(void *surface)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)surface;
    if (surf == NULL)
    {
        return;
    }
    if (surf->pixels && (uint32_t)surf->pixels >= (uint32_t)&extmem_start)
    {
        extmem_free(surf->pixels);
    }
    else if (surf->pixels)
    {
        free(surf->pixels);
    }
    free(surf);
}

static long VL_T4_GetSurfaceMemUse(void *surface)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)surface;
    return surf->width * surf->height;
}

static void VL_T4_GetSurfaceDimensions(void *surface, int *w, int *h)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)surface;
    if (w)
        *w = surf->width;
    if (h)
        *h = surf->height;
}

static void VL_T4_RefreshPaletteAndBorderColor(void *screen)
{
    uint8_t r, g, b;
    for (int i = 0; i < 16; i++)
    {
        r = VL_EGARGBColorTable[vl_emuegavgaadapter.palette[i]][0];
        g = VL_EGARGBColorTable[vl_emuegavgaadapter.palette[i]][1];
        b = VL_EGARGBColorTable[vl_emuegavgaadapter.palette[i]][2];
        uint16_t c = ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0); //rgba 565 for the TFT
        palette[i] = c;
    }
}

static int VL_T4_SurfacePGet(void *surface, int x, int y)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)surface;
    return ((uint8_t *)surf->pixels)[y * surf->width + x];
}

static void VL_T4_SurfaceRect(void *dst_surface, int x, int y, int w, int h, int colour)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    for (int _y = y; _y < y + h; ++_y)
    {
        memset(((uint8_t *)surf->pixels) + (_y * surf->width) + x, colour, CK_Cross_min(w, surf->width - x));
    }
}

static void VL_T4_SurfaceRect_PM(void *dst_surface, int x, int y, int w, int h, int colour, int mapmask)
{
    mapmask &= 0xF;
    colour &= mapmask;

    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    for (int _y = y; _y < y + h; ++_y)
    {
        for (int _x = x; _x < x + w; ++_x)
        {
            uint8_t *p = ((uint8_t *)surf->pixels) + _y * surf->width + _x;
            *p &= ~mapmask;
            *p |= colour;
        }
    }
}

static void VL_T4_SurfaceToSurface(void *src_surface, void *dst_surface, int x, int y, int sx, int sy, int sw, int sh)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)src_surface;
    VL_T4_Surface *dest = (VL_T4_Surface *)dst_surface;
    for (int _y = sy; _y < sy + sh; ++_y)
    {
        memcpy(((uint8_t *)dest->pixels) + (_y - sy + y) * dest->width + x, ((uint8_t *)surf->pixels) + _y * surf->width + sx, sw);
    }
}

static void VL_T4_SurfaceToSelf(void *surface, int x, int y, int sx, int sy, int sw, int sh)
{
    VL_T4_Surface *srf = (VL_T4_Surface *)surface;
    bool directionX = sx > x;
    bool directionY = sy > y;

    if (directionY)
    {
        for (int yi = 0; yi < sh; ++yi)
        {
            memmove(((uint8_t *)srf->pixels) + ((yi + y) * srf->width + x), ((uint8_t *)srf->pixels) + ((sy + yi) * srf->width + sx), sw);
        }
    }
    else
    {
        for (int yi = sh - 1; yi >= 0; --yi)
        {
            memmove(((uint8_t *)srf->pixels) + ((yi + y) * srf->width + x), ((uint8_t *)srf->pixels) + ((sy + yi) * srf->width + sx), sw);
        }
    }
}

static void VL_T4_UnmaskedToSurface(void *src, void *dst_surface, int x, int y, int w, int h)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_UnmaskedToPAL8(src, surf->pixels, x, y, surf->width, w, h);
}

static void VL_T4_UnmaskedToSurface_PM(void *src, void *dst_surface, int x, int y, int w, int h, int mapmask)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_UnmaskedToPAL8_PM(src, surf->pixels, x, y, surf->width, w, h, mapmask);
}

static void VL_T4_MaskedToSurface(void *src, void *dst_surface, int x, int y, int w, int h)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_MaskedToPAL8(src, surf->pixels, x, y, surf->width, w, h);
}

static void VL_T4_MaskedBlitToSurface(void *src, void *dst_surface, int x, int y, int w, int h)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_MaskedBlitClipToPAL8(src, surf->pixels, x, y, surf->width, w, h, surf->width, surf->height);
}

static void VL_T4_BitToSurface(void *src, void *dst_surface, int x, int y, int w, int h, int colour)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_1bppToPAL8(src, surf->pixels, x, y, surf->width, w, h, colour);
}

static void VL_T4_BitToSurface_PM(void *src, void *dst_surface, int x, int y, int w, int h, int colour, int mapmask)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_1bppToPAL8_PM(src, surf->pixels, x, y, surf->width, w, h, colour, mapmask);
}

static void VL_T4_BitXorWithSurface(void *src, void *dst_surface, int x, int y, int w, int h, int colour)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_1bppXorWithPAL8(src, surf->pixels, x, y, surf->width, w, h, colour);
}

static void VL_T4_BitBlitToSurface(void *src, void *dst_surface, int x, int y, int w, int h, int colour)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_1bppBlitToPAL8(src, surf->pixels, x, y, surf->width, w, h, colour);
}

static void VL_T4_BitInvBlitToSurface(void *src, void *dst_surface, int x, int y, int w, int h, int colour)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)dst_surface;
    VL_1bppInvBlitClipToPAL8(src, surf->pixels, x, y, surf->width, w, h, surf->width, surf->height, colour);
}

static int VL_T4_GetActiveBufferId(void *surface)
{
    (void)surface;
    return 0;
}

static int VL_T4_GetNumBuffers(void *surface)
{
    (void)surface;
    return 1;
}

static void VL_T4_ScrollSurface(void *surface, int x, int y)
{
    VL_T4_Surface *surf = (VL_T4_Surface *)surface;
    int dx = 0, dy = 0, sx = 0, sy = 0;
    int w = surf->width - CK_Cross_max(x, -x), h = surf->height - CK_Cross_max(y, -y);
    if (x > 0)
    {
        dx = 0;
        sx = x;
    }
    else
    {
        dx = -x;
        sx = 0;
    }
    if (y > 0)
    {
        dy = 0;
        sy = y;
    }
    else
    {
        dy = -y;
        sy = 0;
    }
    VL_T4_SurfaceToSelf(surface, dx, dy, sx, sy, w, h);
}

static void VL_T4_FlushParams()
{
}

VL_Backend vl_t4_backend =
    {
        .setVideoMode = &VL_T4_SetVideoMode,
        .createSurface = &VL_T4_CreateSurface,
        .destroySurface = &VL_T4_DestroySurface,
        .getSurfaceMemUse = &VL_T4_GetSurfaceMemUse,
        .getSurfaceDimensions = &VL_T4_GetSurfaceDimensions,
        .refreshPaletteAndBorderColor = &VL_T4_RefreshPaletteAndBorderColor,
        .surfacePGet = &VL_T4_SurfacePGet,
        .surfaceRect = &VL_T4_SurfaceRect,
        .surfaceRect_PM = &VL_T4_SurfaceRect_PM,
        .surfaceToSurface = &VL_T4_SurfaceToSurface,
        .surfaceToSelf = &VL_T4_SurfaceToSelf,
        .unmaskedToSurface = &VL_T4_UnmaskedToSurface,
        .unmaskedToSurface_PM = &VL_T4_UnmaskedToSurface_PM,
        .maskedToSurface = &VL_T4_MaskedToSurface,
        .maskedBlitToSurface = &VL_T4_MaskedBlitToSurface,
        .bitToSurface = &VL_T4_BitToSurface,
        .bitToSurface_PM = &VL_T4_BitToSurface_PM,
        .bitXorWithSurface = &VL_T4_BitXorWithSurface,
        .bitBlitToSurface = &VL_T4_BitBlitToSurface,
        .bitInvBlitToSurface = &VL_T4_BitInvBlitToSurface,
        .scrollSurface = &VL_T4_ScrollSurface,
        .present = &VL_T4_Present,
        .getActiveBufferId = &VL_T4_GetActiveBufferId,
        .getNumBuffers = &VL_T4_GetNumBuffers,
        .flushParams = &VL_T4_FlushParams,
        .waitVBLs = &VL_T4_WaitVBLs};

VL_Backend *VL_Impl_GetBackend()
{
    return &vl_t4_backend;
}
