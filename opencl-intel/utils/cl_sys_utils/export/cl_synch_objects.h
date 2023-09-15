// INTEL CONFIDENTIAL
//
// Copyright 2007 Intel Corporation.
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
/************************************************************************
 * The synchronization objects header declares all objects in Intel OCL
 * runtime that are needed for synchronization. The header is OS independent.
 * The implementation is tuned per needed OS.
 * - OclCondition:
 *      Condition object is a synchronization object that enables a thread to
 *wait on a condition until that condition is set. The object is attached with
 *an external mutex. When the object enters into wait state, the mutex is
 *atomically released and atomically is aquired when the condition is set. The
 *condition may be signaled or broadcast. In case of single, only one waiting
 *thread is released, otherwise, all threads are released
 *
 *  TODO: More objects that may be added: Semaphore, Event, else???
 ************************************************************************/

#include "cl_utils.h"
#include "hw_utils.h"
#include "ittnotify.h"
#ifdef __USE_TBB_CONCURENT_QUEUE
#include "tbb/concurrent_queue.h"
#endif

#include <assert.h>
#include <atomic>
#include <mutex>
#include <queue>

namespace Intel {
namespace OpenCL {
namespace Utils {
template <class T> class AtomicPointer {
public:
  AtomicPointer(T *ptr = nullptr) : m_ptr(ptr) {}
  ~AtomicPointer() {}

  T *test_and_set(T *comparand, T *exchange) {

    return (T *)CAS(&m_ptr, comparand, exchange); // CAS(*ptr, old, new)
  }

  T *exchange(T *val) { return (T *)TAS(&m_ptr, val); }

  operator T *() const { return m_ptr; }
  T *operator->() const { return m_ptr; }

private:
  AtomicPointer(const AtomicPointer &ac) { m_ptr = ac.m_ptr; }
  T *volatile m_ptr;
};

/************************************************************************
 * OclRecursiveMutex:
 * Add number of locked for std::recursive
 ************************************************************************/
class OclRecursiveMutex {
public:
  void Lock() {
    mutex.lock();
    lMutex++;
  };
  void Unlock() {
    mutex.unlock();
#ifdef _DEBUG
    long Val = lMutex--;
    assert(Val != 0);
#else
    lMutex--;
#endif
  }
  bool lockedRecursively() const { return (lMutex > 1); }

protected:
  std::atomic<long> lMutex{0};
  std::recursive_mutex mutex;
};

/************************************************************************
* OclCondition:
*      Condition object is a synchronization object that enables a thread to
wait on a condition
*      until that condition is set.
*      The object is attached with an external mutex. When the object enters
into wait state,
*      the mutex is atomically released and atomically is acquired when the
condition is set.
*      The condition may be signaled or broadcast. In case of single, only one
waiting thread
*      is released, otherwise, all threads are released
           AND THE MUTEX IS NOT AQUIRED???
************************************************************************/
enum COND_RESULT {
  COND_RESULT_OK,  // Return code on success.
  COND_RESULT_FAIL // Return code in case of error.
};

class OclCondition {
public:
  OclCondition();
  ~OclCondition();

  COND_RESULT Wait(std::mutex *m_mutexObj);
  COND_RESULT Signal();

private:
  CONDITION_VAR m_condVar;
};

// The class below encapsulates an OS-dependent event
// Can be used by OclEvent's Wait() method
class OclOsDependentEvent {
public:
  OclOsDependentEvent();
  OclOsDependentEvent(bool AutoReset);
  ~OclOsDependentEvent();

  // Delete copy & move constructor
  OclOsDependentEvent(const OclOsDependentEvent &) = delete;
  OclOsDependentEvent(OclOsDependentEvent &&) = delete;

  // Delete assignment operator
  OclOsDependentEvent &operator=(const OclOsDependentEvent &) = delete;
  OclOsDependentEvent &operator=(OclOsDependentEvent &&) = delete;

  // Initializes the event. Must be called before any use. Can fail.
  bool Init(bool bAutoReset = false);
  // Waits on an initialized event. Returns when the event was fired. Can fail,
  // in which case another method of waiting should be used.
  bool Wait();
  // Fires the event
  void Signal();
  // Reset the event if signaled
  void Reset();

private:
  // The internal, OS-dependent representation of the event.
  EVENT_STRUCTURE m_eventRepresentation;
};

// A class representing a binary semaphore, i.e. an OS-dependent object allowing
// a thread waiting on it to yield. For a user-space implementation, use the
// atomic counters
class OclBinarySemaphore {
public:
  OclBinarySemaphore();
  virtual ~OclBinarySemaphore();

  // Delete copy & move constructor
  OclBinarySemaphore(const OclBinarySemaphore &) = delete;
  OclBinarySemaphore(OclBinarySemaphore &&) = delete;

