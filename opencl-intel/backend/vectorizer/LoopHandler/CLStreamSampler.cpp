/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "CLStreamSampler.h"
#include "LoopUtils/LoopUtils.h"
#include "CLWGBoundDecoder.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"
#include "VectorizerUtils.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"

#define MAX_LOOP_SIZE 1024

#define FLOAT_X_WIDTH__ALIGNMENT 16

namespace intel {

char intel::CLStreamSampler::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(CLStreamSampler, "cl-stream-sampler", "replace read,write image built-ins in loops with stream samplers if possible", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(LoopWIAnalysis)
OCL_INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(CLStreamSampler, "cl-stream-sampler", "replace read,write image built-ins in loops with stream samplers if possible", false, false)

CLStreamSampler::CLStreamSampler() : LoopPass(ID), m_rtServices(NULL) {
  initializeCLStreamSamplerPass(*PassRegistry::getPassRegistry());
}

bool CLStreamSampler::runOnLoop(Loop *L, LPPassManager &LPM) {
  //errs() << "CLStreamSampler on " << L->getHeader()->getNameStr() << "\n";
  if (!L->isLoopSimplifyForm()) return false;

  m_rtServices = static_cast<OpenclRuntime *>(getAnalysis<BuiltinLibInfo>().getRuntimeServices());
  assert(m_rtServices && "runtime services does not exist");

  m_curLoop = L;
  m_header = m_curLoop->getHeader();
  m_context = &m_header->getContext();
  m_M = m_header->getParent()->getParent();
  m_DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  m_WIAnalysis = &getAnalysis<LoopWIAnalysis>();
  m_preHeader = m_curLoop->getLoopPreheader();
  m_latch = m_curLoop->getLoopLatch();
  assert(m_latch && m_preHeader && "should have latch and pre header");
  m_exit = m_curLoop->getExitBlock();

  // Create usefull constants.
  m_one = ConstantInt::get(*m_context, APInt(32, 1));
  m_zero = ConstantInt::get(*m_context, APInt(32, 0));

  // Clear data structures.
  m_streamSize.clear();
  m_readImageAttributes.clear();
  m_writeImageAttributes.clear();
  m_alwaysExecuteOnceBlocks.clear();
  m_firstIterVal.clear();

  // Obtain canonical induction variable and trip count.
  // Work group loops are generated in a way that these should be obtained by
  // standard LLVM api.
  m_indVar = L->getCanonicalInductionVariable();
  if (!m_indVar)
    return false;
  m_tripCount = getTripCountValue(L, m_indVar);
  if (!m_indVar || !m_tripCount) return false;

  // Obtain upper bound on the trip count of the loop.
  m_tripCountUpperBound = getTripCountUpperBound(m_tripCount);
  if (!m_tripCountUpperBound) return false;

  // Get the blocks that always execute once to look for stream opportunities.
  LoopUtils::getBlocksExecutedExactlyOnce(m_curLoop, m_DT,
                                        m_alwaysExecuteOnceBlocks);

  // Replace transposed read_image calls with read_stream call in pre header.
  hoistReadImgCalls();

  // Provided that the loop has exit block (WG loops have exit block),Replace
  // transposed write_image calls with write_stream call in exit block
  if (m_exit) sinkWriteImgCalls();

  // Currently We do not keep the data whether anything changed so for safety
  // return always true.
  return true;
}
  
/// Ripped out of 3.0 LLVM LoopInfo, as a workaround for it not being available in 3.2
/// getTripCount - Return a loop-invariant LLVM value indicating the number of
/// times the loop will be executed.
Value *CLStreamSampler::getTripCountValue(Loop* L, PHINode *IV) const {
  assert(L && IV && "Must pass initialized loop and induction variable");
  // Canonical loops will end with a 'cmp ne I, V', where I is the incremented
  // canonical induction variable and V is the trip count of the loop.
  if (IV->getNumIncomingValues() != 2) return 0;
  
  bool P0InLoop = L->contains(IV->getIncomingBlock(0));
  Value *Inc = IV->getIncomingValue(!P0InLoop);
  BasicBlock *BackedgeBlock = IV->getIncomingBlock(!P0InLoop);
  
  if (BranchInst *BI = dyn_cast<BranchInst>(BackedgeBlock->getTerminator()))
    if (BI->isConditional()) {
      if (ICmpInst *ICI = dyn_cast<ICmpInst>(BI->getCondition())) {
        if (ICI->getOperand(0) == Inc) {
          if (BI->getSuccessor(0) == L->getHeader()) {
            if (ICI->getPredicate() == ICmpInst::ICMP_NE)
              return ICI->getOperand(1);
          } else if (ICI->getPredicate() == ICmpInst::ICMP_EQ) {
            return ICI->getOperand(1);
          }
        }
      }
    }
  
  return 0;
}

unsigned CLStreamSampler::getTripCountUpperBound(Value *tripCount) {
  // If the Loop has constant trip count return it provided that it
  // is not too big.
  if (ConstantInt *CI = dyn_cast<ConstantInt>(tripCount)) {
    unsigned tripConst = CI->getZExtValue();
    if (tripConst <= MAX_LOOP_SIZE) return tripConst;
    return 0;
  }

  // Will hold a value to divide in case the trip count is obtiained by ashr.
  // This the common scenario for vector WG loop.
  unsigned divideBy = 1;
  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(tripCount)) {
    if (BO->getOpcode() == Instruction::AShr ) {
      if (ConstantInt *numBits = dyn_cast<ConstantInt>(BO->getOperand(1))) {
        divideBy = 1 << numBits->getZExtValue();
        tripCount = BO->getOperand(0);
      }
    }
  }

  // Now check if the trip count is Extract from the WG boundaries vector.
  if (ExtractValueInst *EVI = dyn_cast<ExtractValueInst>(tripCount)) {
    if (EVI->getNumIndices() == 1 &&
        *(EVI->idx_begin()) == CLWGBoundDecoder::getIndexOfSizeAtDim(0)) {
      if (CallInst *eeCall = dyn_cast<CallInst>(EVI->getAggregateOperand())) {
        std::string funcName = eeCall->getCalledFunction()->getName().str();
        if (CLWGBoundDecoder::isWGBoundFunction(funcName)) {
          // Trip is get_local_size(0) return known bound.
          return MAX_LOOP_SIZE / divideBy;
        }
      }
    }
  }

  // unable to identify upper bound return 0.
  return 0;
}

void CLStreamSampler::getReadImgAttributes() {
  // Go over all instruction in blocks that execute exactly once in the loop.
  for (unsigned  i=0, e = m_alwaysExecuteOnceBlocks.size(); i < e; ++i) {
    BasicBlock *BB = m_alwaysExecuteOnceBlocks[i];
    for (BasicBlock::iterator BBIt = BB->begin(), BBE = BB->end(); BBIt!= BBE; ++BBIt) {
      // Check if this instruction is a read image call that can be hoisted.
      // If so it will update m_readImageAttributes member.
      CollectReadImgAttributes(dyn_cast<CallInst>(BBIt));
    }
  }
}

bool CLStreamSampler::isHeaderPhiStrided(Value *v ) {
  PHINode *phi = dyn_cast<PHINode>(v);
  if (!phi || //coordinate is not a phi
      phi->getParent() != m_header || // not phi in the loop header
      !m_WIAnalysis->isStrided(phi)) { // not strided
    return false;
  }
  return true;
}

void CLStreamSampler::CollectReadImgAttributes(CallInst *readImgCall) {
  // First check that this is a tranposed read_image call.
  if (!readImgCall) return;
  Function *calledFunc = readImgCall->getCalledFunction();
  if (!calledFunc) return;
  std::string funcName = calledFunc->getName().str();
  if (!m_rtServices->isTransposedReadImg(funcName)) return;

  // Obtain entry in the builtin hash.
  std::auto_ptr<VectorizerFunction> foundFunction =
    m_rtServices->findBuiltinFunction(funcName);
  assert(!foundFunction->isNull() && "should have hash entry");
  assert(foundFunction->getWidth() > 1 && "func should be soa_write_image");
  // Currently supports only transposed write.
  if (foundFunction->isNull()  || foundFunction->getWidth() <= 1) return;

  TranspReadImgAttr attrs;
  attrs.m_call = readImgCall;
  attrs.m_width = foundFunction->getWidth();
  attrs.m_img = readImgCall->getArgOperand(0);
  attrs.m_sampler = readImgCall->getArgOperand(1);
  // Sampler and Image should be invariant.
  if (!m_curLoop->isLoopInvariant(attrs.m_img)) return;
  if (!m_curLoop->isLoopInvariant(attrs.m_sampler)) return;

  // support only stream of transposed read_image.
  if (attrs.m_width <= 1) return;

  // Obtain the coord arguments.
  VectorType *coordTy =
      VectorType::get(Type::getFloatTy(*m_context), attrs.m_width);
  attrs.m_firstCoords.clear();
  for (unsigned i=2; i<4; ++i) {
    Value *arg = readImgCall->getArgOperand(i);
    Value *root = VectorizerUtils::RootInputArgument(arg, coordTy, attrs.m_call);
    if (!root) return;

    if (m_WIAnalysis->isUniform(root)) {
      // Note that isUniform is stronger than isLoopInvariant, since
      // it demands that all vector entries are the same.
      attrs.m_firstCoords.push_back(root);
    } else if (m_WIAnalysis->isStrided(root)) {
      assert(isa<Instruction>(root) && "strided should be instrution");
      Value *initialVal = obtainInitialStridedVal(cast<Instruction>(root));
      assert(initialVal && "could not get inital strided value");
      if (!initialVal) return;
      attrs.m_firstCoords.push_back(initialVal);
    } else {
      return;
    }
  }

  // Collect the colors returned by pointers as the last 4 arguments
  assert (readImgCall->getType()->isVoidTy() && "unexpected return type");
  attrs.m_colors.clear();
  for (unsigned i=4; i<8; ++i) {
    // arguments should be an alloca whose users are the call and the load.
    AllocaInst *AI = dyn_cast<AllocaInst>(readImgCall->getArgOperand(i));
    if (!AI || AI->getNumUses() > 2) return;

    // In case the pointer has only one user it is the call, meaning this color
    // was not used so we just continue.
    if (AI->hasOneUse()) {
      attrs.m_colors.push_back(NULL);
      continue;
    }

    // The pointer has 2 users
    Value *user1 = *(AI->user_begin());
    Value *user2 = *(++(AI->user_begin()));
    LoadInst *load = NULL;
    if (user1 == readImgCall) {
      load = dyn_cast<LoadInst>(user2);
    } else if (user2 == readImgCall){
      load = dyn_cast<LoadInst>(user1);
    }
    if (!load) return;
    attrs.m_colors.push_back(load);
  }

  // Success! Store the attributes in the member small vector.
  m_readImageAttributes.push_back(attrs);
}

void CLStreamSampler::hoistReadImgCalls() {
  // Obtain the stream read from the runtime module.
  bool isPointer64 = m_M->getDataLayout().getPointerSizeInBits(0) == 64;
  Function *LibReadSteamFunc = m_rtServices->getReadStream(isPointer64);
  if (!LibReadSteamFunc) return;
  Function * readStreamFunc = getLibraryFunc(LibReadSteamFunc);
  assert(readStreamFunc && "failed to get read stream func");
  if (!readStreamFunc) return;

  // Obtain attributes of read_image calls.
  getReadImgAttributes();

  // Replace the read image calls with stream calls.
  for (unsigned  i = 0, e = m_readImageAttributes.size(); i < e; ++i) {
    hoistReadImgCall(m_readImageAttributes[i], readStreamFunc);
  }
}

std::pair<Value *, Value *>
CLStreamSampler::createStartStride(Value *coord, Instruction *loc) {
  Value *coord0 = ExtractElementInst::Create(coord, m_zero, "coord.0", loc);
  Value *stride = Constant::getNullValue(coord0->getType());
  if (!m_WIAnalysis->isUniform(coord)) {
    Value *coord1 = ExtractElementInst::Create(coord, m_one, "coord.1", loc);
    assert(coord0->getType()->isFPOrFPVectorTy() && coord1->getType()==coord0->getType() &&
        "expected floating point coordinates" );
    stride = BinaryOperator::Create(
        Instruction::FSub , coord1, coord0, "stride", loc);
  }
  return std::pair<Value *, Value *> (coord0, stride);
}

void CLStreamSampler::hoistReadImgCall(TranspReadImgAttr &attr,
                                            Function *readStreamFunc) {
  // Generate 4 huge Alloca's for storing all the RGBA data...
  SmallVector<Instruction *, 4> colorAllocas;
  SmallVector<Instruction *, 4> colorStorage;
  generateAllocasForStream(attr.m_width, colorAllocas, colorStorage);

  // Pre-prepare index values for input to stream sampler
  VectorType *float2Ty = VectorType::get(Type::getFloatTy(*m_context), 2);
  UndefValue *undefF2 = UndefValue::get(float2Ty);

  // Obtain original  xCoord yCoord and stream size
  Value *xCoord = attr.m_firstCoords[0];
  Value *yCoord = attr.m_firstCoords[1];
  Value *streamSize = getStreamSize(attr.m_width);

  Instruction *loc = m_preHeader->getTerminator();
  // Generate start, stride vector
  std::pair<Value *, Value *> xStartStride = createStartStride(xCoord, loc);
  std::pair<Value *, Value *> yStartStride = createStartStride(yCoord, loc);
  Value *xStart = xStartStride.first;
  Value *yStart = yStartStride.first;
  Value *xStride = xStartStride.second;
  Value *yStride = yStartStride.second;
  Value *start= InsertElementInst::Create(undefF2, xStart, m_zero,"start.0", loc);
  start = InsertElementInst::Create(start, yStart, m_one,"start.1", loc);
  Value *stride = InsertElementInst::Create(undefF2, xStride, m_zero, "stride.0", loc);
  stride = InsertElementInst::Create(stride, yStride , m_one, "stride.1", loc);

  // Prepare arguments for calling the stream sampler.
  FunctionType *streamFuncTy = readStreamFunc->getFunctionType();
  SmallVector<Value *, 9> args;
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                              attr.m_img, streamFuncTy->getParamType(0), loc));
  args.push_back(attr.m_sampler);
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                              start, streamFuncTy->getParamType(2), loc));
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                              stride, streamFuncTy->getParamType(3), loc));
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                              streamSize, streamFuncTy->getParamType(4), loc));
  args.push_back(colorStorage[0]);
  args.push_back(colorStorage[1]);
  args.push_back(colorStorage[2]);
  args.push_back(colorStorage[3]);

  // Generate actual stream samler call instruction.
  CallInst::Create(readStreamFunc, llvm::makeArrayRef(args), "", loc);

  // Replace transposed output with loads from buffers.
  SmallVector<Value *, 2> indices(2, m_indVar);
  indices[0] = ConstantInt::get(m_indVar->getType(), 0);
  ArrayRef<Value *> indicesArr = llvm::makeArrayRef(indices);
  for (unsigned i=0; i<4; ++i) {
    LoadInst *LI = attr.m_colors[i];
    if (!LI) continue;

    // Load from the buffer.
    Value *colorPointer = GetElementPtrInst::CreateInBounds(
                     colorAllocas[i], indicesArr, "calc.address", attr.m_call);
    Value *transpValueLoad = new LoadInst(colorPointer,
               "load.trnsp.val", false, FLOAT_X_WIDTH__ALIGNMENT, attr.m_call);
    LI->replaceAllUsesWith(transpValueLoad);
    LI->eraseFromParent();
  }

  attr.m_call->eraseFromParent();
  //RecursivelyDeleteTriviallyDeadInstructions(attr.m_coordArr);
  for (unsigned i=0; i<2 ; ++i) {
    if (isHeaderPhiStrided(attr.m_firstCoords[i]) ) {
      removeRedundantPHI(cast<PHINode>(attr.m_firstCoords[i]));
    }
  }
}

