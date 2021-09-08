// SPDX-License-Identifier: GPL-2.0
#include <Arduino.h>
#include <SD.h>

extern "C"
{
#include "printf.h"
#include "id_fs.h"
#include "ck_cross.h"
#include "ck_ep.h"
}
static const int MAX_FILES = 12;
File fp[MAX_FILES + 1];

FLASHMEM static int get_handle()
{
    for (int handle = 1; handle <= MAX_FILES; handle++)
    {
        if (!fp[handle])
            return handle;
    }
    return 0;
}

FLASHMEM static File *get_file(int handle)
{
    if (handle > MAX_FILES)
        return NULL;

    if (handle == 0)
        return NULL;

    if (!fp[handle])
        return NULL;

    return &fp[handle];
}

FLASHMEM void FS_Startup()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        printf("FS: SD init failed\n");
    }
}

bool FS_IsFileValid(FS_File handle)
{
    return (get_file(handle) != NULL);
}

FLASHMEM size_t FS_Read(void *ptr, size_t size, size_t nmemb, FS_File handle)
{
    File *fp = get_file(handle);
    if (fp == NULL)
    {
        printf("%s: Could not find file with handle %d\n", __FUNCTION__, handle);
        return 0;
    }

    int num_bytes = nmemb * size;
    int br = fp->read((char *)ptr, num_bytes);
    if (br != num_bytes)
    {
        printf("%s: Read byte mismatch %d vs %d on handle %d\n", __FUNCTION__, br, num_bytes, handle);
    }
    return br / size;
}

FLASHMEM size_t FS_Write(const void *ptr, size_t size, size_t nmemb, FS_File handle)
{
    File *fp = get_file(handle);
    if (fp == NULL)
    {
        printf("%s: Could not find file with handle %d\n", __FUNCTION__, handle);
        return 0;
    }

    int num_bytes = nmemb * size;
    int bw = fp->write((char *)ptr, num_bytes);
    if (bw != num_bytes)
    {
        printf("Write byte mismatch %d %d\n", bw, num_bytes);
    }
    return bw / size;
}

FLASHMEM size_t FS_SeekTo(FS_File handle, size_t offset)
{
    File *fp = get_file(handle);
    if (fp == NULL)
    {
        printf("%s: Could not find file with handle %d\n", __FUNCTION__, handle);
        return 0;
    }

    if (!fp->seek(offset, SeekSet))
    {
        printf("Could not seek file with handle %d to offset %d\n", handle, offset);
    }
    return offset;
}

FLASHMEM void FS_CloseFile(FS_File handle)
{
    File *fp = get_file(handle);
    if (fp != NULL)
    {
        fp->close();
    }
}

FLASHMEM size_t FS_GetFileSize(FS_File handle)
{
    File *fp = get_file(handle);
    if (fp == NULL)
    {
        printf("%s: Could not find file with handle %d\n", __FUNCTION__, handle);
        return 0;
    }
    return fp->size();
}

FLASHMEM static FS_File open_file(const char *filename, int mode)
{
    int handle = get_handle();
    if (handle == 0)
    {
        printf("%s: Could not find handle for file %s\n", __FUNCTION__, filename);
        return 0;
    }
    fp[handle] = SD.open(filename, mode);
    if (!fp[handle])
    {
        printf("%s: Could not open file %s\n", __FUNCTION__, filename);
        return 0;
    }
    return handle;
}

FLASHMEM FS_File FS_OpenKeenFile(const char *filename)
{
    return open_file(filename, FILE_READ);
}

FLASHMEM FS_File FS_OpenOmniFile(const char *filename)
{
    return open_file(filename, FILE_READ);
}

FLASHMEM FS_File FS_OpenUserFile(const char *filename)
{
    return open_file(filename, FILE_READ);
}

FLASHMEM FS_File FS_CreateUserFile(const char *filename)
{
    return open_file(filename, FILE_WRITE_BEGIN);
}

FLASHMEM bool FS_IsKeenFilePresent(const char *filename)
{
    int handle = open_file(filename, FILE_READ);
    if (handle)
    {
        FS_CloseFile(handle);
        return true;
    }
    return false;
}

FLASHMEM bool FS_IsOmniFilePresent(const char *filename)
{
    int handle = open_file(filename, FILE_READ);
    if (handle)
    {
        FS_CloseFile(handle);
        return true;
    }
    return false;
}

FLASHMEM bool FS_IsUserFilePresent(const char *filename)
{
    int handle = open_file(filename, FILE_READ);
    if (handle)
    {
        FS_CloseFile(handle);
        return true;
    }
    return false;
}

FLASHMEM bool FSL_IsGoodOmniPath(const char *ext)
{
    return true;
}

FLASHMEM bool FSL_IsGoodUserPath()
{
    return true;
}

FLASHMEM char *FS_AdjustExtension(const char *filename)
{
    static char newname[16];
    strcpy(newname, filename);
    size_t fnamelen = strlen(filename);
    newname[fnamelen - 3] = ck_currentEpisode->ext[0];
    newname[fnamelen - 2] = ck_currentEpisode->ext[1];
    newname[fnamelen - 1] = ck_currentEpisode->ext[2];
    return newname;
}

FLASHMEM size_t FS_ReadInt8LE(void *ptr, size_t count, FS_File handle)
{
    return FS_Read(ptr, 1, count, handle);
}

FLASHMEM size_t FS_ReadInt16LE(void *ptr, size_t count, FS_File handle)
{
    count = FS_Read(ptr, 2, count, handle);
#ifdef CK_CROSS_IS_BIGENDIAN
    uint16_t *uptr = (uint16_t *)ptr;
    for (size_t loopVar = 0; loopVar < count; loopVar++, uptr++)
        *uptr = CK_Cross_Swap16(*uptr);
#endif
    return count;
}

FLASHMEM size_t FS_ReadInt32LE(void *ptr, size_t count, FS_File handle)
{
    count = FS_Read(ptr, 4, count, handle);
#ifdef CK_CROSS_IS_BIGENDIAN
    uint32_t *uptr = (uint32_t *)ptr;
    for (size_t loopVar = 0; loopVar < count; loopVar++, uptr++)
        *uptr = CK_Cross_Swap32(*uptr);
#endif
    return count;
}

FLASHMEM size_t FS_WriteInt8LE(const void *ptr, size_t count, FS_File handle)
{
    return FS_Write(ptr, 1, count, handle);
}

FLASHMEM size_t FS_WriteInt16LE(const void *ptr, size_t count, FS_File handle)
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

FLASHMEM size_t FS_WriteInt32LE(const void *ptr, size_t count, FS_File handle)
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

FLASHMEM size_t FS_ReadBoolFrom16LE(void *ptr, size_t count, FS_File handle)
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

FLASHMEM size_t FS_WriteBoolTo16LE(const void *ptr, size_t count, FS_File handle)
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
    char buff[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);
    return FS_Write(buff, 1, strnlen(buff, sizeof(buff)), handle);
}

FLASHMEM bool FS_LoadUserFile(const char *filename, mm_ptr_t *ptr, int *memsize)
{
    FS_File handle = FS_OpenUserFile(filename);

    if (!FS_IsFileValid(handle))
    {
        *ptr = 0;
        *memsize = 0;
        return false;
    }

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
