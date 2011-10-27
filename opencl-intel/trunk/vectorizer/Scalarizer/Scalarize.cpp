/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#include "Scalarize.h"

namespace intel {


ScalarizeFunction::ScalarizeFunction() : FunctionPass(ID)
{
  m_rtServices = RuntimeServices::get();
  V_ASSERT(m_rtServices && "Runtime services were not initialized!");

  // Initialize SCM buffers and allocation
  m_SCMAllocationArray = new SCMEntry[ESTIMATED_INST_NUM];
  m_SCMArrays.push_back(m_SCMAllocationArray);
  m_SCMArrayLocation = 0;
  
  V_PRINT(scalarizer, "ScalarizeFunction constructor\n");
}

ScalarizeFunction::~ScalarizeFunction()
{
  releaseAllSCMEntries();
  delete[] m_SCMAllocationArray;
  V_PRINT(scalarizer, "ScalarizeFunction destructor\n");
}



bool ScalarizeFunction::runOnFunction(Function &F)
{
  // Scalarization is done only on functions which return void (kernels)
  if (!F.getReturnType()->isVoidTy())  {
    return false;
  }

  m_currFunc = &F;
  m_moduleContext = &(m_currFunc->getContext());

  V_PRINT(scalarizer, "\nStart scalarizing function: " << m_currFunc->getName() << "\n");

  // Prepare data structures for scalarizing a new function
  m_scalarizableRootsMap.clear();
  m_usedVectors.clear();
  m_removedInsts.clear();
  m_SCM.clear();
  releaseAllSCMEntries();
  m_DRL.clear();

  // Scan function for CALL instructions. Find their "real" (pre-cast) values
  preScalarizeScanFunctions();

  // Scalarization. Iterate over all the instructions
  // Always hold the iterator at the instruction following the one being scalarized (so the
  // iterator will "skip" any instructions that are going to be added in the scalarization work)
  inst_iterator sI = inst_begin(m_currFunc);
  inst_iterator sE = inst_end(m_currFunc);
  while (sI != sE)
  {
    Instruction *currInst = &*sI;
    // Move iterator to next instruction BEFORE scalarizing current instruction
    ++sI; 
    dispatchInstructionToScalarize(currInst);
  }

  resolveVectorValues();

  // Resolved DRL entries
  resolveDeferredInstructions();

  // Iterate over removed insts and delete them
  SmallPtrSet<Instruction*, ESTIMATED_INST_NUM>::iterator ri = m_removedInsts.begin();
  SmallPtrSet<Instruction*, ESTIMATED_INST_NUM>::iterator re = m_removedInsts.end();
  SmallPtrSet<Instruction*, ESTIMATED_INST_NUM>::iterator index = ri;

  for (;index != re; ++index) {
    // get rid of old users
    if (Value* val = dyn_cast<Value>(*index)) {
      UndefValue *undefVal = UndefValue::get((*index)->getType());
      (val)->replaceAllUsesWith(undefVal);
    }
    V_ASSERT((*index)->use_empty() && "Unable to remove used instruction");
    (*index)->eraseFromParent();
  }

  V_PRINT(scalarizer, "\nCompleted scalarizing function: " << m_currFunc->getName() << "\n");
  return true;
}





void ScalarizeFunction::dispatchInstructionToScalarize(Instruction *I)
{
  V_PRINT(scalarizer, "\tScalarizing Instruction: " << *I << "\n");

  if (m_removedInsts.count(I))
  {
    V_PRINT(scalarizer, "\tInstruction is already marked for removal. Being ignored..\n");
    return;
  }

  switch (I->getOpcode())
  {
    case Instruction::Add :
    case Instruction::Sub :
    case Instruction::Mul :
      scalarizeInstruction(dyn_cast<BinaryOperator>(I), true);
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
      scalarizeInstruction(dyn_cast<BinaryOperator>(I), false);
      break;
    case Instruction::ICmp :
    case Instruction::FCmp :
      scalarizeInstruction(dyn_cast<CmpInst>(I));
      break;
    case Instruction::Trunc :
    case Instruction::ZExt :
    case Instruction::SExt :
    case Instruction::FPToUI :
    case Instruction::FPToSI :
    case Instruction::UIToFP :
    case Instruction::SIToFP :
    case Instruction::FPTrunc :
    case Instruction::FPExt :
    case Instruction::PtrToInt :
    case Instruction::IntToPtr :
    case Instruction::BitCast :
      scalarizeInstruction(dyn_cast<CastInst>(I));
      break; 
    case Instruction::PHI :
      scalarizeInstruction(dyn_cast<PHINode>(I));
      break;
    case Instruction::Select :
      scalarizeInstruction(dyn_cast<SelectInst>(I));
      break;
    case Instruction::ExtractElement :
      scalarizeInstruction(dyn_cast<ExtractElementInst>(I));
      break;
    case Instruction::InsertElement :
      scalarizeInstruction(dyn_cast<InsertElementInst>(I));
      break;
    case Instruction::ShuffleVector :
      scalarizeInstruction(dyn_cast<ShuffleVectorInst>(I));
      break;
    case Instruction::Call :
      scalarizeInstruction(dyn_cast<CallInst>(I));
      break;

      // The remaining instructions are not supported for scalarization. Keep "as is"
    default :
      recoverNonScalarizableInst(I);
      break;
  }
}

void ScalarizeFunction::recoverNonScalarizableInst(Instruction *Inst)
{
  V_PRINT(scalarizer, "\t\tInstruction is not scalarizable.\n");

  // any vector value should have an SCM entry - even an empty one
  if (isa<VectorType>(Inst->getType())) getSCMEntry(Inst);

  // Iterate over all arguments. Check that they all exist (or rebuilt)
  if (CallInst *CI = dyn_cast<CallInst>(Inst))
  {
    unsigned numOperands = CI->getNumArgOperands();
    for (unsigned i = 0; i < numOperands; i++)
    {
      Value *operand = CI->getArgOperand(i);
      if (isa<VectorType>(operand->getType()))
      {
        // Recover value if needed (only needed for vector values)
        obtainVectorValueWhichMightBeScalarized(operand);
      }
    }
  }
  else
  {    
    unsigned numOperands = Inst->getNumOperands();
    for (unsigned i = 0; i < numOperands; i++)
    {
      Value *operand = Inst->getOperand(i);
      if (isa<VectorType>(operand->getType()))
      {
        // Recover value if needed (only needed for vector values)
        obtainVectorValueWhichMightBeScalarized(operand);
      }
    }
  }
}

void ScalarizeFunction::scalarizeInstruction(BinaryOperator *BI, bool supportsWrap)
{
  V_PRINT(scalarizer, "\t\tBinary instruction\n");
  V_ASSERT(BI && "instruction type dynamic cast failed");
  const VectorType *instType = dyn_cast<VectorType>(BI->getType());
  // Only need handling for vector binary ops
  if (!instType) return;

  // Prepare empty SCM entry for the instruction
  SCMEntry *newEntry = getSCMEntry(BI);

  // Get additional info from instruction
  unsigned numElements = instType->getNumElements();
  V_ASSERT(numElements <= MAX_INPUT_VECTOR_WIDTH && "Inst vector width larger than supported");
  bool has_NSW = (supportsWrap ? BI->hasNoSignedWrap() : false);
  bool has_NUW = (supportsWrap ? BI->hasNoUnsignedWrap() : false);

  // Obtain scalarized arguments
  Value *operand0[MAX_INPUT_VECTOR_WIDTH];
  Value *operand1[MAX_INPUT_VECTOR_WIDTH];
  bool op0IsConst, op1IsConst;
  obtainScalarizedValues(operand0, &op0IsConst, BI->getOperand(0), BI);
  obtainScalarizedValues(operand1, &op1IsConst, BI->getOperand(1), BI);

  // If both arguments are constants, don't bother Scalarizing inst
  if (op0IsConst && op1IsConst) return;

  // Generate new (scalar) instructions
  Value *newScalarizedInsts[MAX_INPUT_VECTOR_WIDTH];
  for (unsigned dup = 0; dup < numElements; dup++)
  {
    newScalarizedInsts[dup] = BinaryOperator::Create(
      BI->getOpcode(),
      operand0[dup],
      operand1[dup],
      BI->getName(),
      BI
      );
    if (has_NSW) cast<BinaryOperator>(newScalarizedInsts[dup])->setHasNoSignedWrap();
    if (has_NUW) cast<BinaryOperator>(newScalarizedInsts[dup])->setHasNoUnsignedWrap();
  }

  // Add new value/s to SCM
  updateSCMEntryWithValues(newEntry, newScalarizedInsts, BI, true);

  // Remove original instruction
  m_removedInsts.insert(BI);
}


void ScalarizeFunction::scalarizeInstruction(CmpInst *CI)
{
  V_PRINT(scalarizer, "\t\tCompare instruction\n");
  V_ASSERT(CI && "instruction type dynamic cast failed");
  const VectorType *instType = dyn_cast<VectorType>(CI->getType());
  // Only need handling for vector compares
  if (!instType) return;

  // Prepare empty SCM entry for the instruction
  SCMEntry *newEntry = getSCMEntry(CI);

  // Get additional info from instruction
  unsigned numElements = instType->getNumElements();
  V_ASSERT(numElements <= MAX_INPUT_VECTOR_WIDTH && "Inst vector width larger than supported");

  // Obtain scalarized arguments
  Value *operand0[MAX_INPUT_VECTOR_WIDTH];
  Value *operand1[MAX_INPUT_VECTOR_WIDTH];
  bool op0IsConst, op1IsConst;
  obtainScalarizedValues(operand0, &op0IsConst, CI->getOperand(0), CI);
  obtainScalarizedValues(operand1, &op1IsConst, CI->getOperand(1), CI);

  // If both arguments are constants, don't bother Scalarizing inst
  if (op0IsConst && op1IsConst) return;

  // Generate new (scalar) instructions
  Value *newScalarizedInsts[MAX_INPUT_VECTOR_WIDTH];
  for (unsigned dup = 0; dup < numElements; dup++)
  {
    newScalarizedInsts[dup] = CmpInst::Create(
      CI->getOpcode(),
      CI->getPredicate(),
      operand0[dup],
      operand1[dup],
      CI->getName(),
      CI
      );
  }

  // Add new value/s to SCM
  updateSCMEntryWithValues(newEntry, newScalarizedInsts, CI, true);

  // Remove original instruction
  m_removedInsts.insert(CI);
}

void ScalarizeFunction::scalarizeInstruction(CastInst *CI)
{
  V_PRINT(scalarizer, "\t\tCast instruction\n");
  V_ASSERT(CI && "instruction type dynamic cast failed");
  const VectorType *instType = dyn_cast<VectorType>(CI->getType());
  
  // For BitCast - we only scalarize if src and dst types have same vector length
  if (isa<BitCastInst>(CI)) {
    if (!instType) return recoverNonScalarizableInst(CI);
    const VectorType *srcType = dyn_cast<VectorType>(CI->getOperand(0)->getType());
    if (!srcType || (instType->getNumElements() != srcType->getNumElements()))
      return recoverNonScalarizableInst(CI);
  }

  // Only need handling for vector cast
  if (!instType) return;

  // Prepare empty SCM entry for the instruction
  SCMEntry *newEntry = getSCMEntry(CI);

  // Get additional info from instruction
  unsigned numElements = instType->getNumElements();
  V_ASSERT(numElements <= MAX_INPUT_VECTOR_WIDTH && "Inst vector width larger than supported");
  V_ASSERT(isa<VectorType>(CI->getOperand(0)->getType()) && "unexpected type!");
  V_ASSERT(cast<VectorType>(CI->getOperand(0)->getType())->getNumElements() == numElements
    && "unexpected vector width");

  // Obtain scalarized argument
  Value *operand0[MAX_INPUT_VECTOR_WIDTH];
  bool op0IsConst;
  obtainScalarizedValues(operand0, &op0IsConst, CI->getOperand(0), CI);

  // If argument is a constant, don't bother Scalarizing inst
  if (op0IsConst) return;

  // Obtain type, which ever scalar cast will cast-to
  const Type *scalarDestType = instType->getElementType();

  // Generate new (scalar) instructions
  Value *newScalarizedInsts[MAX_INPUT_VECTOR_WIDTH];
  for (unsigned dup = 0; dup < numElements; dup++)
  {
    newScalarizedInsts[dup] = CastInst::Create(
      CI->getOpcode(),
      operand0[dup],
      scalarDestType,
      CI->getName(),
      CI
      );
  }

  // Add new value/s to SCM
  updateSCMEntryWithValues(newEntry, newScalarizedInsts, CI, true);

  // Remove original instruction
  m_removedInsts.insert(CI);
}

void ScalarizeFunction::scalarizeInstruction(PHINode *PI)
{
  V_PRINT(scalarizer, "\t\tPHI instruction\n");
  V_ASSERT(PI && "instruction type dynamic cast failed");
  const VectorType *instType = dyn_cast<VectorType>(PI->getType());
  // Only need handling for vector PHI
  if (!instType) return;

  // Prepare empty SCM entry for the instruction
  SCMEntry *newEntry = getSCMEntry(PI);

  // Get additional info from instruction
  const Type *scalarType = instType->getElementType();
  unsigned numElements = instType->getNumElements();
  V_ASSERT(numElements <= MAX_INPUT_VECTOR_WIDTH && "Inst vector width larger than supported");

  // Obtain number of incoming nodes \ PHI values
  unsigned numValues = PI->getNumIncomingValues();

  // Create new (empty) PHI nodes, and place them.
  Value *newScalarizedPHI[MAX_INPUT_VECTOR_WIDTH];
  for (unsigned i = 0; i < numElements; i++)
  {
    newScalarizedPHI[i] = PHINode::Create(scalarType, PI->getName(), PI);
    cast<PHINode>(newScalarizedPHI[i])->reserveOperandSpace(numValues);
  }

  // Iterate over incoming values in vector PHI, and fill scalar PHI's accordingly
  Value *operand[MAX_INPUT_VECTOR_WIDTH];
  bool dummyVal;
  for (unsigned j = 0; j < numValues; j++)
  {
    // Obtain scalarized arguments
    obtainScalarizedValues(operand, &dummyVal, PI->getIncomingValue(j), PI);

    // Fill all scalarized PHI nodes with scalar arguments
    for (unsigned i = 0; i < numElements; i++)
    {
      cast<PHINode>(newScalarizedPHI[i])->addIncoming(operand[i], PI->getIncomingBlock(j));
    }
  }

  // Add new value/s to SCM
  updateSCMEntryWithValues(newEntry, newScalarizedPHI, PI, true);

  // Remove original instruction
  m_removedInsts.insert(PI);
}



void ScalarizeFunction::scalarizeInstruction(SelectInst * SI)
{
  V_PRINT(scalarizer, "\t\tSelect instruction\n");
  V_ASSERT(SI && "instruction type dynamic cast failed");
  const VectorType *instType = dyn_cast<VectorType>(SI->getType());
  // Only need handling for vector select
  if (!instType) return;

  // Prepare empty SCM entry for the instruction
  SCMEntry *newEntry = getSCMEntry(SI);

  // Get additional info from instruction
  unsigned numElements = instType->getNumElements();
  V_ASSERT(numElements <= MAX_INPUT_VECTOR_WIDTH && "Inst vector width larger than supported");

  // Obtain scalarized arguments (select inst has 3 arguments: Cond, TrueVal, FalseVal)
  Value *condOp[MAX_INPUT_VECTOR_WIDTH];
  Value *trueValOp[MAX_INPUT_VECTOR_WIDTH];
  Value *falseValOp[MAX_INPUT_VECTOR_WIDTH];
  bool dummy;
  obtainScalarizedValues(trueValOp, &dummy, SI->getTrueValue(), SI);
  obtainScalarizedValues(falseValOp, &dummy, SI->getFalseValue(), SI);

  // Check if condition is a vector.
  Value *conditionVal = SI->getCondition();
  if (isa<VectorType>(conditionVal->getType())) {
    // Obtain scalarized breakdowns of condition
    obtainScalarizedValues(condOp, &dummy, conditionVal, SI);
  } else {
    // Broadcast the (scalar) condition, to be used by all the insruction breakdowns
    for (unsigned i = 0; i < numElements; i++) condOp[i] = conditionVal;
  }

  // Generate new (scalar) instructions  
  Value *newScalarizedInsts[MAX_INPUT_VECTOR_WIDTH];
  for (unsigned dup = 0; dup < numElements; dup++)
  {
    // Small optimization: Some scalar selects may be redundant (trueVal == falseVal)
    if (trueValOp[dup] != falseValOp[dup])
    {
      newScalarizedInsts[dup] = SelectInst::Create(
        condOp[dup],
        trueValOp[dup],
        falseValOp[dup],
        SI->getName(),
        SI
        );
    }
    else
    {
      // just "connect" the destination value to the true value input
      newScalarizedInsts[dup] = trueValOp[dup];
    }
  }

  // Add new value/s to SCM
  updateSCMEntryWithValues(newEntry, newScalarizedInsts, SI, true);

  // Remove original instruction
  m_removedInsts.insert(SI);
}


void ScalarizeFunction::scalarizeInstruction(ExtractElementInst *EI)
{
  V_PRINT(scalarizer, "\t\tExtractElement instruction\n");
  V_ASSERT(EI && "instruction type dynamic cast failed");

  // Proper scalarization makes "extractElement" instructions redundant
  // Only need to "follow" the scalar element (as the input vector was
  // already scalarized)
  Value *vectorValue = EI->getOperand(0);
  Value *scalarIndexVal = EI->getOperand(1);

  // If the index is not a constant - we cannot statically remove this inst
  if(!isa<ConstantInt>(scalarIndexVal)) return recoverNonScalarizableInst(EI);

  // Obtain the scalarized operands
  Value *operand[MAX_INPUT_VECTOR_WIDTH];
  bool dummy;
  obtainScalarizedValues(operand, &dummy, vectorValue, EI);

  // Connect the "extracted" value to all its consumers
  uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
  V_ASSERT(NULL != operand[scalarIndex] && "SCM error");

  // Replace all users of this inst, with the extracted scalar value
  EI->replaceAllUsesWith(operand[scalarIndex]);

  // Remove original instruction
  m_removedInsts.insert(EI);
}

void ScalarizeFunction::scalarizeInstruction(InsertElementInst *II)
{
  V_PRINT(scalarizer, "\t\tInsertElement instruction\n");
  V_ASSERT(II && "instruction type dynamic cast failed");

  // Proper scalarization makes "InsertElement" instructions redundant.
  // Only need to "follow" the scalar elements and update in SCM
  Value *sourceVectorValue = II->getOperand(0);
  Value *sourceScalarValue = II->getOperand(1);
  Value *scalarIndexVal = II->getOperand(2);

  // If the index is not a constant - we cannot statically remove this inst
  if(!isa<ConstantInt>(scalarIndexVal)) return recoverNonScalarizableInst(II);

  // Prepare empty SCM entry for the instruction
  SCMEntry *newEntry = getSCMEntry(II);

  V_ASSERT(isa<ConstantInt>(scalarIndexVal) && "inst arguments error");
  uint64_t scalarIndex = cast<ConstantInt>(scalarIndexVal)->getZExtValue();
  V_ASSERT(scalarIndex <= dyn_cast<VectorType>(II->getType())->getNumElements() && "index error");

  // Obtain breakdown of input vector
  Value *scalarValues[MAX_INPUT_VECTOR_WIDTH];
  if (isa<UndefValue>(sourceVectorValue))
  {
    // Scalarize the undef value (generate a scalar undef)
    const VectorType *inputVectorType = dyn_cast<VectorType>(sourceVectorValue->getType());
    V_ASSERT(inputVectorType && "expected vector argument");
    UndefValue *undefVal = UndefValue::get(inputVectorType->getElementType());

    // fill new SCM entry with UNDEFs and the new value
    for (unsigned j = 0; j<inputVectorType->getNumElements(); j++)
      scalarValues[j] = undefVal;
    scalarValues[scalarIndex] = sourceScalarValue;
  }
  else
  {
    // Obtain the scalar values of the input vector
    bool dummy;
    obtainScalarizedValues(scalarValues, &dummy, sourceVectorValue, II);
    // Add the new element
    scalarValues[scalarIndex] = sourceScalarValue;
  }

  // Add new value/s to SCM
  updateSCMEntryWithValues(newEntry, scalarValues, II, true);

  // Remove original instruction
  m_removedInsts.insert(II);
}

void ScalarizeFunction::scalarizeInstruction(ShuffleVectorInst * SI)
{
  V_PRINT(scalarizer, "\t\tShuffleVector instruction\n");
  V_ASSERT(SI && "instruction type dynamic cast failed");

  // Proper scalarization makes "ShuffleVector" instructions redundant.
  // Only need to "follow" the scalar elements and update in SCM

  // Grab input vectors types and width
  Value *sourceVector0Value = SI->getOperand(0);
  Value *sourceVector1Value = SI->getOperand(1);
  const VectorType *inputType = dyn_cast<VectorType>(sourceVector0Value->getType());
  V_ASSERT (inputType && inputType == sourceVector1Value->getType() && "vector input error");
  unsigned sourceVectorWidth = inputType->getNumElements();

  // generate an array of values (pre-shuffle), which concatenates both vectors
  Value *allValues[MAX_INPUT_VECTOR_WIDTH * 2] = {NULL};
  bool dummy;

  // Obtain scalarized input values (into concatenated array). if vector was Undef - keep NULL.
  if (!isa<UndefValue>(sourceVector0Value))
  {
    obtainScalarizedValues(allValues, &dummy, sourceVector0Value, SI);
  }
  if (!isa<UndefValue>(sourceVector1Value))
  {
    // Place values, starting in the middle of concatenated array
    obtainScalarizedValues(&(allValues[sourceVectorWidth]), &dummy, sourceVector1Value, SI);
  }

  // Generate array for shuffled scalar values
  Value *newVector[MAX_INPUT_VECTOR_WIDTH];
  unsigned width = SI->getType()->getNumElements();
  V_ASSERT (MAX_INPUT_VECTOR_WIDTH >= width && "Vector size unsupported");

  // Generate undef value, which may be needed as some scalar elements
  UndefValue *undef = UndefValue::get(inputType->getElementType());

  // Go over shuffle order, and place scalar values in array
  for (unsigned i = 0; i < width; i++)
  {
    int maskValue = SI->getMaskValue(i);
    if (maskValue >= 0 && NULL != allValues[maskValue])
    {
      newVector[i] = allValues[maskValue];
    }
    else 
    {
      newVector[i] = undef;
    }
  }

  // Create the new SCM entry
  SCMEntry *newEntry = getSCMEntry(SI);
  updateSCMEntryWithValues(newEntry, newVector, SI, true);

  // Remove original instruction
  m_removedInsts.insert(SI);
}


void ScalarizeFunction::scalarizeInstruction(CallInst *CI)
{
  V_PRINT(scalarizer, "\t\tCall instruction\n");
  V_ASSERT(CI && "instruction type dynamic cast failed");

  // Find corresponding entry in functions hash (in runtimeServices) 
  std::string funcName = CI->getCalledFunction()->getName();
  const RuntimeServices::funcEntry foundFunction = m_rtServices->findBuiltinFunction(funcName);
  if (foundFunction.first && foundFunction.second == 1 && foundFunction.first->isPacketizable)
  {
    scalarizeInputReturnOfScalarCall(CI);
    return;
  }

  // Check if function was nominated for scalarizing in the pre-scalarization scanning phase
  if (!m_scalarizableRootsMap.count(CI))
  {
    // Function was not identified in pre-scanning. Leave as-is
    recoverNonScalarizableInst(CI);
    return;
  }

  // Getting here, the function is a "normal" kind of builtin function.
  unsigned vectorWidth = foundFunction.second;
  V_ASSERT(foundFunction.first && vectorWidth > 1 && "should still have found vector function");
  V_ASSERT(MAX_INPUT_VECTOR_WIDTH >= vectorWidth && "vector size not supported");

  // Find scalar function, using hash entry
  const char *scalarFuncName = foundFunction.first->funcs[0];
  const Function *LibScalarFunc = m_rtServices->findInRuntimeModule(scalarFuncName);
  V_ASSERT(NULL != LibScalarFunc && "function hash error");

  // Declare function in current module (if not declared already)
  Constant *scalarFunctionConstant = m_currFunc->getParent()->getOrInsertFunction(
    scalarFuncName, LibScalarFunc->getFunctionType());
  V_ASSERT(scalarFunctionConstant && "Failed finding or generating function");
  Function *scalarFunction = dyn_cast<Function>(scalarFunctionConstant);
  V_ASSERT(scalarFunction && "Function type mismatch, caused a constant expression cast!");

  // Argument vectors, to be filled with scalar values (for generating the scalar CALLs)
  std::vector<Value*> newArgs[MAX_INPUT_VECTOR_WIDTH];

  // Iterate over all function arguments, and grab their scalarized counterparts.
  // When grabbing the arguments from the actual CALL operands, the first argument is 
  // either the first operand, or the return value (passed
  // by reference, if the CALL inst returns a VOID).
  unsigned argStart = 0;
  if (CI->getType()->isVoidTy()) ++argStart;

  // Collect all the arguments that are NOT scalarized (meaning kept the same for vector
  // and scalar function call). After the scalarization of the CALL, make sure they
  // all still exist (as they may be vectors which were scalarized themselves), and regenerate
  // them if required
  SmallPtrSet<Value*, 2> nonScalarizedVectors;

  unsigned numArguments = scalarFunction->getFunctionType()->getNumParams();
  V_ASSERT (CI->getCalledFunction()->getFunctionType()->getNumParams() == numArguments 
    && "scalar func arguments number mismatches vector func");
  V_ASSERT (CI->getNumArgOperands() == numArguments + argStart && "CALL arguments number error");
  // Iterate over function operands (using the operand index of the CALL instruction)
  for (unsigned argIndex = argStart; argIndex < numArguments + argStart; ++argIndex)
  {
    // Placeholder for scalarized argument
    Value *operand[MAX_INPUT_VECTOR_WIDTH];
    bool dummy;

    // Check if arg has same type for scalar & vector functions (means it shouldn't be scalarized)
    bool isScalarized = (CI->getArgOperand(argIndex)->getType() != 
      scalarFunction->getFunctionType()->getParamType(argIndex - argStart));

    if (isScalarized)
    {
      // Get scalarized values from SCM
      obtainScalarizedValues(operand, &dummy, m_scalarizableRootsMap[CI][argIndex+1], CI);
      // Scatter the scalar values, to argument vectors of scalar function calls
      for (unsigned dup = 0; dup < vectorWidth; ++dup) {
        newArgs[dup].push_back(operand[dup]);
      }
    }
    else
    {
      // Place pointers to the same value in all the argument vectors
      for (unsigned dup = 0; dup < vectorWidth; ++dup)
        newArgs[dup].push_back(CI->getArgOperand(argIndex));

      // Save vector arguments (which werent scalarized)
      if (isa<VectorType>(CI->getArgOperand(argIndex)->getType()))
        nonScalarizedVectors.insert(CI->getArgOperand(argIndex));
    }
  }

  // Create new scalar instructions
  Value *newFuncCalls[MAX_INPUT_VECTOR_WIDTH];
  for (unsigned dup = 0; dup < vectorWidth; ++dup)
  {
    newFuncCalls[dup] = CallInst::Create(scalarFunction,
      newArgs[dup].begin(), newArgs[dup].end(), CI->getName(), CI);
  }

  // Make sure all vector arguments which weren't used as scalarized, still have their
  // original (vector) value intact - or regenerate it if needed
  for (SmallPtrSet<Value*, 2>::iterator iter = nonScalarizedVectors.begin();
    iter != nonScalarizedVectors.end(); ++iter)
  {
    obtainVectorValueWhichMightBeScalarized(*iter);
  }

  // If the function's return value was casted to a different type, we trace all the instructions
  // down to the value with the "proper type", and remove all the cast instructions on the way.
  if (m_scalarizableRootsMap[CI][0] != CI)
  {
    if (CI->getType() == Type::getVoidTy(context()))
    {
      // return value is passed thru a pointer (first argument).
      // No need to do anything here. Just make sure the "cast" is a load inst.
      V_ASSERT(isa<PointerType>(CI->getArgOperand(0)->getType()) && "retval-by-ref is not a pointer");
      V_ASSERT(isa<LoadInst>(m_scalarizableRootsMap[CI][0]) && "unsupported cast of pointer");
      V_ASSERT(cast<LoadInst>(m_scalarizableRootsMap[CI][0])->getPointerOperand() ==
        CI->getArgOperand(0) && "Loaded address doesnt match retval pointer!");
    }
    else 
    {
      // The CALL function has consumers, which are converters from the "Actual" return
      // value, to the "proper" one. These instructions are going to be scalarized, but
      // will eventually be removed thru DCE. For now, their input is replaced from the
      // return value to a dummy constant (as the CALL is going to be removed, so it
      // must have no users). This seems simpler than crawling down the def-use chain
      // and marking all the users for removal...
      Constant *dummyVal = Constant::getNullValue(CI->getType());
      CI->replaceAllUsesWith(dummyVal);
      getSCMEntry(dummyVal);
    }
    // Remove the actual "proper" return value (last Cast or Load inst)
    m_removedInsts.insert(dyn_cast<Instruction>(m_scalarizableRootsMap[CI][0]));
  }

  // Add new value/s to SCM. Convert from "proper" retVal to the new scalar values
  SCMEntry *newEntry = getSCMEntry(m_scalarizableRootsMap[CI][0]);

  // See CSSD100006905
  if (!m_scalarizableRootsMap[CI][0]->getType()->isVectorTy()) {
      return recoverNonScalarizableInst(CI);
  }

  updateSCMEntryWithValues(newEntry, newFuncCalls, m_scalarizableRootsMap[CI][0], true);

  // Remove original instruction
  m_removedInsts.insert(CI);
}


void ScalarizeFunction::scalarizeInputReturnOfScalarCall(CallInst* CI) {
  // obtaining sclarized and packetized funciton types
  std::string name = CI->getCalledFunction()->getNameStr();
  const RuntimeServices::funcEntry foundFunction =
    m_rtServices->findBuiltinFunction(name);
  V_ASSERT(foundFunction.first && "Unknown name");
  const FunctionType* ScalarFunctionType = 
      CI->getCalledFunction()->getFunctionType();
    
  // running over all params checking if parameter should be marked soa
  unsigned numScalarArgs = ScalarFunctionType->getNumParams();
  for (unsigned scalarArgInd=0; scalarArgInd < numScalarArgs; ++scalarArgInd){ 
    Value* curScalarParam = CI->getArgOperand(scalarArgInd);
    if (curScalarParam->getType()->isVectorTy())
    {
      Value *assembeledVector = obtainAssembledVector(curScalarParam, CI);
      CI->setArgOperand(scalarArgInd, assembeledVector);
    }
  }

  // handling case when return type is vector
  const Type* scalarizeRetType = ScalarFunctionType->getReturnType();
  if (scalarizeRetType->isVectorTy()){ 
    handleScalarRetVector(CI);
  }
}

void ScalarizeFunction::handleScalarRetVector(CallInst* callerInst) {
  V_ASSERT(isa<VectorType>(callerInst->getType()) && "unexpected prototype");

  // Clone original inst
  Instruction* clone = callerInst->clone();
  clone->insertBefore(callerInst);
  clone->setName(callerInst->getName() + "_clone");

  // Break result vector into scalars. 
  unsigned numElements = cast<VectorType>(callerInst->getType())->getNumElements();
  std::vector<Value*> newExtractInsts(numElements);
  Instruction* nextInst = ++BasicBlock::iterator(clone);
  for (unsigned i = 0; i < numElements; i++)
  {
    Value* constIndex = ConstantInt::get(Type::getInt32Ty(context()), i);
    newExtractInsts[i] = ExtractElementInst::Create(clone, constIndex, "scalar_vector_ret", nextInst);
  }

  // Update scalar elements in SCM
  SCMEntry* newSCMEntry = getSCMEntry(callerInst);
  updateSCMEntryWithValues(newSCMEntry, &(newExtractInsts[0]), callerInst, true);

  // Erase "original" instruction
  m_removedInsts.insert(callerInst);
}



void ScalarizeFunction::obtainScalarizedValues(Value *retValues[], bool *retIsConstant,
                                               Value *origValue, Instruction *origInst)
{
  V_PRINT(scalarizer, "\t\t\tObtaining scalar value... " << *origValue << "\n");
  const VectorType *origType = dyn_cast<VectorType>(origValue->getType());
  V_ASSERT(origType && "Must have a vector type!");
  unsigned width = origType->getNumElements();
  
  // Set retIsConstant (return value) to True, if the origValue is not an instruction
  if (isa<Instruction>(origValue)) {
    *retIsConstant = false;
  } else  {
    *retIsConstant = true;
  }

  // Lookup value in SCM
  SCMEntry *currEntry = getScalarizedValues(origValue);
  if (currEntry && (NULL != currEntry->scalarValues[0]))
  {
    // Value was found in SCM
    V_PRINT(scalarizer, 
        "\t\t\tFound existing entry in lookup of " << origValue->getName() << "\n");
    for (unsigned i = 0; i < width; i++)
    {
      // Copy values to return array
      V_ASSERT(NULL != currEntry->scalarValues[i] && "SCM entry contains NULL value");
      retValues[i] = currEntry->scalarValues[i];
    }
  }
  else if (isa<UndefValue>(origValue))
  {
    // value is an undefVal. Break it to element-sized undefs
    V_PRINT(scalarizer, "\t\t\tUndefVal constant\n");
    Value *undefElement = UndefValue::get(origType->getElementType());
    for (unsigned i = 0; i < width; i++)
    {
      retValues[i] = undefElement;
    }
  }
  else if (Constant *vectorConst = dyn_cast<Constant>(origValue))
  {
    V_PRINT(scalarizer, "\t\t\tProper constant: " <<  *vectorConst << "\n");
    // Value is a constant. Break it down to scalars by employing a constant expression
    for (unsigned i = 0; i < width; i++)
    {
      retValues[i] = ConstantExpr::getExtractElement(vectorConst,
        ConstantInt::get(Type::getInt32Ty(context()), i));
    }
  }
  else if (isa<Instruction>(origValue) && !currEntry)
  {
    // Instruction not found in SCM. Means it will be defined in a following basic block.
    // Generate a DRL: dummy values, which will be resolved after all scalarization is complete.
    V_PRINT(scalarizer, "\t\t\t*** Not found. Setting DRL. \n");
    const PointerType *dummyType = origType->getElementType()->getPointerTo();
    Constant *dummyPtr = ConstantPointerNull::get(dummyType);
    DRLEntry newDRLEntry;
    newDRLEntry.unresolvedInst = origValue;
    for (unsigned i = 0; i < width; i++)
    {
      // Generate dummy "load" instruction (but don't really place in function)
      retValues[i] = new LoadInst(dummyPtr);
      newDRLEntry.dummyVals[i] = retValues[i];
    }
    // Copy the data into DRL structure
    m_DRL.push_back(newDRLEntry);
  }
  else
  {
    V_PRINT(scalarizer, 
        "\t\t\tCreating scalar conversion for " << origValue->getName() << "\n");
    // Value is an Instruction/global/function argument, and was not converted to scalars yet.
    // Create scalar values (break down the vector) and place in SCM:
    //   %scalar0 = extractelement <4 x Type> %vector, i32 0
    //   %scalar1 = extractelement <4 x Type> %vector, i32 1
    //   %scalar2 = extractelement <4 x Type> %vector, i32 2
    //   %scalar3 = extractelement <4 x Type> %vector, i32 3
    // The breaking instructions will be placed the the head of the function, or right
    // after the instruction (if it is an instruction)
    Instruction *locationInst = &*(inst_begin(m_currFunc));
    Instruction *origInstruction = dyn_cast<Instruction>(origValue);
    if (origInstruction)
    {
      BasicBlock::iterator insertLocation(origInstruction);
      ++insertLocation;
      locationInst = insertLocation;
      // If the insert location is PHI, move the insert location to after all PHIs is the block
      if (isa<PHINode>(locationInst)) {
        locationInst = locationInst->getParent()->getFirstNonPHI();
      }
    }

    // Generate extractElement instructions
    for (unsigned i = 0; i < width; ++i)
    {
      Value *constIndex = ConstantInt::get(Type::getInt32Ty(context()), i);
      retValues[i] = ExtractElementInst::Create(origValue, constIndex, "scalar", locationInst);
    }
    SCMEntry *newEntry = getSCMEntry(origValue);
    updateSCMEntryWithValues(newEntry, retValues, origValue, false);
  }
}


void ScalarizeFunction::obtainVectorValueWhichMightBeScalarized(Value * vectorVal)
{
  m_usedVectors.insert(vectorVal);
}

void ScalarizeFunction::resolveVectorValues()
{
  SmallPtrSet<Value *, ESTIMATED_INST_NUM>::iterator it = m_usedVectors.begin();
  SmallPtrSet<Value *, ESTIMATED_INST_NUM>::iterator e = m_usedVectors.end();
  for (; it !=e ; ++it){
    obtainVectorValueWhichMightBeScalarizedImpl(*it);
  }
}  

void ScalarizeFunction::obtainVectorValueWhichMightBeScalarizedImpl(Value * vectorVal)
{
  V_ASSERT(isa<VectorType>(vectorVal->getType()) && "Must be a vector type");
  if (isa<UndefValue>(vectorVal)) return;

  // ONLY IF the value appears in the SCM - there is a chance it was removed.
  if (!m_SCM.count(vectorVal)) return;
  SCMEntry *valueEntry = m_SCM[vectorVal];

  // Check in SCM entry, if value was really removed
  if (false == valueEntry->isOriginalVectorRemoved) return;

  V_PRINT(scalarizer, "\t\t\tTrying to use a removed value. Reassembling it...\n");
  // The vector value was removed. Need to reassemble it...
  //   %temp.vect.0 = insertelement <4 x type> undef       , type %scalar.0, i32 0
  //   %temp.vect.1 = insertelement <4 x type> %indx.vect.0, type %scalar.1, i32 1
  //   %temp.vect.2 = insertelement <4 x type> %indx.vect.1, type %scalar.2, i32 2
  //   %temp.vect.3 = insertelement <4 x type> %indx.vect.2, type %scalar.3, i32 3
  // Place the re-assembly in the location where the original instruction was
  Instruction *insertLocation = dyn_cast<Instruction>(vectorVal);
  V_ASSERT(insertLocation && "SCM reports a non-instruction was removed. Should not happen");
  // If the original instruction was PHI, place the re-assembly only after all PHIs is the block
  if (isa<PHINode>(insertLocation)) {
    insertLocation = insertLocation->getParent()->getFirstNonPHI();
  }

  Value *assembledVector = UndefValue::get(vectorVal->getType());
  unsigned width = dyn_cast<VectorType>(vectorVal->getType())->getNumElements();
  for (unsigned i = 0; i < width; i++)
  {
    V_ASSERT(NULL != valueEntry->scalarValues[i] && "SCM entry has NULL value");
    Value *constIndex = ConstantInt::get(Type::getInt32Ty(context()), i);
    assembledVector = InsertElementInst::Create(assembledVector,
      valueEntry->scalarValues[i], constIndex, "temp.vect", insertLocation);
    V_PRINT(scalarizer, 
        "\t\t\tCreated vector assembly inst:" << *assembledVector << "\n");
  }
  // Replace the uses of "vectorVal" with the new vector
  vectorVal->replaceAllUsesWith(assembledVector);

  // create SCM entry to represent the new vector value..
  SCMEntry *newEntry = getSCMEntry(assembledVector);
  updateSCMEntryWithValues(newEntry, valueEntry->scalarValues, assembledVector, false);
}

Value *ScalarizeFunction::obtainAssembledVector(Value *vectorVal, Instruction *loc)
{
  // Assemble a vector from the scalarized values.
  const VectorType *vType = dyn_cast<VectorType>(vectorVal->getType());
  V_ASSERT(vType && "param must be a vector");
  Value *assembledVector = UndefValue::get(vType);
  unsigned width = vType->getNumElements();

  // Obtain scalarized values which make the vector input.
  bool dummy;
  Value* inputs[MAX_INPUT_VECTOR_WIDTH];
  obtainScalarizedValues(inputs, &dummy, vectorVal, loc);

  // For each of the scalar values, use insert-elements to create vector
  for (unsigned i = 0; i < width; i++) {
    // Get index
    Value *constIndex = ConstantInt::get(Type::getInt32Ty(context()), i);
    // Place element in vector
    assembledVector = InsertElementInst::Create(
      assembledVector, inputs[i], constIndex, "scalar_arg_vector", loc);
  }
  return assembledVector;
}


ScalarizeFunction::SCMEntry *ScalarizeFunction::getSCMEntry(Value *origValue)
{
  // origValue may be scalar or vector:
  // When the actual returned value of the CALL inst is different from the The "proper" retval 
  // the original CALL inst value may be scalar (i.e. int2 is converted to double which is a scalar)
  V_ASSERT(!isa<UndefValue>(origValue) && "Trying to create SCM to undef value...");
  if (m_SCM.count(origValue)) return m_SCM[origValue];

  // If index of next free SCMEntry overflows the array size, create a new array
  if (m_SCMArrayLocation == ESTIMATED_INST_NUM)
  {
    // Create new SCMAllocationArray, push it to the vector of arrays, and set free index to 0
    m_SCMAllocationArray = new SCMEntry[ESTIMATED_INST_NUM];
    m_SCMArrays.push_back(m_SCMAllocationArray);
    m_SCMArrayLocation = 0;
  }
  // Allocate the new entry, and increment the free-element index
  SCMEntry *newEntry = &(m_SCMAllocationArray[m_SCMArrayLocation++]);

  // Set all primary data in entry
  newEntry->scalarValues[0] = NULL;
  newEntry->isOriginalVectorRemoved = false;

  // Insert new entry to SCM map
  m_SCM.insert(std::pair<Value*, SCMEntry*>(origValue, newEntry));

  return newEntry;
}


void ScalarizeFunction::updateSCMEntryWithValues(ScalarizeFunction::SCMEntry *entry,
                                                 Value *scalarValues[],
                                                 Value *origValue,
                                                 bool isOrigValueRemoved)
{  
  V_ASSERT(isa<VectorType>(origValue->getType()) && "only Vector vals are supported");
  unsigned width = dyn_cast<VectorType>(origValue->getType())->getNumElements();

  entry->isOriginalVectorRemoved = isOrigValueRemoved;

  for (unsigned i = 0; i < width; ++i)
  {
    V_ASSERT(NULL != scalarValues[i] && "Trying to fill SCM with NULL value");
    entry->scalarValues[i] = scalarValues[i];
  }
}


ScalarizeFunction::SCMEntry *ScalarizeFunction::getScalarizedValues(Value *origValue)
{
  if (m_SCM.count(origValue)) return m_SCM[origValue];
  return NULL;
}

void ScalarizeFunction::releaseAllSCMEntries()
{
  V_ASSERT(m_SCMArrays.size() > 0 && "At least one buffer is allocated at all times");
  while (m_SCMArrays.size() > 1)
  {
    // If there are additional allocated entry Arrays, release all of them (leave only the first)
    SCMEntry *popEntry = m_SCMArrays.pop_back_val();
    delete[] popEntry;
  }
  // set the "current" array pointer to the only remaining array
  m_SCMAllocationArray = m_SCMArrays[0];
  m_SCMArrayLocation = 0;
}


void ScalarizeFunction::resolveDeferredInstructions ()
{
  for (unsigned index = 0; index < m_DRL.size(); ++index)
  {
    DRLEntry current = m_DRL[index];
    V_PRINT(scalarizer, 
        "\tDRL Going to fix value of orig inst: " << *current.unresolvedInst << "\n");
    Instruction *vectorInst = dyn_cast<Instruction>(current.unresolvedInst);
    V_ASSERT(vectorInst && "DRL only handles unresolved instructions");
    const VectorType *currType = dyn_cast<VectorType>(vectorInst->getType());
    V_ASSERT(currType && "Cannot have DRL of non-vector value");
    unsigned width = currType->getNumElements();

    SCMEntry *currentInstEntry = getSCMEntry(vectorInst);

    if (currentInstEntry->scalarValues[0] == NULL)
    {
      V_PRINT(scalarizer, "\t\tInst was not scalarized yet, Scalarizing now...\n");
      V_ASSERT(MAX_INPUT_VECTOR_WIDTH >= width && "vector width not supported");
      Value *newInsts[MAX_INPUT_VECTOR_WIDTH];

      // This instruction was not scalarized. Create scalar values and place in SCM.
      //   %scalar0 = extractelement <4 x Type> %vector, i32 0
      //   %scalar1 = extractelement <4 x Type> %vector, i32 1 
      //   %scalar2 = extractelement <4 x Type> %vector, i32 2
      //   %scalar3 = extractelement <4 x Type> %vector, i32 3 
      // Place the vector break-down instructions right after the actual vector
      BasicBlock::iterator insertLocation(vectorInst);
      ++insertLocation;
      // If the insert location is PHI, move the insert location to after all PHIs is the block
      if (isa<PHINode>(insertLocation)) {
        insertLocation = insertLocation->getParent()->getFirstNonPHI();
      }

      for (unsigned i = 0; i < width; i++)
      {
        Value *constIndex = ConstantInt::get(Type::getInt32Ty(context()), i);      
        newInsts[i] = ExtractElementInst::Create(vectorInst, constIndex, "scalar", insertLocation);
      }
      updateSCMEntryWithValues(currentInstEntry, newInsts, vectorInst, false);
    }

    // Connect the resolved values to their consumers
    for (unsigned i = 0; i < width; ++i)
    {
      Instruction *dummyInst = dyn_cast<Instruction>(current.dummyVals[i]);
      V_ASSERT(dummyInst && "Dummy values are all instructions!");
      dummyInst->replaceAllUsesWith(currentInstEntry->scalarValues[i]);
      // Erase dummy instruction (don't use eraseFromParent as the dummy is not in the function)
      delete dummyInst;
    }
  }
  // clear DRL
  m_DRL.clear();
}


} // Namespace


/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createScalarizerPass() {
    return new intel::ScalarizeFunction();
  }
}

/// Support for dynamic loading of modules under Linux
char intel::ScalarizeFunction::ID = 0;
static RegisterPass<intel::ScalarizeFunction> SCALARIZER("scalarize", "Scalarize functions");

