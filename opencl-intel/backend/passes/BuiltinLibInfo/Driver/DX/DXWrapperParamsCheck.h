// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

/**
 * @file   DXWrapperParamsCheck.h
 * @date   Thu march 31 
 * 
 */

#ifndef __DXWRAPPERPARAMSCHECK_H__
#define __DXWRAPPERPARAMSCHECK_H__

#include "RuntimeServices.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace intel {

class DXWrapperParamsCheck {
  // checks for compatability of scalarized \ packetized types
public:

  enum DXWrapperRetType
  {
    DX_RET_VOID,
    DX_RET_VEC,
    DX_RET_SOA,
    DX_RET_ILLEGAL
  };

  /// @brief checks whether types of the DX wrapper should be handled as same scalar type
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @returns whether types of the DX wrapper should be handled as same scalar type
  static bool checkSameTypeScalar(Type* packetizeType, Type* scalarizeType);

  /// @brief checks whether types of the DX wrapper should be handled as same constant type
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeVal scalarized value
  /// @returns whether types of the DX wrapper should be handled as same constant value
  static bool checkSameTypeConstant(Type* packetizeType, Value* scalarizeVal);
  
  /// @brief checks whether types of the DX wrapper should be handled scalar -> vector
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @param packetWidth number of work items correspond tp SOA4 SOA8
  /// @returns whether types of the DX wrapper should be handled as scalar -> vector
  static bool checkVec(Type* packetizeType, Type* scalarizeType, unsigned packetWidth);
  
  
  /// @brief checks whether types of the DX wrapper should be handled vector -> SOA
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @param packetWidth number of work items correspond tp SOA4 SOA8
  /// @returns whether types of the DX wrapper should be handled as vector -> SOA
  static bool checkSOA(Type* packetizeType, Type* scalarizeType, unsigned packetWidth);

  /// @brief checks which type of return (void , scalar\vec , vec\SOA , illegal)
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @param packetWidth number of work items correspond tp SOA4 SOA8
  /// @returns enumuration of return type
  static DXWrapperRetType checkRet(Type* packetizeType, Type* scalarizeType, unsigned packetWidth);

  /// @brief checks if scalar wrapper is masked
  /// @param CI call instruction to be checked
  /// @param runtime services containing the hash to find packetized versoin of scalar function
  /// @returns whether scalar wrapper is masked
  static bool isDXScalarWrapperMasked (CallInst* CI, const RuntimeServices *rtServices);


};

} //namespace

#endif // __PACKETIZER_H__
