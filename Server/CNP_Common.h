/**
 * @file   CNP_Common.h
 * @brief  Common type definitions
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * @date   April 25, 2015  updated code comments
 *
 */


#if !defined(__CNP_COMMON_H__)
#define __CNP_COMMON_H__

#ifndef __CNP_PROTOCOL_H__
    #include "CNP_Protocol.h"
#endif

/// for error checking and data initialization
const cnp::QWORD INVALID_CUSTOMER_ID     = 0;
/// for error checking and data initialization
const cnp::DWORD INVALID_BALANCE         = static_cast<cnp::DWORD>(~0);

const cnp::WORD  g_wServerMajorVersion   = 1;
const cnp::WORD  g_wServerMinorVersion   = 1;

/// Validation helper function
inline bool IsValidCustomerID(const cnp::QWORD& qwID) noexcept
{ return (qwID != INVALID_CUSTOMER_ID); };

/// Validation helper function
inline bool IsValidPIN(cnp::WORD wPIN) noexcept
{ return (wPIN != cnp::INVALID_PIN); };

/// Validation helper function
inline bool IsValidName(const char* szName) noexcept
{ return (szName && szName[0]); };

#endif
