/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef METADATAAPI_UTILS_H
#define METADATAAPI_UTILS_H

#include "MetaDataTraits.h"
#include "MetaDataValue.h"
#include "MetaDataObject.h"
#include "MetaDataIterator.h"
#include "llvm/Value.h"
#include "llvm/Metadata.h"
#include <vector>

namespace Intel
{

///
// Metadata list. It is assumed that
// all the nodes are of the same type ( as specified by the T template parameter)
// Template parameters:
// T - type of the entry node
// Traits - convertor type (see the MDValueTraits )
//
template<class T, class Traits = MDValueTraits<T> >
class MetaDataList: public IMetaDataObject
{
public:
    typedef IMetaDataObject _Mybase;
    typedef MetaDataList<T,Traits> _Myt;
    typedef MetaDataIterator<T, llvm::MDNode, Traits> meta_iterator;
    typedef typename Traits::value_type item_type;
    typedef typename std::vector<item_type>::iterator iterator;
    typedef typename std::vector<item_type>::const_iterator const_iterator;

    MetaDataList(const llvm::MDNode* pNode, bool hasId = false):
        _Mybase(pNode, hasId),
        m_pNode(pNode),
        m_isDirty(false),
        m_isLoaded(false)
    {}

    MetaDataList(const char* name):
        _Mybase(name),
        m_pNode(NULL),
        m_isDirty(false),
        m_isLoaded(false)
    {}

    MetaDataList():
        m_pNode(NULL),
        m_isDirty(false),
        m_isLoaded(false)
    {}

    size_t size() const
    {
        lazyLoad();
        return m_data.size();
    }

    bool empty() const
    {
        lazyLoad();
        return m_data.empty();
    }

    iterator begin()
    {
        lazyLoad();
        return m_data.begin();
    }

    iterator end()
    {
        lazyLoad();
        return m_data.end();
    }

    const_iterator begin() const
    {
        lazyLoad();
        return m_data.begin();
    }

    const_iterator end() const
    {
        return m_data.end();
    }

    void push_back(const item_type& val)
    {
        lazyLoad();
        m_data.push_back(val);
        m_isDirty = true;
    }

    iterator erase(iterator where)
    {
        lazyLoad();
        iterator i = m_data.erase(where);
        m_isDirty = true;
        return i;
    }

    item_type getItem( size_t index)
    {
        lazyLoad();
        return m_data[index];
    }

    void setItem( size_t index, const item_type& item)
    {
        lazyLoad();
        m_data[index] = item;
        m_isDirty = true;
    }

    bool dirty() const
    {
        if( m_isDirty )
        {
            return true;
        }

        if( !m_isLoaded )
        {
            return false;
        }

        for( const_iterator i = m_data.begin(), e = m_data.end(); i != e; ++i )
        {
            if( Traits::dirty(*i) )
            {
                return true;
            }
        }
        return false;
    }

    bool hasValue() const
    {
        return m_pNode != NULL || dirty();
    }

    void discardChanges()
    {
        if( !dirty() )
        {
            return;
        }

        for( iterator i = m_data.begin(), e = m_data.end(); i != e; ++i )
        {
            Traits::discardChanges(*i);
        }

        m_isDirty = false;
    }

    virtual void save(llvm::LLVMContext &context, llvm::MDNode* pNode) const
    {
        if( m_pNode == pNode && !dirty() )
        {
            return;
        }

        if(pNode->getNumOperands() != size() + _Mybase::getStartIndex() )
        {
            pNode->replaceAllUsesWith(generateNode(context));
            llvm::MDNode::deleteTemporary(pNode);
            return;
        }

        _Mybase::save(context, pNode);

        meta_iterator mi(pNode, _Mybase::getStartIndex());
        meta_iterator me(pNode);
        const_iterator i = begin();
        const_iterator e = end();

        for(; i != e; ++i, ++mi )
        {
            Traits::save(context, *mi, *i);
        }
    }

    virtual llvm::Value* generateNode(llvm::LLVMContext &context) const
    {
        llvm::SmallVector< llvm::Value*, 5> args;

        llvm::Value* pIDNode = _Mybase::generateNode(context);

        if( NULL != pIDNode )
        {
            args.push_back( pIDNode );
        }

        for( const_iterator i = m_data.begin(), e = m_data.end(); i != e; ++i )
        {
            args.push_back( Traits::generateValue(context, *i));
        }

        return llvm::MDNode::get(context,args);
    }

protected:

