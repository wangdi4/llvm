#include "SelectLower.h"
#include "Logger.h"
#include <sstream>
namespace intel {

bool SelectLower::runOnFunction(Function &F) {
  // For each Basic Block
  std::vector<SelectInst*> selects;

  // Find all selects
  for (Function::iterator bb = F.begin(), bbe = F.end(); bb != bbe ; ++bb) {
    BasicBlock *block = bb;
    for (BasicBlock::iterator it = block->begin(), e = block->end(); it != e ; ++it) {
      if (SelectInst* sl = dyn_cast<SelectInst>(it)) {
        if (dyn_cast<VectorType>(sl->getCondition()->getType())) {
          selects.push_back(sl);
        }
      }
    }
  }

  // For each select
  for (std::vector<SelectInst*>::iterator it = selects.begin(), e = selects.end(); it != e; ++it) {  
    SelectInst* select = *it;
    const VectorType *tp = dyn_cast<VectorType>(select->getType());
    unsigned numelem = tp->getNumElements();

    Value* new_return =  UndefValue::get(select->getType());
    // for each component
    for (unsigned i=0; i< numelem; ++i) {
      // create regular select
      Constant* index = ConstantInt::get(Type::getInt32Ty(F.getParent()->getContext()), i);
      Value* A    = ExtractElementInst::Create(select->getTrueValue() , index, "A"   , select);
      Value* B    = ExtractElementInst::Create(select->getFalseValue(), index, "B"   , select);
      Value* cond = ExtractElementInst::Create(select->getCondition() , index, "cond", select);
      Value* new_sel = SelectInst::Create(cond, A, B, select->getName() + "_comp", select); 
      new_return = InsertElementInst::Create(new_return, new_sel , index, "merge", select);
    }
    select->replaceAllUsesWith(new_return);
  }

  return true;
}


} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createSelectLowerPass() {
    return new intel::SelectLower();
  }
}

char intel::SelectLower::ID = 0;
static RegisterPass<intel::SelectLower> CLISelectLowerX("selectlower", "Lower vector selects to scalar selects");

