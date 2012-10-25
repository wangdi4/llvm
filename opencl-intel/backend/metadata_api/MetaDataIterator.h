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

  File Name: MetaDataApi.h

  \****************************************************************************/
#ifndef METADATAITERATOR_H
#define METADATAITERATOR_H

#include "llvm/Value.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Metadata.h"
#include "llvm/Support/Atomic.h"
#include <list>
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
        if( isNil() )
            throw std::exception();

        return m_pNode->getOperand(m_index);
    }
    ///
    // returns the current item in the list
    value_type get()
    {
        if( isNil() )
            throw std::exception();

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
