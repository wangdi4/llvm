/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "CLWGLoopBoundaries.h"
#include "CLWGBoundDecoder.h"
#include "LoopUtils/LoopUtils.h"
#include "MetaDataApi.h"
#include "CompilationUtils.h"
#include "common_dev_limits.h"

#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instructions.h"

#include "OCLPassSupport.h"
#include <set>

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char CLWGLoopBoundaries::ID = 0;

OCL_INITIALIZE_PASS(CLWGLoopBoundaries, "cl-loop-bound", "create loop boundaries array function", false, false)

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
  bool changed = false;

  Intel::MetaDataUtils mdUtils(&M);
  if ( !mdUtils.isKernelsHasValue() ) {
    //Module contains no MetaData information, thus it contains no kernels
    return changed;
  }

  // Get the kernels using the barrier for work group loops.
  Intel::MetaDataUtils::KernelsList::const_iterator itr = mdUtils.begin_Kernels();
  Intel::MetaDataUtils::KernelsList::const_iterator end = mdUtils.end_Kernels();
  for (; itr != end; ++itr) {
    Function *pFunc = (*itr)->getFunction();
    Intel::KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pFunc);
    //No need to check if NoBarrierPath Value exists, it is guaranteed that
    //KernelAnalysisPass ran before CLWGLoopBoundries pass.
    if (kimd->getNoBarrierPath()) {
      //Kernel that should not be handled in Barrier path.
      changed |= runOnFunction(*pFunc);
    }
  }
  return changed;
}

