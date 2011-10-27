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

File Name:  NEATVector.h

\*****************************************************************************/
#ifndef NEAT_VECTOR_H
#define NEAT_VECTOR_H

#include "NEATValue.h"
#include "VectorWidth.h"
#define MAX_VECTOR_WIDTH 16

namespace Validation
{
  struct NEATVector
  {
  public:
    // default ctor. to enable inserting into structures with default ctor
    NEATVector() {}

    NEATVector(VectorWidth width);

    ~NEATVector();
    void SetWidth(VectorWidth width);
    VectorWidth GetWidth() const;
    size_t GetSize() const;
    NEATValue& operator[](int i);
    const NEATValue& operator[](int i) const;
  private:
    VectorWidthWrapper m_Width;
    NEATValue m_Values[MAX_VECTOR_WIDTH];
  };
}

#endif // NEAT_VECTOR_H