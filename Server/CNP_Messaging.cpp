/**
 * @file   CNP_Messaging.cpp
 * @brief  Server Message processing implementation
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * 
 */

#include <chrono>
#include <vector>
#include <iostream>
#include <iomanip>
#include <mutex>

#include "CNP_ServerDB.h"
#include "CNP_Session.h"
#include "CNP_Messaging.h"


extern SessionMap_t                         g_SessionInfo;
std::mutex                                  g_SessionMutex; 

extern AccountMap_t                         g_AccountInfo;
std::mutex                                  g_AccountMutex;

extern TransactionMap_t                     g_TransactionInfo;
std::mutex                                  g_TransactionMutex;


#ifdef __linux__

pid_t gettid( void );

#elif _MSC_VER

DWORD gettid( void );

#endif

/*
struct SERVER_RESPONSE
{
    cnp::STD_HDR m_Hdr;
    union 
    {
        cnp::prim::_BALANCE_QUERY_RESPONSE     m_BalanceQueryResponse;
        cnp::prim::_CONNECT_RESPONSE           m_ConnectResponse;
        cnp::prim::_CREATE_ACCOUNT_RESPONSE    m_CreateAccountResponse;
        cnp::prim::_DEPOSIT_RESPONSE           m_DepositResponse;
        cnp::prim::_LOGOFF_RESPONSE            m_LogOffResponse;
        cnp::prim::_LOGON_RESPONSE             m_LogOnResponse;
        cnp::prim::_STAMP_PURCHASE_RESPONSE    m_StampPurchaseResponse;
        cnp::prim::_WITHDRAWAL_RESPONSE        m_WithdrawalResponse;
#ifdef _MSC_VER
        cnp::prim::_TRANSACTION_QUERY_RESPONSE_10 m_TransactionQueryResponse;
#else
        cnp::prim::_TRANSACTION_QUERY_RESPONSE    m_TransactionQueryResponse;
#endif
    };

    SERVER_RESPONSE(void)
    {  memset(this, 0, sizeof(*this)); };

    SERVER_RESPONSE(const cnp::CONNECT_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_ConnectResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::BALANCE_QUERY_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_BalanceQueryResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::CREATE_ACCOUNT_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_CreateAccountResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::DEPOSIT_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_DepositResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::LOGOFF_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_LogOffResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::LOGON_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_LogOnResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::STAMP_PURCHASE_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_StampPurchaseResponse = msg.m_Response; };

    SERVER_RESPONSE(const cnp::WITHDRAWAL_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_WithdrawalResponse = msg.m_Response; };

#ifdef _MSC_VER
    SERVER_RESPONSE(const cnp::TRANSACTION_QUERY_RESPONSE_10& msg)
        : m_Hdr(msg.m_Hdr)
    { m_TransactionQueryResponse = msg.m_Response; };
#else
    SERVER_RESPONSE(const cnp::TRANSACTION_QUERY_RESPONSE& msg)
        : m_Hdr(msg.m_Hdr)
    { m_TransactionQueryResponse = msg.m_Response; };
#endif

    SERVER_RESPONSE& operator=(const SERVER_RESPONSE& rhs)
    { if (this != &rhs)
          memcpy(this, &rhs, sizeof(SERVER_RESPONSE)); 
      return *this; };
};
*/



cnp::WORD ProcessConnectRequest(const void* pMsg, size_t cbMsgLen, CNP_Socket* pSocket)
{
    const cnp::CONNECT_REQUEST* pReqMsg = reinterpret_cast<const cnp::CONNECT_REQUEST*>( pMsg );

    cnp::CER_TYPE cerRR    = cnp::CER_ERROR;
    cnp::WORD wNewClientID = cnp::INVALID_CLIENT_ID;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client: NA  " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;

// 1. Verify the Validation Key
    if (pReqMsg->get_ClientValidationKey() == cnp::g_dwValidationKey)
    {
// 2. Check to see if the server can support the client's protocol version
        if ((pReqMsg->get_ClientMajorVersion() <= g_wServerMajorVersion) && 
            (pReqMsg->get_ClientMinorVersion() <= g_wServerMinorVersion))
        {
// 3. Generate a unique ClientID for the session
            auto itS = g_SessionInfo.rbegin();
            if (itS != g_SessionInfo.rend())
            {
                wNewClientID = itS->first + 1;
            }
            else
            {
                wNewClientID = 1;
            }

// 4. Update the session state table
            // lock g_SessionInfo
            std::lock_guard<std::mutex> SessionLock(g_SessionMutex);
            
            SESSION_INFO newSession(wNewClientID, SS_CONNECTED, pSocket);
            g_SessionInfo.insert(SessionMap_t::value_type(wNewClientID, newSession));

            cerRR = cnp::CER_SUCCESS;
        }
        else
        {
            cerRR = cnp::CER_UNSUPPORTED_PROTOCOL;
        }

    }
    else
    {
        cerRR = cnp::CER_AUTHENICATION_FAILED;
    }

// 5. Generate the Server Response Message
    cnp::CONNECT_RESPONSE respMsg(cerRR,
                                  wNewClientID,
                                  g_wServerMajorVersion,
                                  g_wServerMinorVersion,
                                  pReqMsg->get_Sequence(),
                                  pReqMsg->get_Context());

// 6. Que the response for dispatching
//    g_queSvrRespMsg.Push(respMsg);

    pSocket->Send(&respMsg, respMsg.get_Size());
    return wNewClientID;
};

