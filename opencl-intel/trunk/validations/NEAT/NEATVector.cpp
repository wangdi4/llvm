/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  NEATVector.cpp

\*****************************************************************************/
#include "NEATVector.h"

namespace Validation
{

    NEATVector::NEATVector(VectorWidth width):
    m_Width(width)
    {

    }

    NEATVector::~NEATVector()
    {
    }

    size_t NEATVector::GetSize() const
    {
        return m_Width.GetSize();
    }

    VectorWidth NEATVector::GetWidth() const
    {
        return m_Width.GetValue();
    }

    void NEATVector::SetWidth( VectorWidth in_width)
    {
        m_Width.SetValue(in_width);
    }

    NEATValue& NEATVector::operator[](int i)
    {
        if(m_Width.GetValue() == INVALID_WIDTH)
            throw Exception::IllegalFunctionCall("Trying to get value from NEATVector with unspecified type");
        if(m_Width.GetSize()<=(size_t)i)
            throw Exception::OutOfRange("Index is out of range");
        return m_Values[i];
    }

    const NEATValue& NEATVector::operator[](int i) const
    {
      if(m_Width.GetValue() == INVALID_WIDTH)
        throw Exception::IllegalFunctionCall("Trying to get value from NEATVector with unspecified type");
      if(m_Width.GetSize()<=(size_t)i)
        throw Exception::OutOfRange("Index is out of range");
      return m_Values[i];
    }
}
