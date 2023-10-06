/**
 *  @file   CNP_Protocol.h
 *  @brief  Contains type definitions required to support 5580 Computer Networks 
 *          Project (CNP) Protocol 
 *
 *  @mainpage
 * 
 *  @author Mark L. Short
 *  @date   March 25, 2015
 *
 *  <b> Course : </b>
 *
 *   5580 Computer Networks
 *
 *  <b> Objective: </b>
 *
 *   Objective of this group activity is to design an application layer protocol 
 *   standard that will be used by all groups for the project.  The protocol must
 *   provide the basic ATM Banking functionality of:
 *    - Establish a connection
 *    - Creating an Account
 *         - First Name
 *         - Last Name
 *         - Email Address
 *         - SSN
 *         - Driver's License #
 *         - User selected PIN
 *    - Logging On 
 *         - PIN & First Name
 *    - Deposit
 *         - Cash or Check
 *    - Withdrawal
 *    - Stamp Purchase
 *    - Transaction History Query
 *
 * <b> Implementation Notes </b>
 *
 *   1. In addition to those required functions, the following were implemented:
 *     - Logging Off (explicit)
 *     - Balance Query
 *
 *  2.  Those types with the prefixed '_' are intentionally 'uglified' to discourage
 *      their direct use.  Additionally, they have been wrapped in the 'prim'
 *      namespace to further obscure them from direct use.
 *
 *      This is a common naming procedure used to denote an 'internal' type within
 *      a published API specification or protocol to not directly use.
 *
 *  3.  Regarding inheritance.
 *
 *      Even though, from an implementation stand-point, C++ inheritance would
 *      have afforded the ability to avoid a lot of "helper" method duplication;
 *      from the position of affording any byte-wise guarantee of consistency
 *      across various compilers and platforms, there was no way to insure that
 *      the use of inheritance would not, covertly, introduce additional hidden 
 *      bytes or other compiler generated information.
 *
 *      So, it was explicitly avoided in the message protocol implementation.
 *
 */

#if !defined(__CNP_PROTOCOL_H__)
#define __CNP_PROTOCOL_H__

#ifdef _MSC_VER
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS
    #endif
#endif

#include <string.h>

// set structure alignment to 1 byte
#pragma pack(push, 1)

/** @defgroup Msgs CNP Protocol Messages 
 *  
 *  @{
 */
/** @defgroup SvrMsgs Server Messages
 *
 *  @{
 *  @} 
 */ // end of SvrMsgs
/** @defgroup CltMsgs Client Messages
 *
 *  @{
 *  @} 
 */ // end of CltMsgs
/** @defgroup TypeDefs Message TypeDefs
 *
 *  @{
 *  @} 
 */ // end of CltMsgs
/** @} */ // end of Msgs

namespace cnp
{
    typedef unsigned short      WORD;   ///< 16bit type definition
    typedef unsigned long       DWORD;  ///< 32bit type definition
    typedef unsigned long long  QWORD;  ///< 64bit type definition

/// Helper macro that calculates count of elements in an array
#ifndef COUNTOF
    #define COUNTOF(_array)  (sizeof(_array) / sizeof(_array[0]))
#endif

#ifndef MAKE_MSG_TYPE
    #define   MAKE_MSG_TYPE(type, sub) ((sub << 16) + type)
#endif

#ifndef MAKE_ERROR_RESULT
    #define   MAKE_ERROR_RESULT(facility, sub) ((facility << 16) + sub)
#endif

/**
 *  @brief Global message sequence number
 *
 *  The following is a little complicated, but I will try to explain what is going on.
 *
 *  g_dwSequenceNumber is a static global variable intended to be auto-incremented by
 *  the client as part of constructing request messages.  It needs to be defined such 
 *  that it is accessible by various message constructors, but at the same time does 
 *  not create a linker error as it can be included in multiple .cpp files. 
 *  (resulting in multiple instances across various COMDATs)
 *
 *  So, we are telling the linker to just use one of the instances here if it is
 *  finds multiple instance declarations.
 *
 *  Another complication is that MSVC++ & GNUC++ cannot agree on how to do this.
 *
 *  @sa https://gcc.gnu.org/onlinedocs/gcc/Vague-Linkage.html
 *  @sa https://msdn.microsoft.com/EN-US/library/5tkz6s71(v=VS.120,d=hv.2).aspx
 */
#ifdef  __GNUC__
    __attribute__ ((weak)) DWORD g_dwSequenceNumber = 0;
#elif   _MSC_VER
    __declspec(selectany) extern DWORD g_dwSequenceNumber = 0;
#else
    #pragma message("Compiler not supported")
#endif

// ============== const definitions =====================

/// CNP Protocol version
constexpr WORD  g_wMajorVersion   = 1;  ///< Protocol major version (i.e. 1.x)
constexpr WORD  g_wMinorVersion   = 1;  ///< Protocol minor version (i.e. x.3)

 /// CNP Validation Key
constexpr DWORD g_dwValidationKey = 0x00DEAD01;

/// [first,last,email] name field lengths
/**
 *  @sa CNP_CREATE_ACCOUNT_REQUEST, CNP_LOGON_REQUEST
 */
constexpr size_t MAX_NAME_LEN     = 32;

 /// Used for error checking & default initialization
constexpr WORD   INVALID_CLIENT_ID = static_cast<WORD>(~0);
 /// Used for error checking & default initialization
constexpr WORD   INVALID_PIN = 0;

/// Supported CNP Message Types (CMT_)
enum CNP_MSG_TYPE
{
    CMT_INVALID           = 0x00,  ///< used for initialization and error checking
    CMT_CONNECT           = 0x50,
    CMT_CREATE_ACCOUNT    = 0x51,
    CMT_LOGON             = 0x52,
    CMT_LOGOFF            = 0x53,
    CMT_DEPOSIT           = 0x54,
    CMT_WITHDRAWAL        = 0x55,
    CMT_BALANCE_QUERY     = 0x56,
    CMT_TRANSACTION_QUERY = 0x57,
    CMT_PURCHASE_STAMPS   = 0x58
};

/// Supported CNP Message Subtypes (CMS_)
enum CNP_MSG_SUBTYPE
{
    CMS_INVALID           = 0x00,  ///< used for initialization and error checking
    CMS_REQUEST           = 0x01,
    CMS_RESPONSE          = 0x02
};

/**
 *  @brief Constructed Message Type IDs
 *
 *  @ingroup TypeDefs
 */
enum MSG_TYPE
{
     MT_INVALID                    = MAKE_MSG_TYPE(CMT_INVALID, CMS_INVALID),

     MT_CONNECT_REQUEST            = MAKE_MSG_TYPE(CMT_CONNECT, CMS_REQUEST),
     MT_CONNECT_RESPONSE           = MAKE_MSG_TYPE(CMT_CONNECT, CMS_RESPONSE),

     MT_CREATE_ACCOUNT_REQUEST     = MAKE_MSG_TYPE(CMT_CREATE_ACCOUNT, CMS_REQUEST),
     MT_CREATE_ACCOUNT_RESPONSE    = MAKE_MSG_TYPE(CMT_CREATE_ACCOUNT, CMS_RESPONSE),

     MT_LOGON_REQUEST              = MAKE_MSG_TYPE(CMT_LOGON, CMS_REQUEST),
     MT_LOGON_RESPONSE             = MAKE_MSG_TYPE(CMT_LOGON, CMS_RESPONSE),

     MT_LOGOFF_REQUEST             = MAKE_MSG_TYPE(CMT_LOGOFF, CMS_REQUEST),
     MT_LOGOFF_RESPONSE            = MAKE_MSG_TYPE(CMT_LOGOFF, CMS_RESPONSE),

