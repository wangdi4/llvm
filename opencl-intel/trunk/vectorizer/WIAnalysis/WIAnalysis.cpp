#include "WIAnalysis.h"
#include "llvm/Support/CommandLine.h"
#include "Mangler.h"
#include "llvm/Support/Debug.h"
#include <string>

namespace intel {

static cl::opt<bool>
PrintWiaCheck("print-wia-check", cl::init(false), cl::Hidden,
  cl::desc("Debug wia-check analysis"));

const unsigned int WIAnalysis::MinIndexBitwidthToPreserve = 16;

/// Define shorter names for dependencies, for clarity of the conversion maps
#define UNI WIAnalysis::UNIFORM
#define SEQ WIAnalysis::CONSECUTIVE
#define PTR WIAnalysis::PTR_CONSECUTIVE
#define STR WIAnalysis::STRIDED
#define RND WIAnalysis::RANDOM

/// Dependency maps (define output dependency according to 2 input deps)

static const WIAnalysis::WIDependancy
add_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, SEQ, PTR, STR, RND},
  /* SEQ */  {SEQ, STR, STR, STR, RND},
  /* PTR */  {PTR, STR, STR, STR, RND},
  /* STR */  {STR, STR, STR, STR, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

static const WIAnalysis::WIDependancy
sub_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, STR, RND, RND, RND},
  /* SEQ */  {SEQ, RND, RND, RND, RND},
  /* PTR */  {PTR, RND, RND, RND, RND},
  /* STR */  {STR, RND, RND, RND, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};


static const WIAnalysis::WIDependancy
mul_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, STR, STR, STR, RND},
  /* SEQ */  {STR, RND, RND, RND, RND},
  /* PTR */  {STR, RND, RND, RND, RND},
  /* STR */  {STR, RND, RND, RND, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};

static const WIAnalysis::WIDependancy
select_conversion[WIAnalysis::NumDeps][WIAnalysis::NumDeps] = {
  /*          UNI, SEQ, PTR, STR, RND */
  /* UNI */  {UNI, STR, STR, STR, RND},
  /* SEQ */  {STR, SEQ, STR, STR, RND},
  /* PTR */  {STR, STR, PTR, STR, RND},
  /* STR */  {STR, STR, STR, STR, RND},
  /* RND */  {RND, RND, RND, RND, RND}
};


bool WIAnalysis::runOnFunction(Function &F) {

  if (! m_rtServices->orderedWI()) {
    return false;
  }

  m_deps.clear();
  m_changed1.clear();
  m_changed2.clear();
  m_pChangedNew = &m_changed1;
  m_pChangedOld = &m_changed2;

  // Compute the  first iteration of the WI-dep according to ordering
  // intstructions this ordering is generally good (as it ususally correlates
  // well with dominance).
  inst_iterator it = inst_begin(F);
  inst_iterator  e = inst_end(F);
  for (; it != e; ++it) {
    calculate_dep(&*it);
  }

  // Recursively check if WI-dep changes and if so reclaculates 
  // the WI-dep and marks the users for re-checking.
  // This procedure is guranteed to converge since WI-dep can only 
  // become less unifrom (uniform->consecutive->ptr->stride->random).
  updateDeps();

  if(PrintWiaCheck) {
    outs() << F.getNameStr() << "\n";
    for (it = inst_begin(F); it != e; ++it) {
      Instruction *I = &*it;
      outs()<<"WI-RunOnFunction " <<m_deps[I] <<" "<<*I <<" " << "\n";
    }
  }
  return false;
}

void WIAnalysis::updateDeps() {

  // As lonst as we have values to update
  while(!m_pChangedNew->empty()) {
    // swap between changedSet pointers - recheck the newChanged(now old) 
    std::swap(m_pChangedNew, m_pChangedOld);
    // clear the newChanged set so it will be filled with the users of 
    // instruction which their WI-dep canged during the current iteration
    m_pChangedNew->clear();
    // update all changed values
    std::set<const Value*>::iterator it = m_pChangedOld->begin();
    std::set<const Value*>::iterator e = m_pChangedOld->end();
    for(; it != e; ++it) {
      // remove first instruction
      // calculate its new dependencey value
      calculate_dep(*it);
    }
  }
}

