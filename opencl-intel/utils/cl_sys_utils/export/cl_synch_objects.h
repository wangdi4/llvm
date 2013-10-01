/////////////////////////////////////////////////////////////////////////
// cl_synch_objects.h
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2013 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel's prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel's suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

#pragma once
/************************************************************************
 * The synchronization objects header declares all objects in Intel OCL
 * runtime that are needed for synchronization. The header is OS independent.
 * The implementation is tuned per needed OS.
 * The available objects are:
 * - OclMutex:
 *      This object can be acquire by only one thread at a time.
 *      Use it to mutually exclusive access to a shared resource.
 *      Best suited for Critical Section gaurd.
 * - OclNamedMutex:
 *      Same as Mutex, but provides an option to share the object
 *      between 2 threads according to a unique name.
 *      Is expected to use only when sharing and synchronizing objects in different processes.
 * - OclAutoMutex:
 *      By using AutoMutex, you can lock the mutex and unlock it through the
 *      object Constructor/Destructor. Use it to prevent nested lock/unlock logic.
 * - OclCondition:
 *      Condition object is a synchronization object that enables a thread to wait on a condition
 *      until that condition is set.
 *      The object is attached with an external mutex. When the object enters into wait state,
 *      the mutex is atomically released and atomically is aquired when the condition is set.
 *      The condition may be signaled or broadcast. In case of single, only one waiting thread
 *      is released, otherwise, all threads are released
 *
 *  TODO: More objects that may be added: Semaphore, Event, else???
 ************************************************************************/

#include <tbb/concurrent_queue.h>
#include <queue>
#include <assert.h>

#include "cl_utils.h"
#include "hw_utils.h"
#include "cl_sys_defines.h"

namespace Intel { namespace OpenCL { namespace Utils {

    const unsigned int DEFAULT_SPIN_COUNT       = 4000;
    const bool         SUPPORT_RECURSIVE_LOCK   = true;
    const bool         NO_RECURSIVE_LOCK        = false;

    //Call this function from within spinning loops
    void InnerSpinloopImpl();

	template<class T=void>
	class AtomicPointer
	{
	public:
		AtomicPointer(T* ptr = NULL) : m_ptr(ptr) {}
		~AtomicPointer() {}

		T* test_and_set(T* comparand, T* exchange)
		{

			return (T*)CAS(&m_ptr, comparand, exchange);   // CAS(*ptr, old, new)
		}

		T* exchange(T* val)
		{
			return (T*)TAS(&m_ptr, val);
		}

		operator T*() const {return m_ptr;}
		T* operator ->() const {return m_ptr;}

	private:
		AtomicPointer(const AtomicPointer& ac) {m_ptr = ac.m_ptr; }
		T* volatile m_ptr;
	};

	class AtomicCounter
	{
	public:
		AtomicCounter(long initVal = 0) : m_val(initVal) {}
        AtomicCounter(const AtomicCounter& ac) {m_val = ac.m_val;}
		~AtomicCounter() {}

		long operator++();               //prefix. Returns new val
		long operator++(int alwaysZero); //postfix. Returns previous val
		long operator--();
		long operator--(int alwaysZero); //second argument enforced by the language, defaults to 0 by the compiler
		long add(long val); //returns new val
		long test_and_set(long comparand, long exchange);
		long exchange(long val);
		operator long() const; //casting operator        

	private:
		
		volatile long m_val;
	};

    /************************************************************************
     * IMutex:
     * An abstract mutex interface. Used as an input to the AutoMutex objects
     ************************************************************************/
    class IMutex
    {
    public:
		IMutex() {}
        virtual void Lock()=0;
        virtual void Unlock()=0;
        virtual ~IMutex(){}
	private:
		//Disallow copying
		IMutex(const IMutex& im) {}
    };