     MT_DEPOSIT_REQUEST            = MAKE_MSG_TYPE(CMT_DEPOSIT, CMS_REQUEST),
     MT_DEPOSIT_RESPONSE           = MAKE_MSG_TYPE(CMT_DEPOSIT, CMS_RESPONSE),

     MT_WITHDRAWAL_REQUEST         = MAKE_MSG_TYPE(CMT_WITHDRAWAL, CMS_REQUEST),
     MT_WITHDRAWAL_RESPONSE        = MAKE_MSG_TYPE(CMT_WITHDRAWAL, CMS_RESPONSE),

     MT_BALANCE_QUERY_REQUEST      = MAKE_MSG_TYPE(CMT_BALANCE_QUERY, CMS_REQUEST),
     MT_BALANCE_QUERY_RESPONSE     = MAKE_MSG_TYPE(CMT_BALANCE_QUERY, CMS_RESPONSE),

     MT_TRANSACTION_QUERY_REQUEST  = MAKE_MSG_TYPE(CMT_TRANSACTION_QUERY, CMS_REQUEST),
     MT_TRANSACTION_QUERY_RESPONSE = MAKE_MSG_TYPE(CMT_TRANSACTION_QUERY, CMS_RESPONSE),

     MT_PURCHASE_STAMPS_REQUEST    = MAKE_MSG_TYPE(CMT_PURCHASE_STAMPS, CMS_REQUEST),
     MT_PURCHASE_STAMPS_RESPONSE   = MAKE_MSG_TYPE(CMT_PURCHASE_STAMPS, CMS_RESPONSE)
};
/**
 *  @brief Message Facility Code Types (CFC)
 *
 *  Used in the creation of result codes returned to the client.
 *  The purpose is to help provide the client useful diagnostic information
 *  regarding associating specific errors with a particular facility or
 *  functional subsystem.
 *
 *  @ingroup TypeDefs
 */
enum CFC_TYPE
{
    CFC_CONNECT,             ///< Connection validation related issues 
    CFC_CREDENTIALS,         ///< Logon related issues
    CFC_FUNCTIONAL,          ///< Invalid arguments or Client state
    CFC_ACCOUNT,             ///< Account related errors related to balances, etc.
    CFC_UNDEFINED            ///< Other error categories not explicitly defined
};

/** 
 *  @brief CNP Error Result Types (CER)
 *
 *  @ingroup Typedefs
 */
enum CER_TYPE
{
    CER_SUCCESS              = 0,  ///< Success!
    CER_AUTHENICATION_FAILED = MAKE_ERROR_RESULT(CFC_CONNECT, 0x01),     ///< Invalid validation key
    CER_UNSUPPORTED_PROTOCOL = MAKE_ERROR_RESULT(CFC_CONNECT, 0x02),     ///< Protocol version not supported
    CER_INVALID_CLIENT_ID    = MAKE_ERROR_RESULT(CFC_CREDENTIALS, 0x01), ///< Invalid client ID found
    CER_INVALID_NAME_PIN     = MAKE_ERROR_RESULT(CFC_CREDENTIALS, 0x02), ///< Invalid name or pin
    CER_INVALID_ARGUMENTS    = MAKE_ERROR_RESULT(CFC_FUNCTIONAL, 0x01),  ///< Invalid arguments used
    CER_CLIENT_NOT_LOGGEDON  = MAKE_ERROR_RESULT(CFC_FUNCTIONAL, 0x02),  ///< Client not logged-on
    CER_DRAWER_BLOCKED       = MAKE_ERROR_RESULT(CFC_FUNCTIONAL, 0x03),  ///< Mechanical Failure
    CER_INSUFFICIENT_FUNDS   = MAKE_ERROR_RESULT(CFC_ACCOUNT, 0x01),     ///< Insufficient funds available
    CER_ACCOUNT_NOT_FOUND    = MAKE_ERROR_RESULT(CFC_ACCOUNT, 0x02),     ///< Client account does not exist
    CER_ACCOUNT_EXISTS       = MAKE_ERROR_RESULT(CFC_ACCOUNT, 0x03),     ///< Prior account already exists
    CER_ERROR                = (~0)     ///< Generic error result
};


constexpr bool Succeeded(cnp::CER_TYPE cerRR) noexcept
{ return (cerRR == cnp::CER_SUCCESS); };


/**
 *  @brief CNP Deposit types (DT)
 *
 *  @ingroup TypeDefs
 */
enum DEPOSIT_TYPE
{
    DT_INVALID   = 0,     ///< for initialization and error checking
    DT_CASH      = 0x01,  ///< Cash Deposit
    DT_CHECK     = 0x02   ///< Check Deposit
};

/**
 *  @brief CNP Transaction types (TT)
 *
 *  @ingroup TypeDefs
 */
enum TRANSACTION_TYPE
{
    TT_INVALID        = 0,     ///< for initialization and error checking
    TT_DEPOSIT        = 0x01,  ///< Deposit Transaction
    TT_WITHDRAWAL     = 0x02,  ///< Withdrawal Transaction
    TT_STAMP_PURCHASE = 0x03   ///< Stamp Purchase Transaction
};


/** 
 *  @brief A Customer Transaction Record
 *
 *  |  Field(s)      | Begin Byte | End Byte |
 *  | :------------- | :--------: | :------: |
 *  | m_dwID         |  0         | 3        |
 *  | m_qwDateTime   |  4         | 11       |
 *  | m_dwAmount     |  12        | 15       |
 *  | m_wType        |  16        | 17       |
 *
 *  @sa TRANSACTION_TYPE
 *  @ingroup TypeDefs
 */
struct TRANSACTION
{
    DWORD        m_dwID;        ///< A Server generated unique sequential ID associated with each transaction
    QWORD        m_qwDateTime;  ///< a 64bit UTC value that represents number of seconds since Epoch
    DWORD        m_dwAmount;    ///< Amount excluding decimal point (i.e. $100.00 would be 10000)
    WORD         m_wType;       ///< The transaction type, represented as TT_DEPOSIT or TT_WITHDRAWAL

    constexpr TRANSACTION(void) noexcept
        : m_dwID(),
          m_qwDateTime(),
          m_dwAmount(),
          m_wType(static_cast<WORD>(TT_INVALID))
    { };

    constexpr TRANSACTION(DWORD dwID,
                          const QWORD& qwDateTime,
                          DWORD dwAmount,
                          WORD  wType) noexcept
        : m_dwID(dwID),
          m_qwDateTime(qwDateTime),
          m_dwAmount(dwAmount),
          m_wType(wType)
    { };

    inline DWORD get_ID(void) const noexcept
    { return m_dwID; };

    inline DWORD get_Amount(void) const noexcept
    { return m_dwAmount; };

    inline const QWORD& get_DateTime(void) const noexcept
    { return m_qwDateTime; };

    inline TRANSACTION_TYPE get_Type(void) const noexcept
    { return static_cast<TRANSACTION_TYPE>(m_wType); };
};

// ==================== CNP Message Primitives =================================
namespace prim
{

/** 
 *  @brief Connect Request Primitive
 *
 *  |  Field(s)        | Begin Byte | End Byte |
 *  | :--------------- | :--------: | :------: |
 *  | m_wMajorVersion  |  0         | 1        |
 *  | m_wMinorVersion  |  2         | 3        |
 *  | m_dwValidationKey|  4         | 7        |
 *
 */
struct _CONNECT_REQUEST
{
    WORD         m_wMajorVersion;    ///< Client Major Protocol version number
    WORD         m_wMinorVersion;    ///< Client Minor Protocol version number
    DWORD        m_dwValidationKey;  ///< Used by Server to authenticate the connection

