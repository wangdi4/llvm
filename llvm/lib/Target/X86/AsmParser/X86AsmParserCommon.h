//===-- X86AsmParserCommon.h - Common functions for X86AsmParser ---------===//
//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_ASMPARSER_X86ASMPARSERCOMMON_H
#define LLVM_LIB_TARGET_X86_ASMPARSER_X86ASMPARSERCOMMON_H

#include "llvm/Support/MathExtras.h"

namespace llvm {

inline bool isImmSExti16i8Value(uint64_t Value) {
  return isInt<8>(Value) ||
         (isUInt<16>(Value) && isInt<8>(static_cast<int16_t>(Value)));
}

inline bool isImmSExti32i8Value(uint64_t Value) {
  return isInt<8>(Value) ||
         (isUInt<32>(Value) && isInt<8>(static_cast<int32_t>(Value)));
}

inline bool isImmSExti64i8Value(uint64_t Value) {
  return isInt<8>(Value);
}

inline bool isImmSExti64i32Value(uint64_t Value) {
  return isInt<32>(Value);
}

inline bool isImmUnsignedi8Value(uint64_t Value) {
  return isUInt<8>(Value) || isInt<8>(Value);
}

#if INTEL_CUSTOMIZATION
inline bool isImmUnsignedi1Value(uint64_t Value) {
  return isUInt<1>(Value);
}

inline bool isImmUnsignedi2Value(uint64_t Value) {
  return isUInt<2>(Value);
}
#endif // INTEL_CUSTOMIZATION

inline bool isImmUnsignedi4Value(uint64_t Value) {
  return isUInt<4>(Value);
}

} // End of namespace llvm

#endif