Value *CLStreamSampler::getStreamSize(unsigned width) {
  // If stream size for this width was already computed return it.
  for (unsigned i=0, e = m_streamSize.size(); i<e; ++i) {
    if (m_streamSize[i].first == width ) return m_streamSize[i].second;
  }

  // Stream size for the width was not computed yet. Calculate it now.
  Value *widthConst = ConstantInt::get(m_tripCount->getType(), width);
  // Default location in case trip count is constant \ global
  Instruction *loc = m_header->getParent()->getEntryBlock().getFirstNonPHI();
  assert(m_tripCount && "NULL trip count ?");
  if (Instruction *tripCountInst = dyn_cast<Instruction>(m_tripCount)) {
    assert(tripCountInst != tripCountInst->getParent()->getTerminator() &&
        "trip count can not be the termiantor of a block");
    loc = &*(++(BasicBlock::iterator(tripCountInst)));
  }
  Value *streamSize = BinaryOperator::Create(Instruction::Mul, m_tripCount,
                                             widthConst, "stream_size", loc);
  m_streamSize.push_back(std::pair<unsigned, Value *>(width, streamSize));
  return streamSize;
}

void CLStreamSampler::sinkWriteImgCalls() {
  // Obtain the stream write from the runtime module.
  bool isPointer64 = m_M->getDataLayout().getPointerSizeInBits(0) == 64;
  Function *LibWriteStreamFunc = m_rtServices->getWriteStream(isPointer64);
  if (!LibWriteStreamFunc) return;
  Function * writeStreamFunc = getLibraryFunc(LibWriteStreamFunc);
  assert(writeStreamFunc && "failed to get write stream func");
  if (!writeStreamFunc) return;

  // Obtain attributes of read_image calls.
  getWriteImgAttributes();

  // Replace the write image calls with stream calls.
  for (unsigned  i = 0, e = m_writeImageAttributes.size(); i < e; ++i) {
    sinkWriteImgCall(m_writeImageAttributes[i], writeStreamFunc);
  }
}

