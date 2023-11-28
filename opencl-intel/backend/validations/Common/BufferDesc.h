// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __BUFFER_DESC_H__
#define __BUFFER_DESC_H__

#include "DataType.h"
#include "IMemoryObjectDesc.h"
#include "NEATValue.h"
#include "TypeDesc.h"
#include "VectorWidth.h"
#include "llvm/Support/DataTypes.h" // LLVM data types
#include <cstddef>                  // for std::size_t

namespace Validation {
/// @brief Buffer description structure.
/// Describes the data which is stored into a buffer.
class BufferDesc : public IMemoryObjectDesc {
public:
  /// @brief default ctor.
  /// Fills object with INVALID data which should be later filled with correct
  /// values Default ctor is enabled for reading/writing object to file
  BufferDesc() : m_numOfElements(0), m_elemenType() {}

  /// @brief ctor of buffer description for buffer of vector data types.
  /// @param in_numOfVectors - number of vectors in buffer
  /// @param in_vw - Vector Width i.e. Number of elements in vector
  ///                from enum VectorWidth. V1, ... V16
  /// @param in_dt - data type of elements in buffer
  /// @param in_isNEAT - this buffer contains NEAT intervals
  BufferDesc(const std::size_t in_numOfVectors, const VectorWidth in_vw,
             const DataTypeVal in_dt, const bool in_isNEAT = false)
      : m_numOfElements(in_numOfVectors) {
    VectorWidthWrapper vww(in_vw);
    // There is no 1-element vector data types!
    // Assume that user wants create buffer of scalar elements.
    // i.e. int1 = int;
    if (vww.GetSize() == 1) {
      m_elemenType.SetNumOfSubTypes(0);
      m_elemenType.SetType(ConvertDataType(in_dt));
    } else {
      m_elemenType = TypeDesc(TVECTOR, vww.GetSize());
      TypeDesc vectorElemDesc;
      vectorElemDesc.SetNumOfSubTypes(0);
      TypeVal vectorElemType = ConvertDataType(in_dt);
      vectorElemDesc.SetType(vectorElemType);
      m_elemenType.SetSubTypeDesc(0, vectorElemDesc);
    }
    m_elemenType.SetNeat(in_isNEAT);
  }

  /// @brief Computes buffer size.
  std::size_t GetSizeInBytes() const override {
    return m_numOfElements * m_elemenType.GetSizeInBytes();
  }

  TypeDesc GetElementDescription() const { return m_elemenType; }
  void SetElementDecs(TypeDesc in_elemType) { m_elemenType = in_elemType; }

  virtual bool IsNEAT() const override { return m_elemenType.IsNEAT(); }
  virtual void SetNeat(const bool inNEAT) override {
    m_elemenType.SetNeat(inNEAT);
  }

  virtual IMemoryObjectDesc *Clone() const override {
    return new BufferDesc(*this);
  }

  virtual std::string ToString() { return ("BufferDescriptor"); }

  /// @brief get Name of class
  virtual std::string GetName() const override { return GetBufferDescName(); }

  /// @brief get static Name of class
  static std::string GetBufferDescName() { return "BufferDesc"; }

  /// Get number of elements stored in Buffer
  std::size_t NumOfElements() const { return m_numOfElements; }
  /// Set number of elements stored in Buffer
  /// @param in_num - number of elements
  void SetNumOfElements(const std::size_t in_num) { m_numOfElements = in_num; }

  // TODO: This method works only for buffer of vectors. Replace it with more
  // general method, which will work for all types of buffers.
  /// Get number of elements in vector.
  // For basic data types returns 1. If it's called for buffer of structures,
  // this method will throw an exception. WARNING!!! This method is deprecated.
  std::size_t SizeOfVector() const {
    // HACK!!! TODO: Remove all calls of that method for non-vector data types.
    if (m_elemenType.GetType() == TVECTOR) {
      return m_elemenType.GetNumberOfElements();
    } else if (!m_elemenType.IsComposite()) {
      return 1;
    } else {
      throw Exception::IllegalFunctionCall(
          "SizeOfVector can be call only for buffer with vector elements! "
          "Currently buffer holds elements of " +
          m_elemenType.TypeToString());
    }
  }

  /// if BufferDesc has floating point values
  inline bool IsFloatingPoint() const { return m_elemenType.IsFloatingPoint(); }

  inline bool operator==(const BufferDesc &a) const {
    bool ret = (m_numOfElements == a.NumOfElements());
    ret &= (m_elemenType == a.m_elemenType);
    return ret;
  }

  inline bool operator!=(const BufferDesc &a) const { return !(*this == a); }

private:
  // TODO: Remove that method after deletion of old legacy code which uses old
  // BufferDesc.
  TypeVal ConvertDataType(DataTypeVal dtv) const {
    TypeVal ret = UNSPECIFIED_TYPE;
    switch (dtv) {
    case F16:
      ret = THALF;
      break;
    case F32:
      ret = TFLOAT;
      break;
    case F64:
      ret = TDOUBLE;
      break;
    case I8:
      ret = TCHAR;
      break;
    case I16:
      ret = TSHORT;
      break;
    case I32:
      ret = TINT;
      break;
    case I64:
      ret = TLONG;
      break;
    case U8:
      ret = TUCHAR;
      break;
    case U16:
      ret = TUSHORT;
      break;
    case U32:
      ret = TUINT;
      break;
    case U64:
      ret = TULONG;
      break;
    default:
      ret = INVALID_TYPE;
    }
    return ret;
  }

  std::size_t m_numOfElements; ///< Number of elements in Buffer. Each element
                               ///< could be scalar, vector or structure.
  TypeDesc m_elemenType;
};

} // namespace Validation
#endif // __BUFFER_DESC_H__
