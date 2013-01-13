//===- llvm/Analysis/SPIRVerifier.h - SPIR IR Verifier ----------*- C++ -*-===//
// 
// Copyright (c) 2012 The Khronos Group Inc.  All rights reserved.
//
// NOTICE TO KHRONOS MEMBER:
//
// AMD has assigned the copyright for this object code to Khronos.
// This object code is subject to Khronos ownership rights under U.S. and
// international Copyright laws.
//
// Permission is hereby granted, free of charge, to any Khronos Member
// obtaining a copy of this software and/or associated documentation files
// (the "Materials"), to use, copy, modify and merge the Materials in object
// form only and to publish, distribute and/or sell copies of the Materials
// solely in object code form as part of conformant OpenCL API implementations,
// subject to the following conditions:
//
// Khronos Members shall ensure that their respective ICD implementation,
// that is installed over another Khronos Members' ICD implementation, will
// continue to support all OpenCL devices (hardware and software) supported
// by the replaced ICD implementation. For the purposes of this notice, "ICD"
// shall mean a library that presents an implementation of the OpenCL API for
// the purpose routing API calls to different vendor implementation.
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Materials.
//
// KHRONOS AND AMD MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
// SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
// IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND AMD DISCLAIM ALL WARRANTIES
// WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
// IN NO EVENT SHALL KHRONOS OR AMD BE LIABLE FOR ANY SPECIAL, INDIRECT,
// INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
// FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
// NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
// THE USE OR PERFORMANCE OF THIS SOURCE CODE.
//
// U.S. Government End Users.   This source code is a "commercial item" as
// that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
// "commercial computer software" and "commercial computer software
// documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
// and is provided to the U.S. Government only as a commercial end item.
// Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
// 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
// source code with only those rights set forth herein.
// 
//===----------------------------------------------------------------------===//
//
// This file defines the function verifier interface, that can be used for some
// sanity checking of input to the system, and for checking that transformations
// haven't done something bad.
//
// This does not provide LLVM style verification. It instead assumes that the
// LLVM verifier has already been run and the IR is well formed.
//
// To see what specifically is checked, look at the top of SPIRVerifier.cpp
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_SPIR_VERIFIER_H
#define LLVM_ANALYSIS_SPIR_VERIFIER_H

#include <string>
#include "llvm/Analysis/Verifier.h"

  // SPIR Address space enumerations. See 2.2 of the SPIR spec for details.
namespace llvm {

class FunctionPass;
class Module;
class Function;

/// @brief An enumeration to specify the action to be taken if errors found.
///
/// This enumeration is used in the functions below to indicate what should
/// happen if the verifier finds errors. Each of the functions that uses
/// this enumeration as an argument provides a default value for it. The
/// actions are listed below.
/*
enum VerifierFailureAction {
  AbortProcessAction,   ///< verifyModule will print to stderr and abort()
  PrintMessageAction,   ///< verifyModule will print to stderr and return true
  ReturnStatusAction    ///< verifyModule will just return true
};
*/

  // SPIR Address space enumerations. See 2.2 of the SPIR spec for details.
  enum AddressSpaces {
    SPIRAS_PRIVATE  = 0, // OpenCL Private address space
    SPIRAS_GLOBAL   = 1, // OpenCL Global address space
    SPIRAS_CONSTANT = 2, // OpenCL Constant address space
    SPIRAS_LOCAL    = 3, // OpenCL Local address space
    SPIRAS_GLOBAL_HOST = 4, // OpenCL Global address space
                            // with endian(host) attribute
    SPIRAS_CONSTANT_HOST = 5 // OpenCL Constant address space
                             // with endian(host) attribute
  };

  // Table 13, SPIR spec section 2.1.3.
  enum AddressingModes {
    CLK_ADDRESS_MIRRORED_REPEAT = 0,
    CLK_ADDRESS_REPEAT = 1,
    CLK_ADDRESS_CLAMP_TO_EDGE = 2,
    CLK_ADDRESS_CLAMP = 3,
    CLK_ADDRESS_NONE = 4
  };

  // Table 13, SPIR spec section 2.1.3.
  enum FilterMode {
    CLK_FILTER_NEAREST = 0,
    CLK_FILTER_LINEAR = 1
  };

  // Table 13, SPIR spec section 2.1.3.
  enum NormalizedCoords {
    CLK_NORMALIZED_COORDS_TRUE  = 0,
    CLK_NORMALIZED_COORDS_FALSE = 1
  };

/// @brief Create a verifier pass.
///
/// Check a Lightweight SPIR module for compatibility.  When the pass is used, the
/// action indicated by the \p action argument will be used if errors are
/// found. This pass checks to make sure that the features and version
/// are valid for the specific vendor.
FunctionPass *createLightweightSPIRVerifierPass(
  VerifierFailureAction action, ///< Action to take
  std::string CoreFeat,
  std::string KhrFeat,
  unsigned SPIRMajor, ///< Maximum supported SPIR Major version number.
  unsigned SPIRMinor, ///< Maximum supported SPIR Minor version number.
  unsigned OCLMajor, ///< Maximum supported OpenCL Major version number.
  unsigned OCLMinor ///< Maximum supported OpenCL Minor version number.
  );

/// Check a Heavyweight SPIR module for compatibility.  When the pass is used, the
/// action indicated by the \p action argument will be used if errors are
/// found. This pass checks to make sure that the features and version
/// are valid for the specific vendor. This pass also checks that the
/// SPIR binary complies to the SPIR specification.
FunctionPass *createHeavyweightSPIRVerifierPass(
  VerifierFailureAction action, ///< Action to take
  std::string CoreFeat,
  std::string KhrFeat,
  unsigned SPIRMajor, ///< Maximum supported SPIR Major version number.
  unsigned SPIRMinor, ///< Maximum supported SPIR Minor version number.
  unsigned OCLMajor, ///< Maximum supported OpenCL Major version number.
  unsigned OCLMinor ///< Maximum supported OpenCL Minor version number.
  );

} // End llvm namespace

#endif // LLVM_ANALYSIS_SPIR_VERIFIER_H_
