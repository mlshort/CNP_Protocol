/**
 * @file   CNP_Messaging.h
 * @brief  Message processing function prototypes
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 *
 */

#if !defined(__CNP_MESSAGING_H__)
#define __CNP_MESSAGING_H__

// forward declaration
class CNP_Socket;

cnp::WORD ProcessConnectRequest         (const void* pMsg, size_t cbLen, CNP_Socket* pSocket);

bool      ProcessBalanceQueryRequest    (const void* pMsg, size_t cbLen);
bool      ProcessCreateAccountRequest   (const void* pMsg, size_t cbLen);
bool      ProcessDepositRequest         (const void* pMsg, size_t cbLen);
bool      ProcessLogoffRequest          (const void* pMsg, size_t cbLen);
bool      ProcessLogonRequest           (const void* pMsg, size_t cbLen);
bool      ProcessStampPurchaseRequest   (const void* pMsg, size_t cbLen);
bool      ProcessTransactionQueryRequest(const void* pMsg, size_t cbLen);
bool      ProcessWithdrawalRequest      (const void* pMsg, size_t cbLen);

bool      ProcessDisconnect             (cnp::WORD wClientID);

#endif