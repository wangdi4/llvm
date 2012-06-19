/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "llvm/Support/CommandLine.h"
#include "Packetizer.h"
#include "Mangler.h"
#include "VectorizerUtils.h"

static cl::opt<unsigned>
CLIPacketSize("packet-size", cl::init(0), cl::Hidden,
  cl::desc("force packetization size"));

static cl::opt<bool>
EnableScatterGatherSubscript("subscript", cl::init(false), cl::Hidden,
  cl::desc("Enable vectorized scatter/gather operations"));

static cl::opt<bool>
EnableScatterGatherSubscript_v4i8("subscript-v4i8", cl::init(false), cl::Hidden,
  cl::desc("Enable vectorized scatter/gather operations on v4i8 data types"));

// Following check to be removed once Resolver handles scatter/gathers of
// types not supported by target.  For now, refrain from generating them.
static bool isGatherScatterType(VectorType *VecTy) {
  unsigned NumElements = VecTy->getNumElements();
  Type *ElemTy = VecTy->getElementType();
  if (EnableScatterGatherSubscript_v4i8 &&
      (NumElements == 4) &&
      (ElemTy->isIntegerTy(8)))
    return true;
  return ((NumElements == 16) &&
          (ElemTy->isFloatTy() ||
           ElemTy->isIntegerTy(32) ||
           ElemTy->isDoubleTy() ||
           ElemTy->isIntegerTy(64)));
}

namespace intel {

PacketizeFunction::PacketizeFunction(bool SupportScatterGather) : FunctionPass(ID)
{
  m_shuffleCtr = 0;
  m_extractCtr = 0;
  m_insertCtr  = 0;
  m_getElemPtrCtr = 0;
  m_nonConsecCtr = 0;
  m_noVectorFuncCtr = 0;
  m_cannotHandleCtr = 0;
  m_allocaCtr = 0;
  UseScatterGather = SupportScatterGather || EnableScatterGatherSubscript;
  m_rtServices = RuntimeServices::get();
  V_ASSERT(m_rtServices && "Runtime services were not initialized!");

  // VCM buffer allocation
  m_VCMAllocationArray = new VCMEntry[ESTIMATED_INST_NUM];
  m_VCMArrayLocation = 0;
  m_VCMArrays.push_back(m_VCMAllocationArray);
  V_PRINT(packetizer, "PacketizeFunction constructor\n");
}

PacketizeFunction::~PacketizeFunction()
{
  // Erase all the VCMEntry arrays (except for the first)
  releaseAllVCMEntries();
  delete[] m_VCMAllocationArray;
  V_PRINT(packetizer, "PacketizeFunction destructor\n");
}


bool PacketizeFunction::runOnFunction(Function &F)
{
  m_currFunc = &F;
  m_moduleContext = &(m_currFunc->getContext());
  m_packetWidth = m_rtServices->getPacketizationWidth();

  // Force command line packetization size
  if (CLIPacketSize) m_packetWidth = CLIPacketSize;

  V_ASSERT(4 <= m_packetWidth && MAX_PACKET_WIDTH >= m_packetWidth &&
    "Requested packetization width out of range!");

  // Packetization is possible only on functions which return void (kernels)
  if (!F.getReturnType()->isVoidTy())  {
    return false;
  }
  V_PRINT(packetizer, "\nStarting packetization of: " << m_currFunc->getName());
  V_PRINT(packetizer, " to Width: " << m_packetWidth << "\n");

  // Obtain WIAnalysis of the function
  m_depAnalysis = &getAnalysis<WIAnalysis>();
  V_ASSERT(m_depAnalysis && "Unable to get pass");

  // Prepare data structures for packetizing a new function (may still have
  // data left from previous function vectorization)
  m_removedInsts.clear();
  m_VCM.clear();
  m_BAG.clear();
  m_deferredResMap.clear();
  m_deferredResOrder.clear();
  releaseAllVCMEntries();

  V_STAT(
  V_PRINT(vectorizer_stat, "Packetizer Statistics on function "<<F.getName()<<":\n");
  V_PRINT(vectorizer_stat, "======================================================\n");
  m_shuffleCtr = 0;
  m_extractCtr = 0;
  m_insertCtr  = 0;
  m_getElemPtrCtr = 0;
  for (int i = 0; i < Instruction::OtherOpsEnd; i++) m_nonPrimitiveCtr[i] = 0;
  for (int i = 0; i < Instruction::OtherOpsEnd; i++) m_castCtr[i] = 0;
  m_nonConsecCtr = 0;
  m_noVectorFuncCtr = 0;
  m_cannotHandleCtr = 0;
  m_allocaCtr = 0;
  )

  // Iterate over all the instructions. Always hold the iterator at the instruction
  // following the one being packetized (so the iterator will "skip" any instructions
  // that are going to be added in the packetization work
  inst_iterator vI = inst_begin(m_currFunc);
  inst_iterator vE = inst_end(m_currFunc);
  while (vI != vE)
  {
    Instruction *currInst = &*vI;
    ++vI;
    dispatchInstructionToPacketize(currInst);
  }

  resolveDeferredInstructions();

  // Iterate over removed instructions and delete them
  SmallPtrSet<Instruction*, ESTIMATED_INST_NUM>::iterator iterStart = m_removedInsts.begin();
  SmallPtrSet<Instruction*, ESTIMATED_INST_NUM>::iterator iterEnd = m_removedInsts.end();
  SmallPtrSet<Instruction*, ESTIMATED_INST_NUM>::iterator index = iterStart;
  for (index = iterStart; index != iterEnd; ++index)
  {
    Instruction *curInst = *index;
    m_depAnalysis->invalidateDepend(curInst);
	curInst->replaceAllUsesWith(UndefValue::get(curInst->getType()));
    curInst->eraseFromParent();
  }

  V_STAT(
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_shuffleCtr<<" shuffle instructions\n");
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_extractCtr<<" extractelem instructions\n");
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_insertCtr<<" insertelem instructions\n");
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_getElemPtrCtr<<" getelemptr instructions\n");
  for (int i = 0; i < Instruction::OtherOpsEnd; i++) {
    if (m_nonPrimitiveCtr[i] > 0) {
      V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_nonPrimitiveCtr[i]<<" "<<Instruction::getOpcodeName(i) 
                               << " instructions of non-primitive type (int, float, intV, floatV)\n");
    }
  }
  for (int i = 0; i < Instruction::OtherOpsEnd; i++) {
    if (m_castCtr[i] > 0) {
      V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_castCtr[i]<<" "<<Instruction::getOpcodeName(i) 
                               << " cast instructions not supporting vector type\n");
    }
  }
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_nonConsecCtr<<" instructions with non-consecutive indices\n");
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_noVectorFuncCtr<<" call instructions when no vector function found in hash or runtime module, or function has side effect\n");
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_cannotHandleCtr<<" instructions with unsupported packet width, or couldn't transpose\n");
  V_PRINT(vectorizer_stat, "Couldn't packetize "<<m_allocaCtr<<" alloca instructions\n");

  V_PRINT(vectorizer_stat_excel, m_shuffleCtr <<"\t\t\t\t\t\t\t\t\t\t\t\t\t\tCouldn't packetize shuffle instructions\n");
  V_PRINT(vectorizer_stat_excel, "\t"<<m_extractCtr <<"\t\t\t\t\t\t\t\t\t\t\t\t\tCouldn't packetize extractelem instructions\n");
  V_PRINT(vectorizer_stat_excel, "\t\t"<<m_insertCtr <<"\t\t\t\t\t\t\t\t\t\t\t\tCouldn't packetize insertelem instructions\n");
  V_PRINT(vectorizer_stat_excel, "\t\t\t"<<m_getElemPtrCtr <<"\t\t\t\t\t\t\t\t\t\t\tCouldn't packetize getelemptr instructions\n");
  for (int i = 0; i < Instruction::OtherOpsEnd; i++) {
    if (m_nonPrimitiveCtr[i] > 0) {
      V_PRINT(vectorizer_stat_excel, "\t\t\t\t"<<m_nonPrimitiveCtr[i] <<"\t\t\t\t\t\t\t\t\t\tCouldn't packetize "<<Instruction::getOpcodeName(i)
                                     <<" instructions of non-primitive type (int, float, intV, floatV)\n");
    }
  }
  for (int i = 0; i < Instruction::OtherOpsEnd; i++) {
    if (m_castCtr[i] > 0) {
      V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t"<<m_castCtr[i] <<"\t\t\t\t\t\t\t\t\tCouldn't packetize" <<Instruction::getOpcodeName(i)
                                     <<" cast instructions not supporting vector type\n");
    }
  }
  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t"<<m_nonConsecCtr <<"\t\t\t\t\t\t\t\tCouldn't packetize instructions with non-consecutive indices\n");
  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t\t"<<m_noVectorFuncCtr <<"\t\t\t\t\t\t\tCouldn't packetize call instructions when no vector function found in hash or runtime module, or function has side effect\n");
  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t\t\t"<<m_cannotHandleCtr <<"\t\t\t\t\t\tCouldn't packetize instructions with unsupported packet width, or couldn't transpose\n");
  V_PRINT(vectorizer_stat_excel, "\t\t\t\t\t\t\t\t\t"<<m_allocaCtr <<"\t\t\t\t\tCouldn't packetize alloca instructions\n");
  )

  V_PRINT(packetizer, "\nCompleted vectorizing function: " << m_currFunc->getName() << "\n");
  return true;
}