    /// Default constructor
    constexpr _CONNECT_REQUEST() noexcept
        : m_wMajorVersion(0),
          m_wMinorVersion(0),
          m_dwValidationKey(0)
    { };

    /// Initialization constructor
    constexpr _CONNECT_REQUEST(WORD  wMajorVersion,
                               WORD  wMinorVersion, 
                               DWORD dwKey) noexcept
        : m_wMajorVersion  (wMajorVersion),
          m_wMinorVersion  (wMinorVersion),
          m_dwValidationKey(dwKey)
    { };
};


/**
 *  @brief Connection Response Primitive
 *
 *  |  Field(s)        | Begin Byte | End Byte |
 *  | :--------------- | :--------: | :------: |
 *  | m_dwResult       |  0         | 3        |
 *  | m_wMajorVersion  |  4         | 5        |
 *  | m_wMinorVersion  |  6         | 7        |
 *  | m_wClientID      |  8         | 9        |
 *
 *  @sa cnp::CER_TYPE
 */
struct _CONNECT_RESPONSE
{
     DWORD       m_dwResult;         ///< Success or Error code from cnp::CER_TYPE
     WORD        m_wMajorVersion;    ///< Server Major Protocol version number
     WORD        m_wMinorVersion;    ///< Server Minor Protocol version number
     WORD        m_wClientID;        ///< generated by the Server and is required in 
                                     ///< all subsequent request messages by the Client

//#ifndef _MSC_VER
 /**
  *  @brief Initialization Constructor
  * 
  *  @param [in] dwResult         Server generated cnp::CER_TYPE result
  *  @param [in] wMajorVersion    Current Server major protocol version number
  *  @param [in] wMinorVersion    Current Server minor protocol version number
  *  @param [in] wClientID        Server generated ClientID
  *
  */
    constexpr _CONNECT_RESPONSE(DWORD dwResult = cnp::CER_ERROR,        
                                WORD  wMajorVersion = 0,                
                                WORD  wMinorVersion = 0,                
                                WORD  wClientID = INVALID_CLIENT_ID) noexcept
        : m_dwResult(dwResult),
          m_wMajorVersion(wMajorVersion),
          m_wMinorVersion(wMinorVersion),
          m_wClientID(wClientID)
    { };
//#endif
};

/// Create Account Request Primitive
struct _CREATE_ACCOUNT_REQUEST
{
    char         m_szFirstName[MAX_NAME_LEN];   ///< User's First Name
    char         m_szLastName[MAX_NAME_LEN];    ///< User's Last Name
    char         m_szEmailAddress[MAX_NAME_LEN];///< User's Email Address
    WORD         m_wPIN;                        ///< User's Personal Identification Number
    DWORD        m_dwSSNumber;                  ///< (optional) User's Social Security Number
    DWORD        m_dwDLNumber;                  ///< (optional) User's Drivers License Number

    /// Default constructor
    constexpr _CREATE_ACCOUNT_REQUEST() noexcept
        : m_szFirstName{0},
          m_szLastName{0},
          m_szEmailAddress{0},
          m_wPIN(0),
          m_dwSSNumber(0),
          m_dwDLNumber(0)
    { };

    /// Initialization constructor
    _CREATE_ACCOUNT_REQUEST(const char* szFirstName, 
                            const char* szLastName,
                            const char* szEmailAddress,
                            WORD wPIN, DWORD dwSSNumber, DWORD dwDLNumber) noexcept
        : m_szFirstName{0},
          m_szLastName{0},
          m_szEmailAddress{0},
          m_wPIN(wPIN),
          m_dwSSNumber(dwSSNumber),
          m_dwDLNumber(dwDLNumber)
    { 
      set_FirstName(szFirstName);
      set_LastName(szLastName); 
      set_EmailAddress(szEmailAddress);
    };

/**
 *  @param [in] szSet  address containing null-terminated first name
 */
    void set_FirstName(const char* szSet) noexcept
    { if (szSet)
         strncpy(m_szFirstName, szSet, COUNTOF(m_szFirstName) - 1); };

/**
 *  @param [in] szSet  address containing null-terminated last name
 */
    void set_LastName(const char* szSet) noexcept
    { if (szSet)
         strncpy(m_szLastName, szSet, COUNTOF(m_szLastName) - 1); };

/**
 *  @param [in] szSet  address containing null-terminated email address
 */
    void set_EmailAddress(const char* szSet) noexcept
    { if (szSet)
          strncpy(m_szEmailAddress, szSet, COUNTOF(m_szEmailAddress) - 1); };
};

/**
 *  @brief Create Account Response Primitive
 *
 *  @sa cnp::CER_TYPE
 */
struct _CREATE_ACCOUNT_RESPONSE
{
    DWORD        m_dwResult;  ///< Success or Error code from cnp::CER_TYPE
 
// #ifndef _MSC_VER
    constexpr _CREATE_ACCOUNT_RESPONSE(DWORD dwResult = cnp::CER_ERROR) noexcept
        : m_dwResult(dwResult)
    { };
// #endif
};

/**
 *  @brief Logon Request Primitive
 */
struct _LOGON_REQUEST
{
    char         m_szFirstName[MAX_NAME_LEN]; ///< User's first name
    WORD         m_wPIN;                      ///< Personal Identification Number

    /// Default constructor
    constexpr _LOGON_REQUEST() noexcept
        : m_szFirstName{0},
          m_wPIN(INVALID_PIN)
    { };

    /// Initialization constructor
    _LOGON_REQUEST(const char* szFirstName, WORD wPIN) noexcept
        : m_szFirstName{0},
          m_wPIN(wPIN)
    { set_FirstName(szFirstName); };

/**
 *  @param [in] szSet   address containing null terminated first name
 */
    void set_FirstName(const char* szSet) noexcept
    { if (szSet)
         strncpy(m_szFirstName, szSet, COUNTOF(m_szFirstName) - 1); };
};


/**
 *  @brief Logon Response Primitive
 *
 *  @sa cnp::CER_TYPE
 */
struct _LOGON_RESPONSE
{
    DWORD        m_dwResult;   ///< Success or Error code from cnp::CER_TYPE
 
    constexpr _LOGON_RESPONSE(DWORD dwResult = cnp::CER_ERROR) noexcept
        : m_dwResult(dwResult)
    { };
};

/// Logoff Request Primitive
struct _LOGOFF_REQUEST
{
    constexpr _LOGOFF_REQUEST() noexcept
    { };
};

/**
 *  @brief Logoff Response Primitive
 * 
 *  @sa cnp::CER_TYPE
 */
struct _LOGOFF_RESPONSE
{
    DWORD        m_dwResult;   ///< Success or Error code from cnp::CER_TYPE

    constexpr _LOGOFF_RESPONSE(DWORD dwResult = cnp::CER_ERROR) noexcept
        : m_dwResult(dwResult)
    { };
};


/**
 *  @brief Deposit Request Primitive
 * 
 *  @sa cnp::DEPOSIT_TYPE
 */
struct _DEPOSIT_REQUEST
{
    DWORD        m_dwAmount;  ///< Amount excluding decimal point (i.e. $100.00 would be 10000)
    WORD         m_wType;     ///< cnp::DT_CASH or cnp::DT_CHECK

