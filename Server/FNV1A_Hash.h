/**
 * @file   FNV1A_Hash.h
 * @brief  FNV1A Hash function prototype
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 *
 * <b> Cite: </b>
 *         The function implementation was based off
 *         of the FNV1a hash algorithm, that has been
 *         released to public domain.
 *
 * @sa http://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */

#if !defined(__FNV1A_HASH_H__)
#define __FNV1A_HASH_H__

#include <stddef.h>

/**
   @brief A FNV1a hash implementation

    Performs a hash computation against a series of bytes.
    The series may contain embedded NULL characters and does
    not require a NULL-terminated string as the target.

    @param [in] pKey      address of sequence of bytes
    @param [in] cbLen     count of byte (cb) length of sequence

    @retval size_t   containing results of hash
 */
size_t FNV1A_Hash(const char* pKey, size_t cbLen) noexcept;

#endif