Function *CLStreamSampler::getLibraryFunc(Function *LibFunc) {
  assert(LibFunc && "null argument");
  
  // We do not use getOrInsertFunction() here because we may already
  // have the function, but with a slightly wrong prototype, because
  // images are opaque types. So, first check if we have the function,
  // and only if we don't, insert it.
  Constant* funcConst = m_M->getFunction(LibFunc->getName());
  if (!funcConst) {
    using namespace Intel::OpenCL::DeviceBackend;
    funcConst = CompilationUtils::importFunctionDecl(m_M, LibFunc);
  }
    
  Function *F = dyn_cast<Function>(funcConst);
  assert(F && "failed to insert declaration");
  return F;
}

void CLStreamSampler::getWriteImgAttributes() {
  // Go over all instruction in blocks that execute exactly once in the loop.
  for (unsigned  i=0, e = m_alwaysExecuteOnceBlocks.size(); i < e; ++i) {
    BasicBlock *BB = m_alwaysExecuteOnceBlocks[i];
    for (BasicBlock::iterator BBIt = BB->begin(), BBE = BB->end(); BBIt!= BBE; ++BBIt) {
      // Check if this instruction is a write image call that can be hoisted.
      // If so it will update m_writeImageAttributes member.
      CollectWriteImgAttributes(dyn_cast<CallInst>(BBIt));
    }
  }
}

