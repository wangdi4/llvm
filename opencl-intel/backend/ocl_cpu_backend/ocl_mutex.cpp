#include "ocl_mutex.h"
#include <assert.h>


#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

// Creates the mutex section object.
OclMutex::OclMutex( unsigned int uiSpinCount )
{
    m_mutexHndl = new CRITICAL_SECTION();
    InitializeCriticalSectionAndSpinCount( (LPCRITICAL_SECTION)m_mutexHndl, uiSpinCount );
}

// Destroys the critical section object.
OclMutex::~OclMutex()
{
    DeleteCriticalSection( (LPCRITICAL_SECTION)m_mutexHndl );
    delete m_mutexHndl;
    m_mutexHndl = nullptr;
}

// Take the lock on this critical section.
// If lock is acquired, all other threads are blocked on this lock until
// the current thread unlocked it.
void OclMutex::Lock()
{
    EnterCriticalSection( (LPCRITICAL_SECTION)m_mutexHndl );
}
// Release the lock
void OclMutex::Unlock()
{
    LeaveCriticalSection( (LPCRITICAL_SECTION)m_mutexHndl );
}


OclAutoMutex::OclAutoMutex( IMutex* mutexObj, bool bAutoLock )
{
    m_mutexObj = mutexObj;
    if( bAutoLock )
    {
        m_mutexObj->Lock();
    }
}

OclAutoMutex::~OclAutoMutex()
{
    m_mutexObj->Unlock();
}
#else
#include <pthread.h>
OclMutex::OclMutex(unsigned int uiSpinCount) : m_uiSpinCount(uiSpinCount)
{
    m_mutexAttr = new pthread_mutexattr_t;
    if (0 != pthread_mutexattr_init((pthread_mutexattr_t*)m_mutexAttr))
    {
        assert(0 && "Failed init pthread mutex attribute");
    }
    if (0 != pthread_mutexattr_settype((pthread_mutexattr_t*)m_mutexAttr, PTHREAD_MUTEX_RECURSIVE))
    {
        assert(0 && "Failed settype pthread mutex attribute");
    }
    m_mutexHndl = new pthread_mutex_t;
    if (0 != pthread_mutex_init((pthread_mutex_t*)m_mutexHndl, (pthread_mutexattr_t*)m_mutexAttr))
    {
        assert(0 && "Failed initialize pthread mutex");
    }
}

/************************************************************************
 * Destroys the critical section object.
 ************************************************************************/
OclMutex::~OclMutex()
{
    if (0 != pthread_mutex_destroy((pthread_mutex_t*)m_mutexHndl))
    {
        assert(0 && "Failed destroy pthread mutex");
    }
    delete((pthread_mutex_t*)m_mutexHndl);
    m_mutexHndl = nullptr;

    if (0 != pthread_mutexattr_destroy((pthread_mutexattr_t*)m_mutexAttr))
    {
        assert(0 && "Failed destroy pthread mutex attribute");
    }
    delete((pthread_mutexattr_t*)m_mutexAttr);
    m_mutexAttr = nullptr;
}

/************************************************************************
 * Take the lock on this critical section.
 * If lock is acquired, all other threads are blocked on this lock until
 * the current thread unlocked it.
 ************************************************************************/
void OclMutex::Lock()
{
    pthread_mutex_lock((pthread_mutex_t*)m_mutexHndl);
}
/************************************************************************
 * Release the lock
 ************************************************************************/
void OclMutex::Unlock()
{
    pthread_mutex_unlock((pthread_mutex_t*)m_mutexHndl);
}
/************************************************************************
 *
 ************************************************************************/
OclAutoMutex::OclAutoMutex(IMutex* mutexObj, bool bAutoLock)
{
    m_mutexObj = mutexObj;
    if ( bAutoLock )
    {
        m_mutexObj->Lock();
    }
}

OclAutoMutex::~OclAutoMutex()
{
    m_mutexObj->Unlock();
}

#endif
