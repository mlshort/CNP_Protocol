/******************************************/
/**
  @file   ThreadMisc.h
  @brief Interface for the Thread Related Classes/Templates.


  @author Mark L. Short
  @date   February 1, 2006
  @date   April   15, 2015  ported to LINUX
*/
/******************************************/



//////////////////////////////////////////////////////////////////////
// ThreadMisc.h: 
//
//            Thread Related Helper Classes/Templates
//
//////////////////////////////////////////////////////////////////////

#if !defined(__THREAD_MISC_H__)
#define __THREAD_MISC_H__

#ifndef _MUTEX_
    #include <mutex>
#endif

/// @brief "Stack-based" Template
///
///
/// This implementation does NOT actually contain a synchronization
/// object, but is used to manipulate an existing one.
///
/// @warning This template class expects 'class _Ty' to support the 
/// Lock/Unlock methods
template<class _Ty>
class TLock
{
    _Ty* m_pCS;

protected:

/// Declare the Default and Copy Constructor protected 
/// so the class cannot be instantiated in this manner
    TLock(const TLock<_Ty>&):m_pCS(0) {};
    TLock(_Ty* pCS):m_pCS(pCS)        {};

public:
    TLock():m_pCS(0)                  {};

    void Lock(void)
        { if (m_pCS) m_pCS->Lock(); };

    void Unlock(void)
        { if (m_pCS) m_pCS->Unlock(); };

    void SetLock(_Ty* pCS)
        { m_pCS = pCS; Lock(); };

};


/// @brief "Stack-based" Template
///
/// TAutoLock encapsulates some of the mundane operations of its
/// base class - TLock.
///
/// Is meant to be instantiated on the Stack so that
/// on return of the 'Locked' function, it is Unlocked with
/// the destruction of the object.
///
/// @warning This template class expects 'class _Ty' to support the 
/// Lock/Unlock methods
template<class _Ty>
class TAutoLock : public TLock<_Ty>
{

protected:

/// Declare the Default and Copy Constructor protected 
/// so the class cannot be instantiated in this manner
    TAutoLock() {};
    TAutoLock(const TAutoLock<_Ty>& o) {};
public:
    explicit TAutoLock(_Ty* pCS)
        : TLock<_Ty>(pCS)
        { TLock<_Ty>::Lock(); };

    ~TAutoLock()
        { TLock<_Ty>::Unlock(); };
};

// forward declaration
#ifdef _USE_CRITICAL_SECTIONS

class CAutoCriticalSection;

typedef TLock<CAutoCriticalSection>     CLock;
typedef TAutoLock<CAutoCriticalSection> CAutoLock;

#else

typedef TLock<std::recursive_mutex>     CLock;
typedef TAutoLock<std::recursive_mutex> CAutoLok;

#endif

#ifdef _MSCVER
/// CAutoCriticalSection provides methods for obtaining and releasing ownership of a critical 
/// section object.
///
/// Automatically initializes the critical section object in the constructor & deletes critical 
/// section object in the destructor.
///
/// You should not utilize CAutoCriticalSection in global objects or static class members if 
/// you want to eliminate the CRT startup code.
class CAutoCriticalSection
{
    CRITICAL_SECTION m_sec;

public:
/// Initializes private critical section member.
    CAutoCriticalSection()  
    { 
        ::InitializeCriticalSection(&m_sec); 
     };

/// Releases all resources used by private critical section member.
    ~CAutoCriticalSection() 
    { 
        ::DeleteCriticalSection    (&m_sec); 
    };

/// The Lock function waits for ownership of encapsulated critical section.
/// The function returns when the calling thread is granted ownership.
    void lock  (void);

/// The Unlock function releases ownership of encapsulated critical section.
    void unlock(void);

    friend CAutoLock;
    friend CLock;

////////////////////////////////////////////////////////////////////////////
// Prevent canonical behavior
//     A critical section object cannot be moved or copied. The process must 
//     also not modify the object, but must treat it as logically opaque. 
///////////
private:
    CAutoCriticalSection(const CAutoCriticalSection&);
    CAutoCriticalSection& operator =(const CAutoCriticalSection&);

};


inline  void 
CAutoCriticalSection::lock(void)
    { 
        ::EnterCriticalSection     (&m_sec); 
    };

inline  void 
CAutoCriticalSection::unlock(void)
    { 
        ::LeaveCriticalSection     (&m_sec); 
    };
#else

typedef std::recursive_mutex  CAutoCriticalSection;

#endif

#endif