bool ProcessCreateAccountRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::CREATE_ACCOUNT_REQUEST* pReqMsg = reinterpret_cast<const cnp::CREATE_ACCOUNT_REQUEST*>( pMsg );   

    cnp::CER_TYPE cerRR = cnp::CER_ERROR;
    cnp::WORD wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket = nullptr;
    
    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate the Name & PIN
        const char* szName = pReqMsg->get_FirstName();
        cnp::WORD   wPIN   = pReqMsg->get_PIN();
        if (IsValidName(szName) && IsValidPIN(wPIN))
        {
// 3. Make sure the Name+PIN combo doesn't already exist
            cnp::QWORD qwCustomerID = GenerateCustomerID(szName, strlen(szName), wPIN);
            
// NOTE - (neither the const nor the non-const versions of 'find' modify the container).
// No mapped values are accessed: concurrently accessing or modifying elements is safe.
// @TODO - reader-writer lock would be more appropriate for this            
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
                cerRR = cnp::CER_ACCOUNT_EXISTS;
            }
            else
            {
// 4. Create & add the ACCOUNT_INFO
                // lock g_AccountInfo
                std::lock_guard<std::mutex> AccountLock(g_AccountMutex);

                ACCOUNT_INFO newAccount(pReqMsg->m_Request, qwCustomerID, 0);

                g_AccountInfo.insert(AccountMap_t::value_type(qwCustomerID, newAccount) );
// 5. Update the session state table
                itS->second.set_State(SS_ACCOUNT_CREATED);
                cerRR = cnp::CER_SUCCESS;
            }
        }
        else
        {
            cerRR = cnp::CER_INVALID_NAME_PIN;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }
// 6. Generate the Server Response Message
    cnp::CREATE_ACCOUNT_RESPONSE respMsg(cerRR,
                                         pReqMsg->get_ClientID(),
                                         pReqMsg->get_Sequence(),
                                         pReqMsg->get_Context());

// 7. Que the Server Response for Dispatching
//    g_queSvrRespMsg.Push(respMsg);

    if (pSocket)
    {
        pSocket->Send(&respMsg, respMsg.get_Size());
    }

    return cnp::Succeeded(cerRR);
};

bool ProcessLogonRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::LOGON_REQUEST* pReqMsg = reinterpret_cast<const cnp::LOGON_REQUEST*>( pMsg );    
    
    cnp::CER_TYPE cerRR = cnp::CER_ERROR;
    cnp::WORD wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate the Name & PIN
        const char* szName = pReqMsg->get_FirstName();
        cnp::WORD   wPIN   = pReqMsg->get_PIN();
        if (IsValidName(szName) && IsValidPIN(wPIN))
        {
            cnp::QWORD qwCustomerID = GenerateCustomerID(szName, strlen(szName), wPIN);

// 3. Make sure an account with the Name+PIN combo exists
            
// NOTE - (neither the const nor the non-const versions of 'find' modify the container).
// No mapped values are accessed: concurrently accessing or modifying elements is safe.
// @TODO - reader-writer lock would be more appropriate for this 
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
// 4. Update the SESSION_INFO to record the client as logged on
                itS->second.set_CustomerID(qwCustomerID);
                itS->second.set_State(SS_LOGGED_ON);
                cerRR = cnp::CER_SUCCESS;
            }
            else
            {
// 5. Else, deny the logon request if no account exists
                cerRR = cnp::CER_ACCOUNT_NOT_FOUND;
            }
        }
        else
        {
            cerRR = cnp::CER_INVALID_NAME_PIN;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }
// 6. Generate the Server Response Message
    cnp::LOGON_RESPONSE respMsg(cerRR,
                                pReqMsg->get_ClientID(),
                                pReqMsg->get_Sequence(),
                                pReqMsg->get_Context());

// 7. Que the server response for dispatching
//    g_queSvrRespMsg.Push(respMsg);
    if (pSocket)
        pSocket->Send(&respMsg, respMsg.get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessLogoffRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::LOGOFF_REQUEST* pReqMsg = reinterpret_cast<const cnp::LOGOFF_REQUEST*>( pMsg );    
    
    cnp::CER_TYPE cerRR = cnp::CER_ERROR;
    cnp::WORD wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate they are logged on
        cnp::QWORD qwCustomerID = itS->second.get_CustomerID();
        if (IsValidCustomerID(qwCustomerID))
        {
// 3. Update the SESSION_INFO table & clear their customer ID, 
//    but leave them in the session table for now
            itS->second.set_CustomerID(INVALID_CUSTOMER_ID);

            cerRR = cnp::CER_SUCCESS;
        }
        else
        {
            cerRR = cnp::CER_CLIENT_NOT_LOGGEDON;
        }
// 4. Update the session state
        itS->second.set_State(SS_LOGGED_OFF);
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }

// Generate the Server Response Message
    cnp::LOGOFF_RESPONSE respMsg(cerRR,
                                 pReqMsg->get_ClientID(),
                                 pReqMsg->get_Sequence(),
                                 pReqMsg->get_Context());

// Que the server response for dispatching
//    g_queSvrRespMsg.Push(respMsg);
    if (pSocket)
        pSocket->Send(&respMsg, respMsg.get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessDepositRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::DEPOSIT_REQUEST* pReqMsg = reinterpret_cast<const cnp::DEPOSIT_REQUEST*>( pMsg );
    
    cnp::CER_TYPE cerRR = cnp::CER_ERROR;
    cnp::WORD wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;

// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate they have an account and are logged on
        cnp::QWORD qwCustomerID = itS->second.get_CustomerID();
        if (IsValidCustomerID(qwCustomerID))
        {
// NOTE - (neither the const nor the non-const versions of 'find' modify the container).
// No mapped values are accessed: concurrently accessing or modifying elements is safe.
// @TODO - reader-writer lock would be more appropriate for this 
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
// 3. Update the account balance
                itA->second.incr_Balance(pReqMsg->get_Amount());

// 4. Record the transaction
                // lock g_TransactionInfo
                std::lock_guard<std::mutex> TransLock(g_TransactionMutex);

                cnp::DWORD dwNewID = g_TransactionInfo.size() + 1;

                cnp::QWORD qwNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                TRANSACTION_INFO newTrans(dwNewID,
                                          qwNow,
                                          pReqMsg->get_Amount(),
                                          cnp::TT_DEPOSIT,
                                          qwCustomerID);

                g_TransactionInfo.insert( TransactionMap_t::value_type(dwNewID, newTrans) );

                cerRR = cnp::CER_SUCCESS;
            }
            else
            {
                cerRR = cnp::CER_ACCOUNT_NOT_FOUND;
            }
        }
        else
        {
            cerRR = cnp::CER_INVALID_NAME_PIN;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }
// Generate the Server Response Message
    cnp::DEPOSIT_RESPONSE respMsg(cerRR,
                                  pReqMsg->get_ClientID(),
                                  pReqMsg->get_Sequence(),
                                  pReqMsg->get_Context());

// Que the server response for dispatching
//    g_queSvrRespMsg.Push(respMsg);
    if (pSocket)
        pSocket->Send(&respMsg, respMsg.get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessWithdrawalRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::WITHDRAWAL_REQUEST* pReqMsg = reinterpret_cast<const cnp::WITHDRAWAL_REQUEST*>( pMsg );
    
    cnp::CER_TYPE cerRR = cnp::CER_ERROR;
    cnp::WORD wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate they have an account and are logged on
        cnp::QWORD qwCustomerID = itS->second.get_CustomerID();
        if (IsValidCustomerID(qwCustomerID))
        {
// NOTE - (neither the const nor the non-const versions of 'find' modify the container).
// No mapped values are accessed: concurrently accessing or modifying elements is safe.
// @TODO - reader-writer lock would be more appropriate for this 
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
// 3. Check available balance
                cnp::DWORD dwCurBalance = itA->second.get_Balance();
                cnp::DWORD dwWithdrawal = pReqMsg->get_Amount();
                if (dwWithdrawal <= dwCurBalance)
                {
// 4. Decrement account balance
                    itA->second.decr_Balance(dwWithdrawal);
                    
// 5. Generate and record the transaction
                    // lock g_TransactionInfo
                    std::lock_guard<std::mutex> TransLock(g_TransactionMutex);

                    cnp::DWORD dwNewID = g_TransactionInfo.size() + 1;

                    cnp::QWORD qwNow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    TRANSACTION_INFO newTrans(dwNewID,
                                              qwNow,
                                              pReqMsg->get_Amount(),
                                              cnp::TT_WITHDRAWAL,
                                              qwCustomerID);

                    g_TransactionInfo.insert( TransactionMap_t::value_type(dwNewID, newTrans) );

                    cerRR = cnp::CER_SUCCESS;
                }
                else
                {
                    cerRR = cnp::CER_INSUFFICIENT_FUNDS;
                }
            }
            else
            {
                cerRR = cnp::CER_ACCOUNT_NOT_FOUND;
            }
        }
        else
        {
            cerRR = cnp::CER_INVALID_NAME_PIN;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }

// 6. Generate the Server Response Message
    cnp::WITHDRAWAL_RESPONSE respMsg(cerRR,
                                     pReqMsg->get_ClientID(),
                                     pReqMsg->get_Sequence(),
                                     pReqMsg->get_Context());

//  7. Que the server response for dispatching
//    g_queSvrRespMsg.Push(respMsg);
    if (pSocket)
       pSocket->Send(&respMsg, respMsg.get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessBalanceQueryRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::BALANCE_QUERY_REQUEST* pReqMsg = reinterpret_cast<const cnp::BALANCE_QUERY_REQUEST*>( pMsg );
    
    cnp::CER_TYPE cerRR  = cnp::CER_ERROR;
    cnp::DWORD dwBalance = INVALID_BALANCE;
    cnp::WORD  wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket  = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate they have an account and are logged on
        cnp::QWORD qwCustomerID = itS->second.get_CustomerID();
        if (IsValidCustomerID(qwCustomerID))
        {
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
// 3. Retrieve their current balance.
                dwBalance = itA->second.get_Balance();

                cerRR = cnp::CER_SUCCESS;
            }
            else
            {
                cerRR = cnp::CER_ACCOUNT_NOT_FOUND;
            }
        }
        else
        {
            cerRR = cnp::CER_CLIENT_NOT_LOGGEDON;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }

// Generate the Server Response Message
    cnp::BALANCE_QUERY_RESPONSE respMsg(cerRR,
                                        pReqMsg->get_ClientID(),
                                        dwBalance,
                                        pReqMsg->get_Context(),
                                        pReqMsg->get_Sequence());

// Que the server response for dispatching
//    g_queSvrRespMsg.Push(respMsg);
   if (pSocket)
       pSocket->Send(&respMsg, respMsg.get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessTransactionQueryRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::TRANSACTION_QUERY_REQUEST* pReqMsg = reinterpret_cast<const cnp::TRANSACTION_QUERY_REQUEST*>( pMsg );
    
    cnp::CER_TYPE cerRR   = cnp::CER_ERROR;
    cnp::WORD wClientID   = pReqMsg->get_ClientID();
    cnp::WORD wTransCount = 0;
    std::vector<cnp::TRANSACTION> vecTransactions;
    CNP_Socket* pSocket = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate they have an account and are logged on
        cnp::QWORD qwCustomerID = itS->second.get_CustomerID();
        if (IsValidCustomerID(qwCustomerID))
        {
// NOTE - (neither the const nor the non-const versions of 'find' modify the container).
// No mapped values are accessed: concurrently accessing or modifying elements is safe.
// @TODO - reader-writer lock would be more appropriate for this 
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
                cnp::DWORD dwStart = pReqMsg->get_StartID();
                cnp::WORD  wCount  = pReqMsg->get_TransactionCount();
                auto itT = g_TransactionInfo.lower_bound(dwStart);

                if (itT != g_TransactionInfo.end())
                {
                    vecTransactions.reserve(wCount);

                    cnp::WORD i = 0;
                    for (i = 0; itT != g_TransactionInfo.end() && i < wCount; ++itT)
                    {
                        if (itT->second.get_CustomerID() == qwCustomerID)
                        {
                            vecTransactions.push_back(itT->second);
                            i++;
                        }
                    }

                    wTransCount = i;
                }

                cerRR = cnp::CER_SUCCESS;
            }
            else
            {
                cerRR = cnp::CER_ACCOUNT_NOT_FOUND;
            }
        }
        else
        {
            cerRR = cnp::CER_CLIENT_NOT_LOGGEDON;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }

    // declare a buffer on the stack
    char rgBuffer[512];