WIAnalysis::WIDependancy WIAnalysis::whichDepend(const Value* val){
  V_PRINT("WIA","Asking about "<<*val<<"\n");
  if (! m_rtServices->orderedWI()) {
  V_PRINT("WIA","Random!!\n");
    if(PrintWiaCheck) {
        outs()<<"whichDepend function "<< "WIA" <<"Random!!"<<"4"<< "\n";
    }
 
    return WIAnalysis::RANDOM;
  }
  V_ASSERT(m_pChangedNew->empty() && "set should be empty before query");
  V_ASSERT(val && "Bad value");
  if (m_deps.find(val) == m_deps.end()) {
    // We do not expect instructions not in the map, in that case take the safe
    // way return random on release (assert on debug). For non-instruction 
    // (arguments, constants) return uniform.
    bool isInst = isa<Instruction>(val); 
    V_ASSERT(!isInst && "should not have new instruciton");
    if (isInst) return WIAnalysis::RANDOM;
    return WIAnalysis::UNIFORM;
  }
  V_PRINT("WIA","It is "<<m_deps[val]<<"\n");
  if(PrintWiaCheck) {
    outs()<<"whichDepend function "<< "WIA " <<m_deps[val] <<" "<<*val <<" " << "\n";
  }
  return m_deps[val];
}

void WIAnalysis::invalidateDepend(const Value* val){
  if (m_deps.find(val) != m_deps.end()) {
    m_deps.erase(val);
  }
}

bool WIAnalysis::isControlFlowUniform(const Function* F) {
  V_ASSERT(F && "Bad Function");

  /// Place out-masks
  for (Function::const_iterator it = F->begin(), e  = F->end(); it != e ; ++it) {
    if (dyn_cast<ReturnInst>(it->getTerminator())) continue;
    WIAnalysis::WIDependancy dep = whichDepend(it->getTerminator());
    if (dep != WIAnalysis::UNIFORM) {
      // Found a branch which diverges on the input
      return false;
    }
  }
  // All branches are uniform
  return true;
}

