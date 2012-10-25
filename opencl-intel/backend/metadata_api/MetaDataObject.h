/****************************************************************************
  Copyright (c) Intel Corporation (2012,2013).

  INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
  LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
  ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
  PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
  DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
  PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
  including liability for infringement of any proprietary rights, relating to
  use of the code. No license, express or implied, by estoppels or otherwise,
  to any intellectual property rights is granted herein.

  File Name: MetaDataObject.h

  \****************************************************************************/
#ifndef METADATAOBJECT_H
#define METADATAOBJECT_H

#include "llvm/Value.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Metadata.h"
#include "llvm/Support/Atomic.h"
#include <list>

namespace Intel
{

///
// Base interface for the metadata struct object
// Implements the reference counting behavior and support for dirty flag
struct IMetaDataObject
{
    IMetaDataObject():
        m_refCount(0)
    {}

    virtual ~IMetaDataObject()
    {}

    void addRef()
    {
        llvm::sys::AtomicIncrement(&m_refCount);
    }

    void releaseRef()
    {
        if( llvm::sys::AtomicDecrement(&m_refCount) == 0 )
            delete this;
    }

    virtual bool dirty() const = 0;

    virtual void discardChanges() = 0;

private:
    llvm::sys::cas_flag m_refCount;
};

///
// Smart pointer for handling the IMetaDataObject interfaces
template<class T>
class MetaObjectHandle
{
public:

    explicit MetaObjectHandle(T *rhs = 0) throw ()
        : m_ptr(rhs)
    {
        addRef();
    }

    MetaObjectHandle(const MetaObjectHandle<T>& rhs) throw ()
        : m_ptr(rhs.get())
    {
        addRef();
    }

    template<class _Other>
    MetaObjectHandle(const MetaObjectHandle<_Other>& rhs) throw ()
        : m_ptr(rhs.get())
    {
        addRef();
    }

    template<class _Other>
    operator MetaObjectHandle<_Other>() const throw ()
    {
        return (MetaObjectHandle<_Other>(*this));
    }

    template<class _Other>
    MetaObjectHandle<T>& operator=(const MetaObjectHandle<_Other>& rhs) throw ()
    {
        reset(rhs.get());
        return (*this);
    }

    MetaObjectHandle<T>& operator=(const MetaObjectHandle<T>& rhs) throw ()
    {
        reset(rhs.get());
        return (*this);
    }

    ~MetaObjectHandle()
    {
        releaseRef();
    }

    T& operator*() const throw ()
    {
        assert(m_ptr != 0);
        return (*get());
    }

    T *operator->() const throw ()
    {
        assert(m_ptr != 0);
        return (get());
    }

    T *get() const throw ()
    {
        return (m_ptr);
    }

    T** getOutPtr() throw ()
    {
        return (&m_ptr);
    }

    void addRef()
    {
        if(m_ptr)
           m_ptr->addRef();
    }

    void releaseRef()
    {
        if(m_ptr)
            m_ptr->releaseRef();
        m_ptr = 0;
    }

    T *release() throw ()
    {   // !!! This method do not decrement the reference cound and thus should be used with care !!!
        T *_Tmp = m_ptr;
        m_ptr = 0;
        return (_Tmp);
    }

    void reset(const T* rhs = 0)
    {
        releaseRef();
        m_ptr = rhs;
        addRef();
    }

    bool dirty() const
    {
        if(m_ptr)
            return m_ptr->dirty();
        return false;
    }

    void discardChanges()
    {
        if(m_ptr)
            m_ptr->discardChanges();
    }

    llvm::Value* generateNode(llvm::LLVMContext& context) const
    {
        if(m_ptr)
            return m_ptr->generateNode(context);
        return NULL;
    }

    void save(llvm::LLVMContext& context, llvm::MDNode* pNode) const
    {
        if(m_ptr)
            m_ptr->save(context, pNode);
    }

private:
    T *m_ptr;    // the wrapped object pointer
};

} //namespace
#endif
