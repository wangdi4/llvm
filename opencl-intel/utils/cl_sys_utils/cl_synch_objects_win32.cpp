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

#include "cl_synch_objects.h"
#include "hw_utils.h"
#include "tbb/concurrent_queue.h"
#include <assert.h>
#include <stdio.h> // Todo: replace printf with log mechanisem
#include <windows.h>

/************************************************************************
 * This file is the Windows implementation of the cl_synch_objects interface
 ************************************************************************/
using namespace Intel::OpenCL::Utils;

/************************************************************************
 *
 ************************************************************************/
OclCondition::OclCondition() {
  STATIC_ASSERT(sizeof(m_condVar) == sizeof(CONDITION_VARIABLE));
  // Initializing the condition variable
  InitializeConditionVariable((CONDITION_VARIABLE *)&m_condVar);
}

/************************************************************************
 * Condition distructor must be called when there are no threads waiting
 * on this condition.
 * Else, the behavior is undefined.
 ************************************************************************/
OclCondition::~OclCondition() {}

/************************************************************************
 *
 ************************************************************************/
COND_RESULT OclCondition::Wait(std::mutex *mutexObj) {
  assert(nullptr != mutexObj && "mutexObj must be valid object");
  if (nullptr == mutexObj) {
    return COND_RESULT_FAIL;
  }
  if (0 == SleepConditionVariableCS(
               (CONDITION_VARIABLE *)&m_condVar,
               (PCRITICAL_SECTION)(mutexObj->native_handle()), INFINITE)) {
    return COND_RESULT_FAIL;
  }
  return COND_RESULT_OK;
}

/************************************************************************
 *
 ************************************************************************/
COND_RESULT OclCondition::Signal() {
  WakeAllConditionVariable((CONDITION_VARIABLE *)&m_condVar);
  return COND_RESULT_OK;
}

/************************************************************************
 * OclOsDependentEvent implementation
 ************************************************************************/
OclOsDependentEvent::OclOsDependentEvent() : m_eventRepresentation(nullptr) {}

OclOsDependentEvent::OclOsDependentEvent(bool bAutoReset)
    : m_eventRepresentation(nullptr) {
  Init(bAutoReset);
}

OclOsDependentEvent::~OclOsDependentEvent() {
  if (m_eventRepresentation) {
    CloseHandle((HANDLE)m_eventRepresentation);
  }
  m_eventRepresentation = nullptr;
}

bool OclOsDependentEvent::Init(bool bAutoReset /* = false */) {
  if (m_eventRepresentation != nullptr) // event already initialized
  {
    // Todo: raise error?
    return true;
  }
  m_eventRepresentation = CreateEvent(nullptr, !bAutoReset, FALSE, nullptr);
  return m_eventRepresentation != nullptr;
}

bool OclOsDependentEvent::Wait() {
  if (nullptr == m_eventRepresentation) {
    // event not initialized
    return false;
  }
  return WAIT_OBJECT_0 == WaitForSingleObject(m_eventRepresentation, INFINITE);
}

void OclOsDependentEvent::Signal() {
  if (m_eventRepresentation != nullptr) {
    SetEvent(m_eventRepresentation);
  } else {
    assert(m_eventRepresentation && "m_eventRepresentation is NULL pointer");
  }
}

void OclOsDependentEvent::Reset() {
  if (m_eventRepresentation != nullptr) {
    ResetEvent(m_eventRepresentation);
  } else {
    assert(m_eventRepresentation && "m_eventRepresentation is NULL pointer");
  }
}

/************************************************************************
 * OclOsDependentEvent implementation
 ************************************************************************/
OclBinarySemaphore::OclBinarySemaphore() {
  m_semaphore = CreateEvent(nullptr, false, false, nullptr);
}

OclBinarySemaphore::~OclBinarySemaphore() { CloseHandle(m_semaphore); }

void OclBinarySemaphore::Signal() { SetEvent(m_semaphore); }

void OclBinarySemaphore::Wait() { WaitForSingleObject(m_semaphore, INFINITE); }

/************************************************************************
 * AtomicBitField implementation
 ************************************************************************/
AtomicBitField::AtomicBitField()
    : m_size(0), m_oneTimeFlag(0), m_isInitialize(false), m_eventLock() {
  m_bitField = nullptr;
  m_eventLock.Init(false);
}

AtomicBitField::~AtomicBitField() {
  if (m_bitField) {
    free(m_bitField);
  }
}

// Disable std::fill_n warning
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

void AtomicBitField::init(unsigned int size, bool initVal) {
  // test if already initialized (by other thread)
  if ((m_oneTimeFlag != 0) ||
      (InterlockedCompareExchange(&m_oneTimeFlag, 1, 0) != 0)) {
    if (m_isInitialize) {
      return;
    }
    m_eventLock.Wait();
    return;
  }
  if (size <= 0) {
    assert(
        0 &&
        "Error occurred while trying to create bit field array, invalid size");
  }
  m_size = size;
  m_bitField = (long *)malloc(sizeof(long) * m_size);
  if (nullptr == m_bitField) {
    assert(
        0 &&
        "Error occurred while trying to create bit field array, malloc failed");
    m_eventLock.Signal();
    return;
  }
  if (initVal) {
    std::fill_n(m_bitField, m_size, 1);
  } else {
    memset(m_bitField, 0, sizeof(long) * m_size);
  }
  m_isInitialize = true;
  m_eventLock.Signal();
}

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(pop)
#endif

long AtomicBitField::bitTestAndSet(unsigned int bitNum) {
  if ((nullptr == m_bitField) || (bitNum >= m_size)) {
    return -1;
  }
  return InterlockedCompareExchange((m_bitField + bitNum), 1, 0);
}

/************************************************************************
 * OclReaderWriterLock implementation
 ************************************************************************/
OclReaderWriterLock::OclReaderWriterLock() {
  STATIC_ASSERT(
      sizeof(void *) ==
      sizeof(SRWLOCK)); // We assume that SRWLOCK defined as struct{void*}
  InitializeSRWLock((PSRWLOCK)&m_rwLock);
}

OclReaderWriterLock::~OclReaderWriterLock() {
#ifdef _DEBUG
  assert((writeEnter == 0) && (readEnter == 0) &&
         "Writers or Readers are active in destructor");
#endif
}

void OclReaderWriterLock::EnterRead() {
  AcquireSRWLockShared((PSRWLOCK)&m_rwLock);
#ifdef _DEBUG
  readEnter++;
  assert(writeEnter == 0 && "No writer is allowed insde EnterRead()");
#endif
}

void OclReaderWriterLock::LeaveRead() {
#ifdef _DEBUG
  readEnter--;
#endif
  ReleaseSRWLockShared((PSRWLOCK)&m_rwLock);
}

void OclReaderWriterLock::EnterWrite() {
  AcquireSRWLockExclusive((PSRWLOCK)&m_rwLock);
#ifdef _DEBUG
  writeEnter++;
  assert((writeEnter == 1) && (readEnter == 0) &&
         "Only single writer and no readers are allowed insde EnterWrite()");
#endif
}

void OclReaderWriterLock::LeaveWrite() {
#ifdef _DEBUG
  writeEnter--;
#endif
  ReleaseSRWLockExclusive((PSRWLOCK)&m_rwLock);
}