    /// Initialization constructor
    constexpr _DEPOSIT_REQUEST(DWORD dwAmount = 0, DEPOSIT_TYPE Type = DT_INVALID) noexcept
        : m_dwAmount(dwAmount),
          m_wType(static_cast<WORD>(Type))
    { };
};


/**
 *  @brief Deposit Response Primitive
 * 
 *  @sa cnp::CER_TYPE
 */
struct _DEPOSIT_RESPONSE
{
    DWORD        m_dwResult;    ///< Success or Error code from cnp::CER_TYPE

    constexpr _DEPOSIT_RESPONSE(DWORD dwResult = cnp::CER_ERROR) noexcept
        : m_dwResult(dwResult)
    { };
};

/**
 *  @brief Withdrawal Request Primitive
 */
struct _WITHDRAWAL_REQUEST
{
    /// Amount excluding decimal point (i.e. $100.00 would be 10000)
    DWORD        m_dwAmount;    

    /// Default constructor
    constexpr _WITHDRAWAL_REQUEST() noexcept
        : m_dwAmount(0)
    { };

    /// Initialization constructor
    constexpr _WITHDRAWAL_REQUEST(DWORD dwAmount) noexcept
        : m_dwAmount(dwAmount)
    { };
};


/**
 *  @brief Withdrawal Response Primitive
 *
 *  @sa cnp::CER_TYPE
 */
struct _WITHDRAWAL_RESPONSE
{
    DWORD        m_dwResult;    ///< Success or Error code from cnp::CER_TYPE

    constexpr _WITHDRAWAL_RESPONSE(DWORD dwResult = cnp::CER_ERROR) noexcept
        : m_dwResult(dwResult)
    { };
};

/**
 *  @brief Purchase Stamp Request Primitive
 */
struct _STAMP_PURCHASE_REQUEST
{
    /// Amount excluding decimal point (i.e. $100.00 would be 10000)
    DWORD        m_dwAmount;    

    /// Default constructor
    constexpr _STAMP_PURCHASE_REQUEST() noexcept
        : m_dwAmount(0)
    { };

    /// Initialization constructor
    constexpr _STAMP_PURCHASE_REQUEST(DWORD dwAmount) noexcept
        : m_dwAmount(dwAmount)
    { };
};


/**
 *  @brief Stamp Purchase Response Primitive
 * 
 *  @sa cnp::CER_TYPE
 */
struct _STAMP_PURCHASE_RESPONSE
{
    DWORD        m_dwResult;    ///< Success or Error code from cnp::CER_TYPE

    constexpr _STAMP_PURCHASE_RESPONSE(DWORD dwResult = cnp::CER_ERROR) noexcept
        : m_dwResult(dwResult)
    { };
};

/**
 *  @brief Transaction Query Request Primitive
 */
struct _TRANSACTION_QUERY_REQUEST
{
    DWORD        m_dwStartID;         ///< the transaction number to begin the query from
    WORD         m_wTransactionCount; ///< the number of transactions requested

    /// Default constructor
    constexpr _TRANSACTION_QUERY_REQUEST() noexcept
        : m_dwStartID(0),
          m_wTransactionCount(0)
    { };

    /// Initialization constructor
    constexpr _TRANSACTION_QUERY_REQUEST(DWORD dwStartID, WORD wTransactionCount) noexcept
        : m_dwStartID(dwStartID),
          m_wTransactionCount(wTransactionCount)
    { };
};

// disable warning : zero-sized array in struct
#ifdef _MSC_VER
    #pragma warning( disable : 4200 )
#endif

/**
  *  @brief Transcation Query Result Primitive
  *  @sa cnp::CER_TYPE
  *  @sa cnp::TRANSACTION
  */
struct _TRANSACTION_QUERY_RESPONSE
{
    DWORD        m_dwResult;          ///< Success or Error code from cnp::CER_TYPE
    WORD         m_wTransactionCount; ///< number of transactions returned in array
    TRANSACTION  m_rgTransactions[];  ///< unsized array of Transaction records

    constexpr _TRANSACTION_QUERY_RESPONSE(DWORD dwResult = cnp::CER_ERROR, WORD wTransactionCount = 0) noexcept
        : m_dwResult(dwResult),
          m_wTransactionCount(wTransactionCount)
    { };

    inline WORD get_TransactionCount(void) const noexcept
    { return m_wTransactionCount; };
};

#ifdef _MSC_VER
struct _TRANSACTION_QUERY_RESPONSE_10
{
    DWORD        m_dwResult;            ///< Success or Error code from cnp::CER_TYPE
    WORD         m_wTransactionCount;   ///< number of transactions returned in array
    TRANSACTION  m_rgTransactions[10];  ///< array of Transaction records

    constexpr _TRANSACTION_QUERY_RESPONSE_10(DWORD dwResult, WORD wTransactionCount) noexcept
        : m_dwResult(dwResult),
          m_wTransactionCount(wTransactionCount)
    { };

    inline WORD get_TransactionCount(void) const noexcept
    { return m_wTransactionCount; };
};
#endif

/**
 *  @brief Balance Query Request Primitive
 */
struct _BALANCE_QUERY_REQUEST
{
    constexpr _BALANCE_QUERY_REQUEST() noexcept
    { };
};


/**
 *  @brief Balance Query Response Primitive
 * 
 *  @sa cnp::CER_TYPE
 */
struct _BALANCE_QUERY_RESPONSE
{
    DWORD        m_dwResult;    ///< Success or Error code from cnp::CER_TYPE
    DWORD        m_dwBalance;   ///< Current Client account balance

    constexpr _BALANCE_QUERY_RESPONSE(DWORD dwResult = cnp::CER_ERROR, DWORD dwBalance = 0) noexcept
        : m_dwResult(dwResult), 
          m_dwBalance(dwBalance)
    { };

    inline DWORD get_Balance(void) const noexcept
    { return m_dwBalance; };
};

}  // namespace prim

/**
 *  @brief CNP Standard Message Header
 *  
 *  This is the header message that is provided as part of all CNP Request and 
 *  Response messages.  The m_wDataLen field is subsequently calculated as:
 *
 *       sizeof(m_Request) or sizeof(m_Response)
 *
 *  to get the size of message excluding the size of the header.
 *
 *  The m_dwContext field is exclusively reserved for application use.  The contents 
 *  of this field is returned to the user in the corresponding result message 
 *  structure without modification.
 *
 *  The m_wClientID value is return by the server in the CONNECTION_RESPONSE message
 *  and is required in all subsequent messages sent by the client to the server.
 *
 *  @sa CONNECT_RESPONSE
 *
 *  |  Field(s)      | Begin Byte | End Byte |
 *  | :------------- | :--------: | :------: |
 *  | m_dwMsgType    |  0         | 3        |
 *  | m_wDataLen     |  4         | 5        |
 *  | m_wClientID    |  6         | 7        |
 *  | m_dwSequence   |  8         | 11       |
 *  | m_dwContext    | 12         | 15       |
 *
 *  @ingroup TypeDefs
 */
struct STD_HDR
{
    DWORD   m_dwMsgType;  ///< Message Type
    WORD    m_wDataLen;   ///< Message data length excluding this header
    WORD    m_wClientID;  ///< Client ID, initially set by the Server & 
                          ///< used by Client in subsequent messages
    DWORD   m_dwSequence; ///< Incremented by the Client, used to match 
                          ///< Server responses to Client requests
    DWORD   m_dwContext;  ///< [Optional] field, reserved for the Client's use

