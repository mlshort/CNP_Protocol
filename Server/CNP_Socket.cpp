/**
 * @file   Server//CNP_Socket.cpp
 * @brief  Server CNP_Socket class implementation
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * 
 * 
 */

#ifdef _MSC_VER
    #define WIN32_LEAN_AND_MEAN

    #include <winsock2.h>
    #include <Ws2tcpip.h>
    #include <stdio.h>
    #include <stdlib.h>

    // Link with ws2_32.lib
    #pragma comment(lib, "Ws2_32.lib")

    inline int CNP_GetLastError(void)
    { return ::WSAGetLastError(); };

#elif __linux__
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/time.h>
    #include <sys/fcntl.h>

    inline int CNP_GetLastError(void)
    { return errno; };

#endif

#include <memory.h>
#include <iostream>

#include "CNP_Socket.h"

#ifdef __linux__
void closesocket(SOCKET socket) 
{ 
    ::close(socket); 
};
#endif

CNP_Socket::CNP_Socket(void)
    : m_hSocket(INVALID_SOCKET),
      m_wPort (0),
      m_iError(0)
{
    memset(&m_LocalAddr, 0, sizeof(m_LocalAddr));
    memset(&m_RemoteAddr, 0, sizeof(m_RemoteAddr));
};

CNP_Socket::CNP_Socket(SOCKET hSocket, const sockaddr_in& remoteAddr)
    : m_hSocket(hSocket),
      m_wPort(0),
      m_iError(0)
{
    memset(&m_LocalAddr, 0, sizeof(m_LocalAddr));
    m_RemoteAddr = remoteAddr;
};

CNP_Socket::~CNP_Socket(void)
{
    Close();
};

bool CNP_Socket::SetBlocking(bool bBlocking /* = true */)
{
    bool bResult = false;
    
    if (m_hSocket != INVALID_SOCKET)
    {
#ifdef __linux__
        int iCurFlags = ::fcntl(m_hSocket, F_GETFL, 0);
        if (bBlocking)
            ::fcntl(m_hSocket, F_SETFL, iCurFlags & ~O_NONBLOCK);
        else
            ::fcntl(m_hSocket, F_SETFL, iCurFlags | O_NONBLOCK);
        bResult = true;
#elif _MSC_VER
        unsigned long nMode = bBlocking ? 0 : 1;
        int iResult = ::ioctlsocket(m_hSocket, FIONBIO, &nMode);

        if (iResult != NO_ERROR)
            std::cerr << "ioctlsocket failed with error:" << iResult << std::endl;
        else
            bResult = true;
#endif
    }

    return bResult;
}


bool CNP_Socket::Create(unsigned short wPort)
{
    bool bResult = false;
    m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
 
    if (m_hSocket != INVALID_SOCKET)
    {
        int iOn = 1;
        SetSocketOption(SOL_SOCKET, SO_REUSEADDR, &iOn, sizeof(iOn));

        m_LocalAddr.sin_family      = AF_INET;
        m_LocalAddr.sin_port        = ::htons(wPort);
        m_LocalAddr.sin_addr.s_addr = INADDR_ANY;
        if (::bind(m_hSocket,(struct sockaddr *) &m_LocalAddr, sizeof(m_LocalAddr)) != SOCKET_ERROR)
        {
            m_wPort = wPort;
            bResult = true;
        }
        else
        {
            m_iError = CNP_GetLastError(); //errno;
            std::cerr << "Failure to bind to port:" << wPort << std::endl;
        }
    }
    else
    {
        m_iError = CNP_GetLastError(); //errno;
        std::cerr << "Failure to create socket" << std::endl;
    }

    return bResult;
};

bool CNP_Socket::Connect(const char* szHostAddress, unsigned short wPort )
{
    if (m_hSocket == INVALID_SOCKET)
    {
        m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_hSocket == INVALID_SOCKET)
        {
            m_iError = CNP_GetLastError(); //errno;
            return false;
        }
    }

    // Fill address machinery of sockets.
    memset(&m_RemoteAddr,0,sizeof(m_RemoteAddr));

    m_RemoteAddr.sin_family      = AF_INET;
