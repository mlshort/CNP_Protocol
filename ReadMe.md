NAME:  CNP_Protocol Project

DIRECTORY STRUCTURE:  
===============================================================================

  \CNP_Protocol  
    + --- \Server  (Server source)  
    + --- \Client  (Client source)  
    + --- \Include (CNP_Protocol.h)  
    + --- \Data             (saved customer account data)  
    + --- \Documentation    (implementation documentation)  
    + --- \Bin              (compiled executable)  
    + --- \Obj              (discardable binary intermediaries)

DESCRIPTION:         see below  
DATE WRITTEN:        April 15, 2015  

 
CNP_Protocol Project Overview
========================================================================  

1. ASSIGNMENT: 5580 Computer Networks

    Objective of this group activity is to design an application layer protocol 
    standard that will be used by all groups for the project. The protocol must
    provide the basic ATM Banking functionality of:

    - Establish a connection

    - Creating an Account
        * First Name
        * Last Name
        * Email Address
        * SSN
        * Driver's License #
        * User selected PIN

    - Logging On 
        * PIN & First Name

    - Deposit
        * Cash or Check

    - Withdrawal

    - Stamp Purchase

    - Transaction History Query

BACKGROUND
===============================================================================

* 2.1. Design a protocol & present it to the class. The class will determine
       (vote) on the best protocol to universally adopt for each team's class project.

* 2.2. Implement a multi-threaded Server & Client on Linux using the class
       selected protocol.
   
* The protocol (\Include\CNP_Protocol.h) was unanimously adopted by the class.

IMPLEMENTATION
===============================================================================

* 3.1. Protocol:

    - I had originally considered implementing a protocol based on ASN.1; 
      however, I reconsidered being concerned that it would be beyond the 
      other student's ability to understand. Instead, I went with as simple 
      a protocol I could that used fixed-sized data fields. Additionally, 
      I provided helper constructors that formatted the protocol messages 
      for the students.

    - I introduced the class to the use of "#pragma pack(push, 1)" to ensure 
      correct structure member data alignment regarding defining a protocol.

    - Surprisingly, the concept of UTC time was new to the class and I had 
      to spend time explaining its relevance & importance in a network protocol.

    - I provided coding examples on how to use the protocol & the ClientID 
      field:

       - The Server generates it in response to a successful CONNECT_REQUEST 
         from the Client.

       - The Client then uses it from that point on in all further messages.

       - The Server, after issuing a ClientID, enters it into a local runtime 
         table as a key that maps to each Client's session state.

       - The Server, verifies each subsequent message received from any Client 
         against that client's session state using the ClientID.

       - The ClientID is not persisted and is only valid during the actual time 
         a Client is connected. On disconnect, the same ClientID may be 
         re-issued to another newly connected Client.

    - I provided coding examples on how to persist customer Account and Transaction 
      tables using std::map.

    - **Why no inheritance?**

      - Even though, from an implementation stand-point, C++ inheritance would have 
        afforded the ability to avoid a lot of "helper" method duplication; from the 
        position of affording any byte-wise guarantee of consistency across various 
        compilers and platforms, there was no way to insure that the use of inheritance 
        would not, covertly, introduce additional hidden bytes or other compiler 
        generated information.

* 3.2. Server / Client

    - I found the actual implementation of the Server / Client unnecessarily 
      burdensome and difficult. This was due to an uncooperative IT infrastructure, 
      erratic connectivity and the strict requirement that the Server / Client be 
      developed on one of the campus Linux servers. Additionally, there was 
      only one Linux system on the entire campus that could support a C++-11 build 
      environment (which I was heavily vested in).

    - To address this, I ultimately ended up porting the entire project over to 
      Windows in order to have access to a reliable development environment that 
      I could develop, run and test the Client / Server on; and then, port the 
      resulting code base back to Linux.
      
RESULTS
===============================================================================