    /// Default constructor
    constexpr STD_HDR() noexcept
        : m_dwMsgType (MT_INVALID),
          m_wDataLen  (0),
          m_wClientID (INVALID_CLIENT_ID),
          m_dwSequence(0),
          m_dwContext (0)
    { };

/**
 *  @brief Initialization constructor
 *
 *  @param [in] dwMsgType    MSG_TYPE
 *  @param [in] wDataLen     Message data length excluding this header
 *  @param [in] wClientID    Client ID, initially set by the Server & 
 *                           used by Client in subsequent messages
 *  @param [in] dwSequence   Incremented by the Client, used to match 
 *                           Server responses to Client requests
 *  @param [in] dwContext    [Optional] field for Client's use
 */
    constexpr STD_HDR(DWORD dwMsgType, 
            WORD  wDataLen, 
            WORD  wClientID  = INVALID_CLIENT_ID, 
            DWORD dwSequence = 0,
            DWORD dwContext  = 0) noexcept
        : m_dwMsgType (dwMsgType),
          m_wDataLen  (wDataLen),
          m_wClientID (wClientID),
          m_dwSequence(dwSequence),
          m_dwContext (dwContext)
    { };

    /// Copy Constructor
    constexpr STD_HDR(const STD_HDR& rhs) noexcept
        : m_dwMsgType (rhs.m_dwMsgType),
          m_wDataLen  (rhs.m_wDataLen),
          m_wClientID (rhs.m_wClientID),
          m_dwSequence(rhs.m_dwSequence),
          m_dwContext (rhs.m_dwContext)
    { };

    inline DWORD get_MsgType(void) const noexcept
    { return m_dwMsgType; };

    inline WORD  get_ClientID(void) const noexcept
    { return m_wClientID; };

    inline DWORD get_Sequence(void) const noexcept
    { return m_dwSequence; };

    inline DWORD get_Context(void) const noexcept
    { return m_dwContext; };
};

/**
 *  @brief [Client] Connect Request message
 * 
 *  The connect request message establishes an authenticated connection 
 *  with the Server. No Client ID is required for this message, but is
 *  provided by Server in CONNECT_RESPONSE message.
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Request       | m_dwValidationKey | 16         | 19       |
 *  |  m_Request       | m_wMajorVersion   | 20         | 21       |
 *  |  m_Request       | m_wMinorVersion   | 22         | 23       |
 *
 *  @ingroup CltMsgs
 */
struct CONNECT_REQUEST
{
    STD_HDR                    m_Hdr;
    prim::_CONNECT_REQUEST     m_Request;

/**
 *  @brief Initialization Constructor
 *
 *  @param [in] wClientID       [Optional] This value is ignored by server on 
 *                              initial connect
 *  @param [in] wMajorVersion   [Optional] Defaulted to g_wMajorVersion
 *  @param [in] wMinorVersion   [Optional] Defaulted to g_wMinorVersion
 *  @param [in] dwValidationKey [Optional] Default to g_dwValidationKey
 *  @param [in] dwContext       [Optional] field for Client's use
 *
 *  @note auto increments the Client's global sequence number
 */
    CONNECT_REQUEST(WORD  wClientID       = 0,
                    WORD  wMajorVersion   = g_wMajorVersion, 
                    WORD  wMinorVersion   = g_wMinorVersion, 
                    DWORD dwValidationKey = g_dwValidationKey,
                    DWORD dwContext       = 0) noexcept
        : m_Hdr( MT_CONNECT_REQUEST, 
                 sizeof(m_Request), 
                 wClientID, 
                 g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                 dwContext ),
          m_Request(wMajorVersion, wMinorVersion, dwValidationKey)
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t get_Size(void) const noexcept
    { return sizeof(*this); };

// ===========================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD      get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline WORD       get_ClientMajorVersion(void) const noexcept
    { return m_Request.m_wMajorVersion; };

    inline WORD       get_ClientMinorVersion(void) const noexcept
    { return m_Request.m_wMinorVersion; };

    inline DWORD      get_ClientValidationKey(void) const noexcept
    { return m_Request.m_dwValidationKey; };
 
};


/**
 *  @brief [Server] Connect Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *  |  m_Response      | m_wMajorVersion   | 20         | 21       |
 *  |  m_Response      | m_wMinorVersion   | 22         | 23       |
 *  |  m_Response      | m_wClientID       | 24         | 25       |
 *
 *  @sa cnp::CONNECT_REQUEST
 *  @ingroup SvrMsgs
 */
struct CONNECT_RESPONSE
{
    STD_HDR                     m_Hdr;
    prim::_CONNECT_RESPONSE     m_Response;

/// Default Constructor
    constexpr CONNECT_RESPONSE() noexcept
        : m_Hdr(),
          m_Response()
    { };

/**
 *  @brief Initialization Constructor
 * 
 *  @param [in] dwResult          Server generated cnp::CER_TYPE result
 *  @param [in] wClientID         Provided by Server on a successful connection response
 *  @param [in] wMajorVersion     current Server major version 
 *  @param [in] wMinorVersion     current Server minor version
 *  @param [in] dwSequence        copied from CONNECT_REQUEST
 *  @param [in] dwContext         copied from CONNECT_REQUEST
 */
    constexpr CONNECT_RESPONSE(DWORD dwResult,
                     WORD  wClientID,
                     WORD  wMajorVersion,
                     WORD  wMinorVersion,
                     DWORD dwSequence, 
                     DWORD dwContext) noexcept
        : m_Hdr(MT_CONNECT_RESPONSE, 
                sizeof(m_Response), 
                wClientID, 
                dwSequence, 
                dwContext)
    { m_Response = { dwResult, wMajorVersion, wMinorVersion, wClientID}; };

    inline DWORD    get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD     get_ClientID(void) const noexcept
    { return m_Response.m_wClientID; };

    inline DWORD    get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t    get_Size(void) const noexcept
    { return sizeof(*this); };
};


/**
 *  @brief [Client] Create Account Request message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Request       | m_szFirsName      | 16         | 47       |
 *  |  m_Request       | m_szLastName      | 48         | 79       |
 *  |  m_Request       | m_szEmailAddress  | 80         | 111      |
 *  |  m_Request       | m_wPIN            | 112        | 113      |
 *  |  m_Request       | m_dwSSNumber      | 114        | 117      |
 *  |  m_Request       | m_dwDLNumber      | 118        | 121      |
 *
 *  @ingroup CltMsgs
 */
struct CREATE_ACCOUNT_REQUEST
{
    STD_HDR                        m_Hdr;
    prim::_CREATE_ACCOUNT_REQUEST  m_Request;

/// Default constructor
    constexpr CREATE_ACCOUNT_REQUEST() noexcept
        : m_Hdr(),
          m_Request()
    { };

/** 
 *  @brief Initialization constructor
 *
 *  @param [in] wClientID       Provided by Server on a successful connection 
 *                              response
 *  @param [in] szFirstName     Client's first name (used in LOGON messages)
 *  @param [in] szLastName      Client's last name
 *  @param [in] szEmailAddress  Client's email address
 *  @param [in] wPIN            Client Generated PIN (used in LOGON messages)
 *  @param [in] dwSSN           [Optional] Social Security Number field 
 *  @param [in] dwDLN           [Optional] Driver's License Number field
 *  @param [in] dwClientContext [Optional] field for the Client's use
 *
 *  @note auto increments the Client's global sequence number
 */
    CREATE_ACCOUNT_REQUEST(WORD wClientID, 
                           const char* szFirstName,   
                           const char* szLastName,    
                           const char* szEmailAddress,
                           WORD  wPIN,                
                           DWORD dwSSN = 0,           
                           DWORD dwDLN = 0,           
                           DWORD dwClientContext = 0) noexcept
        : m_Hdr(MT_CREATE_ACCOUNT_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwClientContext),
          m_Request(szFirstName, szLastName, szEmailAddress, wPIN, dwSSN, dwDLN)
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t get_Size(void) const noexcept
    { return sizeof(*this); };

// ===========================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD       get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline const char* get_FirstName(void) const noexcept
    { return m_Request.m_szFirstName; };

