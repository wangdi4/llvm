#include "VectorizerUtils.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/InstIterator.h"
#include "Logger.h"


namespace intel{



Value *VectorizerUtils::isExtendedByShuffle(ShuffleVectorInst *SI, const Type *realType)
{
  V_ASSERT(SI && "Expected ShuffleVector instruction as input");

  // The "proper" input is supposed to be in the first vector input,
  // and the first WIDTH shuffle values (locations) are the ordered components of that input.
  // No assumptions are made on the second input, or the trailing shuffled elements.
  // That is legal, as the consumer of the shuffleInst is known to be using only the
  // lower WIDTH elements of the vector.
  // WIDTH is calculated as number of vector Elements of realType
  const VectorType *desiredVectorType = dyn_cast<VectorType>(realType);
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


bool VectorizerUtils::isOpaquePtrPair(const Type *x, const Type *y)
{
  return x->isPointerTy() &&
         y->isPointerTy() &&
         (cast<PointerType>(x))->getElementType()->isOpaqueTy() &&
         (cast<PointerType>(y))->getElementType()->isOpaqueTy() ;
}

Value *VectorizerUtils::RootInputArgument(Value *arg, const Type *rootType, CallInst *CI)
{
  LLVMContext &context = CI->getContext();
  // Is the argument already in the correct type?
  const Type *argType = arg->getType();
  if (argType == rootType) return arg; 

  if (isOpaquePtrPair(argType, rootType))
  {
    //incase of pointer to opaque type bitcast 
    return  new BitCastInst(arg, rootType, "bitcast.opaque.ptr", CI);
  }

  if (isa<PointerType>(argType))
  {
    //If the function argument is in Pointer type, we expect to find the origin of the pointer
    // as an alloca instruction with 2 users: a store (of the original value) and the CALL inst
    // Any other formation will fail the rooting effort
    AllocaInst *allocator = dyn_cast<AllocaInst>(arg);
    if (!allocator || allocator->getAllocatedType() != rootType ||
      allocator->isArrayAllocation() || !allocator->hasNUses(2))
    {
        return NULL;
    }

    // Check the 2 users are really a store and the function call.
    Value *retVal = NULL;
    for (Value::use_iterator i = allocator->use_begin(), e = allocator->use_end(); i != e; ++i)
    {
      // Check for store instruction
      if (StoreInst *storeInst = dyn_cast<StoreInst>(*i))
      {
        // Only a single store is expected...
        if (retVal) return NULL;
        // Keep the value which is being stored.
        retVal = storeInst->getOperand(0);
        // the stored value should be of the expected type
        if (retVal->getType() != rootType) return NULL;
      }
      // Else check for the call instruction
      else if (CallInst *callInst = dyn_cast<CallInst>(*i))
      {
        // check that the call inst is the one we started with
        if (CI != callInst) return NULL;
      }
      else
      {
        // Unexpected consumer of Alloca.
        return NULL;
      }
    }
    V_ASSERT(retVal && "retVal must have been set by now");
    return retVal;
  }

  // arg was passed as a value (not pointer) but of incorrect type. Climb up over instruction's
  // use-def chain, until the value's root is found, or until reaching a non-instruction
  Value *currVal = arg;
  Instruction *inst;
  while (currVal->getType() != rootType && (inst = dyn_cast<Instruction>(currVal)))
  {
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
      if (EE->getVectorOperandType()->getNumElements() != 1) return NULL;
    }
    // Check for the more-complicated ShuffleVector cast
    else if (ShuffleVectorInst *SV = dyn_cast<ShuffleVectorInst>(currVal))
    {
      currVal = isExtendedByShuffle(SV, rootType);
      if (!currVal) return NULL;
    } 
    else
    {
        // maybe we should return NULL ? I am not sure why we ever return NULL!!!
        break;    
    }
    V_PRINT(utils, "climbing through use-def chain: " << *currVal << "\n");
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
  const VectorType *inputType = dyn_cast<VectorType>(SI->getOperand(0)->getType());
  V_ASSERT(inputType && "ShuffleVector is expected to have vector inputs!");
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


Value *VectorizerUtils::RootReturnValue(Value *retVal, const Type *rootType, CallInst *CI)
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
    for (Value::use_iterator i = allocator->use_begin(), e = allocator->use_end(); i != e; ++i)
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
    V_ASSERT(rootRetVal && "Must have rooted the retVal by now");
    return rootRetVal;
  }

  // retval was passed as a value (not pointer) but of incorrect type.
  V_ASSERT (dyn_cast<Instruction>(retVal) == CI && "retVal should be the return of the CALL");

  if (CI->use_begin() == CI->use_end())
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
    Value::use_iterator ui, ue;
    for (ui = instToTest->use_begin(), ue = instToTest->use_end(); ui != ue; ++ui)
    {
      Instruction *userInst = dyn_cast<Instruction>(*ui);
      V_ASSERT(NULL != userInst && "Instruction's user is not an instruction. Unexpected");
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
  V_ASSERT(!retvalUsers.empty() && "retval expected to have at least one user");
  unsigned srcSize = CI->getType()->getPrimitiveSizeInBits();
  unsigned dstSize = rootType->getPrimitiveSizeInBits();
  // Fail if retval is not a primitive type which has a measurable size
  if (0 == srcSize || 0 == dstSize) return false;
  // Fail if the real retval is smaller than the desired size
  if (srcSize < dstSize) return false;

  // If the CALL instruction is in the retvalUsers list, create a dummy inst and replace all
  // users of the inst with the dummy val. This is needed now, so the new conversion from the
  // CALL value will be the only user of the CALL.
  Instruction *dummyInstruction = NULL;
  if (retvalUsers.count(CI))
  {
    const Type *ptrType = PointerType::get(CI->getType(), 0);
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
    V_ASSERT(convertedVal->getType()->getPrimitiveSizeInBits() == dstSize && "cast size error");
    // Bitcast to desired type
    Instruction *castToDesired = new BitCastInst(convertedVal, rootType);
    castToDesired->insertAfter(convertedVal);
    convertedVal = castToDesired;
  }
  V_ASSERT(convertedVal->getType() == rootType && "Cast retval failed");

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
    V_ASSERT(dummyInstruction->use_empty() && "Did not disconnect all dummy users!");
    dummyInstruction->eraseFromParent();
  }
  return convertedVal;
}


Instruction *VectorizerUtils::BitCastValToType(Value *orig, const Type *targetType,
                                                 Instruction *insertPoint)
{
  LLVMContext& context = insertPoint->getContext();
  const Type *currType = orig->getType();
  V_ASSERT (currType != targetType && "should get here in case of same type" );
  unsigned currSize = currType->getPrimitiveSizeInBits();
  unsigned rootSize = targetType->getPrimitiveSizeInBits();
  Instruction *retVal;
  
  if (currSize == rootSize)
  {
    // just bitcast from one to the other
    retVal = new BitCastInst(orig, targetType, "cast_val", insertPoint);
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

Instruction *VectorizerUtils::ExtendValToType(Value *orig, const Type *targetType,
                                              Instruction *insertPoint)
{
  V_ASSERT(orig->getType()->getPrimitiveSizeInBits() <= targetType->getPrimitiveSizeInBits() &&
      "expanding when souce is bigger than target");
  return BitCastValToType(orig, targetType, insertPoint);
}

Instruction *VectorizerUtils::TruncValToType(Value *orig, const Type *targetType, Instruction *insertPoint)
{
  V_ASSERT(orig->getType()->getPrimitiveSizeInBits() >= targetType->getPrimitiveSizeInBits() &&
      "trunc when souce is  smaller than target");
  return BitCastValToType(orig, targetType, insertPoint);
}

Instruction *VectorizerUtils::convertValToPointer(Value *orig, const Type *targetType, Instruction *insertPoint)
{
  const PointerType *targetPointerType = dyn_cast<PointerType>(targetType);
  V_ASSERT(targetPointerType && "getting here target type must be a pointer");
  if (!targetPointerType) return NULL;
  const Type *sourceType = orig->getType();
  V_ASSERT(targetPointerType->getElementType() == sourceType && "pointer nust of orig type");
  AllocaInst *ptr = new AllocaInst(sourceType, "allocated_val" , insertPoint);
  new StoreInst(orig, ptr, insertPoint);
  return ptr;
}


Value *VectorizerUtils::getCastedArgIfNeeded(Value *inputVal, const Type *targetType, Instruction *insertPoint)
{
  const Type *sourceType = inputVal->getType();
  
  // incase of same type do noting
  if (sourceType == targetType) return inputVal;

  if (isOpaquePtrPair(sourceType,targetType))
  {
    return  new BitCastInst(inputVal, targetType, "bitcast.opaque.ptr", insertPoint); 
  }
  
  // no support for case when not the same type ans source is a pointer
  if (sourceType->isPointerTy())
  {
      V_ASSERT(0 && "no support for case when not the same type ans source is a pointer");
      return NULL;
  }

  // if targetType is a pointer and not the same type we assume the pointer type match the value
  // so we allcoate a pointer and store the original input value
  if (targetType->isPointerTy()) return convertValToPointer(inputVal, targetType, insertPoint);
  
  // convert the orig into target type by bitcasting and Zext\trunc if needed
  return ExtendValToType(inputVal, targetType, insertPoint);
}


Instruction *VectorizerUtils::getCastedRetIfNeeded(CallInst *CI, const Type *targetType)
{
  // incase of same type do noting
  const Type *sourceType = CI->getType();
  if (sourceType == targetType) return CI;

  // this is ugly but I could not think of something better
  // putting alloca Instruction just after call instruction
  // will use this alloca as insert point for casting and remove it in the end
  Instruction *insertPoint = new AllocaInst(IntegerType::get(CI->getContext(), 1));
  insertPoint->insertAfter(CI);
  Instruction *castedInst = TruncValToType(CI, targetType, insertPoint);
  insertPoint->eraseFromParent();
  return castedInst;
}




} // nampespace intel