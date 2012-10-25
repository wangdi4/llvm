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

  File Name: MetaDataApiUtils.h

  \****************************************************************************/
#ifndef METADATAAPI_UTILS_H
#define METADATAAPI_UTILS_H

#include "llvm/Value.h"
#include "llvm/Metadata.h"
#include <list>
#include "MetaDataTraits.h"
#include "MetaDataValue.h"
#include "MetaDataObject.h"
#include "MetaDataIterator.h"

namespace Intel
{

///
// Metadata list. It is assumed that
// all the nodes are of the same type ( as specified by the T template parameter)
// Template parameters:
// T - type of the entry node
// C - convertor type (see the MDValueTraits )
//
template<class T, class C = MDValueTraits<T> >
class MetaDataList
{
public:
    typedef MetaDataList<T,C> _Myt;
    typedef MetaDataIterator<T, llvm::MDNode, C> meta_iterator;
    typedef typename C::value_type item_type;
    typedef typename std::list<item_type>::iterator iterator;
    typedef typename std::list<item_type>::const_iterator const_iterator;

    MetaDataList(const llvm::MDNode* pNode):
        m_pNode(pNode),
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
            if( C::dirty(*i) )
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
            C::discardChanges(*i);
        }

        m_isDirty = false;
    }

    virtual void save(llvm::LLVMContext &context, llvm::MDNode* pNode) const
    {
        if( m_pNode == pNode && !dirty() )
        {
            return;
        }

        if(pNode->getNumOperands() != size() )
        {
            pNode->replaceAllUsesWith(generateNode(context));
            llvm::MDNode::deleteTemporary(pNode);
            return;
        }

        meta_iterator mi(pNode,0);
        meta_iterator me(pNode);
        const_iterator i = begin();
        const_iterator e = end();

        for(; i != e; ++i, ++mi )
        {
            C::save(context, *mi, *i);
        }

    }

    virtual llvm::Value* generateNode(llvm::LLVMContext &context) const
    {
        llvm::SmallVector< llvm::Value*, 5> args;

        for( const_iterator i = m_data.begin(), e = m_data.end(); i != e; ++i )
        {
            args.push_back( C::generateValue(context, *i));
        }

        return llvm::MDNode::get(context,args);
    }

protected:

    virtual unsigned int getStartIndex() const
    {
        return 0;
    }

    virtual void lazyLoad() const
    {
        if( m_isLoaded || NULL == m_pNode )
        {
            return;
        }

        for(meta_iterator i(m_pNode, getStartIndex()), e(m_pNode); i != e; ++i )
        {
            m_data.push_back(i.get());
        }

        m_isLoaded = true;
    }


protected:
    const llvm::MDNode* m_pNode;
    bool m_isDirty;
    mutable bool m_isLoaded;
    mutable std::list<item_type> m_data;
};

template<class T, class C = MDValueTraits<T> >
class MetaDataNamedList: public MetaDataList<T,C>
{
public:
    typedef MetaDataList<T,C> Super;
    typedef MetaDataIterator<T, llvm::MDNode, C> meta_iterator;
    typedef typename C::value_type item_type;
    typedef typename std::list<item_type>::iterator iterator;
    typedef typename std::list<item_type>::const_iterator const_iterator;

    MetaDataNamedList(const llvm::MDNode* pNode):
        Super(pNode),
        m_id(getIdNode(pNode))
    {}

    MetaDataNamedList(const char* name):
        Super(),
        m_id(name)
    {}

    llvm::StringRef getId()
    {
        return m_id.get();
    }

    void save(llvm::LLVMContext &context, llvm::MDNode* pNode) const
    {
        if( Super::m_pNode == pNode && !Super::dirty() )
        {
            return;
        }

        if(pNode->getNumOperands() != Super::size() + 1) // +1 for the name node
        {
            pNode->replaceAllUsesWith(generateNode(context));
            llvm::MDNode::deleteTemporary(pNode);
            return;
        }

        m_id.save(context, pNode->getOperand(0));

        meta_iterator mi(pNode,1);
        meta_iterator me(pNode);
        const_iterator i = Super::begin();
        const_iterator e = Super::end();

        for(; i != e; ++i, ++mi )
        {
            C::save(context, *mi, *i);
        }
    }

    llvm::Value* generateNode(llvm::LLVMContext &context) const
    {
        llvm::SmallVector< llvm::Value*, 5> args;

        args.push_back( m_id.generateNode(context));

        for( const_iterator i = Super::m_data.begin(), e = Super::m_data.end(); i != e; ++i )
        {
            args.push_back( C::generateValue(context, *i));
        }

        return llvm::MDNode::get(context,args);
    }

protected:

    llvm::Value* getIdNode(const llvm::MDNode* pNode)
    {
        if( NULL == pNode)
        {
            return NULL; //this is allowed for optional nodes
        }

        if( !pNode->getNumOperands() )
        {
            throw "Named list doesn't have a name node";
        }

        llvm::MDString* pIdNode = llvm::dyn_cast<llvm::MDString>(pNode->getOperand(0));

        if( NULL == pIdNode )
        {
            throw "Named list id node is not a string";
        }

        return pIdNode;
    }

    unsigned int getStartIndex() const
    {
        return 1;
    }


private:
    MetaDataValue<std::string>  m_id;
};


template<class T, class C = MDValueTraits<T> >
class NamedMDNodeList
{
public:
    typedef NamedMDNodeList<T,C> _Myt;
    typedef MetaDataIterator<T,llvm::NamedMDNode,C> meta_iterator;
    typedef typename C::value_type item_type;
    typedef typename std::list<item_type>::iterator iterator;
    typedef typename std::list<item_type>::const_iterator const_iterator;

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
                C::save(context, *mi, *i);
                ++i;
                ++mi;
            }
            else
            {
                assert( i != e && mi == me );
                pNode->addOperand(llvm::cast<llvm::MDNode>(C::generateValue(context, *i)));
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
            if( C::dirty(*i) )
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
            C::discardChanges(*i);
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
    mutable std::list<item_type> m_data;
    bool m_isDirty;
    mutable bool m_isLoaded;
};

bool isNamedNode(const llvm::Value* pNode, const char* name);

} //namespace
#endif