  // Delete assignment operator
  OclBinarySemaphore &operator=(const OclBinarySemaphore &) = delete;
  OclBinarySemaphore &operator=(OclBinarySemaphore &&) = delete;

  // Signals the semaphore.
  void Signal();

  // Consumes a signal
  void Wait();

protected:
  BINARY_SEMAPHORE m_semaphore;
};

#ifdef __USE_TBB_CONCURENT_QUEUE
template <class T> class OclConcurrentQueue {
public:
  OclConcurrentQueue() {}
  ~OclConcurrentQueue() {}

  bool IsEmpty() const;
  T Top();
  T PopFront();
  void PushBack(const T &newNode);
  bool TryPop(T &val);

private:
  typedef typename tbb::concurrent_queue<T>::const_iterator
      TTypeConcurrentQueueConstIterator;
  tbb::concurrent_queue<T> m_queue;
};
#endif
template <class T> class OclNaiveConcurrentQueue {
public:
  OclNaiveConcurrentQueue() {}
  ~OclNaiveConcurrentQueue() {}

  bool IsEmpty() const;
  T Top();
  T PopFront();
  void PushBack(const T &newNode);
  bool TryPop(T &val);

private:
  std::queue<T> m_queue;
  std::mutex m_queueLock;
};

template <class T, class S> class OclNaiveConcurrentMap {
public:
  OclNaiveConcurrentMap() {}
  ~OclNaiveConcurrentMap() {}

  bool IsEmpty();
  void Insert(const T &key, const S &val);
  S Find(const T &key);
  void Erase(const T &key);
  bool IsFound(const T &key, S &val);
  void Clear();

private:
  std::map<T, S> m_map;
  std::mutex m_mapLock;

  bool IsEmptyInternal() const;
};

/* AtomicBitField define a bit field array which support the operations:
   Bit test and reset.
   Bit test and set
*/
class AtomicBitField {
public:
  AtomicBitField();
  virtual ~AtomicBitField();

  AtomicBitField(const AtomicBitField &) = delete;
  AtomicBitField &operator=(const AtomicBitField &) = delete;

  /* Initialize a new bit field array of size 'size' and set its' initial value
     to 'initVal'. The size must be greater than zero. The initialization
     process performs only once. (If 2 or more threads are trying to initialize
     the bit field array, only one will success) Must be call before using
     bitTestAndRest / bitTestAndSet operations.
  */
  void init(unsigned int size, bool initVal);
  /*
     Set atomically the appropriate bit in bit field array.
     On success, return the initial value of the appropriate bit, otherwise
     return -1.
  */
  long bitTestAndSet(unsigned int bitNum);

  // Used for debug and log purposes
  operator unsigned long long() {
    unsigned long long val = 0;
    for (unsigned int i = 0; i < m_size; ++i) {
      val |= (m_bitField[i] & 0x1) << (i);
    }
    return val;
  }

private:
  unsigned int m_size;
  long *m_bitField;
  volatile long m_oneTimeFlag;
  volatile bool m_isInitialize;
  OclOsDependentEvent m_eventLock;
};

///////////////////////////////////////////////////////////////////////////////////////
// Basic ReadWriteLock implemenation
// Using standart OS mechanism, SLIM RreadWrite lock on Windows and
// pthread_rwlock on Linux The lock implementation in not recursive Optimization
// of ReadRead path, in case of contention, waiting thread is scheduled out
///////////////////////////////////////////////////////////////////////////////////////
class OclReaderWriterLock {
public:
  OclReaderWriterLock();
  ~OclReaderWriterLock();

  void EnterRead();
  void LeaveRead();
  void EnterWrite();
  void LeaveWrite();

protected:
  READ_WRITE_LOCK m_rwLock;

#ifdef _DEBUG
  std::atomic<long> readEnter{0};
  std::atomic<long> writeEnter{0};
#endif
};

class OclAutoReader {
public:
  OclAutoReader(OclReaderWriterLock *mutexObj, bool bAutoLock = true)
      : m_mutexObj(mutexObj) {
    assert((nullptr != m_mutexObj) && "Got invalid object");
    if (bAutoLock) {
      m_mutexObj->EnterRead();
    }
  }
  ~OclAutoReader() { m_mutexObj->LeaveRead(); }

private:
  OclReaderWriterLock *m_mutexObj;
};

class OclAutoWriter {
public:
  OclAutoWriter(OclReaderWriterLock *mutexObj, bool bAutoLock = true)
      : m_mutexObj(mutexObj) {
    assert(m_mutexObj);
    if (bAutoLock) {
      m_mutexObj->EnterWrite();
    }
  }
  ~OclAutoWriter() {
    assert(m_mutexObj);
    m_mutexObj->LeaveWrite();
  }

private:
  OclReaderWriterLock *m_mutexObj;
};

// includes for the template classes
#include "cl_synch_objects.hpp"

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