WIAnalysis::WIDependancy WIAnalysis::getDependency(const Value *val) {
  
  if (m_deps.find(val) == m_deps.end()) {
    m_deps[val] = WIAnalysis::UNIFORM;
  }
  return m_deps[val];
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const Value* val) {
  V_ASSERT(val && "Bad value");

  if (! isa<Instruction>(val)) {
    // Not an instruction, must be a constant or an argument
    // Could this vector type be of a constant which
    // is not uniform ?
    return WIAnalysis::UNIFORM;
  }

  const Instruction* inst = dyn_cast<Instruction>(val);
  V_ASSERT(inst && "This Value is not an Instruction");

  // New instruction to consider
  if (m_deps.find(inst) == m_deps.end()) {
    m_deps[inst] = WIAnalysis::UNIFORM;
  }

  // Our initial value
  WIDependancy orig = m_deps[inst];
  WIDependancy dep = orig;

  // LLVM does not have compile time polymorphisms
  // TODO: to make things faster we may want to sort the list below according
  // to the order of their probability of appearance.
  if      (const BinaryOperator *BI = dyn_cast<BinaryOperator>(inst))         dep = calculate_dep(BI);
  else if (const CallInst *CI = dyn_cast<CallInst>(inst))                     dep = calculate_dep(CI);
  else if (isa<CmpInst>(inst))                                                dep = calculate_dep_simple(inst);
  else if (isa<ExtractElementInst>(inst))                                     dep = calculate_dep_simple(inst);
  else if (const GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(inst))  dep = calculate_dep(GEP);
  else if (isa<InsertElementInst>(inst))                                      dep = calculate_dep_simple(inst);
  else if (isa<InsertValueInst>(inst))                                        dep = calculate_dep_simple(inst); 
  else if (const PHINode *Phi = dyn_cast<PHINode>(inst))                      dep = calculate_dep(Phi); 
  else if (isa<ShuffleVectorInst>(inst))                                      dep = calculate_dep_simple(inst); 
  else if (isa<StoreInst>(inst))                                              dep = calculate_dep_simple(inst);
  else if (const TerminatorInst *TI = dyn_cast<TerminatorInst>(inst))         dep = calculate_dep(TI);
  else if (const SelectInst *SI = dyn_cast<SelectInst>(inst))                 dep = calculate_dep(SI);
  else if (isa<AllocaInst>(inst))                                             dep = WIAnalysis::RANDOM;
  else if (const CastInst *CI = dyn_cast<CastInst>(inst))                     dep = calculate_dep(CI);
  else if (isa<ExtractValueInst>(inst))                                       dep = calculate_dep_simple(inst);
  else if (isa<LoadInst>(inst))                                               dep = calculate_dep_simple(inst);
  else if (const VAArgInst *VAI = dyn_cast<VAArgInst>(inst))                  dep = calculate_dep(VAI);

  // If the value was changed in this calculation
  if (dep!=orig) {
    // Save the new value of this instruction
    m_deps[inst] = dep;
    // Register for update all of the dependent values of this updated
    // instruction.
    Value::const_use_iterator it = inst->use_begin();
    Value::const_use_iterator e  = inst->use_end();
    for (; it != e; ++it) {
      m_pChangedNew->insert(*it);
    }
  }

  return dep;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep_simple(const Instruction *I) {
  // simply check that all operans are uniform, if so return uniform, else random
  const unsigned nOps = I->getNumOperands();
  for (unsigned i=0; i<nOps; ++i) {
    const Value *op = I->getOperand(i);
    WIAnalysis::WIDependancy dep = getDependency(op);
    if (dep != WIAnalysis::UNIFORM)
      return WIAnalysis::RANDOM;
  }
  return WIAnalysis::UNIFORM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const BinaryOperator* inst) {
  // Calculate the dependency type for each of the operands
  Value *op0 = inst->getOperand(0);
  Value *op1 = inst->getOperand(1);

  WIAnalysis::WIDependancy dep0 = getDependency(op0);
  WIAnalysis::WIDependancy dep1 = getDependency(op1);

  // For whatever binary operation,
  // uniform returns uniform
  if ( WIAnalysis::UNIFORM == dep0 && WIAnalysis::UNIFORM == dep1) {
    return WIAnalysis::UNIFORM;
  }
  
  // FIXME:: assumes that the X value does not cross the +/- border - risky !!!
  // The pattern (and (X, C)), where C preserves the lower k bits of the value,
  // is often used for truncating of numbers in 64bit. We assume that the index
  // properties are not hurt by this.
  if (inst->getOpcode() == Instruction::And) {
    ConstantInt *C0 = dyn_cast<ConstantInt>(inst->getOperand(0));
    ConstantInt *C1 = dyn_cast<ConstantInt>(inst->getOperand(1));
    // Use any of the constants. Instcombine places constants on Op1
    // so try Op1 first. 
    if (C1 || C0) {
      ConstantInt *C = C1 ? C1 : C0;
      WIAnalysis::WIDependancy dep = C1 ? dep0 : dep1;
      // Cannot look at bit pattern of huge integers.
      if (C->getBitWidth() < 65) {
        uint64_t val = C->getZExtValue();
        uint64_t ptr_mask = (1<<MinIndexBitwidthToPreserve)-1;
        // Zero all bits above the lower k bits that we are interested in
        val &= (ptr_mask);
        // Make sure that all of the remaining bits are active
        if (val == ptr_mask) { return dep; }
      }
    }
  }
  
  // FIXME:: assumes that the X value does not cross the +/- border - risky !!!
  // The pattern (ashr (shl X, C)C) is used for truncating of numbers in 64bit
  // The constant C must leave at least 32bits of the original number
  if (BinaryOperator* SHL = dyn_cast<BinaryOperator>(inst->getOperand(0))) {
    if (inst->getOpcode() == Instruction::AShr && SHL->getOpcode() == Instruction::Shl) {
      ConstantInt * c_ashr = dyn_cast<ConstantInt>(inst->getOperand(1));
      ConstantInt * c_shl  = dyn_cast<ConstantInt>(SHL->getOperand(1));
      const IntegerType *AshrTy = cast<IntegerType>(inst->getType());
      if (c_ashr && c_shl && c_ashr->getZExtValue() == c_shl->getZExtValue()) {
        // If wordWidth - shift_width >= 32 bits
        if ((AshrTy->getBitWidth() - c_shl->getZExtValue()) >= MinIndexBitwidthToPreserve ) {
          // return the dep of the original X
          return getDependency(SHL->getOperand(0));
        }
      }
    }
  }

  switch (inst->getOpcode()) {
    // Addition simply adds the stride value, except for ptr_consecutive
    // which is promoted to strided.
    // Another exception is when we subtract the tid: 1 - X which turns the
    // tid order to random.
  case Instruction::Add:
  case Instruction::FAdd:
    return add_conversion[dep0][dep1];
  case Instruction::Sub:
  case Instruction::FSub:
    return sub_conversion[dep0][dep1];

  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::Shl:
    if ( WIAnalysis::UNIFORM == dep0 || WIAnalysis::UNIFORM == dep1) {
      // If one of the sides is uniform, then we can adopt
      // the other side (stride*uniform is still stride).
      // stride size is K, where K is the uniform input.
      // An exception to this is ptr_consecutive, which is
      // promoted to strided.
      return mul_conversion[dep0][dep1];
    }
  default:
    //TODO: Support more arithmetic if needed
    return WIAnalysis::RANDOM;
  }
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const CallInst* inst) {
  //TODO: This function requires much more work, to be correct:
  //   2) Some functions (dot_prod, cross_prod) provide "measurable"
  //   behavior (Uniform->strided).
  //   This information should also be obtained from RuntimeServices somehow.

  // Check if call is TID-generator
  bool err, isTidGen;
  unsigned dim = 0;
  isTidGen = m_rtServices->isTIDGenerator(inst, &err, &dim);
  // We do not vectorize TID with variable dimention
  V_ASSERT((!err) && "TIDGen inst receives non-constant input. Cannot vectorize!");
  // All WI's are consecutive along the zero dimention
  if (isTidGen && dim == 0) return WIAnalysis::CONSECUTIVE;

  // Check if function is declared inside "this" module
  if (! inst->getCalledFunction()->isDeclaration()) {
    // For functions declared in this module - it is unsafe to assume anything
    return WIAnalysis::RANDOM;
  }

  // Check if the function is in the table of functions
  Function *origFunc = inst->getCalledFunction();
  std::string origFuncName = origFunc->getName();

  // Barriers are uniform
  if (m_rtServices->isSyncFunc(origFuncName)) return WIAnalysis::UNIFORM;
  
  std::string scalarFuncName = origFuncName;
  bool isMangled = Mangler::isMangledCall(scalarFuncName);
  bool MaskedMemOp = (Mangler::isMangledLoad(scalarFuncName) || 
                      Mangler::isMangledStore(scalarFuncName));
  
  // First remove any name-mangling (for example, masking), from the function name
  if (isMangled) {
    scalarFuncName = Mangler::demangle(scalarFuncName);
  }

  // Do not consider functions such as get_global_id as 'random'
  bool KnownUniform = m_rtServices->isKnownUniformFunc(scalarFuncName);

  // Look for the function in the builtin functions hash
  const RuntimeServices::funcEntry foundFunction =
    m_rtServices->findBuiltinFunction(scalarFuncName);
  if (!foundFunction.first && !MaskedMemOp && !isTidGen && !KnownUniform) {
    return WIAnalysis::RANDOM;
  }

  // Iterate over all input dependencies. If all are uniform - propagate it.
  // otherwise - return RANDOM
  unsigned numParams = inst->getNumArgOperands();

  bool isAllUniform = true;
  for (unsigned i = 0; i < numParams; ++i)
  {
    // Operand 0 is the function's name
    Value* op = inst->getArgOperand(i);
    WIDependancy dep = getDependency(op);
    if (WIAnalysis::UNIFORM != dep) {
      isAllUniform = false;
      break; // Uniformity check failed. no need to continue
    }
  }
  if (isAllUniform) return WIAnalysis::UNIFORM;
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const GetElementPtrInst* inst) {
  // running over the pointer and all indices argumets except for the last 
  // here we assume the pointer is the first operand
  unsigned num = inst->getNumIndices();
  for (unsigned i=0; i < inst->getNumIndices(); ++i) {
    const Value* op = inst->getOperand(i);
    WIAnalysis::WIDependancy dep = getDependency(op);
    if (dep != WIAnalysis::UNIFORM) {
      return WIAnalysis::RANDOM;
    }
  }
  const Value* lastInd = inst->getOperand(num);
  WIAnalysis::WIDependancy lastIndDep = getDependency(lastInd);

  if (WIAnalysis::UNIFORM == lastIndDep)
    return WIAnalysis::UNIFORM;
  if (WIAnalysis::CONSECUTIVE == lastIndDep)
    return WIAnalysis::PTR_CONSECUTIVE;
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const PHINode* inst) {
  unsigned num = inst->getNumIncomingValues();
  std::vector<WIDependancy> dep;
  for (unsigned i=0; i < num; ++i)
  {
    Value* op = inst->getIncomingValue(i);
    dep.push_back(getDependency(op));
  }
  WIDependancy totalDep = dep[0];

  for (unsigned i=1; i < num; ++i)
  {
    totalDep = select_conversion[totalDep][dep[i]];
  }

  return totalDep;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const TerminatorInst* inst) {
  // Instruction has no return value
  // Just need to know if this inst is uniform or not
  // because we may want to avoid predication if the control flows
  // in the function are uniform...
  switch (inst->getOpcode())
  {
  case Instruction::Br:
    {
      const BranchInst * brInst = cast<BranchInst>(inst);
      if (brInst->isConditional())
      {
        // Conditional branch is uniform, if its condition is uniform
        Value* op = brInst->getCondition();
        WIAnalysis::WIDependancy dep = getDependency(op);
        if ( WIAnalysis::UNIFORM == dep ) {
          return WIAnalysis::UNIFORM;
        }
        return WIAnalysis::RANDOM;
      }
      // Unconditional branch is non TID-dependent
      return WIAnalysis::UNIFORM;
    }
  case Instruction::IndirectBr:
    // TODO: Define the dependency requirements of indirectBr
  default:
    return WIAnalysis::RANDOM;
  }
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const SelectInst* inst) {
  Value* op0 = inst->getOperand(0); // mask
  WIAnalysis::WIDependancy dep0 = getDependency(op0);
  if (WIAnalysis::UNIFORM == dep0) {
    Value* op1 = inst->getOperand(1);
    Value* op2 = inst->getOperand(2);
    WIAnalysis::WIDependancy dep1 =getDependency(op1);
    WIAnalysis::WIDependancy dep2 =getDependency(op2);
    // Incase of constant scalar select we can choose according to the mask.
    if (ConstantInt *C = dyn_cast<ConstantInt>(op0)) {
      uint64_t val = C->getZExtValue();
      if (val) return dep1;
      else return dep2;
    }
    // Select the "weaker" dep, but if only one dep is ptr_consecutive,
    // it must be promoted to strided ( as this data may
    // propagate to Load/Store instructions.
    return select_conversion[dep1][dep2];
  }
  // Incase the mask is non-uniform the select outcome can be a combination
  // so we don't know nothing about it.
  return WIAnalysis::RANDOM;
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const CastInst* inst) {
  Value* op0 = inst->getOperand(0);
  WIAnalysis::WIDependancy dep0 = getDependency(op0);

  // independent remains independent
  if (WIAnalysis::UNIFORM == dep0) return dep0;

  switch (inst->getOpcode())
  {  
  case Instruction::SExt:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::UIToFP:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::SIToFP:
    return dep0;
  case Instruction::ZExt:
  case Instruction::BitCast:
    return WIAnalysis::RANDOM;
  // FIXME:: assumes that the value does not cross the +/- border - risky !!!!
  case Instruction::Trunc: {
    const Type *destType = inst->getDestTy();
    const IntegerType *intType = dyn_cast<IntegerType>(destType);
    if (intType && (intType->getBitWidth() >= MinIndexBitwidthToPreserve)) {
      return dep0;
    }
    return WIAnalysis::RANDOM;
  }
  default:
    V_ASSERT(false && "no such opcode");
    // never get here
    return WIAnalysis::RANDOM;
  }
}

WIAnalysis::WIDependancy WIAnalysis::calculate_dep(const VAArgInst* inst) {
  V_ASSERT(false && "Are we supporting this ??");
  return WIAnalysis::RANDOM;
}


} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  void* createWIAnalysisPass() {
    return new intel::WIAnalysis();
  }
}

char intel::WIAnalysis::ID = 0;
static RegisterPass<intel::WIAnalysis>
CLIWIAnalysis("WIAnalysis", "WIAnalysis provides work item dependency info");

