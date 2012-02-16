/*********************************************************************************************
 * Copyright ? 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include <sstream>
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Constants.h"
#include "CLWGLoopCreator.h"
#include "LoopUtils.h"
#include "CLWGBoundDecoder.h"
#include <set>

extern "C" void fillNoBarrierPathSet(Module *M, std::set<std::string>& noBarrierPath);

char intel::CLWGLoopCreator::ID = 0;
namespace intel {

CLWGLoopCreator::CLWGLoopCreator(SmallVectorImpl<Function*> *vectFunctions, 
                                 SmallVectorImpl<int> *vectWidths) : 
ModulePass(ID),
m_rtServices(static_cast<OpenclRuntime *>(RuntimeServices::get())),
m_vectFunctions(vectFunctions),
m_vectWidths(vectWidths)
{
  assert(m_rtServices);
}

CLWGLoopCreator::~CLWGLoopCreator()
{
}

bool CLWGLoopCreator::runOnModule(Module &M) {
  // First obtain kernels from metadata.
  SmallVector<Function *, 8> kernels;

  LoopUtils::GetOCLKernel(M, kernels);
  
  // Obtain vector kernel and widths from input buffer. 
  SmallVector<int, 8> vectWidths;
  SmallVector<Function *, 8> vectKernels;
  
  if (m_vectFunctions && m_vectFunctions->size()) {
    // vectorized functions exist and non empty so use it.
    assert(m_vectWidths && "null width vector");
    assert(m_vectFunctions->size() == kernels.size() &&
           m_vectWidths->size() == kernels.size() &&
           "mismatch size of width, functions vector");
    for (unsigned i=0, e = kernels.size(); i < e; ++i){
      vectWidths.push_back((*m_vectWidths)[i]);
      vectKernels.push_back((*m_vectFunctions)[i]);
    }
  } else {
    vectWidths.assign(kernels.size(), 0);
    vectKernels.assign(kernels.size(), NULL);
  }

  // Only process kernels that KernelAnalysis found sutiable.
  std::set<std::string> NoBarrier;
  fillNoBarrierPathSet(&M, NoBarrier);
  bool changed = false;
  for (unsigned i=0, e=kernels.size(); i<e; ++i) {
    Function *F = kernels[i];
    if (!F) continue;
    std::string funcName = F->getNameStr();
    if (!NoBarrier.count(funcName)) continue;
    
    // We can create loops for this kernel - runOnFunction on it!!
    changed |= runOnFunction(*F, vectKernels[i], vectWidths[i]);
    // The pass inlined and erased the vector Function.
    if (vectKernels[i]) (*m_vectFunctions)[i] = NULL;
  }
  return changed;
}

bool CLWGLoopCreator::runOnFunction(Function& F, Function *vectorFunc,
                                    unsigned packetWidth) {
  // Update member fields with the current kernel.
  m_F = &F;
  m_M = F.getParent();
  m_vectorFunc = vectorFunc;
  m_packetWidth = packetWidth;
  m_context = &F.getContext();
  m_numDim = m_rtServices->getNumJitDimensions();
  m_baseGids.assign(m_numDim, NULL);
  generateConstants();

  // Collect get**id and return instructions in the scalar kernel.
  m_scalarRet = getFunctionData(m_F, m_gidCallsSc, m_lidCallsSc);
    
  // Mark scalar kernel entry and create new entry block for boundaries
  // calculation.
  m_scalarEntry = &m_F->getEntryBlock();
  m_scalarEntry->setName("scalar_kernel_entry");
  m_newEntry = BasicBlock::Create(*m_context, "", &F, m_scalarEntry);

  // Create early exit call to obtain boundaries from.
  m_EECall = createEECall();
  
  // Obtain loops boundaries from early exit call.
  getLoopsBoundaries();

  // Create WG loops
  loopRegion WGLoopRegion = m_vectorFunc ? 
      createVectorAndRemainderLoops() : 
      AddWGLoops(m_scalarEntry, false, m_scalarRet, m_gidCallsSc, m_lidCallsSc,
                  m_initGIDs, m_loopSizes);
  
  assert(WGLoopRegion.m_preHeader && WGLoopRegion.m_exit &&
      "loops entry,exit not initialized");
  
  // Connect the new entry block with the WG loops.
  BranchInst::Create(WGLoopRegion.m_preHeader, m_newEntry);

  // Create Return on WG loops exit.
  ReturnInst::Create(*m_context, WGLoopRegion.m_exit);

  // create conditional jump over the WG loops incase of uniform early exit.
  handleUniformEE(WGLoopRegion.m_exit);
  return true;
}


// if the vector kernel exists than we create the following code:
// 
// max_vector = 
// if(vectorLoopSize != 0)
//   vector loops
// if (scalarLoopSize != 0)
//   scalar loops
// retrun 
CLWGLoopCreator::loopRegion CLWGLoopCreator::createVectorAndRemainderLoops(){
  // Collect get**id and return instructions in the vector kernel.
  m_vectorRet = getFunctionData(m_vectorFunc, m_gidCallsVec, m_lidCallsVec);

  // Inline the vector kernel into the scalar kernel.
  BasicBlock *vecEntry = inlineVectorFunction(m_scalarEntry);

  // Obtain boundaries for the vector loops and scalar remainder loops.
  loopBoundaries dim0Boundaries = 
      getVectorLoopBoundaries(m_initGIDs[0], m_loopSizes[0]);
  VVec initGIDs = m_initGIDs; // hard copy.
  VVec loopSizes = m_loopSizes; // hard copy.

  // Create vector loops.
  loopSizes[0] = dim0Boundaries.m_vectorLoopSize;
  loopRegion vectorBlocks = AddWGLoops(vecEntry, true, m_vectorRet,
                            m_gidCallsVec, m_lidCallsVec, initGIDs, loopSizes);

  // Create scalar loops.
  initGIDs[0] = dim0Boundaries.m_maxVector;
  loopSizes[0] = dim0Boundaries.m_scalarLoopSize;
  loopRegion scalarBlocks = AddWGLoops(m_scalarEntry, false, m_scalarRet,
                            m_gidCallsSc, m_lidCallsSc, initGIDs, loopSizes);
  
  // Create blocks to jump over the loops.
  BasicBlock *loopsEntry = 
      BasicBlock::Create(*m_context, "vect_if", m_F, vectorBlocks.m_preHeader);
  BasicBlock *scalarIf = 
      BasicBlock::Create(*m_context, "scalarIf", m_F, scalarBlocks.m_preHeader);
  BasicBlock *retBlock = BasicBlock::Create(*m_context, "ret", m_F);

  // Execute the vector loops if(vectorLoopSize != 0).
  Instruction *vectcmp = new ICmpInst(*loopsEntry, CmpInst::ICMP_NE,
      dim0Boundaries.m_vectorLoopSize, m_constZero, "");
  BranchInst::Create(vectorBlocks.m_preHeader, scalarIf, vectcmp, loopsEntry);
  BranchInst::Create(scalarIf, vectorBlocks.m_exit);

  // execute the scalar loops if(scalarLoopSize != 0)   
  Instruction *scalarCmp = new ICmpInst(*scalarIf, CmpInst::ICMP_NE,
      dim0Boundaries.m_scalarLoopSize, m_constZero, "");
  BranchInst::Create(scalarBlocks.m_preHeader, retBlock, scalarCmp, scalarIf);
  BranchInst::Create(retBlock, scalarBlocks.m_exit);
  return loopRegion(loopsEntry, retBlock);
}

void CLWGLoopCreator::moveAllocaToEntry(BasicBlock *BB) {
  Instruction *loc = m_newEntry->getFirstNonPHI();
  for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I!=E;) {
    AllocaInst *AI = dyn_cast<AllocaInst>(I++);
    if (AI) AI->moveBefore(loc);
  }
}

CallInst *CLWGLoopCreator::createEECall() {
  // Obtain early exit function. Function should have the same arguments
  // as the kernel.
  std::string funcName = m_F->getNameStr();
  std::string EEFuncName = CLWGBoundDecoder::encodeWGBound(funcName);
  Function *EEFunc = m_M->getFunction(EEFuncName);
  assert("early exit function must exist!!!");
  std::vector<Value *> args;
  unsigned i=0;
  for(Function::arg_iterator argIt = m_F->arg_begin(), argE = m_F->arg_end();
      argIt != argE; ++argIt, ++i) {
    // Sanity: checks that early exit function has the same argument types.
    assert(argIt->getType() == m_F->getFunctionType()->getParamType(i) && 
        "mismatch types between function and Eearly exit");
    args.push_back(argIt);
  }

  // Return a call in the new entry block.
  return CallInst::Create(EEFunc, llvm::makeArrayRef(args),
                                        "early_exit_call", m_newEntry);
}

void CLWGLoopCreator::handleUniformEE(BasicBlock *retBlock) {
  // Obtain uniform early exit condition.
  // If it is equal to 0 jump to return block.
  Instruction *loc = ++BasicBlock::iterator(m_EECall);
  unsigned uniInd = CLWGBoundDecoder::getUniformIndex();
  Instruction *uniEECond = ExtractValueInst::Create(m_EECall, uniInd, "", loc);
  Value *truncCond = 
      new TruncInst(uniEECond, IntegerType::get(*m_context, 1), "", loc);
  // Split the basic block after obtain the uniform early exit condition, 
  // and conditionally jump to the WG loops.
  BasicBlock *WGLoopsEntry = m_newEntry->splitBasicBlock(loc, "WGLoopsEntry");
  m_newEntry->getTerminator()->eraseFromParent();
  BranchInst::Create(WGLoopsEntry, retBlock, truncCond, m_newEntry);
}

void CLWGLoopCreator::generateConstants() {
  m_indTy = LoopUtils::getIndTy(m_M);
  m_constOne = ConstantInt::get(m_indTy, 1);
  m_constZero = ConstantInt::get(m_indTy, 0);
  m_constPacket = ConstantInt::get(m_indTy, m_packetWidth);
}

void CLWGLoopCreator::collectTIDCallInst(const char *name, IVecVec &tidCalls,
                                         Function *F) {
  IVec emptyVec;
  tidCalls.assign(3, emptyVec);
  SmallVector<CallInst *, 4> allDimTIDCalls;
  LoopUtils::getAllCallInFunc(name, F, allDimTIDCalls);
  
  for (unsigned i=0, e=allDimTIDCalls.size(); i<e; ++i) {
    CallInst *CI = allDimTIDCalls[i];
    ConstantInt *C = dyn_cast<ConstantInt>(CI->getArgOperand(0));
    assert(C && "tid arg must be constant");
    unsigned dim = C->getValue().getZExtValue();
    assert(dim <3 && "tid not in range");
    tidCalls[dim].push_back(CI);
  }
}


ReturnInst * CLWGLoopCreator::getSingleRet(Function *F) {
  assert(F && "null Function argument");
  
  // Get all return instructions in the kernel.
  typedef SmallVector<ReturnInst *, 2> RetVec;
  RetVec rets;
  for(Function::iterator bbi = F->begin(), bbe = F->end();
      bbi!=bbe; ++bbi) {
    if (ReturnInst *RI = dyn_cast<ReturnInst>(bbi->getTerminator())) {
      rets.push_back(RI);
    }
  }

  // If  there is only one return it.
  assert(rets.size() && "kernel with no return instructions");
  assert(F->getReturnType()->isVoidTy() && "return should be void");
  if (rets.size() == 1) return rets[0];
  
  // More than single ret merge them all into a single basic block
  BasicBlock *mergeRet = BasicBlock::Create(*m_context, "merge_ret", F );
  for(RetVec::iterator retI = rets.begin(), retE = rets.end();
      retI!=retE; ++retI) {
    BasicBlock *bb = (*retI)->getParent();
    (*retI)->eraseFromParent();
    BranchInst::Create(mergeRet, bb);
  }
  ReturnInst *RI = ReturnInst::Create(*m_context, mergeRet);
  return RI;
}


ReturnInst *CLWGLoopCreator::getFunctionData(Function *F, IVecVec &gids,
                                          IVecVec &lids) {
  // Collect all get_local_id, get_global_id and single return.
  collectTIDCallInst(GET_GID_NAME, gids, F);
  collectTIDCallInst(GET_LID_NAME, lids, F);
  return getSingleRet(F);
}


Instruction *CLWGLoopCreator::obtainBaseGID(unsigned dim) {
 // If it is already been cachced return the cached value.
 assert(m_baseGids.size() > dim && "base gid vec uninitialized");
 if (m_baseGids[dim]) return m_baseGids[dim];
 
 // Calculate the value.
 const char *baseGID = m_rtServices->getBaseGIDName();
 m_baseGids[dim] = LoopUtils::getWICall(m_M, baseGID, m_indTy, dim, m_newEntry,
                                        "gid_ref");
 return m_baseGids[dim];
}

void CLWGLoopCreator::compute_dimStr(unsigned dim, bool isVector) {
   std::stringstream dimStream;
   dimStream << "dim_" << dim << "_";
   if (isVector) dimStream << "vector_";
   m_dimStr = dimStream.str();
}

void CLWGLoopCreator::getLoopsBoundaries() {
  // Extract the initial global id and loop sizes from the early exit call.
  m_loopSizes.clear();
  m_initGIDs.clear();
  for (unsigned dim = 0; dim < m_numDim; ++dim) {
    unsigned lowerInd = CLWGBoundDecoder::getInitGIDIndex(dim);
    Value *initGID =
        ExtractValueInst::Create(m_EECall, lowerInd, "", m_newEntry);
    unsigned loopSizeInd = CLWGBoundDecoder::getSizeIndex(dim);
    Value *loopSize =  
        ExtractValueInst::Create(m_EECall, loopSizeInd, "", m_newEntry);
    m_initGIDs.push_back(initGID);
    m_loopSizes.push_back(loopSize);
  }
}

CLWGLoopCreator::loopRegion 
CLWGLoopCreator::AddWGLoops(BasicBlock *kernelEntry, bool isVector, 
      ReturnInst *ret, IVecVec &GIDs, IVecVec &LIDs, VVec &initGIDs,
      VVec &loopSizes) {
  assert(kernelEntry && ret && "uninitialized parameters");
  // Move allocas in the entry kernel entry block to the new entry block.
  moveAllocaToEntry(kernelEntry);

  // Inital head and latch are the kernel entry and return block respectively.
  BasicBlock *head = kernelEntry;
  BasicBlock *latch = ret->getParent();

  // Erase original return instruction.
  ret->eraseFromParent();

  // Incase of vector kernel the tid generators are incremented by the packet 
  // width. Incase of scalar loop increment by 1.
  Value *dim0IncBy = isVector ? m_constPacket : m_constOne;
  for (unsigned dim =0; dim < m_numDim; ++dim) {
    compute_dimStr(dim, isVector);
    Value *incBy = dim == 0 ? dim0IncBy : m_constOne;
    //Create the loop. 
    loopRegion blocks = createLoop(head, latch, loopSizes[dim]);

    // Modify get***id accordingly.
    Value *initGID = initGIDs[dim];
    if (GIDs[dim].size()) {
      replaceTIDsWithPHI(GIDs[dim], initGID, incBy, head, blocks.m_preHeader,
                         latch);
    }
    if (LIDs[dim].size()) {
      Value *initLID = BinaryOperator::Create(Instruction::Sub, initGID,
          obtainBaseGID(dim), m_dimStr+"sub_lid", m_newEntry);
      replaceTIDsWithPHI(LIDs[dim], initLID, incBy, head, blocks.m_preHeader,
                         latch);
    }

    // head, latch for the next loop are the pre header and exit 
    // block respectively.
    head = blocks.m_preHeader;
    latch = blocks.m_exit;
  }
   
  return loopRegion(head, latch);
}


// transforms code as follows:
// prehead:
//     br head
// head:
//     indVar = phi (0 preHead, latch incIndVar)
//          :
// latch: //(can be the same as head)
//          :
//     incIndVar = add indVar, 1
//     x = Icmp eq incIndVar ,loopSize
//     br x, exit, oldEntry
// exit:
//     
//  all get_local_id \ get_global_id are replaced with indVar
CLWGLoopCreator::loopRegion 
CLWGLoopCreator::createLoop(BasicBlock *head, BasicBlock *latch,
                            Value *loopSize) {  
  // Creating Blocks to wrap the code as described above.
  BasicBlock *preHead = 
      BasicBlock::Create(*m_context, m_dimStr+"pre_head", m_F, head);
  BasicBlock *exit = 
      BasicBlock::Create(*m_context, m_dimStr+"exit", m_F);
  exit->moveAfter(latch);
  BranchInst::Create(head, preHead);
  
  // Insert induction variable phi in the head entry.
  PHINode *indVar = head->empty() ?
      PHINode::Create(m_indTy, 2, m_dimStr+"ind_var", head) :
      PHINode::Create(m_indTy, 2, m_dimStr+"ind_var", head->begin());
  
  // Increment induction variable.
  Value *incIndVar = BinaryOperator::Create(Instruction::Add, indVar, 
                                    m_constOne, m_dimStr+"inc_ind_var", latch);
  
  // Create compare and conditionally branch out from latch.
  Instruction *compare = new ICmpInst(*latch, CmpInst::ICMP_EQ, incIndVar,
                                  loopSize, m_dimStr+"cmp.to.max");
  BranchInst::Create(exit, head, compare, latch);
  
  // Upadte induction variable phi with the incoming values.
  indVar->addIncoming(m_constZero, preHead);  
  indVar->addIncoming(incIndVar, latch);  
  return loopRegion(preHead, exit);
}


void CLWGLoopCreator::replaceTIDsWithPHI(IVec &TIDs, Value *initVal,
     Value *incBy,BasicBlock *head, BasicBlock *preHead, BasicBlock *latch) {
  assert(TIDs.size() && "unexpected emty tid vector");
  PHINode *dimTID = 
      PHINode::Create(m_indTy, 2, m_dimStr+"tid", head->getFirstNonPHI());
  Value *incTID = BinaryOperator::Create(Instruction::Add, dimTID, incBy, 
                                  m_dimStr+"inc_tid", latch->getTerminator());
  dimTID->addIncoming(initVal, preHead);  
  dimTID->addIncoming(incTID, latch);
  for (IVec::iterator tidIt = TIDs.begin(), tidE = TIDs.end();
       tidIt != tidE; ++tidIt) {
    (*tidIt)->replaceAllUsesWith(dimTID);
    (*tidIt)->eraseFromParent();
  }
}


/*
BasicBlock *CLWGLoopCreator::inlineVectorFunction(BasicBlock *BB) {
  // this is a fast way for inlining by just moving the block list
  // of the vector kernel into scalar kernel.
  assert(m_vectorFunc && "should not be called on null"); 
  assert(m_F->getFunctionType() == m_vectorFunc->getFunctionType() &&
      "vector and scalar functtion type mismatch"); 
  assert(BB && "uninitialized parameters");
  
  BasicBlock * vectorEntryBlock = &m_vectorFunc->getEntryBlock();
  vectorEntryBlock->setName("vector_kernel_entry");
    // insert block of func into m_F just before callBlock
  m_F->getBasicBlockList().splice(BB, m_vectorFunc->getBasicBlockList());
  Function::arg_iterator VArgIt = m_vectorFunc->arg_begin();
  Function::arg_iterator argIt = m_F->arg_begin();
  Function::arg_iterator argE = m_F->arg_end();
  for (; argIt != argE; ++argIt, ++VArgIt) {
 	VArgIt->replaceAllUsesWith(argIt);
  }
  assert (!m_vectorFunc->getNumUses() && "vector kernel should have no use");
  if (!m_vectorFunc->getNumUses()) {
    m_vectorFunc->eraseFromParent();
  }
  return vectorEntryBlock;
}
/*/

