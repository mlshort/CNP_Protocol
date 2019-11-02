/**
 * @file   FNV1A_Hash.cpp
 * @brief  FNV1A Hash function implementation
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * @date   April 25, 2015 corrected variable naming that previously implied Key
 *                        parameter was null-terminated, which it is not required.
 * 
 * @sa http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */

#include "FNV1A_Hash.h"

typedef unsigned short WORD;
typedef unsigned long  DWORD;

size_t FNV1A_Hash(const char* pKey, size_t cbLen)
{
    const char* pKeyLoc  = pKey;
    DWORD  dwHash        = 2166136261;
    size_t cbKeyLen      = cbLen;

    for (; cbKeyLen >= 4; cbKeyLen -= 4, pKeyLoc += 4)
    {
        dwHash = ( ( dwHash ^ *(DWORD*) pKeyLoc ) << 5 ) - ( dwHash ^ *(DWORD*) pKeyLoc );
    }

    // Process the remaining bytes
    if (cbKeyLen & 2)
    {
        dwHash = ( ( dwHash ^ *(WORD*) pKeyLoc ) << 5 ) - ( dwHash ^ *(WORD*) pKeyLoc );
        pKeyLoc += 2;
    }
    if (cbKeyLen & 1)
    {
        dwHash = ( ( dwHash ^ *pKeyLoc ) << 5 ) - ( dwHash ^ *pKeyLoc );
    }

    return ( dwHash >> 16 ) ^ dwHash;
}