void PacketizeFunction::dispatchInstructionToPacketize(Instruction *I)
{
  V_PRINT(packetizer, "\tNext instruction to packetize: " << *I << "\n");
  if (m_removedInsts.count(I))
  {
    V_PRINT(packetizer, "\t\tInstruction is marked for removal. Not packetized...\n");
    return;
  }
  bool err = false, isTidGen = false;
  unsigned dim;
  isTidGen = m_rtServices->isTIDGenerator(I, &err, &dim);
  V_ASSERT((!err) && "TIDGen inst receives non-constant input. Cannot vectorize!");
  if (isTidGen && dim == 0)
  {
    V_PRINT(packetizer, 
        "\t\tVectorizing TID creation Instruction: " << I->getName() << "\n");
    return generateSequentialIndices(I);
  }
  if (WIAnalysis::UNIFORM == m_depAnalysis->whichDepend(I))
  {
    // since we are never getting uniform values from other dependencies 
    // (even though sub consecutive consecutive = uniform ) 
    // we can just not packetize all uniform values
    return useOriginalConstantInstruction(I);
  }

  switch (I->getOpcode())
  {
    case Instruction::Add :
    case Instruction::Sub :
    case Instruction::Mul :
      packetizeInstruction(dyn_cast<BinaryOperator>(I), true);
      break;
    case Instruction::FAdd :
    case Instruction::FSub :
    case Instruction::FMul :
    case Instruction::UDiv :
    case Instruction::SDiv :
    case Instruction::FDiv :
    case Instruction::URem :
    case Instruction::SRem :
    case Instruction::FRem :
    case Instruction::Shl :
    case Instruction::LShr :
    case Instruction::AShr :
    case Instruction::And :
    case Instruction::Or :
    case Instruction::Xor :
      packetizeInstruction(dyn_cast<BinaryOperator>(I), false);
      break;
    case Instruction::ICmp :
    case Instruction::FCmp :
      packetizeInstruction(dyn_cast<CmpInst>(I));
      break;
    case Instruction::Trunc :
    case Instruction::ZExt :
    case Instruction::SExt :
    case Instruction::FPTrunc :
    case Instruction::FPExt :
    case Instruction::PtrToInt :
    case Instruction::IntToPtr :
    case Instruction::UIToFP :
    case Instruction::FPToUI :
    case Instruction::FPToSI :
    case Instruction::SIToFP :
    case Instruction::BitCast :
      packetizeInstruction(dyn_cast<CastInst>(I));
      break;
    case Instruction::ShuffleVector :
    case Instruction::ExtractValue :
    case Instruction::InsertValue :
      V_STAT(if (I->getOpcode() == Instruction::ShuffleVector) m_shuffleCtr++;)
      V_STAT(if (I->getOpcode() == Instruction::ExtractValue)  m_extractCtr++;)
      V_STAT(if (I->getOpcode() == Instruction::InsertValue)   m_insertCtr++;)
      duplicateNonPacketizableInst(I);
      break;
    case Instruction::InsertElement :
      packetizeInstruction(dyn_cast<InsertElementInst>(I));
	  break;
    case Instruction::ExtractElement :
      packetizeInstruction(dyn_cast<ExtractElementInst>(I));
      break;
    case Instruction::Select :
      packetizeInstruction(dyn_cast<SelectInst>(I));
      break;
    case Instruction::Alloca :
      packetizeInstruction(dyn_cast<AllocaInst>(I));
      break;
    case Instruction::Load :
      packetizeInstruction(dyn_cast<LoadInst>(I));
      break;
    case Instruction::Store :
      packetizeInstruction(dyn_cast<StoreInst>(I));
      break;
    case Instruction::GetElementPtr :
      packetizeInstruction(dyn_cast<GetElementPtrInst>(I));
      break;
    case Instruction::Call :
      packetizeInstruction(dyn_cast<CallInst>(I));
      break;
    case Instruction::Br :
      packetizeInstruction(dyn_cast<BranchInst>(I));
      break;
    case Instruction::PHI :
      packetizeInstruction(dyn_cast<PHINode>(I));
      break;
    case Instruction::Ret :
      packetizeInstruction(dyn_cast<ReturnInst>(I));
      break;
    default :
      V_ASSERT(0 && "Unsupported instruction for packetization");
      break;
  }
}

void PacketizeFunction::duplicateNonPacketizableInst(Instruction *I)
{
  V_PRINT(packetizer, "\t\tNon-Packetizable Instruction\n");
  Instruction *duplicateInsts[MAX_PACKET_WIDTH];

  // Create cloned instructions
  for (unsigned i = 0; i < m_packetWidth; i++) {
    duplicateInsts[i] = I->clone();
  }

  // Replace operands in duplicates
  unsigned numOperands = I->getNumOperands();

  // Iterate over all operands and replace them
  for (unsigned op = 0; op < numOperands; op++)
  {
    Value * multiOperands[MAX_PACKET_WIDTH];
    obtainMultiScalarValues(multiOperands, I->getOperand(op), I);
    if (isa<CallInst>(I))
    {
      for (unsigned i = 0; i < m_packetWidth; i++)
      {
        dyn_cast<CallInst>(duplicateInsts[i])->setArgOperand(op, multiOperands[i]);
      }
    }
    else
    {
      for (unsigned i = 0; i < m_packetWidth; i++)
      {
        duplicateInsts[i]->setOperand(op, multiOperands[i]);
      }
    }
  }
  // Add new value/s to VCM
  createVCMEntryWithMultiScalarValues(I, duplicateInsts);
  std::string instName = I->getName();
  // Add new instructions into function
  for (unsigned duplicates = 0; duplicates < m_packetWidth; duplicates++)
  {
    duplicateInsts[duplicates]->insertBefore(I);
  }
  // Remove original instruction
  m_removedInsts.insert(I);
}