void CLStreamSampler::CollectWriteImgAttributes(CallInst *writeImgCall) {
  // First check that this is a tranposed write_image call.
  if (!writeImgCall) return;
  Function *calledFunc = writeImgCall->getCalledFunction();
  if (!calledFunc) return;
  std::string funcName = calledFunc->getName().str();
  if (!m_rtServices->isTransposedWriteImg(funcName)) return;

  // Obtain entry in the builtin hash.
  std::auto_ptr<VectorizerFunction> foundFunction =
                     m_rtServices->findBuiltinFunction(funcName);
  assert(!foundFunction->isNull() && "should have hash entry");
  assert(foundFunction->getWidth() > 1 && "func should be soa_write_image");
  // Currently supports only transposed write.
  if (foundFunction->isNull() || foundFunction->getWidth() <= 1) return;

  TranspWriteImgAttr attrs;
  attrs.m_call = writeImgCall;
  attrs.m_width = foundFunction->getWidth();
  attrs.m_img = writeImgCall->getArgOperand(0);
  // Image should be invariant.
  if (!m_curLoop->isLoopInvariant(attrs.m_img)) return;

  // Obtain the colors.
  VectorType *colorTy =
      VectorType::get(Type::getFloatTy(*m_context), attrs.m_width);
  for (unsigned i=3; i<7; ++i) {
    Value *arg = writeImgCall->getArgOperand(i);
    Value *root = VectorizerUtils::RootInputArgument(arg, colorTy, attrs.m_call);
    if (!root) return;
    attrs.m_colors.push_back(root);
  }

  // Obtain x coord.
  Value *xCoord = writeImgCall->getArgOperand(1);
  if (!m_WIAnalysis->isStrided(xCoord)) return;
  assert(xCoord->getType()->isIntegerTy() && "x coord is integer");
  Constant *stride = m_WIAnalysis->getConstStride(xCoord);
  if (!stride) return;
  ConstantInt *strideConst = dyn_cast<ConstantInt>(stride);
  if (strideConst->getZExtValue() != attrs.m_width) return;
  Instruction *xCoordInst = dyn_cast<Instruction>(xCoord);
  assert(xCoordInst && "strided values should be instructions");
  if (!xCoordInst) return;
  Value *firstXCoord = obtainInitialStridedVal(xCoordInst);
  assert(firstXCoord && "unable to obtain first xCoord");
  if (!firstXCoord) return;
  attrs.m_firstXCoord = firstXCoord;

  //Obtain y coord.
  attrs.m_yCoord = getStreamWriteYcoord(writeImgCall->getArgOperand(2));
  if (!attrs.m_yCoord) return;

  // Success! Store the attributes in the member small vector.
  m_writeImageAttributes.push_back(attrs);
}


