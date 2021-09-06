/*
Omnispeak: A Commander Keen Reimplementation
Copyright (C) 2020 Omnispeak Authors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ck_cross.h"
#include "ck_ep.h"

#include "id_fs.h"
#include "id_mm.h"
#include "id_us.h"

static char *FS_GetDataPath(const char *filename)
{
    static char path[32];
    snprintf(path, 32, "%s", filename);
    return path;
}

bool FS_IsFileValid(FS_File handle)
{
    return 0;
}

size_t FS_Read(void *ptr, size_t size, size_t nmemb, FS_File handle)
{
    return 0;
}

size_t FS_Write(const void *ptr, size_t size, size_t nmemb, FS_File handle)
{
    return 0;
}

size_t FS_SeekTo(FS_File handle, size_t offset)
{
    return 0;
}

void FS_CloseFile(FS_File handle)
{
}

size_t FS_GetFileSize(FS_File handle)
{
    return 0;
}

FS_File FS_OpenKeenFile(const char *filename)
{
    return 0;
}

FS_File FS_OpenOmniFile(const char *filename)
{
    return FS_OpenKeenFile(filename);
}

FS_File FS_OpenUserFile(const char *filename)
{
    return 0;
}

FS_File FS_CreateUserFile(const char *filename)
{
    return 0;
}

// Does a handle exist (and is it readable)
bool FS_IsKeenFilePresent(const char *filename)
{
    return true;
}

// Does a handle exist (and is it readable)
bool FS_IsOmniFilePresent(const char *filename)
{
    return true;
}

// Does a handle exist (and is it readable)
bool FS_IsUserFilePresent(const char *filename)
{
}

bool FSL_IsGoodOmniPath(const char *ext)
{
}

bool FSL_IsGoodUserPath()
{
}

void FS_Startup()
{
}

// Adjusts the extension on a filename to match the current episode.
// This function is NOT thread safe, and the string returned is only
// valid until the NEXT invocation of this function.
char *FS_AdjustExtension(const char *filename)
{
    static char newname[16];
    strcpy(newname, filename);
    size_t fnamelen = strlen(filename);
    newname[fnamelen - 3] = ck_currentEpisode->ext[0];
    newname[fnamelen - 2] = ck_currentEpisode->ext[1];
    newname[fnamelen - 1] = ck_currentEpisode->ext[2];
    return newname;
}

size_t FS_ReadInt8LE(void *ptr, size_t count, FS_File handle)
{
    return FS_Read(ptr, 1, count, handle);
}

size_t FS_ReadInt16LE(void *ptr, size_t count, FS_File handle)
{
    count = FS_Read(ptr, 2, count, handle);
#ifdef CK_CROSS_IS_BIGENDIAN
    uint16_t *uptr = (uint16_t *)ptr;
    for (size_t loopVar = 0; loopVar < count; loopVar++, uptr++)
        *uptr = CK_Cross_Swap16(*uptr);
#endif
    return count;
}

size_t FS_ReadInt32LE(void *ptr, size_t count, FS_File handle)
{
    count = FS_Read(ptr, 4, count, handle);
#ifdef CK_CROSS_IS_BIGENDIAN
    uint32_t *uptr = (uint32_t *)ptr;
    for (size_t loopVar = 0; loopVar < count; loopVar++, uptr++)
        *uptr = CK_Cross_Swap32(*uptr);
#endif
    return count;
}

size_t FS_WriteInt8LE(const void *ptr, size_t count, FS_File handle)
{
    return FS_Write(ptr, 1, count, handle);
}

size_t FS_WriteInt16LE(const void *ptr, size_t count, FS_File handle)
{
#ifndef CK_CROSS_IS_BIGENDIAN
    return FS_Write(ptr, 2, count, handle);
#else
    uint16_t val;
    size_t actualCount = 0;
    uint16_t *uptr = (uint16_t *)ptr;
    for (size_t loopVar = 0; loopVar < count; loopVar++, uptr++)
    {
        val = CK_Cross_Swap16(*uptr);
        actualCount += FS_Write(&val, 2, 1, handle);
    }
    return actualCount;
#endif
}

size_t FS_WriteInt32LE(const void *ptr, size_t count, FS_File handle)
{
#ifndef CK_CROSS_IS_BIGENDIAN
    return FS_Write(ptr, 4, count, handle);
#else
    uint32_t val;
    size_t actualCount = 0;
    uint32_t *uptr = (uint32_t *)ptr;
    for (size_t loopVar = 0; loopVar < count; loopVar++, uptr++)
    {
        val = CK_Cross_Swap32(*uptr);
        actualCount += FS_Write(&val, 4, 1, handle);
    }
    return actualCount;
#endif
}

size_t FS_ReadBoolFrom16LE(void *ptr, size_t count, FS_File handle)
{
    uint16_t val;
    size_t actualCount = 0;
    bool *currBoolPtr = (bool *)ptr; // No lvalue compilation error
    for (size_t loopVar = 0; loopVar < count; loopVar++, currBoolPtr++)
    {
        if (FS_Read(&val, 2, 1, handle)) // Should be either 0 or 1
        {
            *currBoolPtr = (val); // NOTE: No need to byte-swap
            actualCount++;
        }
    }
    return actualCount;
}

size_t FS_WriteBoolTo16LE(const void *ptr, size_t count, FS_File handle)
{
    uint16_t val;
    size_t actualCount = 0;
    bool *currBoolPtr = (bool *)ptr; // No lvalue compilation error
    for (size_t loopVar = 0; loopVar < count; loopVar++, currBoolPtr++)
    {
        val = CK_Cross_SwapLE16((*currBoolPtr) ? 1 : 0);
        actualCount += FS_Write(&val, 2, 1, handle);
    }
    return actualCount;
}

int FS_PrintF(FS_File handle, const char *fmt, ...)
{
    uint8_t buff[64]; //FIXME: Is this enough/ok on the stack?
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);
    return FS_Write(buff, 1, strnlen(buff, sizeof(buff)), handle);
}

bool FS_LoadUserFile(const char *filename, mm_ptr_t *ptr, int *memsize)
{
    FS_File handle = FS_OpenUserFile(filename);

    if (!FS_IsFileValid(handle))
    {
        *ptr = 0;
        *memsize = 0;
        return false;
    }

    //Get length of handle
    int length = FS_GetFileSize(handle);

    MM_GetPtr(ptr, length);

    if (memsize)
        *memsize = length;
    int amountRead = FS_Read(*ptr, 1, length, handle);

    FS_CloseFile(handle);

    if (amountRead != length)
        return false;
    return true;
}