    inline const char* get_LastName(void) const noexcept
    { return m_Request.m_szLastName; };

    inline const char* get_EmailAddress(void) const noexcept
    { return m_Request.m_szEmailAddress; };
    
    inline WORD        get_PIN(void) const noexcept
    { return m_Request.m_wPIN; };

    inline DWORD       get_SSNumber(void) const noexcept
    { return m_Request.m_dwSSNumber; };

    inline DWORD       get_DLNumber(void) const noexcept
    { return m_Request.m_dwDLNumber; };
};


/**
 * @brief [Server] Create Account Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *
 *  @sa cnp::CREATE_ACCOUNT_REQUEST
 *  @ingroup SvrMsgs
 */
struct CREATE_ACCOUNT_RESPONSE
{
    STD_HDR                          m_Hdr;
    prim::_CREATE_ACCOUNT_RESPONSE   m_Response;

/// Default constructor
    constexpr CREATE_ACCOUNT_RESPONSE() noexcept
        :m_Hdr(),
         m_Response()
    { };

/**
 *  @brief Initialization Constructor
 *
 *  @param [in] dwResult     Server generated cnp::CER_TYPE result
 *  @param [in] wClientID    Copied from CREATE_ACCOUNT_REQUEST
 *  @param [in] dwSequence   Copied from CREATE_ACCOUNT_REQUEST
 *  @param [in] dwContext    Copied from CREATE_ACCOUNT_REQUEST
 */
    constexpr CREATE_ACCOUNT_RESPONSE (DWORD dwResult,
                             WORD  wClientID,
                             DWORD dwSequence,
                             DWORD dwContext)  noexcept
        : m_Hdr(MT_CREATE_ACCOUNT_RESPONSE, 
                sizeof(m_Response), 
                wClientID, 
                dwSequence, 
                dwContext)
    { m_Response = { dwResult}; };

    inline DWORD    get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD    get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t   get_Size(void) const noexcept
    { return sizeof(*this); };
};

/** 
 *  @brief [Client] Logon Request message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Request       | m_szFirstName     | 16         | 47       |
 *  |  m_Request       | m_wPIN            | 48         | 49       |
 *
 *  @ingroup CltMsgs
 */
struct LOGON_REQUEST
{
    STD_HDR                 m_Hdr;
    prim::_LOGON_REQUEST    m_Request;

/// Default constructor
    constexpr LOGON_REQUEST() noexcept
        : m_Hdr(),
          m_Request()
    { };


/**
 *  @brief  Initialization constructor
 *
 *  @note auto increments the Client's global sequence number
 */
    LOGON_REQUEST(WORD wClientID,         ///< Server provided Client ID 
                  const char* szFirstName,///< Client field used in CREATE_ACCOUNT_REQUEST 
                  WORD  wPIN,             ///< Client field used in CREATE_ACCOUNT_REQUEST
                  DWORD dwContext = 0) noexcept   ///< [Optional] field provided for Client's use
        : m_Hdr(MT_LOGON_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwContext ),
          m_Request(szFirstName, wPIN)
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t get_Size(void) const noexcept
    { return sizeof(*this); };

// ===========================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD      get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline const char* get_FirstName(void) const noexcept
    { return m_Request.m_szFirstName; };
    
    inline WORD        get_PIN(void) const noexcept
    { return m_Request.m_wPIN; };

};


/**
 *  @brief [Server] Logon Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *
 *  @sa cnp::LOGON_REQUEST
 *  @ingroup SvrMsgs
 */
struct LOGON_RESPONSE
{
    STD_HDR                    m_Hdr;
    prim::_LOGON_RESPONSE      m_Response;

/// Default Constructor
    constexpr LOGON_RESPONSE() noexcept
        : m_Hdr(),
          m_Response()
    { };

/// Initialization Constructor    
    constexpr LOGON_RESPONSE(DWORD dwResult,   ///< Server generated cnp::CER_TYPE result
                   WORD  wClientID,  ///< Copied from LOGON_REQUEST
                   DWORD dwSequence, ///< Copied from LOGON_REQUEST
                   DWORD dwContext)  noexcept  ///< Copied from LOGON_REQUEST
       : m_Hdr(MT_LOGON_RESPONSE, 
               sizeof(m_Response), 
               wClientID, 
               dwSequence, 
               dwContext)
    { m_Response = { dwResult }; };

    inline DWORD  get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD  get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t get_Size(void) const noexcept
    { return sizeof(*this); };

};

/**
 *  @brief [Client] Logoff Request message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *
 *  @ingroup CltMsgs
 */

struct LOGOFF_REQUEST
{
    STD_HDR                    m_Hdr;
    prim::_LOGOFF_REQUEST      m_Request;

/// Default Constructor    
    constexpr LOGOFF_REQUEST() noexcept
        : m_Hdr(),
          m_Request()
    { };

/**
 *  @brief  Initialization constructor
 *
 *  @note auto increments the Client's global sequence number
 */
    LOGOFF_REQUEST(WORD  wClientID,      ///< Server generated Client ID
                   DWORD dwContext = 0) noexcept ///< [Optional] Client provided field
        : m_Hdr(MT_LOGOFF_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwContext),
          m_Request()
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t    get_Size(void) const noexcept
    { return sizeof(*this); };

// ===========================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD      get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

};


/**
 *  @brief [Server] Logoff Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *
 *  @sa cnp::LOGOFF_REQUEST
 *  @ingroup SvrMsgs
 */
struct LOGOFF_RESPONSE
{
    STD_HDR                    m_Hdr;
    prim::_LOGOFF_RESPONSE     m_Response;

/// Default Constructor
    constexpr LOGOFF_RESPONSE() noexcept
        :m_Hdr(),
         m_Response()
    { };

/// Initialization Constructor
    constexpr LOGOFF_RESPONSE(DWORD dwResult,    ///< Server generated cnp::CER_TYPE result
                    WORD  wClientID,   ///< Copied from LOGOFF_REQUEST
                    DWORD dwSequence,  ///< Copied from LOGOFF_REQUEST
                    DWORD dwContext)   noexcept  ///< Copied from LOGOFF_REQUEST
       : m_Hdr(MT_LOGOFF_RESPONSE, 
               sizeof(m_Response), 
               wClientID, 
               dwSequence, 
               dwContext)
    { m_Response = { dwResult }; };

    inline DWORD get_MsgType(void) const noexcept
    { return m_Hdr.m_dwMsgType; };

    inline DWORD get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t get_Size(void) const noexcept
    { return sizeof(*this); };
};

/**
 *  @brief [Client] Deposit Request message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Request       | m_dwAmount        | 16         | 19       |
 *  |  m_Request       | m_wType           | 20         | 21       |
 *
 *  @ingroup CltMsgs
 */
struct DEPOSIT_REQUEST
{
    STD_HDR                    m_Hdr;
    prim::_DEPOSIT_REQUEST     m_Request;

/**
 *  @brief  Initialization constructor
 *
 *  @note auto increments the Client's global sequence number
 */
    DEPOSIT_REQUEST(WORD  wClientID,      ///< Server generated Client ID
                    DWORD dwAmount,       ///< Amount to deposit (in cents)
                    DEPOSIT_TYPE Type,    ///< cnp::DT_CASH or cnp::DT_CHECK
                    DWORD dwContext = 0)  noexcept ///< [Optional] Client provided field
        : m_Hdr(MT_DEPOSIT_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++ , // <-- cannot use constexpr here because of this guy
                dwContext),
          m_Request(dwAmount, Type)
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t    get_Size(void) const noexcept
    { return sizeof(m_Hdr) + sizeof(m_Request); };

// ===================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD       get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline DWORD       get_Amount(void) const noexcept
    { return m_Request.m_dwAmount; };
    
