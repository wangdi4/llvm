/*****************************************************************************
 Copyright (c) Intel Corporation (2010).

 INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
 LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
 ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
 PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
 DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
 PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
 including liability for infringement of any proprietary rights, relating to
 use of the code. No license, express or implied, by estoppel or otherwise,
 to any intellectual property rights is granted herein.

 \*****************************************************************************/
/**
 * @file   DXWrapperParamsCheck.h
 * @author ran 
 * @date   Thu march 31 
 * 
 */

#ifndef __DXWRAPPERPARAMSCHECK_H__
#define __DXWRAPPERPARAMSCHECK_H__

#include "llvm/Instructions.h"
#include "RuntimeServices.h"
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
  static bool checkSameTypeScalar(const Type* packetizeType, const Type* scalarizeType);

  /// @brief checks whether types of the DX wrapper should be handled as same constant type
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeVal scalarized value
  /// @returns whether types of the DX wrapper should be handled as same constant value
  static bool checkSameTypeConstant(const Type* packetizeType, Value* scalarizeVal);
  
  /// @brief checks whether types of the DX wrapper should be handled scalar -> vector
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @param packetWidth number of work items correspond tp SOA4 SOA8
  /// @returns whether types of the DX wrapper should be handled as scalar -> vector
  static bool checkVec(const Type* packetizeType, const Type* scalarizeType, unsigned packetWidth);
  
  
  /// @brief checks whether types of the DX wrapper should be handled vector -> SOA
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @param packetWidth number of work items correspond tp SOA4 SOA8
  /// @returns whether types of the DX wrapper should be handled as vector -> SOA
  static bool checkSOA(const Type* packetizeType, const Type* scalarizeType, unsigned packetWidth);

  /// @brief checks which type of return (void , scalar\vec , vec\SOA , illegal)
  /// @param packetizeType packetized parameter Type 
  /// @param scalarizeType scalarized parameter Type 
  /// @param packetWidth number of work items correspond tp SOA4 SOA8
  /// @returns enumuration of return type
  static DXWrapperRetType checkRet(const Type* packetizeType, const Type* scalarizeType, unsigned packetWidth);

  /// @brief checks if scalar wrapper is masked
  /// @param CI call instruction to be checked
  /// @param runtime services containing the hash to find packetized versoin of scalar function
  /// @returns whether scalar wrapper is masked
  static bool isDXScalarWrapperMasked (CallInst* CI, const RuntimeServices *rtServices);


};

} //namespace

#endif // __PACKETIZER_H__
