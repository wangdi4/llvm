#ifndef __INST_TYPE_NAMER_H_
#define __INST_TYPE_NAMER_H_

// This pass is a development utility that sets name to values according 
// to it's type and and opcode (for instructions) and potentiall of it's
// operands.
// the main benefit of that pass is to make comparsion of ll files easier
// since without the matching of the diff viewer is based on the random 
// names assigned by IR printer.
// Author: Ran Chachick

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

#include <vector>

using namespace llvm;

namespace intel {
class nameByInstType : public FunctionPass {
public:

  static char ID;

  bool m_nameAll;
  /// @brief C'tor
  nameByInstType(bool nameAll=true): FunctionPass(ID){
    m_nameAll = nameAll;
  };
  /// @brief D'tor
  ~nameByInstType(){}
  /// @brief Provides name of pass
  virtual llvm::StringRef getPassName() const {
    return "nameByInstType";
  }
  
  ///s@brief LLVM interface.
  virtual bool runOnFunction(Function &F) ;

  ///@brief public API to be use not as a pass.
  void RenameValues(Function &F);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {}
};
}
#endif //define __INST_TYPE_NAMER_H_
