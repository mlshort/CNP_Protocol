/******************************************/
/**
  @file   TSQueue.h
  @brief  interface for the TTSQueue class.

  Thread Safe Queue with Associated Critical Section.

  @author Mark L. Short
  @date February 1, 2006
*/
/******************************************/

#if !defined(__TSQUEUE_H__)
#define __TSQUEUE_H__

#ifndef __THREAD_MISC_H__
  #include "ThreadMisc.h"
#endif

#ifndef _QUEUE_
    #include <queue>
#endif

#ifndef _LIST_
    #include <list>
#endif

#ifndef _VECTOR_
    #include <vector>
#endif


template<class _Ty>
class TTSQueue  
{
    typedef std::queue<_Ty, std::list<_Ty> > que_type;
    typedef typename que_type::size_type     size_type;
    typedef typename std::vector<_Ty>        items_type;
    
    typedef TTSQueue<_Ty>                    _Myt;
    typedef TLock<_Myt>                      _Lock;
    typedef TAutoLock<_Myt>                  _AutoLock;

// mutable - This keyword can only be applied to non-static and non-const data members of a class. 
// If a data member is declared mutable, then it is legal to assign a value to this data member from a 
// const member function.

// m_cs declared mutable to allow locking / unlocking mechanisms to function appropriately
// from data accessors as needed.
    mutable CAutoCriticalSection   m_cs;
    que_type                       m_que;

public:

    TTSQueue()
        :m_cs(), m_que()
        { };

    // does not copy critical section object !
    TTSQueue(const _Myt& o)
        :m_cs(), m_que(o.m_que)
        { };

    _Myt& operator=(const _Myt& rhs)
        { // don't copy the critical section object !
            if (this != &rhs)
               m_que = rhs.m_que;
            return (*this); }

    virtual ~TTSQueue()     
        { };

    void            Lock  (void) const { m_cs.lock();   };
    void            Unlock(void) const { m_cs.unlock(); };

/// Initializes the que by popping all elements until empty.
    void            Init  (bool bLock = true);
/// Returns a snapshot of queued-items.  return == true if queue is not empty
    bool            GetItems(items_type& Items, bool bLock = true) const;
/// Returns a const reference to private stl::queue
    const que_type& GetItems (void) const  { return m_que; };
/// Returns a const reference to the first element at the front of the queue.
/// @warning If the queue is empty, the return value is undefined.
    const _Ty&      Front    (void) const  { return (m_que.front()); };

/// Adds an element to the back of the queue.
    void            Push(const _Ty& Item, bool bLock = true);
/// Removes an element from the front of the queue.
/// Returns true if an element was removed.
    bool            Pop (bool bLock = true);
/// Combines the operations of Pop & Front by returning
/// a copy of the 'popped' element.
    bool            PopFront(_Ty& Item, bool bLock = true);
/// Tests if a queue is empty
    bool            IsEmpty(void) const  { return (m_que.empty()); };
/// Returns the number of elements in the queue.
    size_type       Size   (void) const  { return (m_que.size()); };

};


template<class _Ty>
void TTSQueue<_Ty>::Init(bool bLock /*= true*/)
{ 
    _Lock lock;
    if (bLock) 
        lock.SetLock(this); 

    // clear the queue
    while (m_que.empty() == false)
        m_que.pop();

};


template<class _Ty>
bool TTSQueue<_Ty>::GetItems(items_type& Items, bool bLock /* = true */) const
{  
    bool bResult = false;

    _Lock lock;
    if (bLock)
        lock.SetLock(this);

    if (m_que.empty() == false)
        {
        bResult = true;
// m_que is not empty... now start filling the returned Items for the caller
        for (auto it = m_que.begin(); it != m_que.end(); it++)
            Items.push_back(it->GetItem());
        }

    return bResult;
};

template<class _Ty>
void  TTSQueue<_Ty>::Push(const _Ty& Item, bool bLock /*= true*/)
{ 
    _Lock lock;
    if (bLock) 
       lock.SetLock(this); 

    m_que.push(Item);
};

template<class _Ty>
bool TTSQueue<_Ty>::Pop (bool bLock /*= true*/)
{
    bool bResult = false;

    _Lock lock;
    if (bLock) 
        lock.SetLock(this); 

     if (m_que.empty() == false)
         {
         bResult = true;
         m_que.pop();
         }

     return bResult;
};

// Combines Front and Pop
template<class _Ty>
bool TTSQueue<_Ty>::PopFront(_Ty& Item, bool bLock /*= true*/)
{	
    bool bResult = false;

    _Lock lock;
    if (bLock) 
        lock.SetLock(this); 

    if (m_que.empty() == false)
        {
        bResult = true;	
        Item    = Front();
        m_que.pop();
        }

    return bResult;
}


#endif 
