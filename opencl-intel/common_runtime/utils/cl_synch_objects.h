// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
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
