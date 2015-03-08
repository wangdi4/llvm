/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef METADATAVALUE_H
#define METADATAVALUE_H

#include "MetaDataTraits.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Metadata.h"

namespace Intel
{
///
// Represents the meta data value stored using the positional schema
// The root node is actuall storing the value
template<class T, class Traits = MDValueTraits<T> >
class MetaDataValue
{
public:
    typedef typename Traits::value_type value_type;

    MetaDataValue(llvm::Value* pNode):
        m_pNode(pNode),
        m_value(Traits::load(pNode)),
        m_isDirty(false)
    {
    }

    MetaDataValue():
        m_pNode(NULL),
        m_value(),
        m_isDirty(false)
    {
    }

    MetaDataValue(const value_type& val):
        m_pNode(NULL),
        m_value(val),
        m_isDirty(true)
    {
    }

    operator value_type()
    {
        return m_value;
    }

    value_type get()
    {
        return m_value;
    }

    void set(const value_type&  val)
    {
        m_value = val;
        m_isDirty = true;
    }

    bool dirty() const
    {
        return m_isDirty;
    }

    bool hasValue() const
    {
        return m_pNode != NULL || dirty();
    }

    void discardChanges()
    {
        m_isDirty = false;
    }

    void save(llvm::LLVMContext &context, llvm::Value* pNode) const
    {
        if( m_pNode == pNode && !dirty() )
        {
            // we are saving to our own node but nothing was changed
            return;
        }

        if( !hasValue() )
        {
            return;
        }

        pNode->replaceAllUsesWith(generateNode(context));
    }

    llvm::Value* generateNode(llvm::LLVMContext &context) const
    {
        if( !hasValue() )
        {
            return NULL;
        }
        
        return Traits::generateValue(context, m_value);
    }

private:
    llvm::Value* m_pNode;
    value_type m_value;
    bool m_isDirty;
};


///
// Represents the meta data value stored using the 'named' schema
// The root node should have two operands nodes:
//  - the first one is MDString storing the name
//  - the second one is actual value
template<class T, class Traits = MDValueTraits<T> >
class NamedMetaDataValue
{
public:
    typedef typename Traits::value_type value_type;

    NamedMetaDataValue( llvm::Value* pNode):
        m_pNode(pNode),
        m_id(getIdNode(pNode)),
        m_value(getValueNode(pNode))
    {
    }

    NamedMetaDataValue( const char* name ):
        m_pNode(NULL),
        m_id(name)
    {
    }

    NamedMetaDataValue( const char* name, const value_type& val):
        m_pNode(NULL),
        m_id(name),
        m_value(val)
    {
    }

    operator value_type()
    {
        return (value_type)m_value;
    }

    value_type get()
    {
        return m_value.get();
    }

    void set(const value_type&  val)
    {
        m_value.set(val);
    }

    bool dirty() const
    {
        return m_value.dirty();
    }

    bool hasValue() const
    {
        return m_value.hasValue();
    }

    void discardChanges()
    {
        m_value.discardChanges();
    }

    void save(llvm::LLVMContext &context, llvm::Value* pNode) const
    {
        if( m_pNode == pNode && !dirty() )
        {
            return;
        }

        if( !hasValue() )
        {
            return;
        }

        llvm::MDNode* pMDNode = llvm::dyn_cast<llvm::MDNode>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pMDNode)
        {
            throw "Named value parent node is not of MDNode type";
        }
#endif

        if(pMDNode->getNumOperands() != 2)
        {
            pMDNode->replaceAllUsesWith(generateNode(context));
            return;
        }

        m_id.save(context, pMDNode->getOperand(0));
        m_value.save(context, pMDNode->getOperand(1));
    }

    llvm::Value* generateNode(llvm::LLVMContext &context) const
    {
        llvm::SmallVector< llvm::Value*, 2> args;

        args.push_back( m_id.generateNode(context));
        args.push_back( m_value.generateNode(context));

        return llvm::MDNode::get(context,args);
    }
private:
    llvm::Value* getIdNode(const llvm::Value* pNode)
    {
        if( NULL == pNode)
        {
            return NULL; //this is allowed for optional nodes
        }

        const llvm::MDNode* pMDNode = llvm::dyn_cast<const llvm::MDNode>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pMDNode)
        {
            throw "Named value parent node is not of MDNode type";
        }
#endif

// xmain FIXME throwing exceptions not supported
#if 0
        if( pMDNode->getNumOperands() < 1)
        {
            throw "Named value doesn't have a name node";
        }
#endif

        llvm::MDString* pIdNode = llvm::dyn_cast<llvm::MDString>(pMDNode->getOperand(0));

// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pIdNode )
        {
            throw "Named list id node is not a string";
        }
#endif

        return pIdNode;
    }

    llvm::Value* getValueNode(const llvm::Value* pNode)
    {
        if( NULL == pNode)
        {
            return NULL; //this is allowed for optional nodes
        }

        const llvm::MDNode* pMDNode = llvm::dyn_cast<const llvm::MDNode>(pNode);
// xmain FIXME throwing exceptions not supported
#if 0
        if( NULL == pMDNode)
        {
            throw "Named value parent node is not of MDNode type";
        }
#endif

// xmain FIXME throwing exceptions not supported
#if 0
        if( pMDNode->getNumOperands() < 2)
        {
            throw "Named value doesn't have a value node";
        }
#endif

        return pMDNode->getOperand(1);
    }

private:
    llvm::Value* m_pNode; // root node initialized during the load
    MetaDataValue<std::string>  m_id;
    MetaDataValue<T,Traits> m_value;
};


} //namespace
#endif
