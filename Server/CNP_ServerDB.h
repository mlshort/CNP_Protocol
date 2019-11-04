/**
 * @file   CNP_ServerDB.h
 * @brief  ACCOUNT_INFO & TRANSACTION_INFO struct definitions
 *
 * @author Mark L. Short
 *
 * @date   April 10, 2015  original date
 * @date   April 25, 2015  comments added
 * 
 */

#if !defined(__CNP_SERVER_DB_H__)
#define __CNP_SERVER_DB_H__

#ifndef __CNP_COMMON_H__
    #include "CNP_Common.h"
#endif

#ifndef _MAP_
    #include <map>
#endif

/**
    ACCOUNT_INFO is used to maintain and persist 
    information as it relates to an individual
    customer.  It uses the Customer ID as the key
    field.
*/
struct ACCOUNT_INFO : 
    public cnp::prim::_CREATE_ACCOUNT_REQUEST
{
    typedef cnp::QWORD key_type;
    typedef cnp::prim::_CREATE_ACCOUNT_REQUEST _Base;

    cnp::QWORD  m_qwCustomerID;
    cnp::DWORD  m_dwBalance;

    /// Default Constructor
    ACCOUNT_INFO()
        : _Base(),
          m_qwCustomerID(INVALID_CUSTOMER_ID),
          m_dwBalance   (INVALID_BALANCE)
    { };

    /// Initialization Constructor
    ACCOUNT_INFO(const _Base& base,
                 const cnp::QWORD& qwID = INVALID_CUSTOMER_ID,
                 cnp::DWORD dwBalance = INVALID_BALANCE)
        : _Base(base),
          m_qwCustomerID(qwID),
          m_dwBalance   (dwBalance)
    { };

    /// Copy Constructor
    ACCOUNT_INFO(const ACCOUNT_INFO& rhs)
        : _Base(rhs),
          m_qwCustomerID(rhs.m_qwCustomerID),
          m_dwBalance   (rhs.m_dwBalance)
    { };

  /**
    This method is used to provide a generic interface to retrieve
    a record's primary key field.  In this instance, it is a thin
    wrapper around get_CustomerID().

    @retval cnp::QWORD  containing the primary key value
  */
    inline const cnp::QWORD&  get_PrimaryKey(void) const noexcept
    { return get_CustomerID(); };

    inline const cnp::QWORD&  get_CustomerID(void) const noexcept
    { return m_qwCustomerID; };

    inline cnp::DWORD  get_Balance(void) const noexcept
    { return m_dwBalance; };

    inline void        set_Balance(cnp::DWORD dwSet) noexcept
    { m_dwBalance = dwSet; };

    inline void        decr_Balance(cnp::DWORD dwSet) noexcept
    { m_dwBalance -= dwSet; };

    inline void        incr_Balance(cnp::DWORD dwSet) noexcept
    { m_dwBalance += dwSet; };
};

/**
   TRANSACTION_INFO is used to maintain a listing of all transactions
   related to a specific customer. The Transaction ID is used as the 
   primary key.
 */
struct TRANSACTION_INFO : 
    public cnp::TRANSACTION
{
    typedef cnp::DWORD key_type;

    cnp::QWORD  m_qwCustomerID;

    /// Default Constructor
    TRANSACTION_INFO(void)
        : cnp::TRANSACTION(),
          m_qwCustomerID(INVALID_CUSTOMER_ID)
    { };

/**
  @brief Initialization Constructor

  @param [in] dwID          A Server generated unique sequential ID associated with each transaction
  @param [in] qwDateTime    64bit UTC value that represents number of seconds since Epoch
  @param [in] dwAmount      Amount excluding decimal point (i.e. $100.00 would be 10000)
  @param [in] wType         The transaction type, represented as cnp::TT_DEPOSIT or cnp::TT_WITHDRAWAL
  @param [in] qwCustomerID  Unique customer ID associated with the transaction
*/
    TRANSACTION_INFO(cnp::DWORD         dwID,        
                     const cnp::QWORD&  qwDateTime,  
                     cnp::DWORD         dwAmount,    
                     cnp::WORD          wType,       
                     const cnp::QWORD&  qwCustomerID)
         : m_qwCustomerID(qwCustomerID)
    { m_dwID = dwID;
      m_qwDateTime = qwDateTime;
      m_dwAmount = dwAmount;
      m_wType = wType; };

    /// Copy Constructor
    TRANSACTION_INFO(const TRANSACTION_INFO& rhs)
        : cnp::TRANSACTION(rhs),
          m_qwCustomerID(rhs.m_qwCustomerID)
    { };

 /**
    This method is used to provide a generic interface to retrieve
    TRANSACTION_INFO's primary key field.  In this implementation, 
    it is a wrapper around cnp::TRANSACTION.get_ID().

    @retval cnp::DWORD  containing the primary key value
 */
    inline cnp::DWORD         get_PrimaryKey(void) const noexcept
    { return get_ID(); };

    inline const cnp::QWORD&  get_CustomerID(void) const noexcept
    { return m_qwCustomerID; };

};

typedef std::map<ACCOUNT_INFO::key_type,     ACCOUNT_INFO>     AccountMap_t;
typedef std::map<TRANSACTION_INFO::key_type, TRANSACTION_INFO> TransactionMap_t;

/**
    Generate a unique customer ID from a given name + PIN combination

    @param [in] szFirstName       address of NULL terminated string containing 
                                  customer's first name
    @param [in] cbLen             count of bytes (cb) length of first name string
    @param [in] wPIN              customer's PIN number

    @retval cnp::QWORD containing unique 64bit customer ID
*/
cnp::QWORD GenerateCustomerID(const char* szFirstName, size_t cbLen, cnp::WORD wPIN) noexcept;

/**
   Loads into runtime memory the server database records from the persisted store

   @retval size_t containing the number of records loaded
*/
size_t     LoadServerDB      (void);
/**
   Saves the current server database records to persisted store

   @retval size_t contain the number of records saved to file
*/
size_t     SaveServerDB      (void);

#endif