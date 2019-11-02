/**
 * @file   CNP_ServerDB.cpp
 * @brief  Server DB persistence implementation
 *
 * @author Mark L. Short
 * @date   April 10, 2015
 * @date   April 25, 2015 updated code comments
 * 
 */

#include <fstream>
#include <istream>
#include <iostream>

#include "FNV1A_Hash.h"
#include "CNP_ServerDB.h"

/// File name of server ACCOUNT_INFO table store
const char g_szAccountDBFileName[]    = "..//Data//AccountDB.Dat";
/// File name of server TRANSACTION_INFO table store
const char g_szTransactDBFileName[]   = "..//Data//TransactDB.Dat";

AccountMap_t                 g_AccountInfo;
TransactionMap_t             g_TransactionInfo;


cnp::QWORD GenerateCustomerID(const char* szFirstName, size_t cbLen, cnp::WORD wPIN)
{
    cnp::QWORD qwResult   = INVALID_CUSTOMER_ID;

    cnp::DWORD dwNameHash = FNV1A_Hash(szFirstName, cbLen);

    // following just shifts the computed name hash over to make room for the wPIN
    // 'n << 3' is equivalent to 'n * 8' and used to calculate the number of bits
    // we need to shift

    qwResult = (dwNameHash << (sizeof(wPIN) << 3)) ^ wPIN;  
    return qwResult;
};

/**
  @brief Generic template function for loading STL maps

  LoadServerDB provides a generic function to use for different
  map type containers.

  @pre _MapType is a std::map<_KeyType, _MappedType> collection
  @pre _MappedType implements the get_PrimaryKey() method

  @param [in] szFileName   address of the NULL terminated string that 
                           contains the name of the file to open
  @param [in] Container    a reference to the std::map container instance
                           to insert loaded file records into

  @retval size_t containing the number of records actually loaded
 */
template <class _MapType>
size_t LoadServerDB(const char* szFileName, _MapType& Container)
{
    size_t nResult = 0;
    typedef typename _MapType::value_type  value_type;
    typedef typename _MapType::mapped_type Record_Type;

    std::ifstream ifs(szFileName, std::ios_base::binary);

    while (ifs)
    {
        Record_Type  Record;
        ifs.read(reinterpret_cast<char*>( &Record ), sizeof(Record));

        if (ifs)
        {
            auto pairResult = Container.insert(value_type(Record.get_PrimaryKey(), Record));

            // increment our count of records successfully read in.
            if (pairResult.second)
                nResult++;
        }
    }

    ifs.close();
    return nResult;
};


/**
  @brief Generic template function for persisting STL maps

  SaveServerDB provides a generic function to use for different
  map type containers.

  @pre _MapType is a std::map<_KeyType, _MappedType> collection

  @param [in] szFileName   address of the NULL terminated string that 
                           contains the name of the file to open for 
                           saving
  @param [in] Container    a reference to the std::map container instance
                           to iterate for persistence

  @retval size_t containing the number of records actually saved
 */
template <class _MapType>
size_t SaveServerDB(const char* szFileName, const _MapType& Container)
{
    size_t nResult = 0;
    std::ofstream ofs(szFileName, std::ios_base::binary);

    if (ofs)
    {
        for (const auto& it : Container)
        {
            ofs.write(reinterpret_cast<const char*>( &(it.second) ), sizeof(it.second) );
            nResult++;
        }
    }

    ofs.close();
    return nResult;
};

size_t LoadServerDB(void)
{
    size_t nResult = 0;

    nResult += LoadServerDB(g_szAccountDBFileName,  g_AccountInfo);
    nResult += LoadServerDB(g_szTransactDBFileName, g_TransactionInfo);

    return nResult;
};

size_t SaveServerDB(void)
{
    size_t nResult = 0;

    nResult += SaveServerDB(g_szAccountDBFileName,  g_AccountInfo);
    nResult += SaveServerDB(g_szTransactDBFileName, g_TransactionInfo);

    return nResult;
};