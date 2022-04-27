// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once
#include <assert.h>

namespace OCLCRT
{

namespace Utils
{

void clSleep( int milliseconds );


class IMutex
{
public:
    IMutex() {}
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
    virtual ~IMutex(){}
private:
    //Disallow copying
    IMutex( const IMutex& im ) {}
};


class OclMutex: public IMutex
{
public:
    OclMutex( unsigned int uiSpinCount = 4000 );
    virtual ~OclMutex ();
    void Lock();
    void Unlock();
protected:
    void* m_mutexHndl;
    void* m_mutexAttr;
private:
    unsigned int m_uiSpinCount;
};


class OclAutoMutex
{
public:
    OclAutoMutex( IMutex* mutexObj, bool bAutoLock = true );
    ~OclAutoMutex();

private:
    IMutex* m_mutexObj;
};

// Wait until being signaled
class OclBinarySemaphore
{
public:
    OclBinarySemaphore();
    virtual ~OclBinarySemaphore();

    // Signals the semaphore.
    void Signal();

    // Consumes a signal
    void Wait();
protected:
    void* m_semaphore;
};

} // namespace Utils
} // namespace OCLCRT