// do an in-place new to instantiate the following structure
// using the constructor, but on top of the buffer without allocating additional
// memory on the heap.

    cnp::TRANSACTION_QUERY_RESPONSE* pRspMsg = new (rgBuffer) 
                cnp::TRANSACTION_QUERY_RESPONSE( cerRR,
                                                 wClientID,
                                                 wTransCount,
                                                 pReqMsg->get_Sequence(),
                                                 pReqMsg->get_Context() );

    int i = 0;
    for (auto it : vecTransactions)
    {
        pRspMsg->m_Response.m_rgTransactions[i++] = it;
    }

    // Que the server response for dispatching
//    g_queSvrRespMsg.Push(*pRspMsg);
    if (pSocket)
        pSocket->Send(pRspMsg, pRspMsg->get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessStampPurchaseRequest(const void* pMsg, size_t cbMsgLen)
{
    const cnp::STAMP_PURCHASE_REQUEST* pReqMsg = reinterpret_cast<const cnp::STAMP_PURCHASE_REQUEST*>( pMsg );
    cnp::CER_TYPE cerRR = cnp::CER_ERROR;
    cnp::WORD wClientID = pReqMsg->get_ClientID();
    CNP_Socket* pSocket = nullptr;

    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << " MsgLen:" << cbMsgLen << std::endl;
// 1. Validate the connection
    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        pSocket = itS->second.m_pSocket;
// 2. Validate they have an account and are logged on
        cnp::QWORD qwCustomerID = itS->second.get_CustomerID();
        if (IsValidCustomerID(qwCustomerID))
        {
// NOTE - (neither the const nor the non-const versions of 'find' modify the container).
// No mapped values are accessed: concurrently accessing or modifying elements is safe.
// @TODO - reader-writer lock would be more appropriate for this 
            auto itA = g_AccountInfo.find(qwCustomerID);
            if (itA != g_AccountInfo.end())
            {
// 3. Check available balance
                cnp::DWORD dwCurBalance = itA->second.get_Balance();
                cnp::DWORD dwWithdrawal = pReqMsg->get_Amount();
                if (dwWithdrawal <= dwCurBalance)
                {
// 4. Decrement the account balance
                    itA->second.decr_Balance(dwWithdrawal);

// 5. Generate and record the transaction

                    // lock g_TransactionInfo
                    std::lock_guard<std::mutex> TransLock(g_TransactionMutex);

                    cnp::DWORD dwNewID = g_TransactionInfo.size() + 1;
                    cnp::QWORD qwNow   = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    TRANSACTION_INFO newTrans(dwNewID,
                                              qwNow,
                                              pReqMsg->get_Amount(),
                                              cnp::TT_STAMP_PURCHASE,
                                              qwCustomerID);

                    g_TransactionInfo.insert( TransactionMap_t::value_type(dwNewID, newTrans) );

                    cerRR = cnp::CER_SUCCESS;
                }
                else
                {
                    cerRR = cnp::CER_INSUFFICIENT_FUNDS;
                }
            }
            else
            {
                cerRR = cnp::CER_ACCOUNT_NOT_FOUND;
            }
        }
        else
        {
        cerRR = cnp::CER_CLIENT_NOT_LOGGEDON;
        }
    }
    else
    {
        cerRR = cnp::CER_INVALID_CLIENT_ID;
    }

// 6. Generate Server Response Message
    cnp::STAMP_PURCHASE_RESPONSE respMsg(cerRR,
                                         pReqMsg->get_ClientID(),
                                         pReqMsg->get_Sequence(),
                                         pReqMsg->get_Context());

// 7. Que the server response for dispatching
//    g_queSvrRespMsg.Push(respMsg);
    if (pSocket)
        pSocket->Send(&respMsg, respMsg.get_Size());

    return cnp::Succeeded(cerRR);
};

bool ProcessDisconnect(cnp::WORD wClientID)
{
    std::cout << "[" << std::setw(5) << gettid() 
              << "] Client:" << std::setw(4) << wClientID << " " << __FUNCTION__ 
              << std::endl;
    bool bResult = false;

    // lock g_SessionInfo
    std::lock_guard<std::mutex> SessionLock(g_SessionMutex);

    auto itS = g_SessionInfo.find(wClientID);
    if (itS != g_SessionInfo.end())
    {
        g_SessionInfo.erase(itS);
        bResult = true;
    }

    return bResult;
};
