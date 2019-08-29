// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "CLWGLoopCreator.h"
#include "LoopUtils/LoopUtils.h"
#include "CLWGBoundDecoder.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "MetadataAPI.h"
#include "CompilationUtils.h"
#include "OclTune.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DIBuilder.h"

#include <sstream>
#include <set>

static unsigned MAX_OCL_NUM_DIM = 3;

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

char CLWGLoopCreator::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(CLWGLoopCreator, "cl-loop-creator", "create loops opencl kernels", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(CLWGLoopCreator, "cl-loop-creator", "create loops opencl kernels", false, false)

CLWGLoopCreator::CLWGLoopCreator() :
    ModulePass(ID), m_indTy(nullptr), m_constZero(nullptr), m_constOne(nullptr),
    m_constPacket(nullptr), m_F(nullptr), m_M(nullptr), m_context(nullptr),
    m_newEntry(nullptr), m_scalarEntry(nullptr), m_vectorFunc(nullptr),
    m_packetWidth(0), m_numDim(0), m_rtServices(nullptr), m_scalarRet(nullptr),
    m_vectorRet(nullptr), m_EECall(nullptr), m_vectorizedDim(0)
{
  initializeCLWGLoopCreatorPass(*PassRegistry::getPassRegistry());
}

CLWGLoopCreator::~CLWGLoopCreator()
{
}

bool CLWGLoopCreator::runOnModule(Module &M) {
  using namespace Intel::MetadataAPI;

  bool changed = false;
  if (KernelList(&M).empty() ) {
    //Module contains no MetaData information, thus it contains no kernels
    return changed;
  }

  m_rtServices = static_cast<OpenclRuntime *>(getAnalysis<BuiltinLibInfo>().getRuntimeServices());
  assert(m_rtServices && "expected to have openCL runtime");

  for (auto *pFunc : KernelList(&M)) {
    assert(pFunc && "nullptr is not expected in KernelList!");
    auto skimd = KernelInternalMetadataAPI(pFunc);
    //No need to check if NoBarrierPath Value exists, it is guaranteed that
    //KernelAnalysisPass ran before CLWGLoopCreator pass.
    if (!skimd.NoBarrierPath.get()) {
      //Kernel that should be handled in barrier path, skip it.
      continue;
    }
    unsigned int vectWidth = 0;
    Function *vectKernel = nullptr;
    //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
    //Vectorized is running in all scenarios.
    if (skimd.VectorizedKernel.hasValue() && skimd.VectorizedKernel.get()) {
      //Set the vectorized function
      vectKernel = skimd.VectorizedKernel.get();
      auto vectkimd = KernelInternalMetadataAPI(vectKernel);
      //Set the vectorized width
      vectWidth = vectkimd.VectorizedWidth.get();

      //save the relevant information from the vectorized kernel in skimd
      //prior to erasing this information
      unsigned int vectorizeOnDim = vectkimd.VectorizationDimension.get();
      unsigned int canUniteWG = vectkimd.CanUniteWorkgroups.get();
      skimd.VectorizationDimension.set(vectorizeOnDim);
      skimd.CanUniteWorkgroups.set(canUniteWG);

      skimd.VectorizedKernel.set(nullptr);
      skimd.VectorizedWidth.set(vectWidth);
    }

    // We can create loops for this kernel - runOnFunction on it!!
    changed |= runOnFunction(*pFunc, vectKernel, vectWidth);
  }

  return changed;
}

unsigned CLWGLoopCreator::computeNumDim() {
  using namespace Intel::MetadataAPI;

  if (KernelList(m_F->getParent()).empty())
    return m_rtServices->getNumJitDimensions();
  auto skimd = KernelInternalMetadataAPI(m_F);
  if (skimd.MaxWGDimensions.hasValue())
    return skimd.MaxWGDimensions.get();
  return m_rtServices->getNumJitDimensions();
}

bool CLWGLoopCreator::runOnFunction(Function& F, Function *vectorFunc,
                                    unsigned packetWidth) {
  using namespace Intel::MetadataAPI;

  m_vectorizedDim = 0;
  if (vectorFunc != nullptr) {
    auto vkimd = KernelInternalMetadataAPI(vectorFunc);
    if (vkimd.VectorizationDimension.hasValue()) {
       m_vectorizedDim = vkimd.VectorizationDimension.get();
    }
  }

  bool SubGroupPath = KernelInternalMetadataAPI(&F).KernelHasSubgroups.get();

  // Update member fields with the current kernel.
  m_F = &F;
  m_M = F.getParent();
  m_vectorFunc = vectorFunc;
  m_packetWidth = packetWidth;
  m_context = &F.getContext();
  m_baseGids.assign(MAX_OCL_NUM_DIM, nullptr);
  generateConstants();

  // Collect get**id and return instructions from the kernels.
  m_scalarRet = getFunctionData(m_F, m_gidCallsSc, m_lidCallsSc);

  // Get the number of the for which we need to create work group loops.
  m_numDim = computeNumDim();

  // Mark scalar kernel entry and create new entry block for boundaries
  // calculation.
  m_scalarEntry = &m_F->getEntryBlock();
  m_scalarEntry->setName("scalar_kernel_entry");
  m_newEntry = BasicBlock::Create(*m_context, "", &F, m_scalarEntry);

  // Create early exit call to obtain boundaries from.
  m_EECall = createEECall();

  // Obtain loops boundaries from early exit call.
  getLoopsBoundaries();

  // Create WG loops.
  // If no work group loop are created (no calls to get***id) avoid
  // inlining the vector jit into the scalar since only one work item
  // need to be executed.
  loopRegion WGLoopRegion;
  if (SubGroupPath && m_numDim)
    WGLoopRegion = createVectorAndMaskedRemainderLoops();
  else
    WGLoopRegion = m_vectorFunc && m_numDim ?
      createVectorAndRemainderLoops() :
      AddWGLoops(m_scalarEntry, false, m_scalarRet, m_gidCallsSc, m_lidCallsSc,
                  m_initGIDs, m_loopSizes);

  assert(WGLoopRegion.m_preHeader && WGLoopRegion.m_exit &&
      "loops entry,exit not initialized");

  // Connect the new entry block with the WG loops.
  BranchInst::Create(WGLoopRegion.m_preHeader, m_newEntry);

  // Create return block and connect it to WG loops exit.
  // We must create separate block for the return since the it might be
  // that there are no WG loops (m_numDim=0) and WGLoopRegion.m_exit
  // is not empty.
  BasicBlock *newRet = BasicBlock::Create(*m_context, "", m_F);
  BranchInst::Create(newRet, WGLoopRegion.m_exit);
  ReturnInst::Create(*m_context, newRet);

  // Create conditional jump over the WG loops incase of uniform early exit.
  handleUniformEE(newRet);
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
loopRegion CLWGLoopCreator::createVectorAndRemainderLoops() {
  // Collect get**id and return instructions in the vector kernel.
  m_vectorRet = getFunctionData(m_vectorFunc, m_gidCallsVec, m_lidCallsVec);

  // Inline the vector kernel into the scalar kernel.
  BasicBlock *vecEntry = inlineVectorFunction(m_scalarEntry);

  // Obtain boundaries for the vector loops and scalar remainder loops.
  loopBoundaries dim0Boundaries =
    getVectorLoopBoundaries(m_initGIDs[m_vectorizedDim], m_loopSizes[m_vectorizedDim]);
  VVec initGIDs = m_initGIDs; // hard copy.
  VVec loopSizes = m_loopSizes; // hard copy.

  // Create vector loops.
  loopSizes[m_vectorizedDim] = dim0Boundaries.m_vectorLoopSize;
  loopRegion vectorBlocks = AddWGLoops(vecEntry, true, m_vectorRet,
                            m_gidCallsVec, m_lidCallsVec, initGIDs, loopSizes);

  // Create scalar loops.
  initGIDs[m_vectorizedDim] = dim0Boundaries.m_maxVector;
  loopSizes[m_vectorizedDim] = dim0Boundaries.m_scalarLoopSize;
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

loopRegion CLWGLoopCreator::createVectorAndMaskedRemainderLoops() {
  // Do not create scalar remainder for subgroup kernels
  // as the semantics does not allow execution of WIs in a serial
  // manner.
  // TODO: What we need here is masked vector remainder.
  // As soon as we have that we can add masked loop remainder.

  // Intentionally forget about scalar kernel. Let DCE delete it.

  // Collect get**id and return instructions in the vector kernel.
  m_vectorRet = getFunctionData(m_vectorFunc, m_gidCallsVec, m_lidCallsVec);

  // Inline the vector kernel into the scalar kernel.
  BasicBlock *vecEntry = inlineVectorFunction(m_scalarEntry);

  // Obtain boundaries for the vector loops and scalar loops.
  loopBoundaries dim0Boundaries =
     getVectorLoopBoundaries(m_initGIDs[m_vectorizedDim],
                             m_loopSizes[m_vectorizedDim]);
  VVec initGIDs = m_initGIDs; // hard copy.
  VVec loopSizes = m_loopSizes; // hard copy.

  // Create vector loops.
  loopSizes[m_vectorizedDim] = dim0Boundaries.m_vectorLoopSize;
  loopRegion vectorBlocks = AddWGLoops(vecEntry, true, m_vectorRet,
     m_gidCallsVec, m_lidCallsVec, initGIDs, loopSizes);

  // Create blocks to jump over the loops.
  BasicBlock *loopsEntry =
    BasicBlock::Create(*m_context, "vect_if",
                        m_F, vectorBlocks.m_preHeader);

  BasicBlock *retBlock = BasicBlock::Create(*m_context, "ret", m_F);

  // Execute the vector loops if(vectorLoopSize != 0).
  Instruction *vectcmp = new ICmpInst(*loopsEntry, CmpInst::ICMP_NE,
    dim0Boundaries.m_vectorLoopSize, m_constZero, "");
  BranchInst::Create(vectorBlocks.m_preHeader, retBlock, vectcmp, loopsEntry);

  BranchInst::Create(retBlock, vectorBlocks.m_exit);
  return loopRegion(loopsEntry, retBlock);
}

CallInst *CLWGLoopCreator::createEECall() {
  // Obtain early exit function. Function should have the same arguments
  // as the kernel.
  std::string funcName = m_F->getName().str();
  std::string EEFuncName = CLWGBoundDecoder::encodeWGBound(funcName);
  Function *EEFunc = m_M->getFunction(EEFuncName);
  assert(EEFunc && "early exit function must exist!!!");
  std::vector<Value *> args;
  unsigned i=0;
  for(Function::arg_iterator argIt = m_F->arg_begin(), argE = m_F->arg_end();
      argIt != argE; ++argIt, ++i) {
    Value* arg = &*argIt;
    // Sanity: checks that early exit function has the same argument types.
    assert(arg->getType() == m_F->getFunctionType()->getParamType(i) &&
        "mismatch types between function and Eearly exit");
    args.push_back(arg);
  }

  // Return a call in the new entry block.
  return CallInst::Create(EEFunc, llvm::makeArrayRef(args),
                                        "early_exit_call", m_newEntry);
}

void CLWGLoopCreator::handleUniformEE(BasicBlock *retBlock) {
  // Obtain uniform early exit condition.
  // If it is equal to 0 jump to return block.
  Instruction *loc = &*(++BasicBlock::iterator(m_EECall));
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

ReturnInst *CLWGLoopCreator::getFunctionData(Function *F, IVecVec &gids,
                                          IVecVec &lids) {
  // Collect all get_local_id, get_global_id and single return.
  std::string GID = CompilationUtils::mangledGetGID();
  std::string LID = CompilationUtils::mangledGetLID();
  LoopUtils::collectTIDCallInst(GID.c_str(), gids, F);
  LoopUtils::collectTIDCallInst(LID.c_str(), lids, F);
  BasicBlock *SingleRetBB =
      getAnalysis<UnifyFunctionExitNodes>(*F).getReturnBlock();
  if (!SingleRetBB) {
    // The function has no return instructions at all (it contains an infinite
    // loop for example)
    //
    // To handle this case we need to manually create a artificial exit block
    // which will be used as latch for WG loops
    BasicBlock *MergeRet = BasicBlock::Create(*m_context,
        "artificial_merge_ret", F);
    ReturnInst *RI = ReturnInst::Create(*m_context, MergeRet);

    return RI;
  }

  return cast<ReturnInst>(SingleRetBB->getTerminator());
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
    unsigned lowerInd = CLWGBoundDecoder::getIndexOfInitGIDAtDim(dim);
    Value *initGID =
        ExtractValueInst::Create(m_EECall, lowerInd, "", m_newEntry);
    unsigned loopSizeInd = CLWGBoundDecoder::getIndexOfSizeAtDim(dim);
    Value *loopSize =
        ExtractValueInst::Create(m_EECall, loopSizeInd, "", m_newEntry);
    m_initGIDs.push_back(initGID);
    m_loopSizes.push_back(loopSize);
  }
}

unsigned int CLWGLoopCreator::resolveDimension(unsigned int dim) {
  if (dim == 0)
    return m_vectorizedDim;
  else if (dim > m_vectorizedDim)
    return dim;
  else return dim-1;
}

loopRegion
CLWGLoopCreator::AddWGLoops(BasicBlock *kernelEntry, bool isVector,
      ReturnInst *ret, IVecVec &GIDs, IVecVec &LIDs, VVec &initGIDs,
      VVec &loopSizes) {
  assert(kernelEntry && ret && "uninitialized parameters");
  // Move allocas in the entry kernel entry block to the new entry block.
  CompilationUtils::moveAlloca(kernelEntry, m_newEntry);

  // Inital head and latch are the kernel entry and return block respectively.
  BasicBlock *head = kernelEntry;
  BasicBlock *latch = ret->getParent();

  // Erase original return instruction.
  ret->eraseFromParent();

  // Incase of vector kernel the tid generators are incremented by the packet
  // width. Incase of scalar loop increment by 1.
  Value *dim0IncBy = isVector ? m_constPacket : m_constOne;
  for (unsigned dim =0; dim < m_numDim; ++dim) {
    unsigned resolvedDim = resolveDimension(dim);
    compute_dimStr(resolvedDim, isVector);
    Value *incBy = resolvedDim == m_vectorizedDim ? dim0IncBy : m_constOne;
    //Create the loop.
    loopRegion blocks = LoopUtils::createLoop(head, latch, m_constZero, m_constOne,
                                   loopSizes[resolvedDim], m_dimStr, *m_context);

    // Modify get***id accordingly.
    Value *initGID = initGIDs[resolvedDim];
    if (GIDs[resolvedDim].size()) {
      replaceTIDsWithPHI(GIDs[resolvedDim], initGID, incBy, head, blocks.m_preHeader,
                         latch);
    }
    if (LIDs[resolvedDim].size()) {
      BinaryOperator *initLID = BinaryOperator::Create(Instruction::Sub, initGID,
          obtainBaseGID(resolvedDim), m_dimStr+"sub_lid", m_newEntry);
      initLID->setHasNoSignedWrap();
      initLID->setHasNoUnsignedWrap();
      replaceTIDsWithPHI(LIDs[resolvedDim], initLID, incBy, head, blocks.m_preHeader,
                         latch);
    }

    // head, latch for the next loop are the pre header and exit
    // block respectively.
    head = blocks.m_preHeader;
    latch = blocks.m_exit;
  }

  return loopRegion(head, latch);
}



void CLWGLoopCreator::replaceTIDsWithPHI(IVec &TIDs, Value *initVal,
     Value *incBy,BasicBlock *head, BasicBlock *preHead, BasicBlock *latch) {
  assert(TIDs.size() && "unexpected emty tid vector");
  PHINode *dimTID =
      PHINode::Create(m_indTy, 2, m_dimStr+"tid", head->getFirstNonPHI());
  BinaryOperator *incTID = BinaryOperator::Create(Instruction::Add, dimTID, incBy,
                                  m_dimStr+"inc_tid", latch->getTerminator());
  incTID->setHasNoSignedWrap();
  incTID->setHasNoUnsignedWrap();
  dimTID->addIncoming(initVal, preHead);
  dimTID->addIncoming(incTID, latch);
  for (IVec::iterator tidIt = TIDs.begin(), tidE = TIDs.end();
       tidIt != tidE; ++tidIt) {
    (*tidIt)->replaceAllUsesWith(dimTID);
    (*tidIt)->eraseFromParent();
  }
}

static void dropDISubprogram(Function *F) {
  if (F->hasMetadata(LLVMContext::MD_dbg))
    F->setSubprogram(nullptr);
}

// Remap DILocation of all instructions in the provided basic block
// to be belong the provided function scope.
static void fixDILocation(BasicBlock *BB, Function *ScopeF) {
  auto I = BB->begin();
  while (I != BB->end()) {
    // Current implementation couldn't clone debug intrinsics in proper way.
    // Workaround is to remove these intrinsics. TODO: implement correct
    // handling of cloned debug instrinsics
    if (isa<DbgVariableIntrinsic>(I)) {
      auto ToRemove = I;
      I++;
      ToRemove->eraseFromParent();
      continue;
    }

    if (DebugLoc DL = I->getDebugLoc())
      I->setDebugLoc(
          DebugLoc::get(DL.getLine(), DL.getCol(), ScopeF->getSubprogram()));
    I++;
  }
}

BasicBlock *CLWGLoopCreator::inlineVectorFunction(BasicBlock *BB) {
  // Create denseMap of function arguments
  ValueToValueMapTy valueMap;
  assert(m_F->getFunctionType() == m_vectorFunc->getFunctionType() &&
      "vector and scalar functtion type mismatch");
  Function::const_arg_iterator VArgIt = m_vectorFunc->arg_begin();
  Function::arg_iterator argIt = m_F->arg_begin();
  Function::arg_iterator argE = m_F->arg_end();
  for (; argIt != argE; ++argIt, ++VArgIt) {
    valueMap[&*VArgIt] = &*argIt;
  }

  // create a list for return values
  SmallVector<ReturnInst*, 2> returns;

  // Drop DISubprogram from the vector function to avoid it
  // duplicating while cloning
  dropDISubprogram(m_vectorFunc);

  // Do actual cloning work
  // TODO: replace manual inlining by llvm::InlineFunction()
  CloneFunctionInto(m_F, m_vectorFunc, valueMap, /*ModuleLevelChanges*/false,
                    returns, "vector_func");

  for (auto &VBB : *m_vectorFunc) {
    BasicBlock *clonedBB = dyn_cast<BasicBlock>(valueMap[&VBB]);
    clonedBB->moveBefore(BB);
    fixDILocation(clonedBB, m_F);
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
  m_vectorRet = cast<ReturnInst>(valueMap[m_vectorRet]);
  BasicBlock *vectorEntryBlock =
      dyn_cast<BasicBlock>(valueMap[&*(m_vectorFunc->begin())]);
  // copy stats from vector function to scalar function
  intel::Statistic::copyFunctionStats(*m_vectorFunc, *m_F);
  // Get hold of the entry to the scalar section in the vectorized function...
  assert (!m_vectorFunc->getNumUses() && "vector kernel should have no use");
  if (!m_vectorFunc->getNumUses()) {
    intel::Statistic::removeFunctionStats(*m_vectorFunc);
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
  ModulePass* createCLWGLoopCreatorPass() {
    return new intel::CLWGLoopCreator();
  }
}