Value *CLStreamSampler::getStreamWriteYcoord(Value *v) {
  if (m_curLoop->isLoopInvariant(v)) return v;

  // Also support phi of two invariant values, with invariant conditon
  // In this case convert the phi into select outside the loop.
  PHINode *PN = dyn_cast<PHINode>(v);
  if (!PN) return NULL;
  if (PN->getNumIncomingValues() != 2 ) return NULL;
  Value *inc0 = PN->getIncomingValue(0);
  Value *inc1 = PN->getIncomingValue(1);
  if (!m_curLoop->isLoopInvariant(inc0)  || !m_curLoop->isLoopInvariant(inc1))
    return NULL;
  BasicBlock *BB0 = PN->getIncomingBlock(0);
  BasicBlock *BB1 = PN->getIncomingBlock(1);
  BasicBlock *BBOPred = BB0->getSinglePredecessor();
  BasicBlock *BB1Pred = BB1->getSinglePredecessor();
  if (!BBOPred || BB1Pred != BBOPred) return NULL;

  BranchInst *Br = dyn_cast<BranchInst>(BBOPred->getTerminator());
  if (!Br) return NULL;
  Value *cond = Br->getCondition();
  if (!m_curLoop->isLoopInvariant(cond)) return NULL;

  // Getting here we can repalace the PHI with select.
  Value *yCoord =   SelectInst::Create(cond,
                      PN->getIncomingValueForBlock(Br->getSuccessor(0)),
                      PN->getIncomingValueForBlock(Br->getSuccessor(1)),
                      "phi_merge", m_preHeader->getTerminator());
  PN->replaceAllUsesWith(yCoord);
  PN->eraseFromParent();
  return yCoord;
}