    /************************************************************************
     * OclAutoMutex:
     * The OclAutoMutex class is a convenience class that simplifies
     * locking and unlocking of a Mutex.
     * Use this object to control critical sections in your code
     * In the class constructor, the critical section is locked and unlocked
     * when the object is freed.
     *
     * How to use it?
     * 1) A critical section within a function.
     *      void foo()
     *      {
     *          ... // None critical section
     *          { OclAutoMutex CS(&mutex);
     *          ... // Add your critical section here
     *          } // End of CS
     *          ... // Another critical section
     *      }
     *
     *
     * 2) The entire function is trade-safe.
     *      void foo()
     *      {
     *          OclAutoMutex CS(&mutex); // Lock the function
     *          bool b = IsBool();
     *          if (!b)
     *              return;  // Implicit unlock by calling CS distructor.
     *          else
     *              DoThat();
     *          return;
     *      }
     *
     ************************************************************************/
    class OclAutoMutex
    {
    public:
        OclAutoMutex(IMutex* mutexObj, bool bAutoLock = true);
        ~OclAutoMutex();

    private:
        IMutex* m_mutexObj;
    };
    /************************************************************************
     * OclMutex:
     * This is the basic synchronization object in the system.
     * By locking the Mutex, a thread can exclusively access to a protected resource.
     * Any other thread that wants to access the same resource is blocked until
     * the Mutex is released. If more than one thread is waiting on this Mutex,
     * a waiting thread is selected to acquire the resource next. The order is arbitrary.
     * It is the developer responsibility to use this object correctly, in order to
     * really protect the resource.
     * The OclMutex can be acquired by calling the lock method and be released by
     * calling the unlock method.
     * Use this object wisely, since it can slow-down a multi-threaded code.
     *
     ************************************************************************/
    class OclMutex: public IMutex
    {
	friend class OclCondition;
    public:
        OclMutex(unsigned int uiSpinCount = DEFAULT_SPIN_COUNT, bool recursive = NO_RECURSIVE_LOCK );
        virtual ~OclMutex ();
        void Lock();
        void Unlock();
	protected:
		void* m_mutexHndl;
    private:
		OclMutex(const OclMutex& o);
		OclMutex& operator=(const OclMutex& o);
		void spinCountMutexLock();
		unsigned int m_uiSpinCount;
        bool         m_bRecursive;
    };

	class OclSpinMutex: public IMutex
	{
	public:
		OclSpinMutex();
		void Lock();
		void Unlock();
        bool lockedRecursively() const { return (lMutex > 1); }
	protected:
		AtomicCounter lMutex;
		threadid_t threadId;
	};


    /************************************************************************
     * OclCondition:
     *      Condition object is a synchronization object that enables a thread to wait on a condition
     *      until that condition is set.
     *      The object is attached with an external mutex. When the object enters into wait state,
     *      the mutex is atomically released and atomically is acquired when the condition is set.
     *      The condition may be signaled or broadcast. In case of single, only one waiting thread
     *      is released, otherwise, all threads are released
                AND THE MUTEX IS NOT AQUIRED???
     ************************************************************************/
    enum COND_RESULT
    {
        COND_RESULT_OK,                 // Return code on success.
        COND_RESULT_FAIL                // Return code in case of error.
    };


    class OclCondition
    {
    public:
        OclCondition();
        ~OclCondition();

        COND_RESULT Wait(OclMutex* m_mutexObj);
        COND_RESULT Signal();

    private:
        CONDITION_VAR	m_condVar;
    };

	// The class below encapsulates an OS-dependent event
	// Can be used by OclEvent's Wait() method
	class OclOsDependentEvent
	{
	public:
		OclOsDependentEvent();
		OclOsDependentEvent(bool AutoReset);
		~OclOsDependentEvent();

		// Initializes the event. Must be called before any use. Can fail.
		bool Init(bool bAutoReset = false);
		// Waits on an initialized event. Returns when the event was fired. Can fail, in which case another method of waiting should be used.
		bool Wait();
		// Fires the event
		void Signal();
		// Reset the event if signaled
		void Reset();
	private:
		// The internal, OS-dependent representation of the event.
		EVENT_STRUCTURE m_eventRepresentation;
	};

    // A class representing a binary semaphore, i.e. an OS-dependent object allowing a thread waiting on it to yield.
    // For a user-space implementation, use the atomic counters
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
        BINARY_SEMAPHORE m_semaphore;
    };

	template<class T>
	class OclConcurrentQueue
	{
	public:
		OclConcurrentQueue() {}
		~OclConcurrentQueue() {}

