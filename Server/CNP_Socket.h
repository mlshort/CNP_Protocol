/**
 * @file   Server//CNP_Socket.h
 * @brief  Server CNP_Socket class interface
 *
 * CNP_Socket class provides the basic TCP socket
 * functionality.  It supports both Windows &
 * Linux platforms* 
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * @date   April 25, 2015
 *
 */

#if !defined(__CNP_SOCKET_H__)
#define __CNP_SOCKET_H__

#include <errno.h>

#if defined __linux__
    typedef int SOCKET;
    #ifndef INVALID_SOCKET
        #define INVALID_SOCKET  (SOCKET)(~0)
    #endif
    #ifndef SOCKET_ERROR
        #define SOCKET_ERROR            (-1)
    #endif
    #include <netinet/in.h>
#elif _MSC_VER
    #define WIN32_LEAN_AND_MEAN

    #include <winsock2.h>
    #include <Ws2tcpip.h>
#endif


class CNP_Socket
{
    SOCKET         m_hSocket;
    unsigned short m_wPort;
    sockaddr_in    m_LocalAddr;
    sockaddr_in    m_RemoteAddr;
    int            m_iError;

public:
/**  
    @brief Default Constructor
         
    Performs member data initialization to default values only
*/
    CNP_Socket() noexcept;

/** 
    @brief Initialization Constructor
        
    Performs member data initialization to default values,
    with the exception of the parameters
*/
    CNP_Socket(SOCKET hSocket, const sockaddr_in& remoteAddr) noexcept;

    ~CNP_Socket();

    void Close(void) noexcept;

    bool Create (unsigned short wPort) noexcept;
    bool Connect(const char* szHostAddress, unsigned short wPort) noexcept;
/**
    places the underlying socket in a state in which it is listening for an incoming connection

    @param [in] iBackLog  The maximum length of the queue of pending connections. 
                          If set to SOMAXCONN, the underlying service provider responsible 
                          for socket s will set the backlog to a maximum reasonable value. 
                          There is no standard provision to obtain the actual backlog value
    @retval true  on success
    @retval false on failure
 */
    bool Listen (int iBackLog) noexcept;

    bool Accept (SOCKET& hSocket, sockaddr_in& remoteAddr) noexcept;

/**
    @brief Receives data from the underlying connected socket or a bound connectionless socket

    @param [out] pData      A pointer to the buffer to receive the incoming data
    @param [in] cbLen       The length, in bytes, of the buffer pointed to by the pData parameter
    @param [in] iFlags      Optional parameter that influences the behavior of this function

    @retval int             containing the number of bytes received and the buffer pointed to by the pData 
                            parameter will contain this data received
    @retval 0               if the connection has been gracefully closed
    @retval SOCKET_ERROR    on failure  call GetError() to retrieve the specific error code

 */
    int  Receive(void* pData, size_t cbLen, int iFlags = 0) noexcept;
    int  Send   (const void* pData, size_t cbLen, int iFlags = 0) noexcept;
/**
   @brief Sets the underlying socket option

   @param [in] iLevel    The level at which the option is defined (for example, SOL_SOCKET).
   @param [in] iOption   The socket option for which the value is to be set (for example, SO_BROADCAST). 
                         The iOption parameter must be a socket option defined within the specified level, 
                         or behavior is undefined
   @param [in] pVal      A pointer to the buffer in which the value for the requested option is specified
   @param [in] cbLen     The size, in bytes, of the buffer pointed to by the pVal parameter

   @retval 0            on success
   @retval SOCKET_ERROR on failure  call GetError() to retrieve the specific error code
*/

    int  SetSocketOption(int iLevel, int iOption, const void* pVal, size_t cbLen) noexcept;

    bool SetBlocking    (bool bBlocking = true) noexcept;

#ifdef __linux__
    int  SetSocketRecvTimeout(unsigned int uSecs, unsigned int uMicroSecs) noexcept;
    int  SetSocketSendTimeout(unsigned int uSecs, unsigned int uMicroSecs) noexcept;
#elif _MSC_VER
    int  SetSocketRecvTimeout(unsigned long ulMilliSecs) noexcept;
    int  SetSocketSendTimeout(unsigned long ulMilliSecs) noexcept;
#endif
 /**
    @brief disables sends or receives on underlying socket

    @param [in] iHow    A platform specific flag that describes what types of operation 
                        will no longer be allowed.

    @retval true  on success
    @retval false on failure
 */
    bool Shutdown(int iHow) noexcept;
/**
    @retval int   containing the most recent error code
 */
    constexpr int  GetError(void) const noexcept
    { return m_iError; };

    inline bool WouldBlock(void) const noexcept
#ifdef __linux__
    { return m_iError == EWOULDBLOCK || m_iError == EAGAIN; };
#elif _MSC_VER
    { return m_iError == WSAEWOULDBLOCK; };
#endif

    inline bool Interrupted(void) const noexcept
#ifdef __linux__
    { return m_iError == EINTR; };
#elif _MSC_VER
    { return m_iError == WSAEINTR; };
#endif

};

#endif