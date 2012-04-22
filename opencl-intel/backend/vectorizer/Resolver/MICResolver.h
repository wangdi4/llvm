#ifndef __MICRESOLVER_H_
#define __MICRESOLVER_H_
#include "Resolver.h"

using namespace llvm;

namespace intel {

class MICResolver : public FuncResolver {
public:
  // Pass identification, replacement for typeid
  static char ID;
  /// @brief C'tor
  MICResolver() : FuncResolver(ID) {}

  /// @brief Provides name of pass
  virtual const char *getPassName() const {
    return "MICResolver";
  }

  /// @brief Resolve a call-site. This is a target specific hook.
  /// @param caller Instruction to resolve
  /// @return true if this call was handled by the resolver
  virtual bool TargetSpecificResolve(CallInst* caller);
};

}
#endif //define __MICRESOLVER_H_