    inline WORD        get_DepositType(void) const noexcept
    { return m_Request.m_wType; };
};


/**
 *  @brief [Server] Deposit Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *
 *  @sa cnp::DEPOSIT_REQUEST
 *  @ingroup SvrMsgs
 */
struct DEPOSIT_RESPONSE
{
    STD_HDR                    m_Hdr;
    prim::_DEPOSIT_RESPONSE    m_Response;

/// Default Constructor
    constexpr DEPOSIT_RESPONSE() noexcept
        : m_Hdr(),
          m_Response()
    { };

/// Initialization Constructor
    DEPOSIT_RESPONSE(DWORD dwResult,   ///< Server generated cnp::CER_TYPE result
                     WORD  wClientID,  ///< Copied from DEPOSIT_REQUEST
                     DWORD dwSequence, ///< Copied from DEPOSIT_REQUEST
                     DWORD dwContext)  noexcept ///< Copied from DEPOSIT_REQUEST
        : m_Hdr(MT_DEPOSIT_RESPONSE, 
                sizeof(m_Response), 
                wClientID, 
                dwSequence, 
                dwContext)
    { m_Response = { dwResult }; }; // this syntax required on MSVC++ due
                                    // to using prim::_DEPOSIT_RESPONSE in
                                    // a union

    inline DWORD get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

    size_t get_Size(void) const noexcept
    { return sizeof(*this); };
};


/**
 * @brief [Client] Withdrawal Request message
 * 
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Request       | m_dwAmount        | 16         | 19       |
 * 
 * @ingroup CltMsgs
 *
 */
 struct WITHDRAWAL_REQUEST
 {
    STD_HDR                    m_Hdr;
    prim::_WITHDRAWAL_REQUEST  m_Request;

/**
 *  @brief  Initialization constructor
 *
 *  @note auto increments the Client's global sequence number
 */
    WITHDRAWAL_REQUEST(WORD  wClientID,     ///< Server generated Client ID
                       DWORD dwAmount,      ///< Amount Client wants to withdraw (in cents)
                       DWORD dwContext = 0) noexcept ///< [Optional] Client provided field
        : m_Hdr(MT_WITHDRAWAL_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwContext),
          m_Request(dwAmount)
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t   get_Size(void) const noexcept
    { return sizeof(m_Hdr) + sizeof(m_Request); };

// ============================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD       get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline DWORD       get_Amount(void) const noexcept
    { return m_Request.m_dwAmount; };

};


/**
 *  @brief [Server] Withdrawal Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *
 *  @sa cnp::WITHDRAWAL_REQUEST
 *  @ingroup SvrMsgs
 */
struct WITHDRAWAL_RESPONSE
{
    STD_HDR                      m_Hdr;
    prim::_WITHDRAWAL_RESPONSE   m_Response;

    constexpr WITHDRAWAL_RESPONSE(DWORD dwResult,    ///< Server generated cnp::CER_TYPE result
                        WORD  wClientID,   ///< Copied from WITHDRAWAL_REQUEST
                        DWORD dwSequence,  ///< Copied from WITHDRAWAL_REQUEST
                        DWORD dwContext) noexcept  ///< Copied from WITHDRAWAL_REQUEST
      : m_Hdr(MT_WITHDRAWAL_RESPONSE, 
              sizeof(m_Response), 
              wClientID, 
              dwSequence, 
              dwContext)
    { m_Response = { dwResult }; };

    inline DWORD    get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD    get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

    size_t   get_Size(void) const noexcept
    { return sizeof(*this); };
};

/**
 *  @brief [Client] Balance Query Request message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *
 *  @ingroup CltMsgs
 */

struct BALANCE_QUERY_REQUEST
{
    STD_HDR                        m_Hdr;
    prim::_BALANCE_QUERY_REQUEST   m_Request;

/// Default constructor
    constexpr BALANCE_QUERY_REQUEST() noexcept
        : m_Hdr(),
          m_Request()
    { };

/**
 *  @brief  Initialization constructor
 *
 *  @note auto increments the Client's global sequence number
 */
    BALANCE_QUERY_REQUEST(WORD  wClientID,      ///< Server generated Client ID
                          DWORD dwContext = 0) noexcept ///< [Optional] Client provided field
        : m_Hdr(MT_BALANCE_QUERY_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwContext),
          m_Request()
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t   get_Size(void) const noexcept
    { return sizeof(*this); };

// ============================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD      get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

};


/**
 *  @brief [Server] Balance Query Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *  |  m_Response      | m_dwBalance       | 20         | 23       |
 *
 *  @sa cnp::BALANCE_QUERY_REQUEST
 *  @ingroup SvrMsgs
 */
struct BALANCE_QUERY_RESPONSE
{
    STD_HDR                          m_Hdr;
    prim::_BALANCE_QUERY_RESPONSE    m_Response;

/// Initialization Constructor
    constexpr BALANCE_QUERY_RESPONSE(DWORD dwResult,   ///< Server generated cnp::CER_TYPE result
                           WORD  wClientID,  ///< Copied from BALANCE_QUERY_REQUEST
                           DWORD dwBalance,  ///< Client's current account balance
                           DWORD dwContext,  ///< Copied from BALANCE_QUERY_REQUEST
                           DWORD dwSequence) noexcept ///< Copied from BALANCE_QUERY_REQUEST
        : m_Hdr(MT_BALANCE_QUERY_RESPONSE, 
                sizeof(m_Response), 
                wClientID, 
                dwContext, 
                dwSequence),
         m_Response( dwResult, dwBalance)
    {  };

    inline DWORD     get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD     get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

    inline DWORD     get_Balance(void) const noexcept
    { return m_Response.get_Balance(); };

    size_t    get_Size(void) const noexcept
    { return sizeof(*this); };
};

/**
  @brief [Client] Transaction Query Request message

   |  Message Members |     Field           | Begin Byte | End Byte |
   | :--------------- | :------------------ | :--------: | :------: |
   |  m_Hdr           | m_dwMsgType         |  0         | 3        |
   |  m_Hdr           | m_wDataLen          |  4         | 5        |
   |  m_Hdr           | m_wClientID         |  6         | 7        |
   |  m_Hdr           | m_dwSequence        |  8         | 11       |
   |  m_Hdr           | m_dwContext         | 12         | 15       |
   |  m_Request       | m_dwStartID         | 16         | 19       |
   |  m_Request       | m_wTransactionCount | 20         | 21       |

   @ingroup CltMsgs
 */
struct TRANSACTION_QUERY_REQUEST
{
    STD_HDR                           m_Hdr;
    prim::_TRANSACTION_QUERY_REQUEST  m_Request;

/** 
 *  @brief Initialization constructor
 *
 *  @note auto increments the Client's global sequence number
 */
    TRANSACTION_QUERY_REQUEST(WORD  wClientID,         ///< Server generated Client ID
                              DWORD dwStartID,         ///< Transaction Record ID to begin query from
                              WORD  wTransactionCount, ///< Number of Records requested
                              DWORD dwContext = 0)     noexcept ///< [Optional] Client provided field
        : m_Hdr(MT_TRANSACTION_QUERY_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwContext),
          m_Request(dwStartID, wTransactionCount)
    { };

/**
 *  @retval size_t containing the size of the message in bytes
 */
    size_t get_Size(void) const noexcept
    { return sizeof(*this); };

// ============================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD      get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline DWORD      get_StartID(void) const noexcept
    { return m_Request.m_dwStartID; };

