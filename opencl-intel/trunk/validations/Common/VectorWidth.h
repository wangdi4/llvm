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

File Name:  VectorWidth.h

\*****************************************************************************/
#ifndef __VECTORWIDTH_H__
#define __VECTORWIDTH_H__

#include <map>
#include "llvm/System/DataTypes.h"      // LLVM data types
#include "Exception.h"

namespace Validation
{
  /// Number of components in vector.
  /// Notation V#
  /// # - number of components in vector.
  /// VectorWidth's first element must be 0 and last element must be INVALID_WIDTH
  enum VectorWidth
  {
    V1 = 0,
    V2,
    V3,
    V4,
    V8,
    V16,
    INVALID_WIDTH
  };

  /// helper wrapper class for 
  class VectorWidthWrapper
  {
  public:
    /// @brief default ctor
    /// Fills object with INVALID_WIDTH data which should be later filled with correct value
    VectorWidthWrapper() 
      : m_value(INVALID_WIDTH)
    {
      // init static members
      if (!m_isStaticInit) initStatic();
    }

    explicit VectorWidthWrapper(const VectorWidth& value) 
      : m_value(value) 
    { 
      // init static members
      if (!m_isStaticInit) initStatic(); 

      // need to there is metadata for value
      if (m_metaData.count(m_value) < 1)
      {
        throw Exception::InvalidArgument("Invalid arg. No metadata for this VectorWidth");
      }
    }

    VectorWidth GetValue() const { return m_value; }
    void SetValue(const VectorWidth& value) { m_value = value; }
    std::size_t GetSize() const { return m_metaData[m_value].m_size; }
    std::string ToString() const { return m_metaData[m_value].m_toString; }

    static VectorWidth ValueOf(const std::string& str)
    {
      // init static members
      if (!m_isStaticInit) initStatic(); 
      // need to notice that VectorWidth's first element must be 0 and last element must be INVALID_WIDTH
      // should put a comment in the VectorWidth enum definition
      for (int  i = 0; i < INVALID_WIDTH; i++)
      {
        VectorWidth vectorWidth = (VectorWidth) i;
        // should consider  comparing the strings converted to lower case to allow more flexibility
        if (m_metaData[vectorWidth].m_toString == str) {
          return vectorWidth;
        }
      }
      throw Exception::InvalidArgument("NonSupported Vector Width");
    }

    static VectorWidth ValueOf(const std::size_t& val)
    {
      // init static members
      if (!m_isStaticInit) initStatic(); 
      // need to notice that VectorWidth's first element must be 0 and last element must be INVALID_WIDTH
      // should put a comment in the VectorWidth enum definition
      for (int  i = 0; i < INVALID_WIDTH; i++)
      {
        VectorWidth vectorWidth = (VectorWidth) i;
        // should consider  comparing the strings converted to lower case to allow more flexibility
        if (m_metaData[vectorWidth].m_size == (std::size_t) val) {
          return vectorWidth;
        }
      }
      throw Exception::InvalidArgument("NonSupported Vector Width");
    }

  private:
    ///// Hide assignment operator for BufferAccessor
    // VectorWidthWrapper & operator=(const VectorWidthWrapper &ba) {}
    /// VectorWidth value
    VectorWidth m_value;

    /// init static members
    void static initStatic();

    /// helper class to store associated data with VectorWidth
    class VectorWidthMetadata
    {
    public:
      VectorWidthMetadata(std::size_t size, const std::string& toString) 
        : m_size(size), m_toString(toString) {}

      VectorWidthMetadata() : m_size(0), m_toString(""){}

      /// Number of elements in vector.
      std::size_t m_size;
      /// VectorWidth value as string
      std::string m_toString;
    };

    /// static map from NEATValue to its Metadata
    static std::map<VectorWidth, VectorWidthMetadata> m_metaData;

    /// is static members initialized 
    static bool m_isStaticInit;

  };

} // namespace Validation
#endif // __VECTORWIDTH_H__