    virtual void lazyLoad() const
    {
        if( m_isLoaded || NULL == m_pNode )
        {
            return;
        }

        for(meta_iterator i(m_pNode, _Mybase::getStartIndex()), e(m_pNode); i != e; ++i )
        {
            m_data.push_back(i.get());
        }

        m_isLoaded = true;
    }


protected:
    const llvm::MDNode* m_pNode;
    bool m_isDirty;
    mutable bool m_isLoaded;
    mutable std::vector<item_type> m_data;
};


template<class T, class Traits = MDValueTraits<T> >
class NamedMDNodeList
{
public:
    typedef NamedMDNodeList<T,Traits> _Myt;
    typedef MetaDataIterator<T,llvm::NamedMDNode,Traits> meta_iterator;
    typedef typename Traits::value_type item_type;
    typedef typename std::vector<item_type>::iterator iterator;
    typedef typename std::vector<item_type>::const_iterator const_iterator;

    NamedMDNodeList(const llvm::NamedMDNode* pNode):
        m_pNode(pNode),
        m_isDirty(false),
        m_isLoaded(false)
    {}

    NamedMDNodeList():
        m_pNode(NULL),
        m_isDirty(false),
        m_isLoaded(true)
    {}

    size_t size() const
    {
        lazyLoad();
        return m_data.size();
    }

    bool empty() const
    {
        lazyLoad();
        return m_data.empty();
    }

    iterator begin()
    {
        lazyLoad();
        return m_data.begin();
    }

    iterator end()
    {
        return m_data.end();
    }

    const_iterator begin() const
    {
        lazyLoad();
        return m_data.begin();
    }

    const_iterator end() const
    {
        return m_data.end();
    }

    void push_back(const item_type& val)
    {
        lazyLoad();
        m_data.push_back(val);
        m_isDirty = true;
    }

    item_type getItem( size_t index)
    {
        lazyLoad();
        return m_data[index];
    }

    void setItem( size_t index, const item_type& item)
    {
        lazyLoad();
        m_data[index] = item;
        m_isDirty = true;
    }

    iterator erase(iterator where)
    {
        lazyLoad();
        iterator i = m_data.erase(where);
        m_isDirty = true;
        return i;
    }

    void save(llvm::LLVMContext &context, llvm::NamedMDNode* pNode) const
    {
        if(!dirty())
            return;

        assert( m_isLoaded && "Collection should be loaded at this point (since it is dirty)");

        if(pNode->getNumOperands() > size() )
            pNode->dropAllReferences();

        meta_iterator mi(pNode,0);
        meta_iterator me(pNode);
        const_iterator i = begin();
        const_iterator e = end();

        while( i != e || mi != me )
        {
            if( i != e && mi != me )
            {
                Traits::save(context, *mi, *i);
                ++i;
                ++mi;
            }
            else
            {
                assert( i != e && mi == me );
                pNode->addOperand(llvm::cast<llvm::MDNode>(Traits::generateValue(context, *i)));
                ++i;
            }
        }
    }

    bool dirty() const
    {
        if( m_isDirty )
        {
            return true;
        }

        if( !m_isLoaded )
        {
            return false;
        }

        for( const_iterator i = m_data.begin(), e = m_data.end(); i != e; ++i )
        {
            if( Traits::dirty(*i) )
                return true;
        }
        return false;
    }

    bool hasValue() const
    {
        return m_pNode != NULL || dirty();
    }

    void discardChanges()
    {
        if( !dirty() )
        {
            return;
        }

        for( iterator i = m_data.begin(), e = m_data.end(); i != e; ++i )
        {
            Traits::discardChanges(*i);
        }

        m_isDirty = false;
    }
private:

    void lazyLoad() const
    {
        if( m_isLoaded || NULL == m_pNode )
        {
            return;
        }

        for(meta_iterator i(m_pNode,0), e(m_pNode); i != e; ++i )
        {
            m_data.push_back(i.get());
        }

        m_isLoaded = true;
    }

private:
    const llvm::NamedMDNode* m_pNode;
    mutable std::vector<item_type> m_data;
    bool m_isDirty;
    mutable bool m_isLoaded;
};

bool isNamedNode(const llvm::Value* pNode, const char* name);

} //namespace
#endif
