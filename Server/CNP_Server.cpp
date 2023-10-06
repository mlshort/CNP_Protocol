/**
 * @file   CNP_Server.cpp
 * @brief  Server Main
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <atomic>
#include <thread>
#include <list>
#include <iostream>

#include "CNP_ServerDB.h"
#include "CNP_Socket.h"
#include "CNP_Messaging.h"
#include "CNP_Session.h"
#include "CNP_Server.h"

#ifdef __linux__
    std::atomic_bool g_bTerminate(false);
#elif _MSC_VER
    std::atomic<bool> g_bTerminate = false;
#endif

#ifdef __linux__

#include <unistd.h>
#include <sys/syscall.h>

pid_t GetThreadID( void )
{
    return syscall( __NR_gettid );
};

#elif _MSC_VER

DWORD GetThreadID( void ) noexcept
{
    return ::GetCurrentThreadId();
};

#endif

/**
   A basic data-structure passed to the thread function
 */
struct THREAD_INFO
{
    std::atomic<bool>    m_bTerminate;
    CNP_Socket*          m_pSocket;
    std::thread*         m_pThread;

    THREAD_INFO(void) noexcept
        : m_bTerminate(false),
          m_pSocket(nullptr),
          m_pThread(nullptr)
    { };

    ~THREAD_INFO(void)
    { 
      if (m_pSocket)
         delete m_pSocket; 
      if (m_pThread)
          delete m_pThread;
    };

private:
// Cannot afford for these guys to accidentally
// get invoked or allow shallow copies since
// the destructor is deleting associated member
// pointer memory
    THREAD_INFO(const THREAD_INFO&);
    THREAD_INFO& operator=(const THREAD_INFO&);
};


void ClientThreadHandler(void* pData)
{
    THREAD_INFO*  pInfo    = static_cast<THREAD_INFO*>(pData);
    std::cout << __FUNCTION__ << " ThreadID:" << GetThreadID() << std::endl;
    bool bResult           = false;
    cnp::WORD wClientID    = cnp::INVALID_CLIENT_ID;

    CNP_Socket*   pSocket  = pInfo->m_pSocket;

    char rgBuffer[2048] = { 0 };

    while (pInfo->m_bTerminate == false)
    {
        int cbMsgLen = pSocket->Receive(rgBuffer, sizeof(rgBuffer) - 1);

        if ( cbMsgLen == SOCKET_ERROR )
        {
            if ( pSocket->WouldBlock() || pSocket->Interrupted() )
            {
            // these are safe to ignore and try again
            }
        }
        else if (( cbMsgLen == 0 ) || (cbMsgLen == SOCKET_ERROR))
        {
            // Client has disconnected or terminated
            ProcessDisconnect(wClientID);
            pSocket->Close();
            pInfo->m_bTerminate = true;
        }
        else if (( cbMsgLen > 0 ) && ( cbMsgLen != SOCKET_ERROR ))
        {
            // typecast the buffer to STD_HDR to give easy access to helper methods
            cnp::STD_HDR* pHdr = reinterpret_cast<cnp::STD_HDR*>( rgBuffer );

            switch (pHdr->get_MsgType())
            {
                case  cnp::MT_CONNECT_REQUEST:
                    wClientID = ProcessConnectRequest(rgBuffer, cbMsgLen, pSocket);
                    break;

                case cnp::MT_CREATE_ACCOUNT_REQUEST:
                    bResult = ProcessCreateAccountRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_LOGON_REQUEST:
                    bResult = ProcessLogonRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_LOGOFF_REQUEST:
                    bResult = ProcessLogoffRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_DEPOSIT_REQUEST:
                    bResult = ProcessDepositRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_WITHDRAWAL_REQUEST:
                    bResult = ProcessWithdrawalRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_BALANCE_QUERY_REQUEST:
                    bResult = ProcessBalanceQueryRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_TRANSACTION_QUERY_REQUEST:
                    bResult = ProcessTransactionQueryRequest(rgBuffer, cbMsgLen);
                    break;

                case cnp::MT_PURCHASE_STAMPS_REQUEST:
                    bResult = ProcessStampPurchaseRequest(rgBuffer, cbMsgLen);
                    break;

                default:
                    // invalid message
                    break;
            }
        }
#ifdef _MSC_VER
        ::Sleep(250);
#endif
    }

    std::cout << "Exiting ThreadID:" << GetThreadID() << std::endl;
};

