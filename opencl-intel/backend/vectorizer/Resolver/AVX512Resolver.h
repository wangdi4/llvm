// INTEL CONFIDENTIAL
//
// Copyright 2016-2018 Intel Corporation.
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

#ifndef __MICRESOLVER_H_
#define __MICRESOLVER_H_

#include "Mangler.h"
#include "Resolver.h"

using namespace llvm;

namespace intel {

class AVX512Resolver : public FuncResolver {
public:
  // Pass identification, replacement for typeid
  static char ID;
  /// @brief C'tor
  AVX512Resolver() : FuncResolver(ID) {}

  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "AVX512Resolver";
  }

  /// @brief Resolve a call-site. This is a target specific hook.
  /// @param caller Instruction to resolve
  /// @return true if this call was handled by the resolver
  virtual bool TargetSpecificResolve(CallInst* caller);

  virtual bool isBitMask(const VectorType& vecType) const;
private:
  /// @brief Create gather/scatter call instruction and replace given caller instruction with the new one.
  /// @param caller Instruction to resolve
  /// @param Mask mask value
  /// @param Ptr base address value
  /// @param Index index value
  /// @param Data data to store value for scatter (NULL for gather)
  /// @return the new call instruction for gather/scatter
  Instruction* CreateGatherScatterAndReplaceCall(CallInst* caller, Value *Mask, Value *Ptr, Value *Index, Value *Data, Mangler::GatherScatterType type);

  /// @brief Fix base address and index of gather/scatter instruction if needed to be fixed.
  /// @param caller Instruction to resolve
  /// @param Mask mask value
  /// @param ValidBits value represents max number of valid bits in index
  /// @param IsSigned bit value, 1 - if index value is signed, 0 - otherwise
  /// @param Ptr base address value to fix (IN/OUT)
  /// @param Index index value to fix (IN/OUT)
  void FixBaseAndIndexIfNeeded(
    CallInst* caller,
    Value *Mask,
    Value *ValidBits,
    Value *IsSigned,
    Value *&Ptr,
    Value *&Index);
};

}
#endif //define __MICRESOLVER_H_
