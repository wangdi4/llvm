/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "VectorizerUtils.h"
#include "TypeConversion.h"
#include "FunctionDescriptor.h"
#include "NameMangleAPI.h"
#include "VectorVariant.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"

namespace intel{
using namespace llvm;

void VectorizerUtils::getFunctionsToVectorize(llvm::Module &M,
					      FunctionVariants& funcVars) {
  for (auto it = M.begin(), end = M.end(); it != end; it++) {
    Function& F = *it;
    funcVars[&F] = DeclaredVariants();
    DeclaredVariants& declaredVariants = funcVars[&F];
    AttributeSet attributes = F.getAttributes().getFnAttributes();
    AttributeSet::iterator itA = attributes.begin(0);
    AttributeSet::iterator endA = attributes.end(0);
    for (;itA != endA; ++itA) {
      if (!itA->isStringAttribute())
	continue;
      StringRef attributeKind = itA->getKindAsString();
      if (!VectorVariant::isVectorVariant(attributeKind))
	continue;
      declaredVariants.push_back(attributeKind);
    }
    if (declaredVariants.empty())
      funcVars.erase(&F);
  }
}

void VectorizerUtils::SetDebugLocBy(Instruction *I, const Instruction *setBy) {
  if (!setBy->getDebugLoc().isUnknown()) {
    I->setDebugLoc(setBy->getDebugLoc());
  }
}

void VectorizerUtils::SetDebugLocBy(std::vector<Instruction *> &insts,
                                    Instruction *setBy) {
  if (!setBy->getDebugLoc().isUnknown()) {
    const DebugLoc &dbgloc = setBy->getDebugLoc();
    for (unsigned i=0; i<insts.size(); ++i) {
      insts[i]->setDebugLoc(dbgloc);
    }
  }
}

Value *VectorizerUtils::isExtendedByShuffle(ShuffleVectorInst *SI, Type *realType)
{
  assert(SI && "Expected ShuffleVector instruction as input");

  // The "proper" input is supposed to be in the first vector input,
  // and the first WIDTH shuffle values (locations) are the ordered components of that input.
  // No assumptions are made on the second input, or the trailing shuffled elements.
  // That is legal, as the consumer of the shuffleInst is known to be using only the
  // lower WIDTH elements of the vector.
  // WIDTH is calculated as number of vector Elements of realType
  VectorType *desiredVectorType = dyn_cast<VectorType>(realType);
  if (!desiredVectorType) return NULL;
  unsigned realWidth = desiredVectorType->getNumElements();

  // realWidth must be smaller-or-equal to the width of the shuffleInst result
  if (realWidth > SI->getType()->getNumElements()) return NULL;

  // Check that the shuffle components correspond to the input vector
  for (unsigned i = 0; i < realWidth; ++i)
  {
    unsigned maskValue = SI->getMaskValue(i);
    if (maskValue != i) return NULL;
  }
  return SI->getOperand(0);
}


bool VectorizerUtils::isOpaquePtrPair(Type *x, Type *y)
{
  PointerType *xPtr = dyn_cast<PointerType>(x);
  PointerType *yPtr = dyn_cast<PointerType>(y);
  if (xPtr && yPtr ) {
    StructType *xStructEl = dyn_cast<StructType>(xPtr->getElementType());
    StructType *yStructEl = dyn_cast<StructType>(yPtr->getElementType());
    if (xStructEl && yStructEl){
      return (// in apple the samplers have slightly differnet function names between
              // rt module and kernels IR so I skip checking that name is the same.
              //xStructEl->getName() == yStructEl->getName() && // have the same name
              xStructEl->isEmptyTy() && //x is empty
              yStructEl->isEmptyTy());  //y is empty
    }
  }
  return false;
}

Value *VectorizerUtils::RootInputArgumentBySignature(Value *arg, unsigned int paramNum, CallInst *CI) {
  assert(paramNum <= CI->getNumOperands() && "Requested type of parameter that does not exist");
  if (paramNum > CI->getNumOperands())
    return NULL;

  // Get the (reflection) type from the mangled name
  StringRef mangledName = CI->getCalledFunction()->getName();
  reflection::FunctionDescriptor fdesc = ::demangle(mangledName.str().c_str());
  return RootInputArgument(arg, reflectionToLLVM(CI->getContext(), fdesc.parameters[paramNum]), CI);
}


Value *VectorizerUtils::RootInputArgument(Value *arg, Type *rootType, CallInst *CI) {
  LLVMContext &context = CI->getContext();
  // Is the argument already in the correct type?
  Type *argType = arg->getType();
  if (argType == rootType) return arg;

  if (isOpaquePtrPair(argType, rootType)) {
    //incase of pointer to opaque type bitcast
    return new BitCastInst(arg, rootType, "bitcast.opaque.ptr", CI);
  }

  if (isa<PointerType>(argType)) {
    // If the function argument is in Pointer type, we expect to find the origin of the pointer
    // as an alloca instruction with 2 users: a store (of the original value) and the CALL inst.
    // Any other formation will fail the rooting effort.
    // This has one exception (a hack). If the desired type is a vector of width 3,
    // Apple's clang jumps through all sorts of hoops, and creates a shuffle-bitcast-store pattern.
    AllocaInst *allocator = dyn_cast<AllocaInst>(arg);
    if (!allocator || allocator->getAllocatedType() != rootType ||
      allocator->isArrayAllocation() || !allocator->hasNUses(2)) {
        return NULL;
    }

    const bool is3Vector = (rootType->isVectorTy() &&
      cast<VectorType>(rootType)->getNumElements() == 3);

    // Check the 2 users are really a store and the function call.
    Value *retVal = NULL;
    for (Value::user_iterator i = allocator->user_begin(), e = allocator->user_end(); i != e; ++i) {
      // Check for store instruction
      if (StoreInst *storeInst = dyn_cast<StoreInst>(*i)) {
        // Only a single store is expected...
        if (retVal) return NULL;
        // Keep the value which is being stored.
        retVal = storeInst->getOperand(0);
        // the stored value should be of the expected type
        if (retVal->getType() != rootType) return NULL;
      }
      // Support the bitcast-shuffle-store pattern (for width-3 vectors)
      else if (is3Vector && isa<BitCastInst>(*i)) {
        // Only a single store is expected...
        if (retVal) return NULL;

        BitCastInst* bitCastInst = cast<BitCastInst>(*i);
        // The bitcast must have one user, which is a store
        if (!bitCastInst->hasOneUse()) return NULL;
        StoreInst *storeInst = dyn_cast<StoreInst>(*(bitCastInst->user_begin()));
        if (!storeInst) return NULL;

        // The store value must be the result of a shuffle
        ShuffleVectorInst* shuffle = dyn_cast<ShuffleVectorInst>(storeInst->getOperand(0));
        if (!shuffle) return NULL;

        // Check that shuffle is extending operand of desired (root) type
        retVal = isExtendedByShuffle(shuffle, rootType);
        if (!retVal || retVal->getType() != rootType) return NULL;
      }
      // Else check for the call instruction
      else if (CallInst *callInst = dyn_cast<CallInst>(*i)) {
        // check that the call inst is the one we started with
        if (CI != callInst) return NULL;
      }
      else {
        // Unexpected consumer of Alloca.
        return NULL;
      }
    }
    assert(retVal && "retVal must have been set by now");
    return retVal;
  }

  // arg was passed as a value (not pointer) but of incorrect type. Climb up over instruction's
  // use-def chain, until the value's root is found, or until reaching a non-instruction
  Value *currVal = arg;
  Instruction *inst;
  SmallVector<Value *, 4> valInChain;
  while (currVal->getType() != rootType && (inst = dyn_cast<Instruction>(currVal)))
  {
    valInChain.push_back(currVal);
    // Check for the "simple" BitCast and ZExt/SExt cases
    if ((inst = dyn_cast<BitCastInst>(currVal)) ||
      (inst = dyn_cast<ZExtInst>(currVal)) ||
      (inst = dyn_cast<SExtInst>(currVal)))
    {
      // Climb up to the input of the cast
      currVal = inst->getOperand(0);
    }
    // Check for ExtractElement
    else if (ExtractElementInst *EE = dyn_cast<ExtractElementInst>(currVal))
    {
      // ExtractElement is allowed in a single case: ExtractElement <1 x Type>, 0
      currVal = EE->getVectorOperand();
      if (EE->getVectorOperandType()->getNumElements() != 1)
        return canRootInputByShuffle(valInChain, rootType, CI);
    }
    // Check for the more-complicated ShuffleVector cast
    else if (ShuffleVectorInst *SV = dyn_cast<ShuffleVectorInst>(currVal))
    {
      currVal = isExtendedByShuffle(SV, rootType);
      if (!currVal)
        return canRootInputByShuffle(valInChain, rootType, CI);
    }
    else if (InsertElementInst *IE = dyn_cast<InsertElementInst>(currVal))
    {
      currVal = isInsertEltExtend(IE, rootType);
      if (!currVal)
        return canRootInputByShuffle(valInChain, rootType, CI);
    }
    else
    {
        return canRootInputByShuffle(valInChain, rootType, CI);
    }
  }

  // Check if "desired" type was reached
  if (currVal->getType() == rootType) return currVal;

  // currVal is not an instruction, so its a constant, or global, or kernel
  // argument. So simply cast it to the desired type
  unsigned sourceSize = currVal->getType()->getPrimitiveSizeInBits();
  unsigned targetSize = rootType->getPrimitiveSizeInBits();

  if (Constant *constVal = dyn_cast<Constant>(currVal))
  {
    // Check if both types are of the same size
    if (sourceSize != targetSize)
    {
      // The type sizes mismatch. BitCast to int and resize
      constVal = ConstantExpr::getBitCast(constVal, IntegerType::get(context, sourceSize));
      constVal = ConstantExpr::getIntegerCast(constVal,
        IntegerType::get(context, targetSize), false);
    }
    // Now the sizes match. Bitcast to the desired type
    currVal = ConstantExpr::getBitCast(constVal, rootType);
  }
  else
  {
    // Value may be an input argument, or global of some sort.
    // Cast it at the head of the function to the required type
    Function *currFunc = CI->getParent()->getParent();
    currVal = BitCastValToType(currVal, rootType, &*inst_begin(currFunc));
  }
  return currVal;
}


bool VectorizerUtils::isShuffleVectorTruncate(ShuffleVectorInst *SI)
{
  if (!SI) return false;
  // The "proper" input is supposed to be in the first vector input,
  // and the shuffle values (locations) are the ordered components of that input.
  VectorType *inputType = dyn_cast<VectorType>(SI->getOperand(0)->getType());
  assert(inputType && "ShuffleVector is expected to have vector inputs!");
  unsigned inputWidth = inputType->getNumElements();
  unsigned resultWidth = SI->getType()->getNumElements();
  if (resultWidth > inputWidth) return false;

  for (unsigned i = 0; i < resultWidth; i++)
  {
    unsigned maskValue = SI->getMaskValue(i);
    if (maskValue != i) return false;
  }
  return true;
}


Value *VectorizerUtils::RootReturnValue(Value *retVal, Type *rootType, CallInst *CI)
{
  LLVMContext &context = CI->getContext();
  // Check maybe the return value is of the right type - no need for rooting
  if (retVal->getType() == rootType) return retVal;

  if (isa<PointerType>(retVal->getType()))
  {
    // If the retval is in Pointer type (return by reference), we expect to find the origin of
    // the pointer as an alloca with 2 users: the CALL inst, and a following LOAD of the data.
    // Any other formation will fail the rooting effort
    AllocaInst *allocator = dyn_cast<AllocaInst>(retVal);
    if (!allocator || allocator->isArrayAllocation() ||
      allocator->getAllocatedType() != rootType || !allocator->hasNUses(2))
    {
      return NULL;
    }

    // Check the 2 users are really a load and the function call.
    Value *rootRetVal = NULL;
    for (Value::user_iterator i = allocator->user_begin(), e = allocator->user_end(); i != e; ++i)
    {
      if (LoadInst *loadInst = dyn_cast<LoadInst>(*i))
      {
        rootRetVal = loadInst;
        // Check if the loaded value has the expected type
        if (rootRetVal->getType() != rootType) return NULL;
      }
      else if (CallInst *callInst = dyn_cast<CallInst>(*i))
      {
        // Check that we didnt reach a different call instruction
        if (callInst != CI) return NULL;
      }
      else
      {
        // Any other instruction is unsupported
        return NULL;
      }
    }
    assert(rootRetVal && "Must have rooted the retVal by now");
    return rootRetVal;
  }

  // retval was passed as a value (not pointer) but of incorrect type.
  assert (dyn_cast<Instruction>(retVal) == CI && "retVal should be the return of the CALL");

  if (CI->user_begin() == CI->user_end())
  {
    return retVal;
  }
  // Collect all the users of the retval (thru def-use crawling). Collect only values that
  // have users other than conversion instructions (bitcasts, truncate, etc)
  SmallPtrSet<Instruction*, 8> instructionsToCrawl, retvalUsers;
  // start crawling by inspecting the users of the CALL instruction
  instructionsToCrawl.insert(CI);
  while (!instructionsToCrawl.empty())
  {
    // Extract next value to inspect
    Instruction *instToTest = *(instructionsToCrawl.begin());
    instructionsToCrawl.erase(instToTest);

    // Scan all descendants, looking for retval users
    Value::user_iterator ui, ue;
    for (ui = instToTest->user_begin(), ue = instToTest->user_end(); ui != ue; ++ui)
    {
      Instruction *userInst = dyn_cast<Instruction>(*ui);
      assert(NULL != userInst && "Instruction's user is not an instruction. Unexpected");
      if (isa<BitCastInst>(userInst) || isa<TruncInst>(userInst) ||
        isShuffleVectorTruncate(dyn_cast<ShuffleVectorInst>(userInst)))
      {
        // User is another conversion instruction. Add it to the crawling list
        instructionsToCrawl.insert(userInst);
      }
      else
      {
        // User is a "proper" user of retval. Add inspected instruction to the users list
        retvalUsers.insert(instToTest);
      }
    }
  }
  assert(!retvalUsers.empty() && "retval expected to have at least one user");
  unsigned srcSize = CI->getType()->getPrimitiveSizeInBits();
  unsigned dstSize = rootType->getPrimitiveSizeInBits();
  // Fail if retval is not a primitive type which has a measurable size
  if (0 == srcSize || 0 == dstSize) return NULL;
  // Fail if the real retval is smaller than the desired size
  if (srcSize < dstSize) return NULL;

  // If the CALL instruction is in the retvalUsers list, create a dummy inst and replace all
  // users of the inst with the dummy val. This is needed now, so the new conversion from the
  // CALL value will be the only user of the CALL.
  Instruction *dummyInstruction = NULL;
  if (retvalUsers.count(CI))
  {
    Type *ptrType = PointerType::get(CI->getType(), 0);
    Constant *subExpr = ConstantExpr::getIntToPtr(
      ConstantInt::get(Type::getInt32Ty(context), APInt(32, 0xdeadbeef)), ptrType);
    dummyInstruction = new LoadInst(subExpr);
    dummyInstruction->insertAfter(CI);
    CI->replaceAllUsesWith(dummyInstruction);
    retvalUsers.erase(CI);
    retvalUsers.insert(dummyInstruction);
  }

  // Generate a conversion from the retval to its "proper" type, and place after the CALL inst
  // The conversion may have up-to 3 stages: bitcast to int, truncate, bitcast to required type
  Instruction *convertedVal = CI;
  if (!isa<IntegerType>(CI->getType()))
  {
    // Cast retval to an integer
    Instruction *castToInt = new BitCastInst(convertedVal, IntegerType::get(context, srcSize));
    castToInt->insertAfter(convertedVal);
    convertedVal = castToInt;
  }
  if (srcSize > dstSize)
  {
    // Shrink retval to the desired size
    Instruction *shrink = new TruncInst(convertedVal, IntegerType::get(context, dstSize));
    shrink->insertAfter(convertedVal);
    convertedVal = shrink;
  }
  if (convertedVal->getType() != rootType)
  {
    assert(convertedVal->getType()->getPrimitiveSizeInBits() == dstSize && "cast size error");
    // Bitcast to desired type
    Instruction *castToDesired = new BitCastInst(convertedVal, rootType);
    castToDesired->insertAfter(convertedVal);
    convertedVal = castToDesired;
  }
  assert(convertedVal->getType() == rootType && "Cast retval failed");

  // Go over all the retval users (from retvalUsers list) can connect the to the convertedVal.
  // In case of type mismatch, cast the convertedVal to the desired type first
  for (SmallPtrSet<Instruction*, 8>::iterator ui = retvalUsers.begin(), ue = retvalUsers.end();
    ui != ue; ++ui)
  {
    Instruction *inst = *ui;
    if (inst->getType() == rootType)
    {
      inst->replaceAllUsesWith(convertedVal);
    }
    else
    {
      inst->replaceAllUsesWith(BitCastValToType(convertedVal, inst->getType(), inst));
    }
  }
  // Erase the dummy inst if existed
  if (dummyInstruction)
  {
    assert(dummyInstruction->use_empty() && "Did not disconnect all dummy users!");
    dummyInstruction->eraseFromParent();
  }
  return convertedVal;
}


Instruction *VectorizerUtils::BitCastValToType(Value *orig, Type *targetType,
                                                 Instruction *insertPoint)
{
  LLVMContext& context = insertPoint->getContext();
  Type *currType = orig->getType();
  assert (currType != targetType && "should get here in case of same type" );
  unsigned currSize = currType->getPrimitiveSizeInBits();
  unsigned rootSize = targetType->getPrimitiveSizeInBits();
  Instruction *retVal;

  if (currSize == rootSize)
  {
    // just bitcast from one to the other
    retVal = new BitCastInst(orig, targetType, "cast_val", insertPoint);
  }
  else if (Instruction *shufConvert = convertUsingShuffle(orig, targetType, insertPoint))
  {
    return shufConvert;
  }
  else
  {
    Value *origInt = orig;
    // if orig is not integer bitcast it into integer
    if (!orig->getType()->isIntegerTy())
      origInt = new BitCastInst(orig, IntegerType::get(context, currSize), "cast1", insertPoint);

    // zext / trunc to the targetType size
    if (currSize < rootSize) // Zero-extend
      retVal = new ZExtInst(origInt, IntegerType::get(context, rootSize), "zext_cast", insertPoint);
    else
      retVal = new TruncInst(origInt, IntegerType::get(context, rootSize), "trunc1", insertPoint);

    // if target is not integer bitcast to target type
    if (!targetType->isIntegerTy())
        retVal = new BitCastInst(retVal, targetType, "cast_val", insertPoint);
  }
  return retVal;
}

Instruction *VectorizerUtils::ExtendValToType(Value *orig, Type *targetType,
                                              Instruction *insertPoint)
{
  assert(orig->getType()->getPrimitiveSizeInBits() <= targetType->getPrimitiveSizeInBits() &&
      "expanding when souce is bigger than target");
  return BitCastValToType(orig, targetType, insertPoint);
}

Instruction *VectorizerUtils::TruncValToType(Value *orig, Type *targetType, Instruction *insertPoint)
{
  assert(orig->getType()->getPrimitiveSizeInBits() >= targetType->getPrimitiveSizeInBits() &&
      "trunc when souce is  smaller than target");
  return BitCastValToType(orig, targetType, insertPoint);
}

Instruction *VectorizerUtils::convertValToPointer(Value *orig, Type *targetType, Instruction *insertPoint)
{
  PointerType *targetPointerType = dyn_cast<PointerType>(targetType);
  assert(targetPointerType && "getting here target type must be a pointer");
  if (!targetPointerType) return NULL;
  Type *sourceType = orig->getType();
  assert(targetPointerType->getElementType() == sourceType && "pointer must be of orig type");
  AllocaInst *ptr = new AllocaInst(sourceType, "allocated_val" , insertPoint);
  new StoreInst(orig, ptr, insertPoint);
  return ptr;
}


Value *VectorizerUtils::getCastedArgIfNeeded(Value *inputVal, Type *targetType, Instruction *insertPoint)
{
  Type *sourceType = inputVal->getType();

  // incase of same type do noting
  if (sourceType == targetType) return inputVal;

  if (isOpaquePtrPair(sourceType,targetType))
  {
    return new BitCastInst(inputVal, targetType, "bitcast.opaque.ptr", insertPoint);
  }

  // no support for case when not the same type ans source is a pointer
  if (sourceType->isPointerTy())
  {
      assert(0 && "no support for case when not the same type ans source is a pointer");
      return NULL;
  }

  // if targetType is a pointer and not the same type we assume the pointer type match the value
  // so we allcoate a pointer and store the original input value
  if (targetType->isPointerTy()) return convertValToPointer(inputVal, targetType, insertPoint);

  // convert the orig into target type by bitcasting and Zext if needed
  return ExtendValToType(inputVal, targetType, insertPoint);
}


Instruction *VectorizerUtils::getCastedRetIfNeeded(Instruction *I, Type *targetType)
{
  // incase of same type do noting
  Type *sourceType = I->getType();
  if (sourceType == targetType) return I;

  BasicBlock::iterator itr = (BasicBlock::iterator)(I);
  Instruction *insertPoint = &*(++itr);
  return TruncValToType(I, targetType, insertPoint);
}

CallInst *VectorizerUtils::createFunctionCall(Module *pModule, const std::string &name,
  Type *retType, const SmallVectorImpl<Value *> &args, const SmallVectorImpl<Attribute::AttrKind>& attrs, Instruction* insertBefore) {
  SmallVector<Type *, 8> types;

  for(unsigned int i=0; i<args.size(); ++i) {
    types.push_back(args[i]->getType());
  }

  FunctionType *intr = FunctionType::get(retType, types, false);
  Constant* new_f = pModule->getOrInsertFunction(name.c_str(), intr);
  assert(isa<Function>(new_f) && "mismatch function type");
  Function *F = cast<Function>(new_f);
  for (unsigned i=0; i < attrs.size(); ++i)
    F->addAttribute(~0, attrs[i]);
  CallInst *newCall = CallInst::Create(new_f, ArrayRef<Value*>(args), "", insertBefore);
  for (unsigned i=0; i < attrs.size(); ++i)
    newCall->addAttribute(~0, attrs[i]);

  // Set debug location
  SetDebugLocBy(newCall, insertBefore);

  return newCall;
}

Instruction *VectorizerUtils::createBroadcast(Value *pVal, unsigned int width, Instruction* whereTo, bool insertAfter) {
  Instruction *insertBefore = insertAfter? NULL: whereTo;
  Constant *index = ConstantInt::get(Type::getInt32Ty(pVal->getContext()), 0);
  Constant *zeroVector = ConstantVector::get(std::vector<Constant*>(width, index));
  UndefValue *undefVec = UndefValue::get(VectorType::get(pVal->getType(), width));
  Instruction *tmpInst = InsertElementInst::Create(undefVec, pVal, index, "temp", insertBefore);
  Instruction *shuffle = new ShuffleVectorInst(tmpInst, undefVec, zeroVector , "vector", insertBefore);

  if(insertAfter) {
    // Insert instruction in reverse order
    shuffle->insertAfter(whereTo);
    tmpInst->insertAfter(whereTo);
  }

  if(Instruction *pInst = dyn_cast<Instruction>(pVal)) {
    // Set debug location
    SetDebugLocBy(tmpInst, pInst);
    SetDebugLocBy(shuffle, pInst);
  }

  return shuffle;
}

Instruction *VectorizerUtils::createConsecutiveVector(Value *pVal, unsigned int width, Instruction* whereTo, bool insertAfter) {
  Instruction *broadcastedValue = createBroadcast(pVal, width, whereTo, insertAfter);
  std::vector<Constant*> consecutiveConstants;
  for (unsigned int i = 0; i < width; i++)
    consecutiveConstants.push_back(ConstantInt::get(Type::getInt32Ty(pVal->getContext()), i));
  Constant *incrementalVector = ConstantVector::get(consecutiveConstants);
  Instruction *nullInstruction = NULL;
  BinaryOperator *consecutiveVector =
    BinaryOperator::CreateAdd(broadcastedValue, incrementalVector, "consecutiveValue", nullInstruction);

  // Insert instructions in reverse order after the broadcast value
  consecutiveVector->insertAfter(broadcastedValue);

  if(Instruction *pInst = dyn_cast<Instruction>(pVal)) {
    // Set debug location
    SetDebugLocBy(consecutiveVector, pInst);
  }

  return consecutiveVector;
}

unsigned int VectorizerUtils::getBSR(uint64_t number) {
  unsigned int res = 0;
  for(int i=63; i>=0; --i) {
    if(number & (((uint64_t)0x1)<<i)) {
      res = i;
      break;
    }
  }
  return res;
}

unsigned int VectorizerUtils::getLOG(uint64_t number) {
  assert(number && 0 == (number & (number-1)) && "LOG is valid for power of 2 numbers!");
  return getBSR(number);
}

Type* VectorizerUtils::convertSoaAllocaType(Type *type, unsigned int width) {
  assert(type && "NULL type");
  bool isPointer = type->isPointerTy();
  if (isPointer) {
    type = cast<PointerType>(type)->getElementType();
  }

  std::vector<unsigned int> arraySizes;
  while (ArrayType *arrayType = dyn_cast<ArrayType>(type)) {
    arraySizes.push_back((unsigned int)arrayType->getNumElements());
    type = arrayType->getElementType();
  }

  if (width == 0) {
    // Need to scalarize type (assuming original is vector type)
    VectorType* vType = dyn_cast<VectorType>(type);
    assert(vType && "Base type is not a vector!");
    // Get scalar type of the base original vector type
    type = vType->getElementType();
  } else {
    // Need to vectorize type (assuming original is not vector type)
    assert(!type->isVectorTy() && "Base type is not a scalar!");
    // Create vector type of the alloca base original type
    type = VectorType::get(type, width);
  }

  // Re-create the array types of the original alloca upon the new type.
  for (unsigned int i=(unsigned int)arraySizes.size(); i>0; --i) {
    type = ArrayType::get(type, arraySizes[i-1]);
  }

  if (isPointer) {
    type = type->getPointerTo();
  }
  return type;
}

// rooting a sequence like this:
// %v0 = insertelement <4 x type> undef, type %scalar.0, i32 0
// %v1 = insertelement <4 x type> %v0,   type %scalar.1, i32 1
// into
// %u0 = insertelement <2 x type> undef, type %scalar.0, i32 0
// %u1 = insertelement <2 x type> %v0,   type %scalar.1, i32 1
Value *VectorizerUtils::isInsertEltExtend(Instruction *I, Type *realType) {
  // If I is an extension of vector by insert element then both I and the real
  // type are vectors with the same element type.
  const VectorType *origTy = dyn_cast<VectorType>(I->getType());
  const VectorType *destTy = dyn_cast<VectorType>(realType);
  if (!destTy || !origTy) return NULL;
  const Type *origElTy = origTy->getElementType();
  unsigned origNelts = origTy->getNumElements();
  const Type *destElTy = destTy->getElementType();
  unsigned destNelts = destTy->getNumElements();
  if (origElTy != destElTy || origNelts <= destNelts) return NULL;

  // If I is an extension of vector by insert element than the vector should
  // be created by sequence of insert element instructions to the head of the
  // vector.
  SmallVector<Value *, 16> insertedVals;
  insertedVals.assign(destNelts, NULL);
  Value * val = I;
  while (!isa<UndefValue>(val)) {
    // val is insert element.
    InsertElementInst * IEI = dyn_cast<InsertElementInst>(val);
    if (!IEI) return NULL;

    // Index of insertion is constant < destination type number of elements.
    Value* index = IEI->getOperand(2);
    ConstantInt* C = dyn_cast<ConstantInt>(index);
    if (!C) return NULL;
    unsigned int idx = (unsigned int)C->getZExtValue();
    if (idx >= destNelts) return NULL;

    // Consider only the last insertion to idx.
    if (!insertedVals[idx]) {
      insertedVals[idx] = IEI->getOperand(1);
    }

    // Continue to the next iteration with the vector operand.
    val = IEI->getOperand(0);
  }

  // Reconstruct the vector right after the original insert element.
  assert(I != I->getParent()->getTerminator() &&
      "insert element can not be a terminator of basic block");
  Instruction *loc = (++BasicBlock::iterator(I));
  Value *gatherdVals = UndefValue::get(realType);
  LLVMContext &context = val->getContext();
  for (unsigned i=0; i<destNelts; ++i) {
    Value *val = insertedVals[i];
    if (!val) continue;
    ConstantInt *index = ConstantInt::get(context, APInt(32, i));
    gatherdVals = InsertElementInst::Create(gatherdVals, val, index, "", loc);
  }
  return gatherdVals;
}

Instruction *VectorizerUtils::convertUsingShuffle(Value *v,
                                                  const Type *realType,
                                                  Instruction *loc) {
  // In order to convert using shuffle both v and realType need to be vectors
  // with the same element type.
  const VectorType *destTy = dyn_cast<VectorType>(realType);
  VectorType *vTy = dyn_cast<VectorType>(v->getType());
  if (!destTy || !vTy) return NULL;
  const Type *destElTy = destTy->getElementType();
  const Type *vElTy = vTy->getElementType();
  if (vElTy != destElTy) return NULL;

  // Generate the shuffle vector mask.
  unsigned destNelts = destTy->getNumElements();
  unsigned vNelts = vTy->getNumElements();
  std::vector<Constant*> constants;
  unsigned minWidth = destNelts > vNelts ? vNelts : destNelts;
  LLVMContext &context = v->getContext();
  for (unsigned j=0; j < minWidth; ++j) {
    constants.push_back(ConstantInt::get(context, APInt(32, j)));
  }
  for (unsigned j=minWidth; j<destNelts; ++j) {
    constants.push_back(UndefValue::get(IntegerType::get(context, 32)));
  }
  Constant *mask = ConstantVector::get(constants);

  // Return shuffle instruction.
  UndefValue *undefVect = UndefValue::get(vTy);
  return new ShuffleVectorInst(v, undefVect, mask, "", loc);
}

Value *VectorizerUtils::canRootInputByShuffle(SmallVector<Value *, 4> &valInChain,
                                              const Type * realType,
                                              Instruction *loc) {
  // Run over the chain in reverse order so we try earlier values first.
  unsigned destSize = realType->getPrimitiveSizeInBits();
  for (unsigned int i = 0, e = (unsigned int)valInChain.size(); i < e; ++i) {
    Value *curVal = valInChain[i];
    // Argumetns can be converted only if the root is smaller than realType.
    unsigned  curSize = curVal->getType()->getPrimitiveSizeInBits();
    assert(curSize >= destSize && "root is bigger than the value");
    if (curSize < destSize) continue;

    // Try rooting using shuffle.
    if (Instruction *shuffle = convertUsingShuffle(curVal, realType, loc)) {
      return shuffle;
    }
  }
  return NULL;
}

} // nampespace intel