void CLStreamSampler::sinkWriteImgCall(TranspWriteImgAttr &attr,
                                       Function *writeStreamFunc) {
  // Generate 4 huge Alloca's for storing all the RGBA data...
  SmallVector<Instruction *, 4> colorAllocas;
  SmallVector<Instruction *, 4> colorStorage;
  generateAllocasForStream(attr.m_width, colorAllocas, colorStorage);

  // Add store of the colors to the buffers just before the call.
  SmallVector<Value *, 2> indices;
  indices.push_back(ConstantInt::get(m_indVar->getType(), 0));
  indices.push_back(m_indVar);
  ArrayRef<Value *> indicesArr = llvm::makeArrayRef(indices);
  for (unsigned i=0; i<4; ++i) {
    assert(attr.m_colors[i] && "NULL color argument");
    Value *colorPointer = GetElementPtrInst::CreateInBounds(
                     colorAllocas[i], indicesArr, "calc.address", attr.m_call);
    new StoreInst(attr.m_colors[i], colorPointer, false,
                  FLOAT_X_WIDTH__ALIGNMENT, attr.m_call);
  }

  // Prepare arguments for calling the stream sampler.
  assert(m_exit && "NULL exit block");
  Instruction *loc = m_exit->getFirstNonPHI();
  FunctionType *streamFuncTy = writeStreamFunc->getFunctionType();
  SmallVector<Value *, 8> args;
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                            attr.m_img, streamFuncTy->getParamType(0), loc));
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                      attr.m_firstXCoord, streamFuncTy->getParamType(1), loc));
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                           attr.m_yCoord, streamFuncTy->getParamType(2), loc));
  Value *streamSize = getStreamSize(attr.m_width);
  args.push_back(VectorizerUtils::getCastedArgIfNeeded(
                              streamSize, streamFuncTy->getParamType(3), loc));
  args.push_back(colorStorage[0]);
  args.push_back(colorStorage[1]);
  args.push_back(colorStorage[2]);
  args.push_back(colorStorage[3]);

  // Generate actual stream samler call instruction.
  CallInst::Create(writeStreamFunc, llvm::makeArrayRef(args), "", loc);

  //Remove the original call.and insert to the color array.
  assert(attr.m_call->getType()->isVoidTy() && "write img call should be void");

  // Remove redundant instructions.
  attr.m_call->eraseFromParent();
  //RecursivelyDeleteTriviallyDeadInstructions(attr.m_colorArr);
  if (isHeaderPhiStrided(attr.m_firstXCoord) ) {
    removeRedundantPHI(cast<PHINode>(attr.m_firstXCoord));
  }
}