		bool IsEmpty() const;
		T    Top();
		T    PopFront();
		void PushBack(const T& newNode);
		bool TryPop(T& val);
	private:
		typedef typename tbb::concurrent_queue<T>::const_iterator TTypeConcurrentQueueConstIterator;
		tbb::concurrent_queue<T> m_queue;
	};

	template<class T>
	class OclNaiveConcurrentQueue
	{
	public:
		OclNaiveConcurrentQueue() {}
		~OclNaiveConcurrentQueue() {}

		bool IsEmpty() const;
		T    Top();
		T    PopFront();
		void PushBack(const T& newNode);
		bool TryPop(T& val);

	private:
		std::queue<T>   m_queue;
		OclSpinMutex	m_queueLock;
	};


	/* AtomicBitField define a bit field array which support the operations:
	   Bit test and reset.
	   Bit test and set
	*/
	class AtomicBitField
	{
	public:
		AtomicBitField();
		virtual ~AtomicBitField();

		/* Initialize a new bit field array of size 'size' and set its' initial value to 'initVal'.
		   The size must be greater than zero.
		   The initialization process performs only once. (If 2 or more threads are trying to initialize the bit field array, only one will success)
		   Must be call before using bitTestAndRest / bitTestAndSet operations.
		*/
		void init(unsigned int size, bool initVal);
		/*
		   Reset atomically the appropriate bit in bit field array.
		   On success, return the initial value of the appropriate bit, otherwise return -1.
		*/
		long bitTestAndReset(unsigned int bitNum);
		/*
		   Set atomically the appropriate bit in bit field array.
		   On success, return the initial value of the appropriate bit, otherwise return -1.
		*/
		long bitTestAndSet(unsigned int bitNum);

		// Used for debug and log purposes
		operator unsigned long long()
		{
			unsigned long long val = 0;
			for(unsigned int i=0;i<m_size;++i)
			{
				val |= (m_bitField[i] & 0x1) << (i);
			}
			return val;
		}
		
	private:
		unsigned int m_size;
		long* m_bitField;
		volatile long m_oneTimeFlag;
		volatile bool m_isInitialize;
		OclOsDependentEvent m_eventLock;
	};
	
	///////////////////////////////////////////////////////////////////////////////////////
	// Basic ReadWriteLock implemenation
	// Using standart OS mechanism, SLIM RreadWrite lock on Windows and pthread_rwlock on Linux/Android
	// The lock implementation in not recursive
	// Optimization of ReadRead path, in case of contention, waiting thread is scheduled out
	///////////////////////////////////////////////////////////////////////////////////////
    class OclReaderWriterLock
    {
    public:
        OclReaderWriterLock();
        ~OclReaderWriterLock();

        void EnterRead();
        void LeaveRead();
        void EnterWrite();
        void LeaveWrite();

    protected:
		READ_WRITE_LOCK	m_rwLock;

#ifdef _DEBUG
		AtomicCounter readEnter;
		AtomicCounter writeEnter;
#endif
	};

    class OclAutoReader
    {
    public:
        OclAutoReader(OclReaderWriterLock* mutexObj, bool bAutoLock = true) : m_mutexObj(mutexObj)
        {
            assert( (NULL!=m_mutexObj) && "Got invalid object");
            if (bAutoLock)
            {
                m_mutexObj->EnterRead();
            }
        }
        ~OclAutoReader()
        {
            m_mutexObj->LeaveRead();
        }

    private:
        OclReaderWriterLock* m_mutexObj;
    };

    class OclAutoWriter
    {
    public:
        OclAutoWriter(OclReaderWriterLock* mutexObj, bool bAutoLock = true) : m_mutexObj(mutexObj)
        {
            assert(m_mutexObj);
            if (bAutoLock)
            {
                m_mutexObj->EnterWrite();
            }
        }
        ~OclAutoWriter()
        {
            assert(m_mutexObj);
            m_mutexObj->LeaveWrite();
        }

    private:
        OclReaderWriterLock* m_mutexObj;
    };

//includes for the template classes
#include "cl_synch_objects.hpp"

}}}    // Intel::OpenCL::Utils
