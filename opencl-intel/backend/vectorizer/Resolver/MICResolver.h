#ifndef __MICRESOLVER_H_
#define __MICRESOLVER_H_
#include "Resolver.h"
#include "Mangler.h"

using namespace llvm;

namespace intel {

class MICResolver : public FuncResolver {
public:
  // Pass identification, replacement for typeid
  static char ID;
  /// @brief C'tor
  MICResolver() : FuncResolver(ID) {}

  /// @brief Provides name of pass
  virtual StringRef getPassName() const {
    return "MICResolver";
  }

  /// @brief Resolve a call-site. This is a target specific hook.
  /// @param caller Instruction to resolve
  /// @return true if this call was handled by the resolver
  virtual bool TargetSpecificResolve(CallInst* caller);

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
