/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __REF_COUNT_THREAD_SAFE_H__
#define __REF_COUNT_THREAD_SAFE_H__

#include "Atomics.h"
#include <assert.h>

namespace intel{
template <typename T>

/// Threadsafe Refernce counter pointer
/// Uses atomic instruction
/// Must LINK with Atomics.cpp object file
class RefCountThreadSafe{
public:
//Forge
  RefCountThreadSafe(): m_refCount(0), m_ptr(0)
  {}

  explicit RefCountThreadSafe(T* ptr): m_refCount(0), m_ptr(ptr){
    if (ptr)
      m_refCount = new atomics::atomic_type(1);
  }

  RefCountThreadSafe(const RefCountThreadSafe<T>& other){
    cpy(other);
  }

  ~RefCountThreadSafe(){
    dispose();
  }

  RefCountThreadSafe& operator=(const RefCountThreadSafe<T>& other){
    if(this == &other)
      return *this;
    dispose();
    cpy(other);
    return *this;
  }

  bool isNull() const {
    return (!m_ptr);
  }

  const T& operator*()const{
    sanity();
    return *m_ptr;
  }

  T& operator*(){
    sanity();
    return *m_ptr;
  }

  T* operator->(){
    sanity();
    return m_ptr;
  }

  const T* operator->()const{
    sanity();
    return m_ptr;
  }
  
  const T* get() const {
    return m_ptr;
  }
  
  T* get(){
    return m_ptr;
  }

private:
 void sanity()const{
    assert(m_ptr && "NULL pointer");
    assert(m_refCount && "NULL ref counter");
    assert(*m_refCount && "zero ref counter");
  }

  void cpy(const RefCountThreadSafe<T>& other){
    m_refCount = other.m_refCount;
    m_ptr = other.m_ptr;
    if(m_refCount)
      atomics::AtomicIncrement(m_refCount);
  }

  void dispose(){
    if (!m_refCount)
      return;
    sanity();
    if (0 == atomics::AtomicDecrement(m_refCount)){
      delete m_refCount;
      delete m_ptr;
      m_ptr = 0;
      m_refCount = 0;
    }
  }

  atomics::atomic_type *m_refCount;
  T* m_ptr;
};//End RefCount

}//end namepace

#endif//__REF_COUNT_THREAD_SAFE_H__