    inline WORD       get_TransactionCount(void) const noexcept
    { return m_Request.m_wTransactionCount; };
};

/**
 *  @brief [Server] Transaction Query Response message
 *
 *  |  Message Members |     Field           | Begin Byte | End Byte |
 *  | :--------------- | :------------------ | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType         |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen          |  4         | 5        |
 *  |  m_Hdr           | m_wClientID         |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence        |  8         | 11       |
 *  |  m_Hdr           | m_dwContext         | 12         | 15       |
 *  |  m_Response      | m_dwResult          | 16         | 19       |
 *  |  m_Response      | m_wTransactionCount | 20         | 21       |
 *  |  m_Response      | m_rgTransactions[]  | 22         | ...      |
 *
 *  @sa cnp::TRANSACTION_QUERY_REQUEST
 *  @sa cnp::TRANSACTION
 *  @ingroup SvrMsgs
 */
struct TRANSACTION_QUERY_RESPONSE
{
    STD_HDR                               m_Hdr;
    prim::_TRANSACTION_QUERY_RESPONSE     m_Response;

/// Initialization Constructor
    TRANSACTION_QUERY_RESPONSE(DWORD dwResult,          ///< Server generated cnp::CER_TYPE result
                               WORD  wClientID,         ///< Copied from TRANSACTION_QUERY_REQUEST
                               WORD  wTransactionCount, ///< Actual number of records returned
                               DWORD dwSequence,        ///< Copied from TRANSACTION_QUERY_REQUEST
                               DWORD dwContext) noexcept        ///< Copied from TRANSACTION_QUERY_REQUEST
      : m_Hdr(MT_TRANSACTION_QUERY_RESPONSE, 
              sizeof(m_Response) + wTransactionCount * sizeof(TRANSACTION), 
              wClientID, 
              dwSequence, 
              dwContext)
    { m_Response.m_dwResult = dwResult;
      m_Response.m_wTransactionCount = wTransactionCount; };

    size_t get_Size(void) const noexcept
    { return sizeof(*this) + get_TransactionCount() * sizeof(TRANSACTION); };

    inline DWORD  get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD  get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

    inline WORD   get_TransactionCount(void) const noexcept
    { return m_Response.m_wTransactionCount; };
};

#ifdef _MSC_VER
/**
 *  @brief [Server] Transaction Query Response message
 *
 *  Fixed length Transaction Query Response message
 */
struct TRANSACTION_QUERY_RESPONSE_10
{
    STD_HDR                                  m_Hdr;
    prim::_TRANSACTION_QUERY_RESPONSE_10     m_Response;

/// Initialization Constructor
    TRANSACTION_QUERY_RESPONSE_10(DWORD dwResult,          ///< Server generated cnp::CER_TYPE result
                                  WORD  wClientID,         ///< Copied from TRANSACTION_QUERY_REQUEST
                                  WORD  wTransactionCount, ///< Actual number of records returned
                                  DWORD dwSequence,        ///< Copied from TRANSACTION_QUERY_REQUEST
                                  DWORD dwContext) noexcept        ///< Copied from TRANSACTION_QUERY_REQUEST
      : m_Hdr(MT_TRANSACTION_QUERY_RESPONSE, 
              sizeof(m_Response) + wTransactionCount * sizeof(TRANSACTION), 
              wClientID, 
              dwSequence, 
              dwContext),
        m_Response(dwResult, wTransactionCount)
    { };

    size_t get_Size(void) const noexcept
    { return sizeof(*this) + get_TransactionCount() * sizeof(TRANSACTION); };

    inline DWORD  get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline DWORD  get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

    inline WORD   get_TransactionCount(void) const noexcept
    { return m_Response.m_wTransactionCount; };
};
#endif

/**
 *  @brief [Client] Stamp Purchase Request Message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Request       | m_dwAmount        | 16         | 19       |
 *
 *  @ingroup CltMsgs
 */
struct STAMP_PURCHASE_REQUEST
{
    STD_HDR                       m_Hdr;
    prim::_STAMP_PURCHASE_REQUEST m_Request;

/**
 *  @brief Initialization constructor
 *
 *  @param [in] wClientID   Server generated Client ID
 *  @param [in] dwAmount    Cost of stamps attempting to purchase 
 *                          (in cents) (i.e. 1000 = $10.00)
 *  @param [in] dwContext   [Optional] field provided by the Client
 *
 *  @note auto increments the Client's global sequence number
 */
    STAMP_PURCHASE_REQUEST(WORD  wClientID,
                           DWORD dwAmount,
                           DWORD dwContext = 0) noexcept
        : m_Hdr(MT_PURCHASE_STAMPS_REQUEST, 
                sizeof(m_Request), 
                wClientID, 
                g_dwSequenceNumber++, // <-- cannot use constexpr here because of this guy
                dwContext),
          m_Request(dwAmount)
    { };

    size_t   get_Size(void) const noexcept
    { return sizeof(*this); };

// ============================================================================
// Server Decoding Helper Methods

    inline DWORD      get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };

    inline WORD       get_ClientID(void) const noexcept
    { return m_Hdr.get_ClientID(); };

    inline DWORD      get_Sequence(void) const noexcept
    { return m_Hdr.get_Sequence(); };

    inline DWORD      get_Context(void) const noexcept
    { return m_Hdr.get_Context(); };

    inline DWORD      get_Amount(void) const noexcept
    { return m_Request.m_dwAmount; };

};


/**
 *  @brief [Server] Stamp Purchase Response message
 *
 *  |  Message Members |     Field         | Begin Byte | End Byte |
 *  | :--------------- | :---------------- | :--------: | :------: |
 *  |  m_Hdr           | m_dwMsgType       |  0         | 3        |
 *  |  m_Hdr           | m_wDataLen        |  4         | 5        |
 *  |  m_Hdr           | m_wClientID       |  6         | 7        |
 *  |  m_Hdr           | m_dwSequence      |  8         | 11       |
 *  |  m_Hdr           | m_dwContext       | 12         | 15       |
 *  |  m_Response      | m_dwResult        | 16         | 19       |
 *
 *  @sa cnp::STAMP_PURCHASE_REQUEST
 *  @ingroup SvrMsgs
 */
struct STAMP_PURCHASE_RESPONSE
{
    STD_HDR                         m_Hdr;
    prim::_STAMP_PURCHASE_RESPONSE  m_Response;

/** 
 *  @brief Initialization Constructor
 *
 *  @param [in] dwResult      Server generated cnp::CER_TYPE result
 *  @param [in] wClientID     Copied from STAMP_PURCHASE_REQUEST
 *  @param [in] dwSequence    Copied from STAMP_PURCHASE_REQUEST
 *  @param [in] dwContext     Copied from STAMP_PURCHASE_REQUEST
 */
    constexpr STAMP_PURCHASE_RESPONSE(DWORD dwResult,
        WORD  wClientID,
        DWORD dwSequence,
        DWORD dwContext) noexcept
        : m_Hdr(MT_PURCHASE_STAMPS_RESPONSE,
            sizeof(m_Response),
            wClientID,
            dwSequence,
            dwContext),
         m_Response(dwResult)
    { };

    inline DWORD  get_MsgType(void) const noexcept
    { return m_Hdr.get_MsgType(); };
    
    inline DWORD  get_ResponseResult(void) const noexcept
    { return m_Response.m_dwResult; };

    size_t get_Size(void) const noexcept
    { return sizeof(*this); };
};

} // namespace cnp

// restore the default structure alignment
#pragma pack(pop)

#endif
