/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "CLWGLoopBoundaries.h"
#include "llvm/Constants.h"
#include "LoopUtils.h"
#include <set>

extern "C" void fillNoBarrierPathSet(Module *M, std::set<std::string>& noBarrierPath);

char intel::CLWGLoopBoundaries::ID = 0;
namespace intel {

CLWGLoopBoundaries::CLWGLoopBoundaries() : 
ModulePass(ID),
m_rtServices(static_cast<OpenclRuntime *>(RuntimeServices::get()))
{
  assert(m_rtServices && "expected to have openCL runtime");
}

CLWGLoopBoundaries::~CLWGLoopBoundaries()
{
}

bool CLWGLoopBoundaries::runOnModule(Module &M) {
  SmallVector<Function *, 8> kernels;
  LoopUtils::GetOCLKernel(M, kernels);
  bool changed = false;
  std::set<std::string> NoBarrier;
  fillNoBarrierPathSet(&M, NoBarrier);
  for (unsigned i = 0, e = kernels.size(); i < e; ++i) {
    Function *F = kernels[i];
    std::string funcName = F->getNameStr();
    if (!F || !NoBarrier.count(funcName)) continue;
    changed |= runOnFunction(*F);
  }
  return changed;
}

bool CLWGLoopBoundaries::isUniform(Value *v) {
  Instruction *I = dyn_cast<Instruction>(v);
  if (!I) return true;
  assert(m_Uni.count(I) && "should not query new instructions");
  return m_Uni[I];
}

void CLWGLoopBoundaries::processTIDCall(CallInst *CI, StringRef &name) {
  assert(CI->getType() == m_indTy && "mismatch get***id type");
  ConstantInt *dimC = dyn_cast<ConstantInt>(CI->getArgOperand(0));
  if (!dimC) {
    m_Uni[CI] = false;
    return;
  } 
  unsigned dim = static_cast<unsigned>(dimC->getValue().getZExtValue());
  assert (dim < 3 && "get***id with dim > 2");
  if (dim < m_numDim) {
    m_TIDs[CI] = std::pair<unsigned , bool>(dim, name == GET_GID_NAME);
    m_Uni[CI] = false;
  } else {
    // For apple we do the loop only on 0-dimension so we consider.
    // get***id(1,2) as uniform. 
    m_Uni[CI] = true;
  }
}

bool CLWGLoopBoundaries::isUniformByOps(Instruction *I) {
   for (unsigned i=0; i<I->getNumOperands(); ++i) {
     if (!isUniform(I->getOperand(i))) return false;
   }
   return true;
} 

void CLWGLoopBoundaries::CollcectBlockData(BasicBlock *BB) {
  // run over all instructions in the block (excluding the terminator) 
  for (BasicBlock::iterator I=BB->begin(), E= --(BB->end()); I!=E ; ++I) {
    if (CallInst *CI = dyn_cast<CallInst>(I)) {
      Function *F = CI->getCalledFunction();
      // If the function is defined in the module than it is not a
      // TID and not unifrom.
      if (!F->isDeclaration()) {
        m_Uni[I] = false;
        continue;
      }
      // Check to see if this get***id call
      StringRef name = F->getName();
      if (name==GET_LID_NAME || name==GET_GID_NAME) {
        processTIDCall(CI, name);
        continue;
      }
      // Getting this is a regular builtin uniform if all opernads are uniform
      m_Uni[I] = isUniformByOps(I);
    } else if (isa<AllocaInst>(I)){
      m_Uni[I] = false;
    } else { 
      m_Uni[I] = isUniformByOps(I);
    }
  }
}

bool CLWGLoopBoundaries::runOnFunction(Function& F) {
  m_F = &F;
  m_M = F.getParent();
  m_context = &F.getContext();
  m_numDim = m_rtServices->getNumJitDimensions();
  m_indTy = LoopUtils::getIndTy(m_M);
  m_constOne = ConstantInt::get(m_indTy, 1);
  m_constZero = ConstantInt::get(m_indTy, 0);
  // clear used data structures
  m_TIDs.clear();
  m_Uni.clear();
  m_TIDDesc.clear();
  m_UniDesc.clear();
    
  // Collects uniform data and get***id calls attributes from the 
  // current basic block.
  CollcectBlockData(&m_F->getEntryBlock());

  // Iteratively examines if the entry block branch is early exit branch.
  // If so collect the early exit description and and try to collapse the 
  // code successor into entry block.
  bool earlyExitCollapsed = false;
  do {
    earlyExitCollapsed = findAndCollapseEarlyExit();
  } while (earlyExitCollapsed);

  // Create early exit functions for later use of Loop Generator.
  createWGLoopBoundariesFunction();
  return true;
}

Function *CLWGLoopBoundaries::createLoopBoundariesFunctionDcl() {
  unsigned numEntries = CLWGBoundDecoder::getNumWGBoundArrayEntries(m_numDim);
  std::string funcName = m_F->getNameStr();
  std::string EEFuncName = CLWGBoundDecoder::encodeWGBound(funcName);
  Type *retTy = ArrayType::get(m_indTy, numEntries);
  
  // Check if argTypes was already initalized, if not create it.
  std::vector<Type *> argTypes;
  for(Function::arg_iterator argIt = m_F->arg_begin(), argE = m_F->arg_end(); 
      argIt != argE; ++argIt) {
    argTypes.push_back(argIt->getType());
  }
 
  FunctionType *fType = 
    FunctionType::get(retTy, argTypes, false);
  Function *condFunc = 
    Function::Create(fType, m_F->getLinkage(), EEFuncName, m_M);
  return condFunc;
}

void CLWGLoopBoundaries::recoverInstructions (VMap &valueMap, VVec &roots,
                                           BasicBlock *BB, Function *newF) {
  // Mapping the function argumets.
  for(Function::arg_iterator argIt = m_F->arg_begin(), argE = m_F->arg_end(),
      newArgIt = newF->arg_begin(); argIt != argE; ++argIt, ++newArgIt) {
    valueMap[argIt] = newArgIt;
  }
    
  // Adding all Instructions leading to the boundry to reconstruct set.
  // Creating a clone for each instruction on the path.
  VVec toAdd = roots; //hard copy of roots
  BasicBlock *entry = &m_F->getEntryBlock();
  while (toAdd.size()) {
    Value *cur = toAdd.back();
    toAdd.pop_back();
    // value was already mapped no need to do anything.
    if (valueMap.count(cur)) continue;

    Instruction *I = dyn_cast<Instruction>(cur);
    // If the value is not an instruction (global ,constant) than it
    // should be mapped.to itself.
    if (!I) {
      assert (!isa<Argument>(cur) && "arguments are supposed to be mapped");
      valueMap[cur] = cur;
      continue;
    }

    assert(I->getParent() == entry && "Instruction not in the entry block");
    valueMap[I] = I->clone();
    for (User::op_iterator op=I->op_begin(), opE=I->op_end(); 
         op!=opE; ++op) {
      toAdd.push_back(*op);
    }
  }

  // Running according to the order of the original entry block, and connecting
  // each instruction to the operands in the new function. The order of the 
  // original function ensures the corretness of the order of the inserted 
  // Instructions.
  for (BasicBlock::iterator I=entry->begin(), E=entry->end(); I!=E ; ++I) {
    if (valueMap.count(I)) {
      Instruction *clone = cast<Instruction>(valueMap[I]);
      BB->getInstList().push_back(clone);
      for (User::op_iterator op=clone->op_begin(), opE=clone->op_end(); 
           op!=opE; ++op) {
        Value *&VMSlot = valueMap[*op];
        assert(VMSlot && "all operands should be mapped");
        if (VMSlot) *op = VMSlot;
      }
    }
  }  
}

bool CLWGLoopBoundaries::findAndCollapseEarlyExit() {
// Supported pattern is that entry block ends with conditional branch,
// and has no side effect instructions.
  BasicBlock *entry = &m_F->getEntryBlock();
  BranchInst *Br = dyn_cast<BranchInst>(entry->getTerminator());
  if (!Br) return false;
  if (!Br->isConditional()) return false;
  if (hasSideEffectInst(entry)) return false;

  // Collect Description of early exit if exists.
  BasicBlock *trueSuc = Br->getSuccessor(0);
  BasicBlock *falseSuc = Br->getSuccessor(1);
  BasicBlock *EEremove = NULL;
  BasicBlock *EEsucc = NULL;
  // Checks early exit on true side.
  if (isEarlyExitSucc(trueSuc)) {
    if (isEarlyExitBranch(Br, true)) {
      EEremove = trueSuc;
      EEsucc = falseSuc;
    }
  } 
  // Checks early exit on false side.
  else if (isEarlyExitSucc(falseSuc)) {
    if (isEarlyExitBranch(Br, false)) {
      EEremove = falseSuc;
      EEsucc = trueSuc;
    }
  }
  
  // An early exit was found, remove the branch at the entry block, and merge
  // it with the non exit successor if possible.
  if (EEremove) {
    EEremove->removePredecessor(entry);
    Br->eraseFromParent();
    BranchInst::Create(EEsucc, entry);
    // If the successor has the entry as unique predecessor than we might find 
    // the succsessor is not in a loop and it is safe to scan it for new early
    // exit opportunities.
    if (EEsucc->getUniquePredecessor()) {
      // Collect TID info for the code successor block.
      // Since the entry is the only pred the successor is not part of a loop.
      CollcectBlockData(EEsucc); 
      // Try to Merge the block into it's pred.
      // If the blocks were merged we might have another early exit oppurunity.
      if (MergeBlockIntoPredecessor(EEsucc)) return true;
    }
  }

  return false;
}

bool CLWGLoopBoundaries::collectCond(SmallVector<ICmpInst *, 4>& compares,
                                  IVec &uniformConds, Instruction *root) {
  unsigned rootOp = root->getOpcode();
  assert((rootOp == Instruction::And || rootOp == Instruction::Or) &&
      "supported ops are :and, or ");
  SmallVector<Value *, 4> Cands;
  // First candiatate are the two operands.
  Cands.push_back(root->getOperand(0));
  Cands.push_back(root->getOperand(1));
  do {
    Instruction *cur = dyn_cast<Instruction>(Cands.back());
    if (!cur) return false;
    Cands.pop_back();
    if (isUniform(cur)) {
      // cur is uniform than fill uniformCands.
      uniformConds.push_back(cur);
    } else if (ICmpInst *cmp = dyn_cast<ICmpInst>(cur)) {
      // cur is icmp if relational (<, <=, >, >=) fills compares.
      if (!cmp->isRelational()) return false;
      compares.push_back(cmp);
    } else if (cur->getOpcode() == rootOp) {
      // It is the same as root it's operands are new candidates
      Cands.push_back(cur->getOperand(0));
      Cands.push_back(cur->getOperand(1));
    } else {
      // Not in the pattern.
      return false;
    }
  } while  (Cands.size());
  return true;
}

bool CLWGLoopBoundaries::traceBackBound(ICmpInst *cmp, unsigned TIDInd,
                                     Value *&bound, Value *&tid){
  // The pattern of boundary condition is: icmp TID , Uniform.
  // But more genral pattern is icmp f(TID), Uniform. 
  // In that case the bound will be f_inverse(Unifrom). currently we support
  // only truncation but this can be extended to more complex forms of f.
  assert (TIDInd < 2 && "compare has 2 opernads");
  bound = cmp->getOperand(1-TIDInd);
  tid = cmp->getOperand(TIDInd);
  assert(isUniform(bound) && "non uniform boundary");
  while (Instruction *tidInst = dyn_cast<Instruction>(tid)) {
    switch (tidInst->getOpcode()) {  
      case Instruction::Trunc: {
        // If candidate is trunc instruction than we can safely extend the 
        // bound to have equivalent condition according to sign of comaprison.
        tid = tidInst->getOperand(0);
        if (cmp->isSigned()) {
          bound = new SExtInst(bound, tid->getType(), "sext_cast", cmp);
        } else {
          bound = new ZExtInst(bound, tid->getType(), "zext_cast", cmp);
        }
        break;
      }
      case Instruction::Call: 
        // Only Supported candidate is tid generator itself
        return m_TIDs.count(tid);
      default:
        // No other patterns supported.
        return false;
    }
  }
  return false;
}


bool CLWGLoopBoundaries::checkBoundaryCompare(ICmpInst *cmp, bool EETrueSide,
                                           TIDDescVec& eeVec){
  assert(!(isUniform(cmp->getOperand(0)) && isUniform(cmp->getOperand(1))) && 
          "cmp should not be uniform");
  // For boundary condition one of the ops must be uniform.
  // We check which one the operands (if any) is the uniform operand, and save
  // the index in UniInd 
  // (TIDInd will be the index of the tid generating operand)
  unsigned UniInd = 1;
  if(isUniform(cmp->getOperand(0))) UniInd = 0;
  else if (!isUniform(cmp->getOperand(1)))  return false;
  unsigned TIDInd = 1 - UniInd;

  // Traceback the boundary value.
  Value *bound, *tid;
  if (!traceBackBound(cmp, TIDInd, bound, tid)) return false;
  assert(m_TIDs.count(tid)  && bound && "non valid tid, bound");
  
  //Collect attributes of the compare instruction. 
  CmpInst::Predicate pred = cmp->getPredicate();
  bool isPredUpper = (pred == CmpInst::ICMP_ULT || pred == CmpInst::ICMP_ULE ||
                      pred == CmpInst::ICMP_SLT || pred == CmpInst::ICMP_SLE);
  bool isInclusive = (pred == CmpInst::ICMP_ULE || pred == CmpInst::ICMP_UGE ||
                      pred == CmpInst::ICMP_SLE || pred == CmpInst::ICMP_SGE);
  bool isSigned = cmp->isSigned();
  
  // When decidng whether the uniform value is an upper bound, and whether it 
  // is inclusive we need to take into consideration the index of unifrom value
  // and the whether we exit if the condition is met.
  // for example assuming the compre is pattern is:
  //    %cond = icmp ult %tid, %uni (tid is get***id uni is uniform)
  //    br %cond label %BB1, label %BB2
  // If BB2 is return uni is an upper bound, and exclusive 
  // if BB1 is return uni is lower bound and inclusive
  bool isUpper = isPredUpper ^ (TIDInd == 1) ^ EETrueSide;
  bool containsVal = isInclusive ^ EETrueSide;
 
  // Get the tid properties from the map
  std::pair<unsigned, bool> tidProp= m_TIDs[tid];
  unsigned dim = tidProp.first;
  bool isGID = tidProp.second;
  // Update the boundary descriptions vector with the current compare.
  eeVec.push_back(TIDDesc(bound, dim, isUpper, containsVal, isSigned, isGID));
  return true;
}

bool CLWGLoopBoundaries::isEarlyExitBranch(BranchInst *Br, bool EETrueSide) {
  assert(Br->getParent() == &(m_F->getEntryBlock()) && 
      "expected entry block branch");
  Value *cond = Br->getCondition();
  Instruction *condInst = dyn_cast<Instruction> (cond);
  // Generally we can handle this but this is unexpected.
  assert(condInst && "i1 are expected only as instructions");
  // Sanaity for release.
  if (!condInst) return false;

  // patterns supported are:
  // 1. Completely unifom test (that don't depend on TID)
  // 2. Icmp of TID against uniform value
  // 3. Ands of (1,2) for branching into the code (avoiding early exit)
  // 4. Or of (1,2) for branch into the early exit
  SmallVector<ICmpInst *, 4> compares;
  IVec uniformConds;
  if (isUniform(condInst)) {
    uniformConds.push_back(condInst);
  } else if (ICmpInst *cmp = dyn_cast<ICmpInst>(condInst) ) {
    if(!cmp->isRelational()) return false;
    compares.push_back(cast<ICmpInst>(condInst));
  } else if (EETrueSide && condInst->getOpcode() == Instruction::Or) {
    if (!collectCond(compares, uniformConds, condInst)) return false; 
  } else if (!EETrueSide && condInst->getOpcode() == Instruction::And) {
    if (!collectCond(compares, uniformConds, condInst)) return false;
  }

  //Check that compares have supported pattern.
  TIDDescVec eeVec;
  for(SmallVector<ICmpInst *, 4>::iterator cmpIt = compares.begin(), 
       cmpE = compares.end(); cmpIt != cmpE; ++cmpIt) {
    if (!checkBoundaryCompare(*cmpIt, EETrueSide, eeVec)) return false;
  }
  // All compares are valid, so we can add them to the TIDDesc.
  m_TIDDesc.append(eeVec.begin(), eeVec.end());

  for(IVec::iterator uniIt = uniformConds.begin(), uniE = uniformConds.end();
      uniIt != uniE; ++uniIt) {
    m_UniDesc.push_back(UniDesc(*uniIt, EETrueSide));
  }
  return true;
}

bool CLWGLoopBoundaries::isEarlyExitSucc(BasicBlock *BB){
  do {
    TerminatorInst *TI = BB->getTerminator();
    assert(TI && "no terminator?");
    // Block should have no side effect instructions.
    if (hasSideEffectInst(BB)) return false;
    // If terminator is ret instruction we got to return with no side effect.
    if (isa<ReturnInst>(TI)) return true;
    
    // Terminator is not return so for being early exit successor
    // it must be unconditional branch.
    BranchInst *Br = dyn_cast<BranchInst>(BB->getTerminator());
    if (!Br) return false;
    if (Br->isConditional()) return false;
    BB = Br->getSuccessor(0);
  } while(1);
  return false;
}

bool CLWGLoopBoundaries::hasSideEffectInst(BasicBlock *BB) {
  for (BasicBlock::iterator it = BB->begin(), e = BB->end(); it != e ; ++it) {
    switch (it->getOpcode()){
      // Loads and store have side effect.
      case Instruction::Load :
      case Instruction::Store:
        return true;
      // For calls ask the runtime object.
      case Instruction::Call :
	  {	
        std::string name = (cast<CallInst>(it))->getCalledFunction()->getNameStr();
        if (!m_rtServices->hasNoSideEffect(name)) return true;
		break;
	  }
      default:
        break;
    }    
  }
  return false;
}

Value *getMin(bool isSigned, Value *a, Value *b, BasicBlock *BB) {
  assert(a->getType()->isIntegerTy() && b->getType()->isIntegerTy());
  CmpInst::Predicate pred = isSigned ? CmpInst::ICMP_SLT : CmpInst::ICMP_ULT;
  Instruction *compare = new ICmpInst(*BB, pred, a, b, "");
  return SelectInst::Create(compare, a, b, "", BB);
}

Value *getMax(bool isSigned, Value *a, Value *b, BasicBlock *BB) {
  assert(a->getType()->isIntegerTy() && b->getType()->isIntegerTy());
  CmpInst::Predicate pred = isSigned ? CmpInst::ICMP_SGT : CmpInst::ICMP_UGT;
  Instruction *compare = new ICmpInst(*BB, pred, a, b, "");
  return SelectInst::Create(compare, a, b, "", BB);
}

Value *CLWGLoopBoundaries::correctBound(TIDDesc &td, BasicBlock *BB,
                                     Value *bound) {
  Value *newBound = bound;
  // Lower bound are expexted to inclusive, upper bound are expected to
  // be exclusive. If this is not the case add 1.
  if (!(td.m_containsVal ^ td.m_isUpperBound)) {
    newBound = 
        BinaryOperator::Create(Instruction::Add, newBound, m_constOne, "", BB);
  }

  // Incase bound is not on GID add the base global id to the bound.
  if (!td.m_isGID) {
    newBound = BinaryOperator::Create(
        Instruction::Add, newBound, m_baseGIDs[td.m_dim], "", BB);
  }

  // Incase the bound is changed we make sure that the additions did not 
  // invaidate the result by crossing the +/-  \  maxint\0 border.
  // Thus in case border is crossed we take the original bound instead. We will
  // avoid using it since it is compared after to the origianl boundaries.
  if(newBound != bound) {
    newBound = getMax(td.m_isSigned, bound, newBound, BB);
  }
  return newBound;
}

void CLWGLoopBoundaries::fillInitialBoundaries(BasicBlock *BB) {
  m_lowerBounds.clear();
  m_localSizes.clear();
  m_baseGIDs.clear();
  m_loopSizes.clear();
  const char *baseGIDName = m_rtServices->getBaseGIDName();
  for (unsigned dim=0; dim < m_numDim; ++dim) {
    CallInst *localSize = LoopUtils::getWICall(m_M, GET_LOCAL_SIZE, m_indTy, dim, BB);
    CallInst *baseGID = LoopUtils::getWICall(m_M, baseGIDName, m_indTy, dim, BB, "");
    m_localSizes.push_back(localSize);
    m_baseGIDs.push_back(baseGID);
    m_lowerBounds.push_back(baseGID);
    m_loopSizes.push_back(localSize);
  }
}

void CLWGLoopBoundaries::recoverBoundInstructions(VMap &valueMap, BasicBlock *BB) {
  // collect the boundaries and uniform conditions and recover the instructions
  // leading to them in BB.
  VVec toRecover;
  for (unsigned i = 0, e = m_UniDesc.size(); i < e; ++i) {
    toRecover.push_back(m_UniDesc[i].m_cond);
  }
  for (unsigned i = 0, e = m_TIDDesc.size(); i < e; ++i) {
    toRecover.push_back(m_TIDDesc[i].m_bound);
  }
  recoverInstructions(valueMap, toRecover, BB, BB->getParent());
}

void CLWGLoopBoundaries::obtainEEBoundaries(BasicBlock *BB, VMap &valueMap) {
  // Entry i will be true if there is early exit on dimension i.
  std::vector<bool> hasEE(m_numDim, false); 
  // Temporary vector to hold computation of upper bounds, to be used 
  // later for loop size computation in case of early exit.
  std::vector<Value *> upperBounds (m_numDim, NULL);
  
  // Run through all descriptions, and obtain upperBounds, lowerBounds
  // according to the boundary description.
  for (unsigned i = 0, e = m_TIDDesc.size(); i < e; ++i) {
    TIDDesc &td = m_TIDDesc[i];
    hasEE[td.m_dim] = true; //encountered early exit in dimension i.
    assert(valueMap.count(td.m_bound) && "boundary not in value map");
    // Correct the boundaries if needed. 
    Value *bound = valueMap[td.m_bound];
    Value *EEVal = correctBound(td, BB, bound);
    // Incase no upper bound was set yet first init the buffer with the 
    // trivial one (local_size + base_gid).
    if (!upperBounds[td.m_dim]) {
      upperBounds[td.m_dim] = BinaryOperator::Create(Instruction::Add, 
                    m_localSizes[td.m_dim], m_baseGIDs[td.m_dim], "", BB);
    }
    // Create min\max between the bound and previous upper\lower bound.
    if (td.m_isUpperBound) {
      upperBounds[td.m_dim] = getMin(td.m_isSigned, upperBounds[td.m_dim],
                                     EEVal, BB);
    } else {
      m_lowerBounds[td.m_dim] = 
          getMax(td.m_isSigned, m_lowerBounds[td.m_dim], EEVal, BB);
    }
  }

  // If there is early exit on dimension i set the loop size as the substraction 
  // of the upper and lower bounds. Note that this may be zero or negative and 
  // this is taken care when computing the uniform early exit.
  for (unsigned dim=0; dim < m_numDim; ++dim) {
    if (hasEE[dim]) {
      m_loopSizes[dim] = BinaryOperator::Create(Instruction::Sub, 
                          upperBounds[dim], m_lowerBounds[dim], "", BB);
    }
  }
}

Value *CLWGLoopBoundaries::obtainUniformCond(BasicBlock *BB, VMap &valueMap) {
  assert ((m_UniDesc.size() || m_TIDDesc.size()) && "expected early exit");
  // The condition should be true to go into the kernel so create not
  // incase the early exit is on the true side.
  Value *ret = ConstantInt::get(*m_context, APInt(1,1));
  if (m_UniDesc.size()) {
    for (unsigned i = 0, e = m_UniDesc.size(); i < e; ++i) {
      UniDesc &ud = m_UniDesc[i];
      assert(valueMap.count(ud.m_cond));
      Value *cur = valueMap[ud.m_cond];
      assert(cur->getType() == ret->getType() && "expected i1 type");
      if (ud.m_exitOnTrue) 
          cur = BinaryOperator::CreateNot(cur, "", BB);
      ret = BinaryOperator::Create(Instruction::And, ret, cur, "", BB);
    }
  }

  // If the loop size is different from local_size (becasue of an early exit)
  // than we add check that it is positive.
  for (unsigned dim=0; dim < m_numDim; ++dim) {
    if (m_loopSizes[dim] != m_localSizes[dim]) {
      Instruction *cmp = new ICmpInst(*BB, CmpInst::ICMP_SLT,
          m_constZero, m_loopSizes[dim], "");
      ret = BinaryOperator::Create(Instruction::And, ret, cmp, "", BB);
    }
  }
  // Extend ret to size_t type.
  ret = new ZExtInst(ret, m_indTy, "zext_cast", BB);
  return ret;
}

void CLWGLoopBoundaries::createWGLoopBoundariesFunction() {
  // Get the name for the uniform early exit function.
  Function *BoundFunc = createLoopBoundariesFunctionDcl();
  BasicBlock *BB = BasicBlock::Create(*m_context, "entry" , BoundFunc);

  // Fills local size member vector, and set initial lower\upper bounds.
  fillInitialBoundaries(BB);
  Value *uniformCond = m_constOne;
  if (m_UniDesc.size() || m_TIDDesc.size() ) {
    // Incase there are early exits recover Instructions leading to them
    // into the block and update the value map.
    VMap valueMap;
    recoverBoundInstructions(valueMap, BB);
    // Update upper\lower bounds according to boundary descriptions.
    obtainEEBoundaries(BB, valueMap);
    // Update uniform condition according to uniform early exit descriptions.
    uniformCond = obtainUniformCond(BB, valueMap);
  }
  
  // Insert Boundaries into the array return value.
  Value *retVal = UndefValue::get(BoundFunc->getReturnType());
  for (unsigned dim = 0; dim < m_numDim; ++dim) {
    unsigned loopSizeInd = CLWGBoundDecoder::getSizeIndex(dim);
    retVal = 
        InsertValueInst::Create(retVal, m_loopSizes[dim], loopSizeInd, "", BB);
    unsigned lowerInd = CLWGBoundDecoder::getInitGIDIndex(dim);
    retVal = 
        InsertValueInst::Create(retVal, m_lowerBounds[dim], lowerInd, "", BB);
  }
  // Insert the uniform early exit value to the array retrun value, and return.
  unsigned uniInd = CLWGBoundDecoder::getUniformIndex();
  retVal = InsertValueInst::Create(retVal, uniformCond, uniInd, "", BB);
  ReturnInst::Create(*m_context, retVal, BB);
}

char toChar(bool v) {
  if (v) return 'T';
  return 'F';
}

void CLWGLoopBoundaries::print(raw_ostream &OS, const Module *M) const {
  if ( !M ) return;

  OS << "\nCLWGLoopBoundaries\n";
  
  OS << "found " << m_TIDDesc.size() << " early exit boundaries\n";
  for (unsigned i = 0, e = m_TIDDesc.size(); i < e; ++i) {
    TIDDesc td = m_TIDDesc[i];
    OS << "dim=" <<td.m_dim << ", " 
       << "contains=" << toChar(td.m_containsVal) << ", "
       << "isGID=" << toChar(td.m_isGID) << ", "
       << "isSigned=" << toChar(td.m_isSigned) << ", "
       << "isUpper=" << toChar(td.m_isUpperBound) << "\n"
       << *(td.m_bound) << "\n";
  }

  OS << "\nfound " << m_UniDesc.size() << " uniform early exit conditions\n";
  for (unsigned i = 0, e = m_UniDesc.size(); i < e; ++i) {
    UniDesc ud = m_UniDesc[i];
    OS << "exitOnTrue=" << toChar(ud.m_exitOnTrue) << "\n"
       << *(ud.m_cond) << "\n";
  }
}

} //namespace


extern "C" {
  ModulePass* createCLWGLoopBoundariesPass() {
    return new intel::CLWGLoopBoundaries();
  }
}
static RegisterPass<intel::CLWGLoopBoundaries> CLWGLOOPBOUNDARIES
                    ("cl-loop-bound", "create loop boundaries array function");
