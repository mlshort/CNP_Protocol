/**
 * @file   CNP_Client.cpp
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <time.h>

#include "CNP_Socket.h"
#include "CNP_Client.h"
#include "../Include/CNP_Protocol.h"

char g_szBuffer[512] = { 0 };

#define CASE_CERTYPE(cer) \
        case cer: \
            return (const char*)#cer; 

const char* CerTypeToString(cnp::CER_TYPE cerType) noexcept
{
    using namespace cnp;

    switch (cerType) 
        {
        CASE_CERTYPE(CER_SUCCESS)
            break;
        CASE_CERTYPE(CER_AUTHENICATION_FAILED)
            break;
        CASE_CERTYPE(CER_UNSUPPORTED_PROTOCOL)
            break;
        CASE_CERTYPE(CER_INVALID_CLIENT_ID)
            break;
        CASE_CERTYPE(CER_INVALID_NAME_PIN)
            break;
        CASE_CERTYPE(CER_INVALID_ARGUMENTS)
            break;
        CASE_CERTYPE(CER_CLIENT_NOT_LOGGEDON)
            break;
        CASE_CERTYPE(CER_DRAWER_BLOCKED)
            break;
        CASE_CERTYPE(CER_INSUFFICIENT_FUNDS)
            break;
        CASE_CERTYPE(CER_ACCOUNT_NOT_FOUND)
            break;
        CASE_CERTYPE(CER_ACCOUNT_EXISTS)
            break;
        CASE_CERTYPE(CER_ERROR)
            break;

        default:
            break;
    }

    return "Unknown Error Type";
}

const char* TransTypeToString(cnp::TRANSACTION_TYPE tType) noexcept
{
    switch (tType)
    {
        case cnp::TT_INVALID:
            return "Invalid";
            break;
        case cnp::TT_DEPOSIT:
            return "Deposit";
            break;
        case cnp::TT_WITHDRAWAL:
            return "Withdrawal";
            break;
        case cnp::TT_STAMP_PURCHASE:
            return "Stamp Purchase";
            break;
        default:
            break;
    }

    return "Unknown Type";
};

std::string& RawTimeToLocalTimeString(time_t tRawTime, std::string& strLocalTime)
{
    struct tm* pLocalTime;
    char szBuffer[80] = { 0 };

    pLocalTime = localtime(&tRawTime);

    strftime(szBuffer, COUNTOF(szBuffer) - 1, "%d %b %y %H:%M:%S", pLocalTime);
    strLocalTime = szBuffer;

    return strLocalTime;
};

void PrintBankMenu(void)
{
    std::cout << "Please enter one of following options" << std::endl;
    std::cout << "\t 'c' - Connect Request" << std::endl;
    std::cout << "\t 'a' - Create Account Request" << std::endl;
    std::cout << "\t 'i' - Login Request" << std::endl;
    std::cout << "\t 'o' - Logout Request" << std::endl;
    std::cout << "\t 'b' - Balance Query Request" << std::endl;
    std::cout << "\t 'd' - Deposit Request" << std::endl;
    std::cout << "\t 'w' - Withdrawal Request" << std::endl;
    std::cout << "\t 't' - Transaction Query Request" << std::endl;
    std::cout << "\t 's' - Stamp Purchase Request" << std::endl;
    std::cout << "\t 'x' - Exit" << std::endl;
};

cnp::CER_TYPE SendConnect(CNP_Socket& socket, cnp::WORD& wClientID)
{
    cnp::CER_TYPE        cerResult = cnp::CER_ERROR;
    cnp::CONNECT_REQUEST conReq;

    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&conReq, conReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

    cnp::CONNECT_RESPONSE* pResp = reinterpret_cast<cnp::CONNECT_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    if (cerResult == cnp::CER_SUCCESS)
        wClientID = pResp->get_ClientID();

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;
    
    return cerResult;
};

cnp::CER_TYPE SendCreateAccount(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE cerResult = cnp::CER_ERROR;
    std::string   strFirstName;
    std::string   strLastName;
    std::string   strEmailAddress;
    cnp::WORD     wPIN;
    
    std::cout << "Enter First Name:";
    std::cin  >> strFirstName;

    std::cout << "Enter Last Name:";
    std::cin  >> strLastName;

    std::cout << "Enter Email Address:";
    std::cin  >> strEmailAddress;
 
    std::cout << "Enter PIN:";
    std::cin  >> wPIN;

    cnp::CREATE_ACCOUNT_REQUEST acctReq(wClientID,
                                        strFirstName.c_str(),
                                        strLastName.c_str(),
                                        strEmailAddress.c_str(),
                                        wPIN);

    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&acctReq, acctReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

    cnp::CREATE_ACCOUNT_RESPONSE* pResp = reinterpret_cast<cnp::CREATE_ACCOUNT_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;

    return cerResult;
};

cnp::CER_TYPE SendLogIn(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE  cerResult = cnp::CER_ERROR;
    std::string    strFirstName;
    cnp::WORD      wPIN;
    
    std::cout << "Enter Login First Name:";
    std::cin  >> strFirstName;

    std::cout << "Enter Login PIN:";
    std::cin  >> wPIN;

    cnp::LOGON_REQUEST logReq(wClientID, strFirstName.c_str(), wPIN);

    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&logReq, logReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

    cnp::LOGON_RESPONSE* pResp = reinterpret_cast<cnp::LOGON_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;

    return cerResult;
};

cnp::CER_TYPE SendLogOut(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE       cerResult = cnp::CER_ERROR;
    cnp::LOGOFF_REQUEST loReq(wClientID);
        
    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&loReq, loReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);
    
    cnp::LOGOFF_RESPONSE* pResp = reinterpret_cast<cnp::LOGOFF_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;

    return cerResult;
};

cnp::CER_TYPE SendDeposit(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE  cerResult = cnp::CER_ERROR;
    cnp::DWORD     dwAmount;

    std::cout << "Enter Deposit Amount:";
    std::cin  >> dwAmount;
    
    cnp::DEPOSIT_REQUEST depReq(wClientID, dwAmount, cnp::DT_CASH);
        
    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&depReq, depReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

    cnp::DEPOSIT_RESPONSE* pResp = reinterpret_cast<cnp::DEPOSIT_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;

    return cerResult;
};

cnp::CER_TYPE SendWithdrawal(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE   cerResult = cnp::CER_ERROR;
    cnp::DWORD      dwAmount;
    
    std::cout << "Enter Withdrawal Amount:";
    std::cin  >> dwAmount;

    cnp::WITHDRAWAL_REQUEST withReq(wClientID, dwAmount);

    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&withReq, withReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

    cnp::WITHDRAWAL_RESPONSE* pResp = reinterpret_cast<cnp::WITHDRAWAL_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;

    return cerResult;
};

cnp::CER_TYPE SendBalance(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE              cerResult = cnp::CER_ERROR;
    cnp::BALANCE_QUERY_REQUEST balReq(wClientID);

    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&balReq, balReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);
    
    cnp::BALANCE_QUERY_RESPONSE* pResp = reinterpret_cast<cnp::BALANCE_QUERY_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;
    if (cnp::Succeeded(cerResult))
    {
        std::cout << " Funds Available: $" << std::fixed << std::setprecision(2) << (pResp->get_Balance() / 100.0) << std::endl;
    }

    return cerResult;
};



cnp::CER_TYPE SendTransaction(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE  cerResult = cnp::CER_ERROR;
    bool       bMoreRecords = true;
    cnp::DWORD dwStartID = 0;
    cnp::WORD  wTransCnt = 5;
    while (bMoreRecords)
    {
        cnp::TRANSACTION_QUERY_REQUEST     transReq(wClientID, dwStartID, wTransCnt);

        std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

        socket.Send(&transReq, transReq.get_Size());
        socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

        cnp::TRANSACTION_QUERY_RESPONSE* pResp = reinterpret_cast<cnp::TRANSACTION_QUERY_RESPONSE*>( g_szBuffer );
        cerResult = static_cast<cnp::CER_TYPE>( pResp->get_ResponseResult() );

        std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;
        if (cnp::Succeeded(cerResult))
        {
            cnp::WORD wCnt = pResp->get_TransactionCount();

            for (cnp::WORD i = 0; i < wCnt; i++)
            {
                std::string strDT;
                std::cout << "ID: "     << pResp->m_Response.m_rgTransactions[i].get_ID()
                          << " Date: "  << RawTimeToLocalTimeString(pResp->m_Response.m_rgTransactions[i].get_DateTime(), strDT)
                          << " Amt: $ " << std::setw(8) << std::fixed << std::setprecision(2) << pResp->m_Response.m_rgTransactions[i].get_Amount() / 100.0
                          << " " << TransTypeToString(pResp->m_Response.m_rgTransactions[i].get_Type()) << std::endl;
            }

            if (wCnt < wTransCnt)
                bMoreRecords = false;
            else
                dwStartID = pResp->m_Response.m_rgTransactions[wCnt - 1].get_ID() + 1;
        }
        else
        {
            bMoreRecords = false;
        }
    }

    return cerResult;
};

cnp::CER_TYPE SendStampPurchase(CNP_Socket& socket, cnp::WORD wClientID)
{
    cnp::CER_TYPE   cerResult = cnp::CER_ERROR;
    cnp::DWORD      dwAmount;

    std::cout << "Enter Stamp Purchase Amount:";
    std::cin  >> dwAmount;

    cnp::STAMP_PURCHASE_REQUEST stpReq(wClientID, dwAmount);

    std::cout << "..." << __FUNCTION__ << " Request" << std::endl;

    socket.Send(&stpReq, stpReq.get_Size());
    socket.Receive(g_szBuffer, sizeof(g_szBuffer) - 1);

    cnp::STAMP_PURCHASE_RESPONSE* pResp = reinterpret_cast<cnp::STAMP_PURCHASE_RESPONSE*>( g_szBuffer );
    cerResult = static_cast<cnp::CER_TYPE>(pResp->get_ResponseResult());

    std::cout << "..." << __FUNCTION__ << " Result:" << CerTypeToString(cerResult) << std::endl;

    return cerResult;
};


int main(int argc, char *argv[])
{

#ifdef _MSC_VER
    WORD    wVersionRequested;
    WSADATA wsaData;
    int     iError;

    wVersionRequested = MAKEWORD(2, 2);

    iError = ::WSAStartup(wVersionRequested, &wsaData);
    if (iError != 0) 
    {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        std::cerr << "WSAStartup failed with error:" << iError << std::endl;
        return 1;
    }
#endif
    CNP_Socket    clientSocket;

    cnp::WORD     wClientID = cnp::INVALID_CLIENT_ID;
    cnp::CER_TYPE cerResult = cnp::CER_ERROR;
    char cInputOption   = 0;

    std::string strIP;
    unsigned short wPort;
    
    std::cout << "Enter Server IP Address:";
    std::cin  >> strIP;
    std::cout << "Enter Server Port:";
    std::cin  >> wPort;

    std::cout << "Attempting to connect to " << strIP << ":" << wPort << std::endl;
//    if (clientSocket.Connect("129.120.151.99", 3322))
    if (clientSocket.Connect(strIP.c_str(), wPort))
    {
        std::cout << "Connection Successful!" << std::endl;

        while (cInputOption != 'x')
        {

            switch (cInputOption)
            {

                case 'c':
                    cerResult = SendConnect(clientSocket, wClientID);
                    break;

                case 'a':
                    cerResult = SendCreateAccount(clientSocket, wClientID);
                    break;

                case 'i':
                    cerResult = SendLogIn(clientSocket, wClientID);
                    break;

                case 'o':
                    cerResult = SendLogOut(clientSocket, wClientID);
                    break;

                case 'd':
                    cerResult = SendDeposit(clientSocket, wClientID);
                    break;

                case 'b':
                    cerResult = SendBalance(clientSocket, wClientID);
                    break;

                case 'w':
                    cerResult = SendWithdrawal(clientSocket, wClientID);
                    break;

                case 's':
                    cerResult = SendStampPurchase(clientSocket, wClientID);
                    break;

                case 't':
                    cerResult = SendTransaction(clientSocket, wClientID);
                    break;

                default:
                    PrintBankMenu();
                    break;
            }

            std::cout << "> ";
            std::cin >> cInputOption;
        }
    }


    clientSocket.Close();

#ifdef _MSC_VER
    ::WSACleanup();
#endif

    return 0;
}