//    m_RemoteAddr.sin_addr.s_addr = ::inet_pton(szHostAddress, ); //::inet_addr(szHostAddress);

    if (!::InetPtonA(AF_INET, szHostAddress, &m_RemoteAddr.sin_addr.s_addr))
        return false;

    m_RemoteAddr.sin_port        = ::htons(wPort);

    // connects to peer
    if (SOCKET_ERROR == ::connect(m_hSocket, (struct sockaddr *)&m_RemoteAddr, sizeof(m_RemoteAddr)))
    {
        m_iError = CNP_GetLastError(); //errno;
        return false;
    }

    return true;
}

bool CNP_Socket::Listen(int iBackLog)
{
    bool bResult = false;
    if (m_hSocket != INVALID_SOCKET)
    {
       if (::listen(m_hSocket, iBackLog) != SOCKET_ERROR)
           m_iError = CNP_GetLastError(); //errno;
       else
           bResult = true;
    }

    return bResult;
};

void CNP_Socket::Close(void)
{
    if (m_hSocket != INVALID_SOCKET)
    {
        closesocket(m_hSocket);
        m_hSocket = INVALID_SOCKET;
    }
};

bool CNP_Socket::Accept(SOCKET& hSocket, sockaddr_in& remoteAddr)
{
    bool bResult = false;
#ifdef __linux__
    size_t sin_size = sizeof(struct sockaddr_in);
#elif _MSC_VER
    int sin_size = sizeof(struct sockaddr_in);
#endif

    hSocket  = ::accept(m_hSocket, ( struct sockaddr * )&remoteAddr, &sin_size);
    if (hSocket != INVALID_SOCKET)
    {
        bResult = true;
    }
    else
    {
        m_iError = CNP_GetLastError(); //errno;
    }

    return bResult;
};

int CNP_Socket::Receive(void* pData, size_t cbLen, int iFlags /* = 0 */)
{
    int nResult = ::recv (m_hSocket, static_cast<char*>(pData), cbLen, iFlags);
    m_iError    = CNP_GetLastError(); //errno;

 //   std::cout << __FUNCTION__ << "Len:" << nResult << " LastErr:" << m_iError << std::endl;
    
    return nResult == -1 ? SOCKET_ERROR : nResult;
};

int CNP_Socket::Send(const void* pData, size_t cbLen, int iFlags /* = 0 */)
{
    int nResult = ::send(m_hSocket, static_cast<const char*>(pData), cbLen, iFlags);
    m_iError    = CNP_GetLastError(); //errno;
    return nResult == -1 ? SOCKET_ERROR : nResult;
};

int CNP_Socket::SetSocketOption(int iLevel, int iOption, const void* pVal, size_t cbLen)
{
#ifdef __linux__
    int nResult = ::setsockopt (m_hSocket, iLevel, iOption, pVal, cbLen);
#elif _MSC_VER
    int nResult = ::setsockopt (m_hSocket, iLevel, iOption, static_cast<const char*>(pVal), cbLen);
#endif
    m_iError    = CNP_GetLastError(); //errno;
    return nResult == -1 ? SOCKET_ERROR : nResult;
};

#ifdef __linux__

int CNP_Socket::SetSocketRecvTimeout(unsigned int uSecs, unsigned int uMicroSecs)
{
    struct timeval tv;

    tv.tv_sec  = uSecs;
    tv.tv_usec = uMicroSecs;

    return SetSocketOption( SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );
};

int CNP_Socket::SetSocketSendTimeout(unsigned int uSecs, unsigned int uMicroSecs)
{
    struct timeval tv;

    tv.tv_sec  = uSecs;
    tv.tv_usec = uMicroSecs;

    return SetSocketOption( SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );
};

#elif _MSC_VER

int CNP_Socket::SetSocketRecvTimeout(unsigned long ulMilliSecs)
{
    DWORD dwMS = ulMilliSecs;

    return SetSocketOption( SOL_SOCKET, SO_RCVTIMEO, &dwMS, sizeof(dwMS) );
};

int CNP_Socket::SetSocketSendTimeout(unsigned long ulMilliSecs)
{
    DWORD dwMS = ulMilliSecs;

    return SetSocketOption( SOL_SOCKET, SO_SNDTIMEO, &dwMS, sizeof(dwMS) );
};

#endif

bool CNP_Socket::Shutdown(int iHow)
{
    bool bResult = false;

    if (m_hSocket != INVALID_SOCKET)
    {
        bResult  = (::shutdown(m_hSocket, iHow) != SOCKET_ERROR);
        m_iError = CNP_GetLastError(); //errno;
    }
    return bResult;
};