bool CLWGLoopBoundaries::isUniform(Value *v) {
  Instruction *I = dyn_cast<Instruction>(v);
  if (!I) return true;
  assert(m_Uni.count(I) && "should not query new instructions");
  return m_Uni[I];
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
      // If the function is defined in the module than it is not uniform.
      // If the function is ID generator it is not uniform.
      if (!F || !F->isDeclaration() || m_varibaleTIDCalls.count(CI) ||
          m_TIDs.count(CI)) {
        m_Uni[I] = false;
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

void CLWGLoopBoundaries::processTIDCall(CallInst *CI, bool isGID) {
  assert(CI->getType() == m_indTy && "mismatch get***id type");
  ConstantInt *dimC = dyn_cast<ConstantInt>(CI->getArgOperand(0));
  if (!dimC) {
    m_hasVariableTid = true;
    m_varibaleTIDCalls.insert(CI);
    return;
  }
  unsigned dim = static_cast<unsigned>(dimC->getValue().getZExtValue());
  assert (dim < MAX_WORK_DIM && "get***id with dim > (MAX_WORK_DIM-1)");
  // All dimension above m_numDim are uniform so we don't need to add them.
  if (dim < m_numDim) {
    m_TIDs[CI] = std::pair<unsigned , bool>(dim, isGID);
    m_TIDByDim[dim].push_back(CI);
  }
}

void CLWGLoopBoundaries::collectTIDData() {
  // First clear the tids fata structures
  m_TIDs.clear();
  m_TIDByDim.clear();
  m_TIDByDim.resize(m_numDim); // allocate vector for each dimension
  // Go over all get_global_id
  m_hasVariableTid = false;
  SmallVector<CallInst *, 4> gidCalls;
  LoopUtils::getAllCallInFunc(CompilationUtils::mangledGetGID(), m_F, gidCalls);
  for (unsigned i=0; i<gidCalls.size(); ++i) {
    processTIDCall(gidCalls[i], true);
  }
  // Go over all get_local_id
  SmallVector<CallInst *, 4> lidCalls;
  LoopUtils::getAllCallInFunc(CompilationUtils::mangledGetLID(), m_F, lidCalls);
  for (unsigned i=0; i<lidCalls.size(); ++i) {
    processTIDCall(lidCalls[i], false);
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
  m_Uni.clear();
  m_TIDDesc.clear();
  m_UniDesc.clear();
  m_toRemove.clear();

  // Collect information of get***id calls.
  collectTIDData();
  // Collects uniform data from the current basic block.
  CollcectBlockData(&m_F->getEntryBlock());
  // Check if the function calls an atomic builtin.
  m_hasAtomicCalls = currentFunctionHasAtomicCalls();


  // Iteratively examines if the entry block branch is early exit branch,
  // min\max with uniform value.
  // If so collect the early exit description and try to collapse the
  // code successor into entry block.
  bool earlyExitCollapsed = false;
  bool minMaxBoundaryRemoved = false;
  do {
    minMaxBoundaryRemoved = findAndHandleTIDMinMaxBound();
    earlyExitCollapsed = findAndCollapseEarlyExit();
  } while (minMaxBoundaryRemoved || earlyExitCollapsed);

  // Create early exit functions for later use of Loop Generator.
  createWGLoopBoundariesFunction();

  // Remove all instructions marked for removal.
  for (SmallPtrSet<Instruction *, 8>::iterator it = m_toRemove.begin(),
    e = m_toRemove.end(); it != e; ++it) {
    assert((*it)->getNumUses() == 0 && "no users expected");
    if ((*it)->getNumUses() == 0) (*it)->eraseFromParent();
  }
  return true;
}

Function *CLWGLoopBoundaries::createLoopBoundariesFunctionDcl() {
  unsigned numEntries = CLWGBoundDecoder::getNumWGBoundArrayEntries(m_numDim);
  std::string funcName = m_F->getName().str();
  std::string EEFuncName = CLWGBoundDecoder::encodeWGBound(funcName);
  Type *retTy = ArrayType::get(m_indTy, numEntries);

  // Check if argTypes was already initialized, if not create it.
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

bool CLWGLoopBoundaries::handleBuiltinBoundMinMax(Instruction *tidInst) {
  // The tid only user should be min\max builtin.
  if (!tidInst->hasOneUse()) return false;
  CallInst *CI = dyn_cast<CallInst>(*(tidInst->use_begin()));
  if (!CI) return false;

  // Currently uniformity information is available only for the first block.
  // This can be relaxed when WIAnalysis supports control flow.
  if (CI->getParent() != &(m_F->getEntryBlock())) return false;

  // Check if this is a scalar min/max builtin.
  Function *calledFunc = CI->getCalledFunction();
  if (!calledFunc) return false;
  StringRef fname = calledFunc->getName();
  bool isMinBltn, isSigned;
  if (!m_rtServices->isScalarMinMaxBuiltin(fname, isMinBltn, isSigned))
    return false;
  assert(CI->getNumArgOperands() == 2 && "bad min,max signature");

  // Track the boundary and the tid call.
  Value *bound, *tid;
  if (!traceBackMinMaxCall(CI, bound, tid)) return false;

  // Get the tid properties from the map.
  assert(m_TIDs.count(tid)  && bound && "non valid tid, bound");
  std::pair<unsigned, bool> tidProp = m_TIDs[tid];
  unsigned dim = tidProp.first;
  bool isGID = tidProp.second;

  bool isUpperBound = isMinBltn; // Min creates upper bound, max lower bound.
  bool containsVal = true; // All min \ max are inclusive.
  m_TIDDesc.push_back(TIDDesc(bound, dim, isUpperBound, containsVal,
                    isSigned, isGID));
  CI->replaceAllUsesWith(tidInst);
  m_toRemove.insert(CI);
  return true;
}

bool CLWGLoopBoundaries::handleCmpSelectBoundary(Instruction *tidInst) {
  // The tidInst users should be cmp, select with the same operands.
  // First find the select user.
  if (tidInst->getNumUses() != 2) return false;
  Value *user1 = *(tidInst->use_begin());
  Value *user2 = *(++(tidInst->use_begin()));
  SelectInst *SI = dyn_cast<SelectInst>(user1);
  if (!SI) {
    SI = dyn_cast<SelectInst>(user2);
  }
  if (!SI) return false;

  // Currently uniformity information is available only for the first block.
  // This can be relaxed when WIAnalysis supports control flow.
  if (SI->getParent() != &(m_F->getEntryBlock())) return false;

  // The cmp should be the select mask operand.
  // The select should be the only user of the copmare.
  Value *mask = SI->getCondition();
  ICmpInst *cmp = dyn_cast<ICmpInst>(mask);
  if (!cmp || !cmp->hasOneUse()) return false;

  // The compare and the select should have the same operands.
  // This ensures that that cmp is user of the tidInst.
  Value *trueOp = SI->getTrueValue();
  Value *falseOp = SI->getFalseValue();
  Value *cmpOp0 = cmp->getOperand(0);
  Value *cmpOp1 = cmp->getOperand(1);
  if (!(cmpOp0 == trueOp && cmpOp1 == falseOp) &&
      !(cmpOp1 == trueOp && cmpOp0 == falseOp)) return false;

  // Track the boundary and tid call.
  Value *bound, *tid;
  if (!traceBackCmp(cmp, bound, tid)) return false;
  // Update the eeVec with the boundary descriptions.
  if (!obtainBoundaryCmpSelect(cmp, bound, tid, cmpOp0 == trueOp)) return false;
  // Replace the uses of the select with the original tid call, and mark
  // redundant instructions for removal.
  SI->replaceAllUsesWith(tidInst);
  m_toRemove.insert(SI);
  m_toRemove.insert(cmp);
  return true;
}

bool CLWGLoopBoundaries::obtainBoundaryCmpSelect(ICmpInst *cmp, Value *bound,
                                                 Value *tid, bool isSameOrder){
  // Patterns like a==b ? a : b are handled trivially by instcombine.
  if(!cmp->isRelational()) return false;

  // Get the tid properties from the map
  assert(m_TIDs.count(tid)  && bound && "non valid tid, bound");
  std::pair<unsigned, bool> tidProp = m_TIDs[tid];
  unsigned dim = tidProp.first;
  bool isGID = tidProp.second;

  CmpInst::Predicate pred = cmp->getPredicate();
  bool containsVal = true; // all cmp-select are inclusive.
  assert (isSupportedRelationalComparePredicate(pred) &&
          "unexpected relational cmp predicate");
  bool isPredLower = isComparePredicateLower(pred); // is pred <, <=

  // Note that min\max are always inclusive:
  // (tid> bound ? tid : bound) ~ (tid >= bound ? tid : bound) ~ [bound, ...]
  // Also, if we switch opernads order in both cmp and select we get the
  // same  bounds:
  // (tid > bound ? tid : bound) ~ (bound > tid ? bound : tid) ~ [bound, ...]
  // These notions reduce the patterns into:
  // tid >,>= bound ? tid : bound ~ [bound, ...]
  // tid >,>= bound ? bound : tid ~ [..., bound]
  // tid <,<= bound ? tid : bound ~ [..., bound]
  // tid <,<= bound ? bound : tid ~ [bound, ...]
  bool isUpperBound = (!isPredLower ^ isSameOrder);

  // Update the boundary descriptions vector with the current compare.
  m_TIDDesc.push_back(TIDDesc(bound, dim, isUpperBound, containsVal,
                              cmp->isSigned(), isGID));
  return true;
}

bool CLWGLoopBoundaries::findAndHandleTIDMinMaxBound() {
  // In case there are get***id with variable argument, we can not
  // know who are the users of each dimension.
  if (m_hasVariableTid) return false;

  // In case there is an atomic call we cannot avoid running two work items
  // that use essentially the same id (due to min(get***id(),uniform) as
  // the atomic call may have different consequences for the same id.
  if (m_hasAtomicCalls) return false;

  bool removedMinMaxBound = false;
  assert(m_TIDByDim.size() == m_numDim && "num dimension mismatch");
  for (unsigned dim=0; dim<m_numDim; ++dim) {
    // Should have exactly one tid generator for that dimension.
    if (m_TIDByDim[dim].size() != 1) continue;
    CallInst *CI = m_TIDByDim[dim][0];

    // Allow truncation for 64 bit systems.
    Instruction *tidInst = CI;
    if (CI->hasOneUse()) {
      if(TruncInst *TI = dyn_cast<TruncInst>(*(CI->use_begin()))) {
        tidInst = TI;
      }
    }

    // Check if it matches min\max patterns.
    if (handleCmpSelectBoundary(tidInst) ||
        handleBuiltinBoundMinMax(tidInst)) {
      removedMinMaxBound = true;
    }
  }
  return removedMinMaxBound;
}

bool CLWGLoopBoundaries::currentFunctionHasAtomicCalls() {
  // First obtain all the atomic functions in the module.
  std::set<Function *> atomicFuncs;
  for (Module::iterator fit = m_M->begin(), fe = m_M->end(); fit != fe; ++fit){
    std::string name = fit->getName().str();
    if (m_rtServices->isAtomicBuiltin(name)) atomicFuncs.insert(fit);
  }
  // No atomic functions means there is no atomic call in the current function.
  if (!atomicFuncs.size()) return false;

  // Obtain all the recursive users of the atomic functions.
  std::set<Function *> atomicUsers;
  LoopUtils::fillFuncUsersSet(atomicFuncs, atomicUsers);
  // return true iff the current function is a recursive user of atomic function.
  return atomicUsers.count(m_F);
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
    m_toRemove.insert(Br);
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
  // First candidates are the two operands.
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

bool CLWGLoopBoundaries::traceBackBound(Value *v1, Value *v2, bool isCmpSigned,
                                        Instruction *loc, Value *&bound, Value *&tid){
  // The input values should be tid dependent value compared with uniform one.
  // First We find which is uniform and which is tid-dependent and abort otherwise.
  bool isV1Uniform = isUniform(v1);
  bool isV2Uniform = isUniform(v2);
  if (isV1Uniform == isV2Uniform) return false;
  bound = isV1Uniform ? v1 : v2;
  tid = isV1Uniform ? v2 : v1;

  // The pattern of boundary condition is: comparison between TID and Uniform.
  // But more general pattern is comparison between f(TID) and Uniform.
  // In that case the bound will be f_inverse(Unifrom). currently we support
  // only truncation but this can be extended to more complex forms of f.
  while (Instruction *tidInst = dyn_cast<Instruction>(tid)) {
    switch (tidInst->getOpcode()) {
      case Instruction::Trunc: {
        // If candidate is trunc instruction than we can safely extend the
        // bound to have equivalent condition according to sign of comparison.
        tid = tidInst->getOperand(0);
        if (isCmpSigned) {
          bound = new SExtInst(bound, tid->getType(), "sext_cast", loc);
        } else {
          bound = new ZExtInst(bound, tid->getType(), "zext_cast", loc);
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

bool CLWGLoopBoundaries::traceBackCmp(ICmpInst *cmp, Value *&bound,
                                      Value *&tid) {
  Value *op0 = cmp->getOperand(0);
  Value *op1 = cmp->getOperand(1);
  return traceBackBound(op0, op1, cmp->isSigned(), cmp, bound, tid);
}

bool CLWGLoopBoundaries::traceBackMinMaxCall(CallInst *CI, Value *&bound,
                                             Value *&tid) {
  Value *arg0 = CI->getArgOperand(0);
  Value *arg1 = CI->getArgOperand(1);
  return traceBackBound(arg0, arg1, false, CI, bound, tid);
}

void CLWGLoopBoundaries::replaceTidWithBound (bool isGID, unsigned dim,
                                              Value *toRep) {
  assert(toRep->getType() == m_indTy && "bad type");
  SmallVector<CallInst *, 4> tidCalls;
  if (isGID) LoopUtils::getAllCallInFunc(CompilationUtils::mangledGetGID(), m_F, tidCalls);
  else       LoopUtils::getAllCallInFunc(CompilationUtils::mangledGetLID(), m_F, tidCalls);
  for (unsigned i=0; i<tidCalls.size(); ++i) {
    CallInst *tidCall = tidCalls[i];
    assert(isa<ConstantInt>(tidCall->getArgOperand(0)) && "non const dim");
    ConstantInt *dimConst = cast<ConstantInt>(tidCall->getArgOperand(0));
    unsigned dimArg = dimConst->getZExtValue();
    if (dim == dimArg) {
      // We remove all calls at the end to avoid invalidating internal
      // data structures that keep information about tid calls.
      tidCall->replaceAllUsesWith(toRep);
      m_toRemove.insert(tidCall);
    }
  }
}

bool CLWGLoopBoundaries::isSupportedRelationalComparePredicate(CmpInst::Predicate p) {
  return (p == CmpInst::ICMP_ULT || p == CmpInst::ICMP_ULE ||
          p == CmpInst::ICMP_SLT || p == CmpInst::ICMP_SLE ||
          p == CmpInst::ICMP_UGT || p == CmpInst::ICMP_UGE ||
          p == CmpInst::ICMP_SGT || p == CmpInst::ICMP_SGE );
}

bool CLWGLoopBoundaries::isComparePredicateLower(CmpInst::Predicate p) {
  return (p == CmpInst::ICMP_ULT || p == CmpInst::ICMP_ULE ||
          p == CmpInst::ICMP_SLT || p == CmpInst::ICMP_SLE);
}

bool CLWGLoopBoundaries::isComparePredicateInclusive(CmpInst::Predicate p) {
  return (p == CmpInst::ICMP_ULE || p == CmpInst::ICMP_UGE ||
          p == CmpInst::ICMP_SLE || p == CmpInst::ICMP_SGE);
}

bool CLWGLoopBoundaries::obtainBoundaryEE(ICmpInst *cmp, Value *bound,
                               Value *tid, bool EETrueSide, TIDDescVec& eeVec){
  unsigned TIDInd = isUniform(cmp->getOperand(0)) ? 1 : 0;
  assert(isUniform(cmp->getOperand(1-TIDInd)) && // tid is compared to uniform
         !isUniform(cmp->getOperand(TIDInd)) &&  // tid is not uniform
          "exactly one of the operands must be uniform");
  // Get the tid properties from the map
  assert(m_TIDs.count(tid)  && bound && "non valid tid, bound");
  std::pair<unsigned, bool> tidProp = m_TIDs[tid];
  unsigned dim = tidProp.first;
  bool isGID = tidProp.second;

  CmpInst::Predicate pred = cmp->getPredicate();
  if(!cmp->isRelational()) {
    assert ((pred == CmpInst::ICMP_EQ || pred == CmpInst::ICMP_NE) &&
           "unexpected non relational cmp predicate");
    if ((pred == CmpInst::ICMP_EQ) ^ EETrueSide) {
      // Here is support for cases where bound is the only value for the tid,
      // meaning the branch is one of the two options:
      // a. if (tid == bound) { kernel_code}
      // b. if (tid != bound) exit

      // Since bound is the only valid value for the TID we can replace all
      // the calls with it.
      replaceTidWithBound(isGID, dim, bound);

      // The bound is the only option for tid so we will fill it as both
      // upper bound and lower bound, both inclusive. sign of the comparison
      // is not important, since it is equality.
      // Note that we must still update the eeVec with the bounds since
      // the bound might be out of range (not in [0 - local_size))
      eeVec.push_back(TIDDesc(bound, dim, true, true, false, isGID));
      eeVec.push_back(TIDDesc(bound, dim, false, true, false, isGID));
      return true;
    } else {
      // In general we do not support case where single work item does not execute,
      // meaning the branch is one of the two options:
      // a. if (tid != bound) { kernel_code}
      // b. if (tid == bound) exit
      // However, if bound=0 we can treat this as exclusive lower bound since tid is
      // known to be >=0.
      Constant *constBound = dyn_cast<Constant>(bound);
      // In case the bound is not a constant try constant fold it.
      if (!constBound) {
        Instruction *instBound = dyn_cast<Instruction>(bound);
        if (instBound) {
          constBound = ConstantFoldInstruction(instBound);
        }
      }

      if (constBound && constBound->isNullValue()) {
        eeVec.push_back(TIDDesc(bound, dim, false, false, false, isGID));
        return true;
      }
    }

    // Could not handle non-relational compare.
    return false;
  }

  // Here is the support for relational compare {<, <= , >, >=}
  assert (isSupportedRelationalComparePredicate(pred) &&
            "unexpected relational cmp predicate");
  //Collect attributes of the compare instruction.
  bool isPredLower = isComparePredicateLower(pred); // is pred <, <=
  bool isInclusive = isComparePredicateInclusive(pred); // is pred <=, >=`
  bool isSigned = cmp->isSigned();

  // When deciding whether the uniform value is an upper bound, and whether it
  // is inclusive we need to take into consideration the index of uniform value
  // and the whether we exit if the condition is met.
  // for example assuming the compare is pattern is:
  //    %cond = icmp ult %tid, %uni (tid is get***id uni is uniform)
  //    br %cond label %BB1, label %BB2
  // If BB2 is return uni is an upper bound, and exclusive
  // if BB1 is return uni is lower bound and inclusive
  bool isUpper = isPredLower ^ (TIDInd == 1) ^ EETrueSide;
  bool containsVal = isInclusive ^ EETrueSide;

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
  // Sanity for release.
  if (!condInst) return false;

  // patterns supported are:
  // 1. Completely uniform test (that don't depend on TID)
  // 2. Icmp of TID against uniform value
  // 3. Ands of (1,2) for branching into the code (avoiding early exit)
  // 4. Or of (1,2) for branch into the early exit
  SmallVector<ICmpInst *, 4> compares;
  IVec uniformConds;
  if (isUniform(condInst)) {
    uniformConds.push_back(condInst);
  } else if (ICmpInst *cmp = dyn_cast<ICmpInst>(condInst) ) {
    compares.push_back(cmp);
  } else if (EETrueSide && condInst->getOpcode() == Instruction::Or) {
    if (!collectCond(compares, uniformConds, condInst)) return false;
  } else if (!EETrueSide && condInst->getOpcode() == Instruction::And) {
    if (!collectCond(compares, uniformConds, condInst)) return false;
  } else return false;

  //Check that compares have supported pattern.
  TIDDescVec eeVec;
  for(SmallVector<ICmpInst *, 4>::iterator cmpIt = compares.begin(),
       cmpE = compares.end(); cmpIt != cmpE; ++cmpIt) {
    // We need to be able to track the original tid call and the bound.
    Value *bound, *tid;
    if (!traceBackCmp(*cmpIt, bound, tid)) return false;
    // Finally we need to obtain the early exit description(s) into eeVec.
    if (!obtainBoundaryEE(*cmpIt, bound, tid, EETrueSide, eeVec)) return false;
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
        std::string name = (cast<CallInst>(it))->getCalledFunction()->getName().str();
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
    CallInst *localSize = LoopUtils::getWICall(
    m_M,
    CompilationUtils::mangledGetLocalSize(),
    m_indTy,
    dim,
    BB);
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
  // then we add check that it is positive.
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
    // In case there are early exits recover Instructions leading to them
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
    unsigned loopSizeInd = CLWGBoundDecoder::getIndexOfSizeAtDim(dim);
    retVal =
        InsertValueInst::Create(retVal, m_loopSizes[dim], loopSizeInd, "", BB);
    unsigned lowerInd = CLWGBoundDecoder::getIndexOfInitGIDAtDim(dim);
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