void CLStreamSampler::generateAllocasForStream(unsigned width,
                               SmallVector<Instruction *, 4>& colorAllocas,
                               SmallVector<Instruction *, 4>& colorStorage) {
  // Fill buffers with Alloca's for storing all the output RGBA data...
  Type *colorTy = VectorType::get(Type::getFloatTy(*m_context), width);
  Type *arrTy  = ArrayType::get(colorTy, m_tripCountUpperBound);
  SmallVector<Value *, 2> indices(2, m_zero);
  ArrayRef<Value *> indicesArr = llvm::makeArrayRef(indices);

  Instruction *loc =
      m_header->getParent()->getEntryBlock().getFirstNonPHI();
  for (unsigned i = 0; i < 4; ++i) {
    AllocaInst *AI = new AllocaInst(arrTy, NULL, FLOAT_X_WIDTH__ALIGNMENT,
                                    "stream.read.alloca", loc);
    Instruction *ptr =
        GetElementPtrInst::CreateInBounds(AI, indicesArr, "ptr", loc);
    if (width != 4) {
      // Workaround : even for transpose 8 stream sampler, the buffers have <4 x float>*
      // type, so we bitcast to the pointer accordingly.
      PointerType *float4PtrTy =
         PointerType::get(VectorType::get(Type::getFloatTy(*m_context), 4), 0);
      ptr = CastInst::CreatePointerCast(ptr, float4PtrTy, "ptr.cast", loc);
    }
    colorAllocas.push_back(AI);
    colorStorage.push_back(ptr);
  }
}