* 4.1. Type Definitions

  - CNP Message Types (CMT)

    ```cpp
    enum CNP_MSG_TYPE
    {
        CMT_INVALID           = 0x00,
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

    enum CNP_MSG_SUBTYPE
    {
        CMS_INVALID           = 0x00,
        CMS_REQUEST           = 0x01,
        CMS_RESPONSE          = 0x02
    };
    ```

  - CNP Error Result Types (CER)

    ```cpp
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
    ```


  - CNP Deposit types (DT)

    ```cpp
    enum DEPOSIT_TYPE
    {
        DT_INVALID   = 0,     ///< for initialization and error checking
        DT_CASH      = 0x01,  ///< Cash Deposit
        DT_CHECK     = 0x02   ///< Check Deposit
    };
    ```

  - CNP Transaction types (TT)
    ```cpp
    enum TRANSACTION_TYPE
    {
        TT_INVALID        = 0,     ///< for initialization and error checking
        TT_DEPOSIT        = 0x01,  ///< Deposit Transaction
        TT_WITHDRAWAL     = 0x02,  ///< Withdrawal Transaction
        TT_STAMP_PURCHASE = 0x03   ///< Stamp Purchase Transaction
    };
    ```


* 4.2. Message Structure(s)

  - Standard Message Header (included in all messages)

     |  Field(s)      | Begin Byte | End Byte |
     | :------------- | :--------: | :------: |
     | m_dwMsgType    |  0         | 3        |
     | m_wDataLen     |  4         | 5        |
     | m_wClientID    |  6         | 7        |
     | m_dwSequence   |  8         | 11       |
     | m_dwContext    | 12         | 15       |
 
  - [Client->Server] **Connect Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Request       | m_dwValidationKey | 16         | 19       |
     |  m_Request       | m_wMajorVersion   | 20         | 21       |
     |  m_Request       | m_wMinorVersion   | 22         | 23       |

  - [Server->Client] **Connect Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |
     |  m_Response      | m_wMajorVersion   | 20         | 21       |
     |  m_Response      | m_wMinorVersion   | 22         | 23       |
     |  m_Response      | m_wClientID       | 24         | 25       |

  - [Client->Server] **Create Account Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Request       | m_szFirsName      | 16         | 47       |
     |  m_Request       | m_szLastName      | 48         | 79       |
     |  m_Request       | m_szEmailAddress  | 80         | 111      |
     |  m_Request       | m_wPIN            | 112        | 113      |
     |  m_Request       | m_dwSSNumber      | 114        | 117      |
     |  m_Request       | m_dwDLNumber      | 118        | 121      |

  - [Server->Client] **Create Account Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |
 
  - [Client->Server] **Logon Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Request       | m_szFirstName     | 16         | 47       |
     |  m_Request       | m_wPIN            | 48         | 49       |

  - [Server->Client] **Logon Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |

  - [Client->Server] **Logoff Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |

  - [Server->Client] **Logoff Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |

  - [Client->Server] **Deposit Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Request       | m_dwAmount        | 16         | 19       |
     |  m_Request       | m_wType           | 20         | 21       |

  - [Server->Client] **Deposit Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |

  - [Client->Server] **Withdrawal Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Request       | m_dwAmount        | 16         | 19       |

  - [Server->Client] **Withdrawal Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |


  - [Client->Server] **Balance Query Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |

  - [Server->Client] **Balance Query Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |
     |  m_Response      | m_dwBalance       | 20         | 23       |

  - [Client->Server] **Transaction Query Request**

     |  Message Members |     Field           | Begin Byte | End Byte |
     | :--------------- | :------------------ | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType         |  0         | 3        |
     |  m_Hdr           | m_wDataLen          |  4         | 5        |
     |  m_Hdr           | m_wClientID         |  6         | 7        |
     |  m_Hdr           | m_dwSequence        |  8         | 11       |
     |  m_Hdr           | m_dwContext         | 12         | 15       |
     |  m_Request       | m_dwStartID         | 16         | 19       |
     |  m_Request       | m_wTransactionCount | 20         | 21       |

  - [Server->Client] **Transaction Query Response**

     |  Message Members |     Field           | Begin Byte | End Byte |
     | :--------------- | :------------------ | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType         |  0         | 3        |
     |  m_Hdr           | m_wDataLen          |  4         | 5        |
     |  m_Hdr           | m_wClientID         |  6         | 7        |
     |  m_Hdr           | m_dwSequence        |  8         | 11       |
     |  m_Hdr           | m_dwContext         | 12         | 15       |
     |  m_Response      | m_dwResult          | 16         | 19       |
     |  m_Response      | m_wTransactionCount | 20         | 21       |
     |  m_Response      | m_rgTransactions[]  | 22         | ...      |
     |                  | _m_dwID_            | 22         | 25       |
     |                  | _m_qwDateTime_      | 26         | 33       |
     |                  | _m_dwAmount_        | 34         | 37       |
     |                  | _m_wType_           | 37         | 39       |

  - [Client->Server] **Stamp Purchase Request**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Request       | m_dwAmount        | 16         | 19       |

  - [Server->Client] **Stamp Purchase Response**

     |  Message Members |     Field         | Begin Byte | End Byte |
     | :--------------- | :---------------- | :--------: | :------: |
     |  m_Hdr           | m_dwMsgType       |  0         | 3        |
     |  m_Hdr           | m_wDataLen        |  4         | 5        |
     |  m_Hdr           | m_wClientID       |  6         | 7        |
     |  m_Hdr           | m_dwSequence      |  8         | 11       |
     |  m_Hdr           | m_dwContext       | 12         | 15       |
     |  m_Response      | m_dwResult        | 16         | 19       |