void PacketizeFunction::packetizeInstruction(BinaryOperator *BI, bool supportsWrap)
{
  V_PRINT(packetizer, "\t\tBinaryOp Instruction\n");
  V_ASSERT(BI && "instruction type dynamic cast failed");
  V_ASSERT(2 == BI->getNumOperands() && "binary operator with num operands other than 2");

  Type * origInstType = BI->getType();

  // If instruction's return type is not primitive - cannot packetize
  if (!origInstType->isIntegerTy() && !origInstType->isFloatingPointTy()) {
    V_PRINT(vectorizer_stat, "<<<<NonPrimitiveCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(BI->getOpcode()) <<" instruction's return type is not primitive\n");
    V_STAT(m_nonPrimitiveCtr[BI->getOpcode()]++;)
    return duplicateNonPacketizableInst(BI);
  }

  bool has_NSW = (supportsWrap ? BI->hasNoSignedWrap() : false);
  bool has_NUW = (supportsWrap ? BI->hasNoUnsignedWrap() : false);

  // Obtain packetized arguments
  Value *operand0;
  Value *operand1;
  obtainVectorizedValue(&operand0, BI->getOperand(0), BI);
  obtainVectorizedValue(&operand1, BI->getOperand(1), BI);

  // Generate packetized instruction
  BinaryOperator *newVectorInst =
    BinaryOperator::Create(BI->getOpcode(), operand0, operand1, BI->getName(), BI);
  if (has_NSW) newVectorInst->setHasNoSignedWrap();
  if (has_NUW) newVectorInst->setHasNoUnsignedWrap();

  // Add new value/s to VCM
  createVCMEntryWithVectorValue(BI, newVectorInst);
  // Remove original instruction
  m_removedInsts.insert(BI);
}


void PacketizeFunction::packetizeInstruction(CastInst * CI)
{
  V_PRINT(packetizer, "\t\tCast Instruction\n");
  V_ASSERT(CI && "instruction type dynamic cast failed");
  V_ASSERT(1 == CI->getNumOperands() && "unexpected arguments number");
  Type *origInstType = CI->getType();
  Type *inputType = CI->getOperand(0)->getType();

  // In general (true for all cast instructions, also BitCast), we can only
  // Packetize if both input and output types are scalar primitives
  if ((!origInstType->isIntegerTy() && !origInstType->isFloatingPointTy()) ||
    (!inputType->isIntegerTy() && !inputType->isFloatingPointTy())) {
    V_PRINT(vectorizer_stat, "<<<<NonPrimitiveCtr("<<__FILE__<<":"<<__LINE__<<"):" <<Instruction::getOpcodeName(CI->getOpcode()) <<" both input and output types should be scalar primitives\n");
    V_STAT(m_nonPrimitiveCtr[CI->getOpcode()]++;)
    return duplicateNonPacketizableInst(CI);
  }

  // Filter-out any cast instructions which do not support Vector types  
  if (!isa<BitCastInst>(CI) && !isa<SExtInst>(CI) && !isa<ZExtInst>(CI) &&
    !isa<TruncInst>(CI) && !isa<FPToSIInst>(CI) &&
    !isa<SIToFPInst>(CI) && !isa<FPToUIInst>(CI) && !isa<UIToFPInst>(CI)) {
      V_STAT(m_castCtr[CI->getOpcode()]++;)
      return duplicateNonPacketizableInst(CI);
  }

  // Obtain packetized arguments
  Type * targetDestType = VectorType::get(origInstType, m_packetWidth);
  Value * operand0;
  obtainVectorizedValue(&operand0, CI->getOperand(0), CI);

  // Generate packetized instruction
  CastInst *newVectorInst =
    CastInst::Create(CI->getOpcode(), operand0, targetDestType, CI->getName(), CI);

  // Add new value/s to VCM
  createVCMEntryWithVectorValue(CI, newVectorInst);
  // Remove original instruction
  m_removedInsts.insert(CI);
}


void PacketizeFunction::packetizeInstruction(CmpInst *CI)
{
  V_PRINT(packetizer, "\t\tCompare Instruction\n");
  V_ASSERT(CI && "instruction type dynamic cast failed");
  Type * origInstType = CI->getOperand(0)->getType();
  V_ASSERT(2 == CI->getNumOperands() && "unexpected number of operands!");

  // If instruction's return type is not primitive - cannot packetize
  if (!origInstType->isIntegerTy() && !origInstType->isFloatingPointTy()) {
    V_PRINT(vectorizer_stat, "<<<<NonPrimitiveCtr("<<__FILE__<<":"<<__LINE__<<"):" <<Instruction::getOpcodeName(CI->getOpcode()) << " instruction's return type is not primitive\n");
    V_STAT(m_nonPrimitiveCtr[CI->getOpcode()]++;)
    return duplicateNonPacketizableInst(CI);
  }

  // Create the vector values
  Value * operand0;
  Value * operand1;
  obtainVectorizedValue(&operand0, CI->getOperand(0), CI);
  obtainVectorizedValue(&operand1, CI->getOperand(1), CI);
  CmpInst *newVectorInst =
    CmpInst::Create(CI->getOpcode(), CI->getPredicate(), operand0, operand1, "", CI);
  newVectorInst->takeName(CI);

  // Add new instruction to VCM
  createVCMEntryWithVectorValue(CI, newVectorInst);

  // Remove original instruction
  m_removedInsts.insert(CI);
}


Instruction* PacketizeFunction::widenScatterGatherOp(MemoryOperation &MO) {
      V_ASSERT(MO.Base && MO.Index && "Bad base and index operands");

  Type *ElemTy = 0;
  if (MO.Data) {
    ElemTy = MO.Data->getType();
  } else {
    ElemTy = MO.Orig->getType();
  }

  VectorType *VecElemTy = VectorType::get(ElemTy, m_packetWidth);

  // Check if this type is supported and if we have a name for it
  if (!isGatherScatterType(VecElemTy)) {
    V_PRINT(gather_scatter_stat, "PACKETIZER: UNSUPPORTED TYPE" << *MO.Orig << "\n");
    return NULL;
  }
  bool is_masked = (MO.Mask != NULL);
  bool is_gather = (MO.Data == NULL);
  std::string name = Mangler::getGatherScatterName(is_masked, is_gather, VecElemTy);

  std::vector<Value*> args;
  std::vector<Type *> types;

  Value *SclrBase[MAX_PACKET_WIDTH];
  obtainMultiScalarValues(SclrBase, MO.Base , MO.Orig);
  MO.Base = SclrBase[0];

  if (MO.Mask)
    obtainVectorizedValue(&MO.Mask, MO.Mask, MO.Orig);

  Type *IndexTy = MO.Index->getType();
  obtainVectorizedValue(&MO.Index, MO.Index, MO.Orig);
  if (!IndexTy->isIntegerTy(32)) {
    Type *IndTy = IntegerType::get(MO.Orig->getContext(), 32);
    VectorType *VecTy = VectorType::get(IndTy, m_packetWidth);
    Instruction* truncInst = new TruncInst(MO.Index, VecTy, "vindex32", MO.Orig);
    MO.Index = truncInst;
    V_PRINT(gather_scatter_stat, "PACKETIZER: TRUNC " << *MO.Index << "\n");
  }
 
  if (MO.Data)
    obtainVectorizedValue(&MO.Data, MO.Data, MO.Orig);

  PointerType *BaseTy = dyn_cast<PointerType>(MO.Base->getType());
  PointerType *StrippedBaseTy =
    PointerType::get(BaseTy->getElementType(),0);
  MO.Base = new BitCastInst(MO.Base, StrippedBaseTy, "stripAS", MO.Orig);

  if (MO.Mask) args.push_back(MO.Mask);
  args.push_back(MO.Base);
  args.push_back(MO.Index);
  if (MO.Data) args.push_back(MO.Data);
  if (MO.Mask) types.push_back(MO.Mask->getType());
  types.push_back(MO.Base->getType());
  types.push_back(MO.Index->getType());
  if (MO.Data) types.push_back(MO.Data->getType());

  if (MO.Data) {
    V_ASSERT(VecElemTy == MO.Data->getType() && "Invalid vector type");
  }

  Type *RetTy = Type::getVoidTy(VecElemTy->getContext());
  if (!MO.Orig->getType()->isVoidTy()) {
    RetTy = VecElemTy;
  }

  FunctionType *intr = FunctionType::get(RetTy, types, false);
  Constant* new_f = m_currFunc->getParent()->getOrInsertFunction(name, intr);
  V_ASSERT(isa<Function> (new_f) && "mismatch function type");
  return CallInst::Create(new_f, ArrayRef<Value*>(args), "", MO.Orig);
}

Instruction* PacketizeFunction::widenMemoryOperand(MemoryOperation &MO) {
  // Obtain the input addresses (but only the first will be used
  // we know that this is an unmasked widen load/store
  Value *inAddr[MAX_PACKET_WIDTH];
  obtainMultiScalarValues(inAddr, MO.Ptr , MO.Orig);

  PointerType *inPtr = dyn_cast<PointerType>(inAddr[0]->getType());
  V_ASSERT(inPtr && "unexpected non-pointer argument");

  // BitCast the "scalar" pointer to a "vector" pointer
  Type *elementType = inPtr->getElementType();
  Type *vectorElementType = VectorType::get(elementType, m_packetWidth);
  PointerType *vectorInPtr = PointerType::get(vectorElementType, 
    inPtr->getAddressSpace());
  Value *bitCastPtr = new BitCastInst(inAddr[0], vectorInPtr,
    "ptrTypeCast", MO.Orig);


  if (!MO.Data) {
    // Create a "vectorized" load
    return new LoadInst(bitCastPtr, MO.Orig->getName(), false, 
      MO.Alignment, MO.Orig);
  } else {
    Value *vData;
    obtainVectorizedValue(&vData, MO.Data, MO.Orig);
    // Create a "vectorized" store
    return new StoreInst(vData, bitCastPtr, false, MO.Alignment, MO.Orig);
  }
}


Instruction* PacketizeFunction::widenMaskedOp(MemoryOperation &MO) {

  std::string name = (MO.Data? Mangler::getStoreName(MO.Alignment): 
                               Mangler::getLoadName(MO.Alignment));

  WIAnalysis::WIDependancy RetDep = m_depAnalysis->whichDepend(MO.Orig);
  V_ASSERT(RetDep != WIAnalysis::UNIFORM && "No need to packetize this instr");
  RetDep = RetDep; //silence warning.

  WIAnalysis::WIDependancy PtrDep = m_depAnalysis->whichDepend(MO.Ptr);
  WIAnalysis::WIDependancy MaskDep = WIAnalysis::UNIFORM;
  if (MO.Mask) MaskDep = m_depAnalysis->whichDepend(MO.Mask);

  // If the pointer is not consecutive then
  // it should be handled via indexed scatter/gather.
  // Everything else is passed to the resolver.
  // In here we only handle consecutive pointers.
  if (PtrDep != WIAnalysis::PTR_CONSECUTIVE) {
    V_PRINT(vectorizer_stat, "<<<<NonConsecCtr("<<__FILE__<<":"<<__LINE__<<"): in "<<Instruction::getOpcodeName(MO.Orig->getOpcode()) << " the pointer is not consecutive\n");
    V_STAT(m_nonConsecCtr++;)
    duplicateNonPacketizableInst(MO.Orig);
    return NULL;
  }

  // Set the mask. We are free to generate scalar or vector masks.
  if (MaskDep == WIAnalysis::UNIFORM) {
    Value *SclrMask[MAX_PACKET_WIDTH];
    obtainMultiScalarValues(SclrMask,MO.Mask ,MO.Orig);
    MO.Mask = SclrMask[0];
  } else {
    obtainVectorizedValue(&MO.Mask, MO.Mask, MO.Orig);
  }

  // Set the pointer (Consecutive). It must pointer to a vector data type.
  Value *SclrPtr[MAX_PACKET_WIDTH];
  obtainMultiScalarValues(SclrPtr, MO.Ptr, MO.Orig);
  MO.Ptr = SclrPtr[0];
  // Widen the load
  // BitCast the "scalar" pointer to a "vector" pointer
  V_ASSERT(PtrDep == WIAnalysis::PTR_CONSECUTIVE);
  PointerType * PtrTy = dyn_cast<PointerType>(MO.Ptr->getType());
  V_ASSERT(PtrTy && "Pointer must be of pointer type");
  Type *ElemType = PtrTy->getElementType();
  Type *VecElemTy = VectorType::get(ElemType, m_packetWidth);
  PointerType *VecTy = PointerType::get(VecElemTy, PtrTy->getAddressSpace());
  MO.Ptr = new BitCastInst(MO.Ptr, VecTy, "ptrTypeCast",MO.Orig);

  // If this is a store
  if (MO.Data) {
    obtainVectorizedValue(&MO.Data, MO.Data, MO.Orig);
  }
  
  // Implement the function call
  std::vector<Value*> args;
  std::vector<Type *> types;
  
  args.push_back(MO.Mask);
  if (MO.Data) args.push_back(MO.Data);
  args.push_back(MO.Ptr);
  types.push_back(MO.Mask->getType());
  if (MO.Data) types.push_back(MO.Data->getType());
  types.push_back(MO.Ptr->getType());

  Type *DT = cast<PointerType>(MO.Ptr->getType())->getElementType();
  if (MO.Data) DT = Type::getVoidTy(DT->getContext());
  FunctionType *intr = FunctionType::get(DT, types, false);
  Constant* new_f = m_currFunc->getParent()->getOrInsertFunction(name, intr);
  return CallInst::Create(new_f, ArrayRef<Value*>(args), "", MO.Orig);
}

void PacketizeFunction::packetizeMemoryOperand(MemoryOperation &MO) {
  V_ASSERT(MO.Orig && "Invalid instruction");

  V_ASSERT(MO.Ptr->getType()->isPointerTy() && "Pointer operand is not a pointer");
  V_ASSERT((!MO.Mask || MO.Mask->getType()->getScalarType()->isIntegerTy()) && 
    "mask must be an integer");

  WIAnalysis::WIDependancy PtrDep = m_depAnalysis->whichDepend(MO.Ptr);
  WIAnalysis::WIDependancy MaskDep = WIAnalysis::UNIFORM;
  if (MO.Mask) MaskDep = m_depAnalysis->whichDepend(MO.Mask);

  // If we were told to look for scat/gath regions AND the pointer access is
  // random (not consecutive or uniform).
  if (UseScatterGather && PtrDep == WIAnalysis::RANDOM) {
    Value *Base = 0;
    Value *Index = 0;
    bool forcei32 = false;
    GetElementPtrInst *Gep = dyn_cast<GetElementPtrInst>(MO.Ptr);
    // If we found the GEP
    if (Gep && Gep->getNumIndices() == 1) {
      Base = Gep->getOperand(0); 
      // and the base is uniform
      if (m_depAnalysis->whichDepend(Base) == WIAnalysis::UNIFORM) {
        Index = Gep->getOperand(1);
        // Try to find the i32 index
        if (!Index->getType()->isIntegerTy(32)) {
          if (ZExtInst* ZI = dyn_cast<ZExtInst>(Index)) Index = ZI->getOperand(0);
          if (SExtInst* SI = dyn_cast<SExtInst>(Index)) Index = SI->getOperand(0);
          // Check for the idiom that keeps the lowest 32bits:
          // %idxprom = ashr exact i64 %XXX, 32
          if (BinaryOperator* BI = dyn_cast<BinaryOperator>(Index)) {
            if (BI->getOpcode() == Instruction::AShr) {
              // Constants are canonicalized to the RHS.
              ConstantInt *C0 = dyn_cast<ConstantInt>(BI->getOperand(1));
              if (C0 && (C0->getBitWidth() < 65))
                forcei32 = (C0->getZExtValue() == 32);
            }
          }

          // Check for the idiom "%idx = and i64 %mul, 4294967295" for using
          // the lowest 32bits.
          if (BinaryOperator* BI = dyn_cast<BinaryOperator>(Index)) {
            if (BI->getOpcode() == Instruction::And) {
              // Constants are canonicalized to the RHS.
              ConstantInt *C0 = dyn_cast<ConstantInt>(BI->getOperand(1));
              if (C0 && (C0->getBitWidth() < 65))
                forcei32 = (C0->getZExtValue() == (unsigned) -1);
            }
          }
        }
        // If the index is not 32-bit, abort.
        if (!Index->getType()->isIntegerTy(32) && !forcei32) {
          V_PRINT(gather_scatter_stat, "PACKETIZER: BASE UNIFORM, INDEX NOT 32" << *MO.Orig <<
                                 " Gep: " << *Gep << " Index: " << *Index << "\n");
          Index = NULL;
        }
      }
      else
        V_PRINT(gather_scatter_stat, "PACKETIZER: BASE NON UNIFORM " << *MO.Orig << " Base: " << *Base << "\n");
    }
    else if (!Gep)
      V_PRINT(gather_scatter_stat, "PACKETIZER: NOT GEP " << *MO.Ptr << "\n");
    else
      V_PRINT(gather_scatter_stat, "PACKETIZER: GEP NOT SINGLE INDEX " << *Gep << "\n");
    MO.Index = Index;
    MO.Base = Base;
    // If we decide to generate a scatter/gather (MO.Index != NULL), and the type of MO.Index is not i32,
    // then it must hold a value truncatable to i32 (forcei32):
    V_ASSERT((!MO.Index || MO.Index->getType()->isIntegerTy(32) || forcei32) && 
              "Index is not i32 but forcei32 is false");
  }
  else
    V_PRINT(gather_scatter_stat, "PACKETIZER: PtrDep NOT RANDOM" << *MO.Orig << "\n");

  // Find the data type;
  Type *DT;
  if (MO.Data) { 
    DT = MO.Data->getType(); // stored type
  } else {
    DT = MO.Orig->getType(); // loaded type
  }

  // Not much we can do for load of non-scalars
  if (!(DT->isFloatingPointTy() || DT->isIntegerTy())) {
    V_PRINT(vectorizer_stat, "<<<<NonPrimitiveCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(MO.Orig->getOpcode()) <<" of non-scalars\n");
    V_STAT(m_nonPrimitiveCtr[MO.Orig->getOpcode()]++;)
    if (MO.Index)
      V_PRINT(gather_scatter_stat, "PACKETIZER: LOAD OF NON-SCALARS " << *MO.Orig << "\n");
    return duplicateNonPacketizableInst(MO.Orig);
  }

  // Indexed scatter/gather in here
  if (MO.Index) {
    V_ASSERT(MO.Index && "Must have an index");
    V_ASSERT(MO.Base && "Index w/o base");
    Instruction *Scat = 
      widenScatterGatherOp(MO);
    if (Scat) {
      createVCMEntryWithVectorValue(MO.Orig, Scat);
      m_removedInsts.insert(MO.Orig);
      V_PRINT(gather_scatter_stat, "PACKETIZER: SUCCESS\n");
      return;
    }
    else MO.Index = NULL;
  }

  V_ASSERT(!MO.Index && "Was not able to handle a scatter/gather inst");

  if (MO.Mask) {
    Instruction *Wide = widenMaskedOp(MO);
    // If wide is null, then we alraedy handled things inside
    if (Wide) {
      // Add new value/s to VCM and remove inst
      createVCMEntryWithVectorValue(MO.Orig, Wide);
      m_removedInsts.insert(MO.Orig);
    }
    return;
  }

  V_ASSERT(!MO.Mask && "Was not able to handle a mask operation");

  // If the pointer is consecutive, and the data type is scalar
  if (PtrDep == WIAnalysis::PTR_CONSECUTIVE) {
    Instruction *Wide = widenMemoryOperand(MO);
    // Add new value/s to VCM and remove inst
    createVCMEntryWithVectorValue(MO.Orig, Wide);
    m_removedInsts.insert(MO.Orig);
    return;
  }

  // Handle random pointer, or load/store of non primitive types.
  V_PRINT(vectorizer_stat, "<<<<NonConsecCtr("<<__FILE__<<":"<<__LINE__<<"): " <<Instruction::getOpcodeName(MO.Orig->getOpcode()) <<": Handles random pointer, or load/store of non primitive types\n");
  V_STAT(m_nonConsecCtr++;)
  return duplicateNonPacketizableInst(MO.Orig);
}




void PacketizeFunction::packetizeInstruction(CallInst *CI)
{
  V_PRINT(packetizer, "\t\tCall Instruction\n");
  V_ASSERT(CI && "instruction type dynamic cast failed");
  Function *origFunc = CI->getCalledFunction();
  std::string origFuncName = origFunc->getName();
  
  // Avoid packetizing fake insert\extract that are used to 
  // obtain the scalar elements of vector arguments\return of scalar built-ins.
  if (Mangler::isFakeExtract(origFuncName) || 
      Mangler::isFakeInsert(origFuncName)) {
    m_removedInsts.insert(CI);
    return;
  }
  
  Function *scalarFunc = origFunc;
  std::string scalarFuncName = origFuncName;
  const char * vectorFuncName;
  bool isMangled = Mangler::isMangledCall(scalarFuncName);
  const Function *LibFunc;

  // Handle packetization of load/store ops
  if (Mangler::isMangledLoad(scalarFuncName) ||
       Mangler::isMangledStore(scalarFuncName)) {
    MemoryOperation MO;
    MO.Mask = CI->getArgOperand(0);
    if (Mangler::isMangledStore(scalarFuncName)) {
      MO.Ptr = CI->getArgOperand(2);
      MO.Data = CI->getArgOperand(1);
      MO.Alignment = Mangler::getMangledStoreAlignment(scalarFuncName);
    } else {
      MO.Ptr = CI->getArgOperand(1);
      MO.Data = 0;
      MO.Alignment = Mangler::getMangledLoadAlignment(scalarFuncName);
    }
    
    MO.Base = 0;
    MO.Index = 0;
    MO.Orig = CI;
    return packetizeMemoryOperand(MO);
  }

  // First remove any name-mangling (for example, masking), from the function name
  if (isMangled) {
    scalarFuncName = Mangler::demangle(scalarFuncName);
    scalarFunc = m_currFunc->getParent()->getFunction(scalarFuncName);
    V_ASSERT(scalarFunc && "Could not find function which was masked, but still exist");
  }
  V_PRINT(packetizer, "\t\t\tCalled function name (demangled): " << scalarFuncName << "\n");

  // Check if called function is a sync point
  if (m_rtServices->isSyncFunc(scalarFuncName)) {
    // Handle sync points (not predicated). 
    // These are functions which are used for synchronizing among work-items
    // in a work group. These functions are left "as is" and not packetized.
    V_PRINT(packetizer, "\t\t\tDetected synchronization function call.\n");
    // if sync function was mangled - remangle it
    scalarFuncName = origFuncName;
    scalarFunc = origFunc;
    isMangled = false;
    // "cheat" the function to use the scalar function as the packetized function
    vectorFuncName = scalarFuncName.c_str();
    LibFunc = m_currFunc->getParent()->getFunction(scalarFuncName); 
    V_ASSERT(LibFunc && "sync function is called, but not defined in module");
  } else {
    // Look for the function in the builtin functions hash
    unsigned vecWidth = 0;
    const RuntimeServices::funcEntry foundFunction =
      m_rtServices->findBuiltinFunction(scalarFuncName);
    if (foundFunction.first) {
      vecWidth = foundFunction.second;
    }

    // If function was not found in hash (or is not scalar), need to duplicate it
    if (vecWidth != 1) {
      V_PRINT(vectorizer_stat, "<<<<NoVectorFuncCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(CI->getOpcode()) <<" Could not find vectorized version for the function:" <<origFuncName<<"\n"); 
      V_STAT(m_noVectorFuncCtr++;)
      return duplicateNonPacketizableInst(CI);
    }

    // Obtain the appropriate vector function.
    vectorFuncName = foundFunction.first->funcs[LOG_(m_packetWidth)];
    V_PRINT(packetizer, "\t\t\tWill convert to: "<< vectorFuncName << "\n");
    // Get new decl out of module.
    LibFunc = m_rtServices->findInRuntimeModule(vectorFuncName);
 
    V_ASSERT(LibFunc && "Mismatch between function hash and runtime module");
    // Fallback in case function was not found in runtime module: just duplicate scalar func
    if (!LibFunc) {
      V_PRINT(vectorizer_stat, "<<<<NoVectorFuncCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(CI->getOpcode()) <<" Could not find vectorized version for the function in runtime module:" <<origFuncName<<"\n"); 
      V_STAT(m_noVectorFuncCtr++;)
      return duplicateNonPacketizableInst(CI);
    }
  }
  
  // TODO:: do we want \ need to support return by pointer?
  // TODO:: need better interface for knowing whether function is masked than is mangled
  // TODO:: is it the way we want to support masked calls ?
  // currently we have only the DX calls and this works for them.
  bool hasNoSideEffects = m_rtServices->hasNoSideEffect(scalarFuncName);
  std::string vectorFuncNameStr = LibFunc->getNameStr();
  bool isMaskedFunctionCall = m_rtServices->isMaskedFunctionCall(vectorFuncNameStr);
  if (!hasNoSideEffects && isMangled && !isMaskedFunctionCall) {
    V_PRINT(vectorizer_stat, "<<<<NoVectorFuncCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(CI->getOpcode()) <<" Vectorized version for the function has side effects:" <<origFuncName<<"\n"); 
    V_STAT(m_noVectorFuncCtr++;)
    return duplicateNonPacketizableInst(CI);
  }

  std::vector<Value *> newArgs;
  if (!obtainNewCallArgs(CI, LibFunc, isMangled, isMaskedFunctionCall, newArgs)) {
    V_PRINT(vectorizer_stat, "<<<<NoVectorFuncCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(CI->getOpcode()) <<" Failed to convert args to vectorized function:" <<origFuncName<<"\n");
    V_STAT(m_noVectorFuncCtr++;)
    return duplicateNonPacketizableInst(CI);
  }

  // Find (or create) declaration for newly called function
  Constant * vectFunctionConst = m_currFunc->getParent()->getOrInsertFunction(
      LibFunc->getNameStr(), LibFunc->getFunctionType(), LibFunc->getAttributes());
  V_ASSERT(vectFunctionConst && "failed generating function in current module");
  Function *vectorFunction = dyn_cast<Function>(vectFunctionConst);
  V_ASSERT(vectorFunction && "Function type mismatch, caused a constant expression cast!");
  
  // Create new instruction
  CallInst * newCall = 
    CallInst::Create(vectorFunction, ArrayRef<Value*>(newArgs), "", CI);

  // updates packetizer data structure with new packetized call
  if (!handleCallReturn(CI, newCall)) {
    m_removedInsts.insert(newCall);
    V_PRINT(vectorizer_stat, "<<<<NoVectorFuncCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(CI->getOpcode()) <<" Failed to convert return value to vectorized function:" <<origFuncName<<"\n");
    V_STAT(m_noVectorFuncCtr++;)
    return duplicateNonPacketizableInst(CI);
  }
  
  // Remove original instruction
  m_removedInsts.insert(CI);
}


bool PacketizeFunction::obtainNewCallArgs(CallInst *CI, const Function *LibFunc,
         bool isMangled, bool isMaskedFunctionCall, std::vector<Value *>& newArgs) {
  // For masked calls we widen the mask as first operand (or creating mask of all 1 
  // incase no call is not masked)
  if (isMaskedFunctionCall) {
    Value *maskV;
    if (isMangled) {
      Value *mask = CI->getArgOperand(0);
      obtainVectorizedValue(&maskV, mask, CI);
    } else {
      Constant *mask = ConstantInt::get(CI->getContext(), APInt(1,1));
      maskV = ConstantVector::get(std::vector<Constant *>(m_packetWidth, mask));
    }
    newArgs.push_back(maskV);
  }
  
  unsigned scalarStart = isMangled ? 1 : 0;
  unsigned numArguments = CI->getNumArgOperands();
  FunctionType *LibFuncTy = LibFunc->getFunctionType();
  for (unsigned argIndex = scalarStart; argIndex < numArguments; ++argIndex) {
    Value *operand;
    Type *neededType = LibFuncTy->getParamType(newArgs.size());
    V_ASSERT(neededType && "argument error");
    // Operands are expected to be packetized, unless the scalar and vector functions
    // receive arguments of the exact same type
    Value *curScalarArg = CI->getArgOperand(argIndex);
    Type *curScalarArgType = curScalarArg->getType();

    // Incase current argument is a vector and runtime says we always 
    // spread vector operands.than try to do it.
    if (m_rtServices->alwaysSpreadVectorParams() && curScalarArgType->isVectorTy()) {
      // here we assume that any scalar vector operand must be spread
      if (!SpreadVectorParam(CI, curScalarArg, LibFuncTy, newArgs)){
        V_ASSERT (0 && "unsupported parameter type");
        return false;  
      }
    } else if (neededType == curScalarArgType) {
      // If a non-packetized argument is needed, simply use the first argument of 
      // the received multi-scalar argument
      Value *multiScalarVals[MAX_PACKET_WIDTH];
      obtainMultiScalarValues(multiScalarVals, curScalarArg, CI);
      operand = multiScalarVals[0];
      newArgs.push_back(operand);
    } else if (curScalarArgType->isVectorTy()) {
      // If vectors are not spread then vector arguments should be packetized
      // in SOA form using array of vectors.
      operand = HandleParamSOA(CI, curScalarArg);
      if (!operand || operand->getType() != neededType) {
        V_ASSERT (0 && "unsupported parameter type");
        return false;
      }
      newArgs.push_back(operand);
    } else {
      // For scalars obtain their corresponding vector.
      obtainVectorizedValue(&operand, curScalarArg, CI);
      operand = VectorizerUtils::getCastedArgIfNeeded(operand, neededType, CI);
      newArgs.push_back(operand);
    }
  }

  // In case the scalar built-in returns a vector and the vector built-in
  // returns void, than the return values are expected to be returned by
  // pointer arguments. here we create alloca in the entry block and add them
  // to the argument list. when handling return we will create load from these
  // pointers.
  if (LibFunc->getReturnType()->isVoidTy() && CI->getType()->isVectorTy()) {
    VectorType *vTy = cast<VectorType>(CI->getType());
    unsigned numElements = vTy->getNumElements();
    Instruction *loc = m_currFunc->getEntryBlock().begin();
    for (unsigned i=0; i<numElements; ++i) {
      PointerType *ptrTy = dyn_cast<PointerType>(LibFuncTy->getParamType(newArgs.size()));
      V_ASSERT(ptrTy && "bad signature");
      if (!ptrTy) return false;
      Type *elTy = ptrTy->getElementType();
      AllocaInst *AI = new AllocaInst(elTy, "", loc);
      newArgs.push_back(AI);
    }
  }
  return true;
}

bool PacketizeFunction::handleCallReturn(CallInst *CI, CallInst * newCall) {
  Type *scalarRetType = CI->getType();
  Type *vectorRetType = newCall->getType();
  // Check if the function's return type is static (not packetized).
  bool retIsPacketized = (scalarRetType != vectorRetType);
  // Check if function's return type is static in size (happens only for
  // wrappers). Such cases report multi-scalar value (broadcast)
  if (!retIsPacketized) {
    if (!scalarRetType->isVoidTy()) {
      Instruction *multiScalarBroadcast[MAX_PACKET_WIDTH];
      for (unsigned i = 0; i < m_packetWidth; ++i) {
        multiScalarBroadcast[i] = newCall;
      }
      createVCMEntryWithMultiScalarValues(CI, multiScalarBroadcast);
    }
  } else if (scalarRetType->isVectorTy()) {
    // In case the scalar builtin returns a vector the following 2 options
    // are suported: return by pointers (a), return by array of vectors(b).
    // a.  <2 x float> foo(...) --> void foo4(..., <4 xfloat>*, <4 x float>*)
    // b.  <2 x float> foo(...) --> [2 x <4 x float>]] foo4(...)
    // array of vectors , or it is the return is by pointers as last arguments.
    if (newCall->getType()->isVoidTy()) {
      // return by pointers.
      return HandleReturnByPointers(CI, newCall); 
    } else {
      // return using array of vectors.
      return HandleReturnValueSOA(CI, newCall);
    }
  } else {
    Instruction *newFuncCall = VectorizerUtils::getCastedRetIfNeeded(newCall, VectorType::get(CI->getType(), m_packetWidth));
    createVCMEntryWithVectorValue(CI, newFuncCall);
  }
  return true;
}


bool PacketizeFunction::obtainInsertElement(Value* val, 
     SmallVectorImpl<Value *> &roots, unsigned nElts, Instruction* place) {
  V_ASSERT(val && "bad value");
  // initialize the roots with NULL values.
  roots.assign(nElts, NULL);
  // For each of the items we are trying to fetch
  for (unsigned i=0; i < nElts; ++i) {
    // Make sure our chain is made of fake insert elements.
    CallInst* fakeIEI = dyn_cast<CallInst>(val);
    if (!fakeIEI) return false;
    std::string insertName = fakeIEI->getCalledFunction()->getNameStr();
    V_ASSERT(Mangler::isFakeInsert(insertName) && "expected fake.insert");
    if (!Mangler::isFakeInsert(insertName)) return false;
    unsigned start = Mangler::isMangledCall(insertName) ? 1 : 0;
    Value* index = fakeIEI->getArgOperand(2 + start);
    // Obtain the constant indext
    ConstantInt* CI = dyn_cast<ConstantInt>(index);
    V_ASSERT(CI && "index should be constant");
    uint64_t idx = CI->getZExtValue();
    // Fetch or create vectorized value
    obtainVectorizedValue(&roots[idx], fakeIEI->getArgOperand(1 + start), place);
    // move to the next Insert-Element in chain
    val = fakeIEI->getArgOperand(0 + start);
  }
  // success!
  return true;
}

bool PacketizeFunction::HandleReturnValueSOA(CallInst* CI, CallInst *soaRet){
  // first validate that the new call return is proper array of vectors.
  V_ASSERT(CI->getType()->isVectorTy() && "expected vector type");
  VectorType *aosType = cast<VectorType>(CI->getType());
  ArrayType *soaType = ArrayType::get(VectorType::get(
      aosType->getElementType(), m_packetWidth) , aosType->getNumElements());
  if (soaType != soaRet->getType()) {
    // failed to handle return - remove call and duplicate original instruciton
    V_ASSERT (0 && "unsupported parameter type");
    soaRet->eraseFromParent();
    return false;
  }

  // Break the array of vectors to it's vector elements.
  unsigned numElements = soaType->getNumElements();
  SmallVector<Instruction*, MAX_INPUT_VECTOR_WIDTH> scalars;
  for (unsigned i=0; i<numElements; i++) {
    Instruction* ext = ExtractValueInst::Create(soaRet, i, "", CI);
    scalars.push_back(ext);
  }

  // Map the elements to fake extracts generated by the scalarizer to
  // the vector elemetns.
  MapVectorMultiReturn(CI, scalars);
  return true;
}

void PacketizeFunction::MapVectorMultiReturn (CallInst* CI,
                 SmallVectorImpl<Instruction *>& returnedVals) {
  // Replace fake extracts (generated by the scalarizer), with a vectorized value
  for (Value::use_iterator ui = CI->use_begin(), ue = CI->use_end(); ui != ue; ++ui) {
    CallInst* fakeEI = dyn_cast<CallInst>(*ui);
    V_ASSERT(fakeEI && "user must be an fake ExtractElement (scalarizer should take care of it");
    std::string extractName = fakeEI->getCalledFunction()->getNameStr();
    V_ASSERT(Mangler::isFakeExtract(extractName) &&
        "user must be an fake ExtractElement (scalarizer should take care of it");

    // Extract index (expected to be a constant)
    unsigned start = Mangler::isMangledCall(extractName) ? 1 : 0;
    Value *scalarIndexVal = fakeEI->getArgOperand(1 + start);
    V_ASSERT(isa<ConstantInt>(scalarIndexVal));
    uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
    V_ASSERT(scalarIndex < returnedVals.size() && "Extracting out of bound error");

    // Place value conversion in VCM entry
    createVCMEntryWithVectorValue(fakeEI, returnedVals[scalarIndex]);
    m_removedInsts.insert(fakeEI);
  }
}

bool PacketizeFunction::HandleReturnByPointers(CallInst* CI, CallInst *newCall) {
  V_ASSERT(CI->getType()->isVectorTy() && "expected vector type");
  VectorType *vTy = cast<VectorType>(CI->getType());
  unsigned numArgs = newCall->getNumArgOperands();
  unsigned numPtrs = vTy->getNumElements();
  V_ASSERT(numArgs > numPtrs && "bad signature");

  // Will contian the loads from the pointers to the returned values
  // generated in the obtain arguments phase.
  SmallVector<Instruction*, MAX_INPUT_VECTOR_WIDTH> loads;

  // create loads from the pointers to the returned values
  // which are the last arguments.to the new call.
  unsigned firstPtr = numArgs - numPtrs;
  for (unsigned i=0; i<numPtrs; i++) {
    Value *ptr = newCall->getArgOperand(i + firstPtr);
    V_ASSERT(ptr->getType()->isPointerTy() && "bad signature");
    if (!ptr) return false;
    Instruction* LI = new LoadInst(ptr, "", CI);
    V_ASSERT(LI->getType()->isVectorTy() && "bad signature");
    V_ASSERT(cast<VectorType>(LI->getType()) ==  
     VectorType::get(vTy->getElementType(), m_packetWidth) && "bad signature");
    loads.push_back(LI);
  }

  // Map the elements to fake extracts generated by the scalarizer to
  // the vector elemetns.
  MapVectorMultiReturn(CI, loads);
  return true;
}

Value *PacketizeFunction::HandleParamSOA(CallInst* CI, Value *scalarParam){
  /// Here we handle vector argument to a scalar built-in by creating an 
  /// array of vectors from the corresponding vectors of it's elements.
  /// Scalar elements are obtained by fake insert calls added by the scalarizer.
  /// foo(<2 float> %a) --> foo4([2 x <4 x float>]%a)
  V_ASSERT(scalarParam->getType()->isVectorTy() && "expected vector type");
  VectorType *aosType = cast<VectorType>(scalarParam->getType());
  ArrayType *soaType = ArrayType::get(VectorType::get(aosType->getElementType(),
                                      m_packetWidth),aosType->getNumElements());
  
  // Try to find the source elements for the vector argument.
  // In case of failure, need to duplicate the call.
  SmallVector<Value *, MAX_INPUT_VECTOR_WIDTH> multiOperands;
  if (! obtainInsertElement(scalarParam, multiOperands, soaType->getNumElements(), CI)) {
    V_ASSERT(false && "Store operations must be preceded by insertvalue insts");
    return NULL;
  }

  // Create array of vectors with the previously obtained vectors.
  Value *soaArr = UndefValue::get(soaType);
  unsigned numElements = soaType->getNumElements();
  for (unsigned i=0; i < numElements ; i++) {
    soaArr = InsertValueInst::Create(soaArr, multiOperands[i], i, "", CI);
  }
  return soaArr;
}

bool PacketizeFunction::SpreadVectorParam(CallInst* CI, Value *scalarParam,
                         FunctionType *LibFuncTy, std::vector<Value *> &args) {
  /// Here we handle vector argument to a scalar built-in by adding the
  /// corresponding vectors of the it's elements to the argumnet list. the 
  /// scalar elements are obtained by fake insert calls added by the scalarizer.
  /// foo(<2 float> %a) --> foo4(<4 x float> %a.x, <4 xfloat> %a.y)
  V_ASSERT(scalarParam->getType()->isVectorTy() && "expexted vector type");
  VectorType *vTy = cast<VectorType>(scalarParam->getType());
  SmallVector<Value *, MAX_INPUT_VECTOR_WIDTH> multiOperands;
  unsigned numElements = vTy->getNumElements();
  if (! obtainInsertElement(scalarParam, multiOperands, numElements, CI)) {
    V_ASSERT(false && "could not get all vectorized values");
    return false;
  }

  for (unsigned i=0; i < numElements ; i++) {
    Type *neededType = LibFuncTy->getParamType(args.size());
    Value *arg = VectorizerUtils::getCastedArgIfNeeded(multiOperands[i],
                                                       neededType, CI);
    V_ASSERT(arg && "could not cast");
    if (!arg) return false;
    args.push_back(arg);
  }
  return true;
}

bool PacketizeFunction::isScatter(InsertElementInst *IEI, InsertElementInst** InsertEltSequence,
              unsigned &AOSVectorWidth, Instruction *& lastInChain) {
  V_ASSERT(IEI && "IEI is NULL");			  
  Value *assembledVector = IEI->getOperand(0);
  // assembly of vectors should start with undef
  if (!isa<UndefValue>(assembledVector)) return false;

  VectorType *vType = dyn_cast<VectorType>(IEI->getType());
  V_ASSERT(vType && "InsertElement should be a vector");
  // currently we only support 32 bit transposes
  if (vType->getElementType()->getScalarSizeInBits() != 32) return false;
  AOSVectorWidth = vType->getNumElements();
  // currently we support only cases where AOSVectorWidth <= m_packetWidth
  if (AOSVectorWidth > m_packetWidth) return false;
  if (AOSVectorWidth < 2) return false;
   
  std::vector<bool> indicesPresent(AOSVectorWidth, false);
  // For each of the items we are trying to fetch
  for (unsigned i=0; i < AOSVectorWidth; ++i) {
    // Make sure our chain is made of InsertElements
    if (!IEI) return false;
    Value* index = IEI->getOperand(2);
    // Make sure that the place in the struct is constant
    ConstantInt* CI = dyn_cast<ConstantInt>(index);
    if (! CI) return false;
    unsigned idx = CI->getZExtValue();
    if (idx >= AOSVectorWidth) return false;
    indicesPresent[idx] = true;
	// checks that the value entered is in vector form 
	// TODO:: this check misses DRL entries!!!!!!!!!!
	Value *insertedValue = IEI->getOperand(1);
    if (!m_VCM.count(insertedValue)) return false;
	VCMEntry * foundEntry = m_VCM[insertedValue];
    if (foundEntry->vectorValue == NULL) return false;
    // saves original value
    InsertEltSequence[idx] = IEI;
    // must have one InsertElementUse (except for the last)
    if (i < (AOSVectorWidth-1))
    {
      if (IEI->getNumUses() != 1) return false;
      IEI = dyn_cast<InsertElementInst>(*(IEI->use_begin()));    
    } 
    else 
    {
      // this Value will be updated in the VCM
      lastInChain = IEI;
    }
  }
  // check that all indices are are inserted
  for (unsigned i=0; i < AOSVectorWidth; ++i) {
    if (!indicesPresent[i]) return false;
  }
  // success!
  return true;
}

// the algorithm transpose [n x m] matrices where n <= m by iteratively merging 
// couples of vectors first by joining single elements, than pairs, quads etc...
// a [4 x 8] wold look like:
// <x1,x2,x3,x4,x5,x6,x7,x8>     <x1,y1,x3,y3,x5,y5,x7,x7>     <x1,y1,z1,w1,x5,y5,z5,w5>
// <y1,y2,y3,y4,y5,y6,y7,y8>     <x2,y2,x4,y4,x6,y6,x8,y8>     <x2,y2,z2,w2,x6,y6,z6,w6>
// <z1,z2,z3,z4,z5,z6,z7,z8> --> <z1,w1,z3,w3,z5,w5,z7,w7> --> <x3,y3,z3,w3,x7,y7,z7,w7>
// <w1,w2,w3,w4,w5,w6,w7,w8>     <z2,w2,z4,w4,z6,w7,z8,w8>     <x4,y4,z4,w4,x8,y8,z8,w8>
// this process follows by break down of the scatterd matrices
// <x1,y1,z1,w1,x5,y5,z5,w5>  -->  <x1,y1,z1,w1>  <x5,y5,z5,w5> etc...
void PacketizeFunction::generateShuffles (unsigned AOSVectorWidth, Instruction *loc,
              Value *inputVectors[MAX_PACKET_WIDTH],
              Instruction *transposedVectors[MAX_PACKET_WIDTH])
{
  V_ASSERT(AOSVectorWidth <= m_packetWidth && "AOSVectorWidth > m_packetWidth");
  V_ASSERT(AOSVectorWidth > 1 && "AOSVectorWidth <= 1");
  
  // computing next power of two and adding undef inputs in case 
  // the origianl vector width is not a power of 2 since the algorithm 
  // only supports power of 2
  unsigned Power2VectorWidth = 1; 
  while (Power2VectorWidth < AOSVectorWidth) Power2VectorWidth <<= 1;
  for (unsigned i=AOSVectorWidth; i<Power2VectorWidth; ++i)
    inputVectors[i] = UndefValue::get(inputVectors[0]->getType());

  // temporary holders 
  int shuffleMaskL [MAX_PACKET_WIDTH]; // holds Low  int mask
  int shuffleMaskH [MAX_PACKET_WIDTH]; // holds High int mask
  //tmps hold input and output of transposing iterations
  Instruction *tmp1 [MAX_PACKET_WIDTH], *tmp2 [MAX_PACKET_WIDTH]; 
  unsigned iter = 0;
  Value **inputs = inputVectors;
  Instruction **outputs = tmp2;
  //transposing the input vectors
  //for (unsigned width=1; width<Power2VectorWidth; width*=2, iter++)
  for (unsigned width = Power2VectorWidth/2; width>=1; width/=2, iter++)
  {
    // switching between inputs and outputs (outputs of the previos iteration are 
    // the inputs of the current iteration)
    inputs  = (Value **)(iter % 2 == 0 ? tmp1 : tmp2 );
    outputs = (iter % 2 == 0 ? tmp2 : tmp1);
    if (iter == 0 ) inputs = inputVectors;
    
    // compute shuffle masks
    unsigned curL = 0, curH=0; //indices for shuffle mask arrays
    int first=0, second=m_packetWidth; // inital values for shuffle mask arrays
    while (curL < m_packetWidth)
    {
      for (unsigned j=0; j<width; j++)  shuffleMaskL[curL++] = first++;
      for (unsigned j=0; j<width; j++)  shuffleMaskH[curH++] = first++;
      for (unsigned j=0; j<width; j++)  shuffleMaskL[curL++] = second++;
      for (unsigned j=0; j<width; j++)  shuffleMaskH[curH++] = second++;
    }
    
    // get constants for shuffles
    Constant *SeqL = createIndicesForShuffles(m_packetWidth, shuffleMaskL);
    Constant *SeqH = createIndicesForShuffles(m_packetWidth, shuffleMaskH);
    
    //creating actual shuffles 
    unsigned ind = 0;
    for (unsigned i=0; i<Power2VectorWidth; i+=(width*2))
    {
      for (unsigned j=0; j<width; j++)
        outputs[ind++] = new ShuffleVectorInst(inputs[i+j], inputs[i+j+width], SeqL , "shuf_transpL", loc);
      for (unsigned j=0; j<width; j++)
        outputs[ind++] = new ShuffleVectorInst(inputs[i+j], inputs[i+j+width], SeqH , "shuf_transpH", loc);
    }
  }
  // incase of full transpose no need to breakdown the transposed vectors
  if (AOSVectorWidth == m_packetWidth)
  {
    for(unsigned i=0; i<m_packetWidth; i++)  transposedVectors[i] = outputs[i];
    return;
  }

  //creating mask constnats for shuffles
  int breakdownIndices[MAX_PACKET_WIDTH];
  for (unsigned i=0; i<m_packetWidth; ++i)
    breakdownIndices[i] = i;
  //filling transposedVectors with shuffle breakdowns
  unsigned transposedInd = 0;
  for(unsigned i=0; i<m_packetWidth; i+=Power2VectorWidth)
  {
    Constant *breakdownConst = createIndicesForShuffles(AOSVectorWidth, breakdownIndices + i);
    Value *undefVect = UndefValue::get(outputs[0]->getType());
    for (unsigned j=0; j< Power2VectorWidth; j++)
      transposedVectors[transposedInd++] = new ShuffleVectorInst(outputs[j], undefVect, breakdownConst , "breakdown", loc);
  }
}

void PacketizeFunction::packetizeInstruction(InsertElementInst *IEI)
{
//  V_PRINT(packetizer, "\t\InsertElement Instruction\n");
  V_ASSERT(IEI && "instruction type dynamic cast failed");
  
  if (m_packetWidth!=8 && m_packetWidth!=4) {
    V_PRINT(vectorizer_stat, "<<<<CannotHandleCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(IEI->getOpcode()) <<" m_packetWidth!=8 && m_packetWidth!=4\n");
    V_STAT(m_cannotHandleCtr++;)
    return duplicateNonPacketizableInst(IEI);
  }

  // check if this is and InsertEltSequence that can be transposed
  InsertElementInst *InsertEltSequence [MAX_PACKET_WIDTH];
  unsigned AOSVectorWidth;
  Instruction *lastInChain;
  bool canTranspose = isScatter(IEI, InsertEltSequence, AOSVectorWidth, lastInChain);
  if (!canTranspose) {
    V_PRINT(vectorizer_stat, "<<<<CannotHandleCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(IEI->getOpcode()) <<" can't be transposed\n");
    V_STAT(m_cannotHandleCtr++;)
    return duplicateNonPacketizableInst(IEI);
  }
  
  // obtaining vector inputs to perform transpose over
  Value *vectorizedInputs [MAX_PACKET_WIDTH];
  for (unsigned i=0; i<AOSVectorWidth; ++i)
    obtainVectorizedValue(&vectorizedInputs[i], InsertEltSequence[i]->getOperand(1), IEI);
  
  // generate shuffles that apply transpose over the set of input vectors  
  Instruction *gatheredValues [MAX_PACKET_WIDTH];
  generateShuffles(AOSVectorWidth, IEI, vectorizedInputs, gatheredValues);
  
  //updating VCM with scalar(vector) transposed vector
  createVCMEntryWithMultiScalarValues(lastInChain , gatheredValues);
  // mark all InsertElts for removal so they won't be packetized
  for (unsigned i=0; i<AOSVectorWidth; ++i)
  {
    m_removedInsts.insert(InsertEltSequence[i]);
  }
}


bool PacketizeFunction::obtainExtracts(Value  *vectorValue,
                             SmallVectorImpl<ExtractElementInst *> &extracts) {
  VectorType *VT = dyn_cast<VectorType>(vectorValue->getType());
  if (!VT) return false;
  unsigned inputVectorWidth = VT->getNumElements();
  if (inputVectorWidth < 2) return false;

  extracts.assign(inputVectorWidth, NULL);
  for (Value::use_iterator useIt = vectorValue->use_begin(), 
       useE = vectorValue->use_end(); useIt != useE; ++useIt) {
    if (ExtractElementInst *EEUser = dyn_cast<ExtractElementInst>(*useIt)) {
      Value * scalarIndexVal = EEUser->getIndexOperand();
      ConstantInt *constInd = dyn_cast<ConstantInt>(scalarIndexVal);
      if (!constInd) return false;
      unsigned ind = constInd->getZExtValue();
      if (ind >= inputVectorWidth || extracts[ind]) return false;
      extracts[ind] = EEUser;
    } 
  }
  return true;
}

void PacketizeFunction::obtainTranspVals32bitV4(SmallVectorImpl<Value *> &IN,
                                           SmallVectorImpl<Instruction *> &OUT,
                                           Instruction *loc) {
  int Seq64L_seq[] = {0,1,4,5};
  int Seq64H_seq[] = {2,3,6,7};

  int Seq32L_seq[] = {0, 2, 4, 6};
  int Seq32H_seq[] = {1, 3, 5, 7};

  Constant *Seq64L = createIndicesForShuffles(m_packetWidth, Seq64L_seq);
  Constant *Seq64H = createIndicesForShuffles(m_packetWidth, Seq64H_seq);
  Constant *Seq32L = createIndicesForShuffles(m_packetWidth, Seq32L_seq);
  Constant *Seq32H = createIndicesForShuffles(m_packetWidth, Seq32H_seq);

  llvm::SmallVector<Value*, 8> Level64;
  Level64.push_back(new ShuffleVectorInst(IN[0], IN[1], Seq64L , "Seq_64_0", loc)); // x0,y0,x1,y1
  Level64.push_back(new ShuffleVectorInst(IN[2], IN[3], Seq64L , "Seq_64_1", loc)); // x2,y2,x3,y3
  Level64.push_back(new ShuffleVectorInst(IN[0], IN[1], Seq64H , "Seq_64_2", loc)); // z0,w0,z1,w1
  Level64.push_back(new ShuffleVectorInst(IN[2], IN[3], Seq64H , "Seq_64_3", loc)); // z2,w2,z3,w3

  OUT.push_back(new ShuffleVectorInst(Level64[0], Level64[1], Seq32L , "Seq_32_0", loc)); // x0,x1,x2,x3
  OUT.push_back(new ShuffleVectorInst(Level64[0], Level64[1], Seq32H , "Seq_32_1", loc)); // y0,y1,y2,y3
  OUT.push_back(new ShuffleVectorInst(Level64[2], Level64[3], Seq32L , "Seq_32_2", loc)); // z0,z1,z2,z3
  OUT.push_back(new ShuffleVectorInst(Level64[2], Level64[3], Seq32H , "Seq_32_3", loc)); // w0,w1,w2,w3
}

void PacketizeFunction::obtainTranspVals32bitV8(SmallVectorImpl<Value *> &IN,
                                           SmallVectorImpl<Instruction *> &OUT,
                                           Instruction *loc){
  llvm::SmallVector<Value*, 8> Level128;
  llvm::SmallVector<Value*, 8> Level64;

  int Seq128L_seq[] = {0,1,2,3,8,9,10,11};
  int Seq128H_seq[] = {4,5,6,7,12,13,14,15};

  int Seq64L_seq[] = {0,1,8,9,4,5,12,13};
  int Seq64H_seq[] = {2,3,10,11,6,7,14,15};

  int Seq32L_seq[] = {0,8,2,10,4,12,6,14};
  int Seq32H_seq[] = {1,9,3,11,5,13,7,15};

  Constant *Seq128L = createIndicesForShuffles(m_packetWidth, Seq128L_seq);
  Constant *Seq128H = createIndicesForShuffles(m_packetWidth, Seq128H_seq);
  Constant *Seq64L = createIndicesForShuffles(m_packetWidth, Seq64L_seq);
  Constant *Seq64H = createIndicesForShuffles(m_packetWidth, Seq64H_seq);
  Constant *Seq32L = createIndicesForShuffles(m_packetWidth, Seq32L_seq);
  Constant *Seq32H = createIndicesForShuffles(m_packetWidth, Seq32H_seq);

  Level128.push_back(new ShuffleVectorInst(IN[0], IN[4], Seq128L , "Seq_128_0", loc));
  Level128.push_back(new ShuffleVectorInst(IN[1], IN[5], Seq128L , "Seq_128_1", loc));
  Level128.push_back(new ShuffleVectorInst(IN[2], IN[6], Seq128L , "Seq_128_2", loc));
  Level128.push_back(new ShuffleVectorInst(IN[3], IN[7], Seq128L , "Seq_128_3", loc));
  Level128.push_back(new ShuffleVectorInst(IN[0], IN[4], Seq128H , "Seq_128_4", loc));
  Level128.push_back(new ShuffleVectorInst(IN[1], IN[5], Seq128H , "Seq_128_5", loc));
  Level128.push_back(new ShuffleVectorInst(IN[2], IN[6], Seq128H , "Seq_128_6", loc));
  Level128.push_back(new ShuffleVectorInst(IN[3], IN[7], Seq128H , "Seq_128_7", loc));

  Level64.push_back(new ShuffleVectorInst(Level128[0], Level128[2], Seq64L , "Seq_64_0", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[1], Level128[3], Seq64L , "Seq_64_1", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[0], Level128[2], Seq64H , "Seq_64_2", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[1], Level128[3], Seq64H , "Seq_64_3", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[4], Level128[6], Seq64L , "Seq_64_4", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[5], Level128[7], Seq64L , "Seq_64_5", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[4], Level128[6], Seq64H , "Seq_64_6", loc));
  Level64.push_back(new ShuffleVectorInst(Level128[5], Level128[7], Seq64H , "Seq_64_7", loc));
  
  OUT.push_back(new ShuffleVectorInst(Level64[0], Level64[1], Seq32L , "Seq_32_0", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[0], Level64[1], Seq32H , "Seq_32_1", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[2], Level64[3], Seq32L , "Seq_32_2", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[2], Level64[3], Seq32H , "Seq_32_3", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[4], Level64[5], Seq32L , "Seq_32_4", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[4], Level64[5], Seq32H , "Seq_32_5", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[6], Level64[7], Seq32L , "Seq_32_6", loc));
  OUT.push_back(new ShuffleVectorInst(Level64[6], Level64[7], Seq32H , "Seq_32_7", loc));
}

void PacketizeFunction::packetizeInstruction(ExtractElementInst *EI)
{
  // ExtractElement instruction, when packetized, is transformed to
  // a sort of matrix transpose
  V_PRINT(packetizer, "\t\tExtractElement Instruction\n");
  V_ASSERT(EI && "instruction type dynamic cast failed");

  SmallVector<ExtractElementInst *, 16> extracts;
  Value *vectorValue = EI->getVectorOperand();
  if (!obtainExtracts(vectorValue, extracts)) {
    V_PRINT(vectorizer_stat, "<<<<CannotHandleCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(EI->getOpcode()) <<" index should be a constant, and the vector width is 2 or more\n");
    V_STAT(m_cannotHandleCtr++;)
    return duplicateNonPacketizableInst(EI);
  }

  unsigned inputVectorWidth = EI->getVectorOperandType()->getNumElements();
  
  // Obtain the packetized version of the vector input (actually multiple vectors)
  SmallVector<Value *, MAX_INPUT_VECTOR_WIDTH> inputOperands;
  inputOperands.assign(MAX_INPUT_VECTOR_WIDTH, NULL);
  obtainMultiScalarValues(&(inputOperands[0]), vectorValue, EI);
  V_ASSERT(inputOperands[0]->getType() == vectorValue->getType() && "input type error");

  // Make the transpose optimization only while arch vector is 4,
  // and the input vector is equal or smaller than that.
  if ((m_packetWidth != 4 && m_packetWidth != 8) ||
    inputVectorWidth > m_packetWidth)
  {
    // No optimized solution implemented for this setup
    V_PRINT(vectorizer_stat, "<<<<CannotHandleCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(EI->getOpcode()) <<" m_packetWidth != 4 && m_packetWidth != 8 || inputVectorWidth > m_packetWidth\n");
    V_STAT(m_cannotHandleCtr++;)
    return duplicateNonPacketizableInst(EI);
  }

  // If inputVectorWidth is smaller than PACKET_WIDTH - extend the input vectors first
  if (inputVectorWidth < m_packetWidth)
  {
    // Create the constant sequence for extending. Will look like (0, 1, undef, undef)
    int extender_sequence[MAX_PACKET_WIDTH];
    for (unsigned k = 0; k < MAX_PACKET_WIDTH; k++)
    {
      if (k < inputVectorWidth) {
        extender_sequence[k] = k;
      } else {
        extender_sequence[k] = -1;
      }
    }
    Constant *vectExtend = createIndicesForShuffles(m_packetWidth, extender_sequence);
    UndefValue *undefVect = UndefValue::get(inputOperands[0]->getType());
    // Replace all the original input operands with their extended versions
    for (unsigned i = 0; i < m_packetWidth; i++)
    {
      inputOperands[i] = new ShuffleVectorInst(inputOperands[i],
        undefVect, vectExtend , "extend_vec", EI);
    }
  }

  // Create the transpose sequence.
  SmallVector<Instruction *, 16> SOA;
  if (m_packetWidth == 8) {
    if (EI->getType()->getScalarType()->getScalarSizeInBits() != 32 ||
      inputVectorWidth < 4) {
        V_PRINT(vectorizer_stat, "<<<<CannotHandleCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(EI->getOpcode()) <<" m_packetWidth == 8 && (getScalarSizeInBits() != 32 || inputVectorWidth < 4)\n");
        V_STAT(m_cannotHandleCtr++;)
        return duplicateNonPacketizableInst(EI);
    }
    obtainTranspVals32bitV8(inputOperands, SOA, EI);
  } else {
    V_ASSERT(4 == m_packetWidth && "only supports packetWidth=4,8");
    obtainTranspVals32bitV4(inputOperands, SOA, EI);
  }
  
  // add new value/s to VCM and Remove original instruction
  for (unsigned i=0, e = extracts.size(); i<e; ++i) {
    ExtractElementInst *curEI = extracts[i];
    if (curEI) {
      createVCMEntryWithVectorValue(curEI, SOA[i]);  
      m_removedInsts.insert(curEI);
    }
  }

}


void PacketizeFunction::packetizeInstruction(SelectInst *SI)
{
  V_PRINT(packetizer, "\t\tSelect Instruction\n");
  V_ASSERT(SI && "instruction type dynamic cast failed");

  // If instruction's return type is not primitive - cannot packetize
  if (!SI->getType()->isIntegerTy() && !SI->getType()->isFloatingPointTy()) {
    V_PRINT(vectorizer_stat, "<<<<NonPrimitiveCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(SI->getOpcode()) <<" instruction's return type is not primitive\n");
    V_STAT(m_nonPrimitiveCtr[SI->getOpcode()]++;)
    return duplicateNonPacketizableInst(SI);
  }

  Value * conditionalOperand[MAX_PACKET_WIDTH];
  Value * trueOperand;
  Value * falseOperand;

  // Check if the selector is uniform (if so, we don't generate a vector select)
  bool isUniformCondition =
    (WIAnalysis::UNIFORM == m_depAnalysis->whichDepend(SI->getCondition()));
  if (!isUniformCondition)
  {
    // While back-end does not support vector selects, fallback to scalar...
    // return duplicateNonPacketizableInst(SI);
  }

  // Obtain vectorized versions of operands
  if (!isUniformCondition) {
    obtainVectorizedValue(conditionalOperand, SI->getCondition(), SI);
  }
  else {
    obtainMultiScalarValues(conditionalOperand, SI->getCondition(), SI);
  }
  obtainVectorizedValue(&trueOperand, SI->getTrueValue(), SI);
  obtainVectorizedValue(&falseOperand, SI->getFalseValue(), SI);
  V_ASSERT(trueOperand->getType() == falseOperand->getType() && "vector type error");

  // Generate new instruction
  SelectInst *newSelect =
    SelectInst::Create(conditionalOperand[0], trueOperand, falseOperand, SI->getName(), SI);

  // Add new value to VCM
  createVCMEntryWithVectorValue(SI, newSelect);

  // Remove original instruction
  m_removedInsts.insert(SI);
}


void PacketizeFunction::packetizeInstruction(AllocaInst *AI)
{
  V_PRINT(packetizer, "\t\tAlloca Instruction\n");
  V_ASSERT(AI && "instruction type dynamic cast failed");
  V_STAT(m_allocaCtr++;)
  return duplicateNonPacketizableInst(AI);
}


void PacketizeFunction::packetizeInstruction(LoadInst *LI) {
  MemoryOperation MO;
  MO.Mask = 0;
  MO.Ptr = LI->getPointerOperand();
  MO.Data = NULL;
  MO.Alignment = LI->getAlignment();
  MO.Base = 0;
  MO.Index = 0;
  MO.Orig = LI;
  return packetizeMemoryOperand(MO);
}

void PacketizeFunction::packetizeInstruction(StoreInst *SI) {
  MemoryOperation MO;
  MO.Mask = 0;
  MO.Ptr = SI->getPointerOperand();
  MO.Data = SI->getValueOperand();
  MO.Alignment = SI->getAlignment();
  MO.Base = 0;
  MO.Index = 0;
  MO.Orig = SI;
  return packetizeMemoryOperand(MO);
}


void PacketizeFunction::packetizeInstruction(GetElementPtrInst *GI)
{
  V_PRINT(packetizer, "\t\tGetElementPtr Instruction\n");
  V_ASSERT(GI && "instruction type dynamic cast failed");
  V_STAT(m_getElemPtrCtr++;)
  return duplicateNonPacketizableInst(GI);
}


void PacketizeFunction::packetizeInstruction(BranchInst *BI)
{
  // Packetizing branches is dangerous, if their condition is not uniform
  // Packetizer relies blindly on control flow to be uniform!
  V_PRINT(packetizer, "\t\tBranch Instruction\n");
  V_ASSERT(BI && "instruction type dynamic cast failed");

  if (BI->isConditional())
  {
    // Obtain the post-packetization conditional, and put instead of existing condition
    Value * nonVectorizedCondition[MAX_PACKET_WIDTH];
    obtainMultiScalarValues(nonVectorizedCondition, BI->getCondition() , BI);
    BI->setCondition(nonVectorizedCondition[0]);
  }

  // Instruction is not replaced with another one
  return useOriginalConstantInstruction(BI);
}


void PacketizeFunction::packetizeInstruction(PHINode *PI)
{
  V_PRINT(packetizer, "\t\tPHI Instruction\n");
  V_ASSERT(PI && "instruction type dynamic cast failed");

  unsigned numValues = PI->getNumIncomingValues();
  Type * retType = PI->getType();

  // If instruction's return type is not primitive - cannot packetize
  if (!retType->isIntegerTy() && !retType->isFloatingPointTy()) {
    V_PRINT(vectorizer_stat, "<<<<NonPrimitiveCtr("<<__FILE__<<":"<<__LINE__<<"): "<<Instruction::getOpcodeName(PI->getOpcode()) <<" instruction's return type is not primitive\n");
    V_STAT(m_nonPrimitiveCtr[PI->getOpcode()]++;)
    return duplicateNonPacketizableInst(PI);
  }

  Type * vectorPHIType = VectorType::get(retType, m_packetWidth);

  // Create new PHI node
  PHINode *newPHINode = PHINode::Create(vectorPHIType, numValues, "vectorPHI", PI);

  // Obtain vectorized versions of incoming values, and place in new PHI node/s
  for (unsigned inputNum = 0; inputNum < numValues; inputNum++)
  {
    Value * origVal = PI->getIncomingValue(inputNum);
    BasicBlock * origBlock = PI->getIncomingBlock(inputNum);
    Value * newValue;
    obtainVectorizedValue(&newValue, origVal, PI);
    // Place vector values in PHI node/s
    newPHINode->addIncoming(newValue, origBlock);
  }

  // Add new value/s to VCM
  createVCMEntryWithVectorValue(PI, newPHINode);

  // Remove original instruction
  m_removedInsts.insert(PI);
}

void PacketizeFunction::packetizeInstruction(ReturnInst *RI)
{
  V_PRINT(packetizer, "\t\tRet Instruction\n");
  V_ASSERT(RI && "instruction type dynamic cast failed");
  V_ASSERT(NULL == RI->getReturnValue());
  // Just use the existing instruction
  return useOriginalConstantInstruction(RI);
}




Constant *PacketizeFunction::createIndicesForShuffles(unsigned width, int *values)
{
  // Generate a vector and fill with given values (as constant integers)
  // Negative values are converted to constant Undefs
  std::vector<Constant*> pre_vect;
  for (unsigned i = 0; i< width; i++)
  {
    if (values[i] >= 0)
    {
      pre_vect.push_back(ConstantInt::get(Type::getInt32Ty(context()), values[i]));
    }
    else
    {
      pre_vect.push_back(UndefValue::get(Type::getInt32Ty(context())));
    }
  }
  return ConstantVector::get(pre_vect);
}


void PacketizeFunction::generateSequentialIndices(Instruction *I)
{
  V_PRINT(packetizer, 
      "\t\tFollowing get_global/local_id, make sequential indices Instructions\n");
  Instruction *tidGenInst = I;

  // If the sequence generator is a masked instruction, need to unmask it first
  CallInst * CI = dyn_cast<CallInst>(I);
  if (CI && Mangler::isMangledCall(CI->getCalledFunction()->getName()))
  {
    std::vector<Value*> params;
    // Create arguments for new function call, ignoring the predicate operand
    for (unsigned j = 1; j < CI->getNumArgOperands(); ++j)
    {
      params.push_back(CI->getArgOperand(j));
    }

    Function *origFunc = m_currFunc->getParent()->getFunction(
      Mangler::demangle(CI->getCalledFunction()->getName()));
    V_ASSERT(origFunc && "error finding unmasked function!");
    tidGenInst = CallInst::Create(origFunc, ArrayRef<Value*>(params), "", I);

    // Remove original instruction
    m_removedInsts.insert(I);
  }

  // Prepare: Obtain the used type of the ID, and make a vector for it
  Type * usedType = tidGenInst->getType();
  V_ASSERT(!isa<VectorType>(usedType) && "expected TID value to be scalar");
  VectorType *vectorIndexType = VectorType::get(usedType, m_packetWidth);

  // Generate the broadcasting of the original ID
  Constant * constZero = ConstantInt::get(Type::getInt32Ty(context()), 0);
  UndefValue *undefVect = UndefValue::get(vectorIndexType);
  Instruction *inst1 = InsertElementInst::Create(undefVect, tidGenInst, constZero, "broadcast1");
  Constant *zeroVector = ConstantVector::get(std::vector<Constant*>(m_packetWidth, constZero));
  Instruction *shuffleInst = new ShuffleVectorInst(inst1, undefVect, zeroVector , "broadcast2");
  inst1->insertAfter(tidGenInst);
  shuffleInst->insertAfter(inst1);

  // Generate the constant vector
  std::vector<Constant*> constList;
  Instruction::BinaryOps addOperation = Instruction::Add;
  if (usedType->isIntegerTy())
  {
    uint64_t constVal = 0;
    for (unsigned j=0; j < m_packetWidth; ++j)
    {
      constList.push_back(ConstantInt::get(usedType, constVal++));
    }
  }
  else if (usedType->isFloatingPointTy())
  {
    addOperation = Instruction::FAdd;
    double constVal = 0.0f;
    for (unsigned j=0; j < m_packetWidth; ++j)
    {
      constList.push_back(ConstantFP::get(usedType, constVal++));
    }
  }
  else
  {
    V_ASSERT(0 && "unsupported type for generating sequential indices");
  }

  // Generate the TID vectors
  BinaryOperator *vectorIndex = BinaryOperator::Create(
    addOperation, shuffleInst, ConstantVector::get(ArrayRef<Constant*>(constList)));
  vectorIndex->insertAfter(shuffleInst);

  // register the new converted value.
  createVCMEntryWithVectorValue(I, vectorIndex);
}


} // namespace


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createPacketizerPass(bool scatterGather = false) {
    return new intel::PacketizeFunction(scatterGather);
  }
}

/// Support for dynamic loading of modules under Linux
char intel::PacketizeFunction::ID = 0;
static RegisterPass<intel::PacketizeFunction> PACKETIZER("packetize", "packetize functions");