void CLStreamSampler::removeRedundantPHI(PHINode *PN) {
  if (!PN->hasOneUse()) return;

  Instruction *user = dyn_cast<Instruction>(*(PN->user_begin()));
  Value *latchVal = PN->getIncomingValueForBlock(m_latch);
  if (!user || user != latchVal || !user->hasOneUse()) return;

  // Remove the phi and the incrementor.
  user->replaceAllUsesWith(UndefValue::get(user->getType()));
  user->eraseFromParent();
  PN->eraseFromParent();
}

Value *CLStreamSampler::calcFirstIterValIfPossible(Instruction *I) {
  // Fast path if we already computed the value of I in the
  // first iteartion return the computed val;
  if (m_firstIterVal.count(I)) return m_firstIterVal[I];

  // On preHeader strided value return the preHeader entry.
  if (isHeaderPhiStrided(I)) {
    PHINode *PN = cast<PHINode>(I);
    Value *initialVal = PN->getIncomingValueForBlock(m_preHeader);
    m_firstIterVal[I] = initialVal;
    return initialVal;
  }

  // On general values create a clone and set the operands inside the
  // loop to their already computed replicates, this might fail if we had not
  // replicated all operands before.
  Instruction *clone = I->clone();
  for (unsigned j=0; j<I->getNumOperands(); ++j ) {
    Instruction *opInst = dyn_cast<Instruction>(I->getOperand(j));
    // Non instruction or invariant instructions operands are unchanged.
    if (!opInst || !m_curLoop->contains(opInst)) continue;

    // On Non-Inaraint operands we need to use the already replicated value.
    if (m_firstIterVal.count(opInst)) {
      clone->setOperand(j, m_firstIterVal[opInst]);
    } else {
      delete clone;
      return NULL;
    }
  }
  clone->insertBefore(m_preHeader->getTerminator());
  m_firstIterVal[I] = clone;
  return clone;
}

Value *CLStreamSampler::obtainInitialStridedVal(Instruction *I) {
  SmallVector<Instruction *, 8> InstToReplicate;
  assert(m_WIAnalysis->isStrided(I) && "expected strided input");
  SmallVector<Value *, 8> workList;
  std::set<Value *> visited;
  workList.push_back(I);
  while (!workList.empty()) {
    Value *cur = workList.back();
    workList.pop_back();
    if (!visited.insert(cur).second) continue;

    // Should replicate only instructions inside the loop.
    Instruction *inst = dyn_cast<Instruction>(cur);
    if (!inst || !m_curLoop->contains(inst)) continue;

    // Supports phi node only in the header block.
    if (isa<PHINode>(inst) && !isHeaderPhiStrided(inst)) return NULL;

    InstToReplicate.push_back(inst);
    workList.insert(workList.end(), inst->op_begin(), inst->op_end());
  }
  // Note that in this stage InstToReplicate must contain I since it
  // was added to the workList, and it is strided value which means
  // it is an instruction inside the loop.


  // Create replicate for all instruction that compute the value
  // of the first iteration. Here we scan InstToReplicate in
  // reverse order since usually ops are inserted after their user.
  // In case not all ops were already replicated we change the order
  // of the InstToReplicate on the fly.
  while (!InstToReplicate.empty()) {
    int replicateIndex = -1;
    for (int i=InstToReplicate.size()-1; i>=0; --i) {
      if (calcFirstIterValIfPossible(InstToReplicate[i])){
        replicateIndex = i;
        break;
      }
    }

    assert(replicateIndex != -1 && "unable to replicate instruction");
    if (replicateIndex == -1) return NULL;

    // Remove the replicated value from the list.
    // Moving the last element into replicateIndex assures that we are not
    // not removing an instruction that was not replicated yet.
    InstToReplicate[replicateIndex] = InstToReplicate[InstToReplicate.size()-1];
    InstToReplicate.pop_back();
  }

  assert(m_firstIterVal.count(I) && "I should be in the map");
  return m_firstIterVal[I];
}

} // namespace intel
extern "C" {
  Pass* createCLStreamSamplerPass() {
    return new intel::CLStreamSampler();
  }
}