BasicBlock *CLWGLoopCreator::inlineVectorFunction(BasicBlock *BB) {
  // Create denseMap of function arguments
  ValueToValueMapTy valueMap;
  assert(m_F->getFunctionType() == m_vectorFunc->getFunctionType() &&
      "vector and scalar functtion type mismatch"); 
  Function::const_arg_iterator VArgIt = m_vectorFunc->arg_begin();
  Function::const_arg_iterator VArgE = m_vectorFunc->arg_end();
  Function::arg_iterator argIt = m_F->arg_begin();
  Function::arg_iterator argE = m_F->arg_end();
  for (; argIt != argE; ++argIt, ++VArgIt) {
 	valueMap[VArgIt] = argIt;
  }
  // create a list for return values
  SmallVector<ReturnInst*, 2> returns;

  // Do actual cloning work
  CloneFunctionInto(m_F, m_vectorFunc, valueMap, false, returns, "vector_func");
  for(Function::iterator bbit = m_vectorFunc->begin(),
      bbe = m_vectorFunc->end(); bbit != bbe; ++bbit){
    BasicBlock *clonedBB = dyn_cast<BasicBlock>(valueMap[bbit]);
    clonedBB->moveBefore(BB);
  }

  for (unsigned dim=0; dim<m_numDim; ++dim) {
    for (unsigned i=0, e = m_gidCallsVec[dim].size(); i<e; ++i) {
      Instruction *inst = dyn_cast<Instruction>(valueMap[m_gidCallsVec[dim][i]]);
      assert(inst);
      m_gidCallsVec[dim][i] = inst;
    }
    for (unsigned i=0, e = m_lidCallsVec[dim].size(); i<e; ++i) {
      Instruction *inst = dyn_cast<Instruction>(valueMap[m_lidCallsVec[dim][i]]);
      assert(inst);
      m_lidCallsVec[dim][i] = inst;
    }
  }
  m_vectorRet = dyn_cast<ReturnInst>(valueMap[m_vectorRet]);
  BasicBlock *vectorEntryBlock = 
      dyn_cast<BasicBlock>(valueMap[m_vectorFunc->begin()]);
  // Get hold of the entry to the scalar section in the vectorized function...
  assert (!m_vectorFunc->getNumUses() && "vector kernel should have no use");
  if (!m_vectorFunc->getNumUses()) {
    m_vectorFunc->eraseFromParent();
  }
  return vectorEntryBlock;
}

