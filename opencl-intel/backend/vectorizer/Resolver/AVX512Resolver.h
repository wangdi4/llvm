// Copyright (c) 2016 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly.

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
