/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __REF_COUNT_H__
#define __REF_COUNT_H__

#include <assert.h>

namespace intel{

template <typename T>
class RefCount{
public:
//Forge
  RefCount(): m_refCount(NULL), m_ptr(NULL){
  }

  RefCount(T* ptr): m_ptr(ptr){
    m_refCount = new int(1);
  }

  RefCount(const RefCount<T>& other){
    cpy(other);
  }

  ~RefCount(){
    if (m_refCount)
      dispose();
  }

  RefCount& operator=(const RefCount<T>& other){
    if(this == &other)
      return *this;
    if (m_refCount)
      dispose();
    cpy(other);
    return *this;
  }

  void init(T* ptr){
    assert(!m_ptr && "overrunning non NULL pointer");
    assert(!m_refCount && "overrunning non NULL pointer");
    m_refCount = new int(1);
    m_ptr = ptr;
  }
//Pointer access
  const T& operator*()const{
    sanity();
    return *m_ptr;
  }

  T& operator*(){
    sanity();
    return *m_ptr;
  }

  operator T*(){
    return m_ptr;
  }
  
  operator const T*()const{
    return m_ptr;
  }

  T* operator->(){
    return m_ptr;
  }

  const T* operator->()const{
    return m_ptr;
  }
private:
  void sanity()const{
    assert(m_ptr && "NULL pointer");
    assert(m_refCount && "NULL ref counter");
    assert(*m_refCount && "zero ref counter");
  }

  void cpy(const RefCount<T>& other){
    m_refCount = other.m_refCount;
    m_ptr = other.m_ptr;
    ++*m_refCount;
  }

  void dispose(){
    sanity();
    if (0 == --*m_refCount){
      delete m_refCount;
      delete m_ptr;
      m_ptr = NULL;
      m_refCount = NULL;
    }
  }

  int* m_refCount;
  T* m_ptr;
};//End RefCount

}//end namepace

#endif//__REF_COUNT_H__