//*/

CLWGLoopCreator::loopBoundaries 
CLWGLoopCreator::getVectorLoopBoundaries(Value *initVal, Value *dimSize) {
  // computes constant log packetWidth
  assert( m_packetWidth && ((m_packetWidth & (m_packetWidth-1)) == 0) &&
      "packet width is not power of 2");
  unsigned logPacket = 0;
  for ( unsigned i=1; i < m_packetWidth; i*=2) ++logPacket;
  Constant *logPacketConst = ConstantInt::get(m_indTy, logPacket);
  
  // vector loops size can be derived by shifting size with log packet bits.
  Value *vectorLoopSize = BinaryOperator::Create(Instruction::AShr, dimSize,
                                     logPacketConst, "vector.size", m_newEntry);
  Value *numVectorWI = BinaryOperator::Create(Instruction::Shl, vectorLoopSize,
                      logPacketConst, "num.vector.wi", m_newEntry);
  Value *maxVector = BinaryOperator::Create(Instruction::Add, numVectorWI,
                                        initVal, "max.vector.gid", m_newEntry);
  Value *scalarLoopSize = BinaryOperator::Create(Instruction::Sub, dimSize,
                                    numVectorWI, "scalar.size", m_newEntry);
  return loopBoundaries(vectorLoopSize, scalarLoopSize, maxVector);
}




}// namespace intel


extern "C" {
  ModulePass* createCLWGLoopCreatorPass(
      SmallVectorImpl<Function*> *vectFunctions = NULL,
      SmallVectorImpl<int> *vectWidths = NULL) {
    return new intel::CLWGLoopCreator(vectFunctions, vectWidths);
  }
}
static RegisterPass<intel::CLWGLoopCreator> CLWGLoopCreator("cl-loop-creator", "create loops opencl kernels");
