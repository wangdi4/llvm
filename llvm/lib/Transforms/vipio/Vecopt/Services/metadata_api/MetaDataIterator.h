/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef METADATAITERATOR_H
#define METADATAITERATOR_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Atomic.h"
#include "MetaDataTraits.h"

namespace Intel
{
///
// Iterator over the meta data nodes list. It is assumed that
// all the nodes are of the same type ( as specified by the T template parameter)
// Template parameters:
// T - type of the entry node
// N - type of the root(parent) node (supported types are MDNode and NamedMDNode )
// C - traits type (see the MDValueTraits )
//
template<class T, class N = llvm::MDNode, class C = MDValueTraits<T> >
class MetaDataIterator
{
public:
    typedef typename C::value_type value_type;
    typedef MetaDataIterator<T,N,C> _Myt;

    ///
    // Ctor. Creates the sentinel iterator. Usually used as an end iterator
    //
    explicit MetaDataIterator( const N* pNode):
        m_pNode(pNode),
        m_index(pNode->getNumOperands())
    {
    }

    ///
    // Ctor. Create the iterator on given index
    explicit MetaDataIterator( const N* pNode, unsigned int index):
        m_pNode(pNode),
        m_index(index)
    {
        assert( index <= pNode->getNumOperands());
    }


    llvm::Value* operator *()
    {
// xmain FIXME throwing exceptions not supported
#if 0
        if( isNil() )
          throw std::exception();
#endif

        return m_pNode->getOperand(m_index);
    }
    ///
    // returns the current item in the list
    value_type get()
    {
// xmain FIXME throwing exceptions not supported
#if 0
        if( isNil() )
            throw std::exception();
#endif


        return C::load(m_pNode->getOperand(m_index));
    }

    _Myt& operator++()
    {
        if( !isNil() )
            ++m_index;
        return (*this);
    }

    _Myt operator++(int)
    {
        _Myt tmp = *this;
        ++*this;
        return tmp;
    }

    bool operator == (const _Myt& rhs )
    {
        return m_pNode == rhs.m_pNode &&
            m_index == rhs.m_index;
    }

    bool operator != (const _Myt& rhs )
    {
        return !this->operator==(rhs);
    }

private:
    bool isNil()
    {
        return m_index == m_pNode->getNumOperands();
    }

private:
    const N* m_pNode; // pointer to the parent node
    unsigned int m_index;
};

} //namespace
#endif