void TerminateHandler(int /*iSignal*/) noexcept
{
    g_bTerminate = true;
}

#ifdef _MSC_VER
BOOL CtrlHandler( DWORD fdwCtrlType ) noexcept
{ 
  switch( fdwCtrlType ) 
  { 
    // Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
      g_bTerminate = true;
      printf( "Ctrl-C event\n\n" );
      return( TRUE );
 
    // CTRL-CLOSE: confirm that the user wants to exit. 
    case CTRL_CLOSE_EVENT: 
      g_bTerminate = true;
      return( TRUE ); 
 
    // Pass other signals to the next handler. 
    case CTRL_BREAK_EVENT: 
      g_bTerminate = true;
      return FALSE; 
 
    case CTRL_LOGOFF_EVENT: 
      g_bTerminate = true;
      return FALSE; 
 
    case CTRL_SHUTDOWN_EVENT: 
      g_bTerminate = true;
      return FALSE; 
 
    default: 
      return FALSE; 
  } 
} 
#endif

int main(int argc, char *argv[])
{


#ifdef __linux__
    if (signal(SIGUSR1, TerminateHandler) == SIG_ERR)
        printf("\ncan't catch SIGUSR1\n");
    if (signal(SIGTERM, TerminateHandler) == SIG_ERR)
        printf("\ncan't catch SIGSTOP\n");
    if (signal(SIGINT, TerminateHandler) == SIG_ERR)
        printf("\ncan't catch SIGSTOP\n");

#elif _MSC_VER

    if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE))
    {
        printf("\nThe Control Handler is installed.\n");
        printf("\n -- Use Ctrl+C to gracefully terminate \n");
    }


    WORD    wVersionRequested;
    WSADATA wsaData{ 0 };
    int     iError;

    wVersionRequested = MAKEWORD(2, 2);

    iError = WSAStartup(wVersionRequested, &wsaData);
    if (iError != 0) 
    {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        std::cerr << "WSAStartup failed with error:" << iError << std::endl;
        return 1;
    }

#endif

// attempt to load persistent server data.
    LoadServerDB();

    std::list<THREAD_INFO*> lstClientThreadInfo;

    unsigned short wPort;
   
    std::cout << "Enter Server [Listening] Port:";
    std::cin  >> wPort;

    CNP_Socket SvrSocket;
    
    if (SvrSocket.Create(wPort))
        std::cout << "Server Listening Socket created on Port:" << wPort << std::endl;

    if (SvrSocket.Listen(10))
        std::cout << "Listening for connections" << std::endl;

    SvrSocket.SetBlocking(false);

    SOCKET      hNewSocket = INVALID_SOCKET;
    sockaddr_in remoteAddr;

    while (g_bTerminate == false)
    {
        if (SvrSocket.Accept(hNewSocket, remoteAddr))
        {
            std::cout << "Accepting a new connection" << std::endl;
            std::cout << "--------------------------" << std::endl;

            THREAD_INFO* pInfo        = new THREAD_INFO();

            pInfo->m_pSocket          = new CNP_Socket(hNewSocket, remoteAddr);
#ifdef __linux__
            pInfo->m_pSocket->SetSocketRecvTimeout(0, 500);
#elif _MSC_VER
            pInfo->m_pSocket->SetSocketRecvTimeout(500);
#endif
            pInfo->m_pThread          = new std::thread(ClientThreadHandler, pInfo);

            lstClientThreadInfo.push_front(pInfo);
        }
        else if (!SvrSocket.WouldBlock())
        {
            std::cerr << "failed to accept new connection" << std::endl;
        }
        else
        {
#ifdef _MSC_VER
            ::Sleep(500);
#endif
        }
    }
    
    std::cout << std::endl << "Caught signal, attempting graceful shutdown" << std::endl;
    for (auto& it : lstClientThreadInfo)
    {
        it->m_bTerminate = true;
        it->m_pThread->join();
#ifdef __linux__
        it->m_pSocket->Shutdown(SHUT_RDWR);
#elif _MSC_VER
        it->m_pSocket->Shutdown(SD_BOTH);
#endif
        delete it;
    }

    SvrSocket.Close();
    SaveServerDB();

#ifdef _MSC_VER
    WSACleanup();
#endif

    return 0;
}
