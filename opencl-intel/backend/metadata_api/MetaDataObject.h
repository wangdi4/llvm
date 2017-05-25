/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef METADATAOBJECT_H
#define METADATAOBJECT_H

#include "MetaDataTraits.h"
#include "MetaDataValue.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"

#include <atomic>

namespace Intel
{

///
// Base interface for the metadata struct object
// Meta data object support the following behavious:
//  * Ref counting
//  * Dirty bit
//  * Optional support for 'named object'
// 'named object' is a metadata object with the first operand containing the 'id' string
struct IMetaDataObject
{
    IMetaDataObject(const llvm::MDNode* pNode, bool hasId):
        m_refCount(0),
        m_id(hasId ? getIdNode(pNode) : NULL)
    {
    }

    IMetaDataObject(const char* name):
        m_refCount(0),
        m_id(name)
    {
    }

    IMetaDataObject():
        m_refCount(0),
        m_id(NULL)
    {}

    virtual ~IMetaDataObject()
    {}

    void addRef()
    {
        m_refCount++;
    }

    void releaseRef()
    {
        if( m_refCount.fetch_sub(1, std::memory_order_relaxed) == 0 )
            delete this;
    }

    llvm::StringRef getId()
    {
        return m_id.get();
    }

    void setId( const std::string& id )
    {
        m_id.set(id);
    }
    
    virtual bool dirty() const = 0;

    virtual void discardChanges() = 0;

protected:

    ///
    // Returns the Id node given the parent MDNode
    // Id node is always a first operand of the parent node and
    // should be stored as MDString
    llvm::Metadata* getIdNode(const llvm::MDNode* pNode) const
    {
        if( NULL == pNode)
        {
            // optional node 
            return NULL; 
        }

        if( !pNode->getNumOperands() )
        {
            throw "Named list doesn't have a name node";
        }

        llvm::MDString* pIdNode = llvm::dyn_cast<llvm::MDString>(pNode->getOperand(0));

        if( NULL == pIdNode )
        {
            throw "Named object id node is not a string";
        }

        return pIdNode;
    }

    ///
    // Returns the start index
    unsigned int getStartIndex() const
    {
        return  m_id.hasValue() ? 1 : 0;
    }

    void save(llvm::LLVMContext &context, llvm::MDNode* pNode) const
    {
        if( m_id.hasValue() )
            m_id.save(context, getIdNode(pNode));
    }

    llvm::Metadata* generateNode(llvm::LLVMContext &context) const
    {
        return m_id.generateNode(context);
    }

private:
    std::atomic<unsigned> m_refCount;
    MetaDataValue<std::string>  m_id;
};

///
// Smart pointer for handling the IMetaDataObject interfaces
template<class T>
class MetaObjectHandle
{
public:
    typedef T ObjectType;

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

    void reset( T* rhs = 0)
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

    llvm::Metadata* generateNode(llvm::LLVMContext& context) const
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

template< class T>
struct MDValueTraits<MetaObjectHandle<T>, void>
{
    typedef MetaObjectHandle<T> value_type;

    static value_type load(llvm::Metadata* pNode)
    {
        if (NULL == pNode) {
          return MetaObjectHandle<T>(new T());
        } else {
          llvm::MDNode* pMDNode = llvm::dyn_cast<llvm::MDNode>(pNode);
          assert(pMDNode && "pNode is not an MDNode value");
          return MetaObjectHandle<T>(new T(pMDNode, false));
        }
    }

    static llvm::Metadata* generateValue(llvm::LLVMContext& context, const value_type& val)
    {
        return val->generateNode(context);
    }

    static bool dirty(const value_type& val)
    {
        return val->dirty();
    }

    static void discardChanges(value_type& val)
    {
        val->discardChanges();
    }

    static void save( llvm::LLVMContext& context, llvm::Metadata* trgt, const value_type& val)
    {
        val->save(context, llvm::cast<llvm::MDNode>(trgt));
    }
};



} //namespace
#endif