* 4.1. Implementation Features:

    - Other than the FNV1a Hash algorithm used, 100% of the work is my own, even 
      the development of a cross-platform CNP_Socket class was my own work.

    - It builds and runs on both Windows & Linux.

    - The server *gracefully* shuts down on both platforms.

       - It makes use of installing a handler for catching the Control-C command to 
         programmatically shutdown the sockets/threads & gracefully terminate.

       - In order for the server to gracefully terminate, the Server socket had to be 
         asynchronous to be able to interrupt an otherwise blocking socket call, that 
         would have prevented it from shutting down otherwise.

       - This is not truly the way an asynchronous socket server should be developed 
         and I had to take some short-cuts due to the time being consumed.

       - It can be built using either the .sln solution file on Windows or the 
         make-files in the Client / Server subdirectories with the "make all" commands on 
         Linux.

    - The source makes use of various C++-11 features such as std::thread, std::mutex, 
      ranged-based for loops, use of auto key word, etc. 

    - Neither the Client nor the Server takes any command-line arguments, you should be 
      able to run them and they prompt you on what to enter.

    - Run the server 1st, obviously. It will prompt you for what port number it should 
      listen on. 

       - this really should have been done via a command-line argument or persisted 
         configuration, but I time did not allow for implementation of this "optional" 
         feature.

    - The server is truly multi-threaded, as will be shown in the server console while 
      it is processing various client messages.

       - I was able to build & run both the Server & Client on both Linux & Windows.

       - I was able to do some cross-platform connection tests with the Server on Linux 
         and Clients on Windows, but I did not have time to try the other way around.

       - I ran tests against the Server with as many as 5 Clients running at a time.

* 4.2. Implementation Shortcomings:

    - There is virtually no input data verification on the Client.

    - There is no checking to see if partial messages are sent or received.

    - I did not use ::htonl, ::htons, etc., like I should have. I was only able to get 
      away with this because both the Client & Server were on the same endian systems.

    - I did not get to make use of my TSQueue (thread-safe queue) template class. I 
      ported it to Linux in preparation of using it. Ultimately, it was not required. 
      I left it in the source anyways for reference, as there are commented sections of 
      the source code where it was clear how I intended to use it. 

REFERENCE(S)
===============================================================================

- Ramananandro, Reis, Leroy, "Formal Verification of Object Layout for C++ Multiple Inheritance", 
  INRIA Paris-Rocquencourt, Texas A&M University, January 26, 2011, http://gallium.inria.fr/~tramanan/cxx/object-layout/slides.pdf
  
- [Abstract Syntax Notation One (via wikipedia)](https://en.wikipedia.org/wiki/Abstract_Syntax_Notation_One)

- [Winsock (via wikipedia)](https://en.wikipedia.org/wiki/Winsock)

- [Coordinated Universal Time (UTC) (via wikipedia)](https://en.wikipedia.org/wiki/Coordinated_Universal_Time)

- [Endianness (via wikipedia)](https://en.wikipedia.org/wiki/Endianness)

- [Data structure alignment (via wikipedia)](https://en.wikipedia.org/wiki/Data_structure_alignment)

- [Alignment (C++ Declarations)](https://msdn.microsoft.com/en-us/library/dn956973.aspx)

- [pack](https://msdn.microsoft.com/en-us/library/2e70t5y1.aspx)
