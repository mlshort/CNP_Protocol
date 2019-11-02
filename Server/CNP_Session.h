/**
 * @file   CNP_Session.h
 * @brief  SESSION_INFO struct definition
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * @date   April 25, 2015 updated comments
 *
 */

#if !defined(__CNP_SESSION_H__)
#define __CNP_SESSION_H__

#ifndef __CNP_COMMON_H__
    #include "CNP_Common.h"
#endif

#ifndef __CNP_SOCKET_H__
    #include "CNP_Socket.h"
#endif

#ifndef _MAP_
    #include <map>
#endif

/**
    A basic enumeration of allowable session states
 */
enum SESSION_STATE 
{
    SS_INVALID = 0,
    SS_CONNECTED,
    SS_ACCOUNT_CREATED,
    SS_LOGGED_ON,
    SS_LOGGED_OFF,
    SS_DISCONNECTING
};

/**
    SESSION_INFO is a runtime only data-structure
    used to maintain an association between Client ID,
    session state, socket connection & Customer ID.
 */
struct SESSION_INFO
{
    typedef cnp::WORD key_type;

    cnp::WORD     m_wClientID; ///< Key field
    cnp::WORD     m_wState;
    CNP_Socket*   m_pSocket;
    cnp::QWORD    m_qwCustomerID;

    /// Initialization Constructor
    SESSION_INFO(cnp::WORD wClientID, SESSION_STATE sState, CNP_Socket* pSocket = nullptr)
        : m_wClientID   (wClientID),
          m_wState      (static_cast<cnp::WORD>(sState)),
          m_pSocket     (pSocket),
          m_qwCustomerID(INVALID_CUSTOMER_ID)
    { };

    inline cnp::WORD        get_ClientID(void) const noexcept
    { return m_wClientID; };

    inline void             set_ClientID(cnp::WORD wSet) noexcept
    { m_wClientID = wSet; };

    inline cnp::WORD        get_State(void) const noexcept
    { return m_wState; };

    inline void             set_State(SESSION_STATE sSet) noexcept
    { m_wState = static_cast<cnp::WORD>(sSet); };

    inline const cnp::QWORD& get_CustomerID(void) const noexcept
    { return m_qwCustomerID; };

    inline void              set_CustomerID(const cnp::QWORD& qwSet) noexcept
    { m_qwCustomerID = qwSet; };

};

typedef std::map<SESSION_INFO::key_type, SESSION_INFO>     SessionMap_t;

#endif