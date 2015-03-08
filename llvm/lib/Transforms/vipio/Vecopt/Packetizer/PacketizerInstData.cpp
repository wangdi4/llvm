/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "Packetizer.h"
#include "VectorizerUtils.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"

namespace intel {

Instruction *PacketizeFunction::findInsertPoint(Value * val)
{
  if (!isa<Instruction>(val))
  {
    // Non instructions have global scope. so insert point is the function head
    return &*(inst_begin(m_currFunc));
  }
  if (!isa<PHINode>(val))
  {
    // For "nromal" instructions, the insert point is right after them
    return dyn_cast<Instruction>(val);
  }

  // Getting here, the instruction is PHINode. Return the last PHI node
  V_ASSERT(isa<PHINode>(val) && "Expecting a PHINode instruction");
  BasicBlock::iterator iter(dyn_cast<PHINode>(val)->getParent()->getFirstNonPHI());
  return --iter;
}

bool PacketizeFunction::isInsertNeededToObtainVectorizedValue(Value * origValue) {
  V_ASSERT((
    origValue->getType()->isIntegerTy() ||
    origValue->getType()->isFloatingPointTy())
    && "Trying to get a packetized value of non-primitive type!");
  if (!isa<Instruction>(origValue))
    return false;
  if (!m_VCM.count(origValue))
    return false;
  // Entry is found in VCM
  VCMEntry * foundEntry = m_VCM[origValue];
  if (foundEntry->vectorValue != NULL)
  {
    // Vectored value is found (pre-prepared)
    return false;
  }
  // Vectored value does not exist, need to create it
  if (foundEntry->isScalarRemoved == false)
  {
    V_ASSERT(!origValue->getType()->isVectorTy() && "Original value is expected to be scalar");
    // Original value is kept, so just need to broadcast it
    //  %temp   = insertelement <4 x Type> undef  , Type %value, i32 0
    //  %vector = shufflevector <4 x Type> %temp, <4 x Type> %undef, <4 x i32> <0,0,0,0>

    // it's better to broadcast a single value than to scalarize the operation,
    // so here we return false.
    return false;
  }

  // Cannot use original value. Must assemble the multi-scalars
  //   %temp.vect.0 = insertelement <4 x type> undef       , type %scalar.0, i32 0
  //   %temp.vect.1 = insertelement <4 x type> %indx.vect.0, type %scalar.1, i32 1
  //   %temp.vect.2 = insertelement <4 x type> %indx.vect.1, type %scalar.2, i32 2
  //   %temp.vect.3 = insertelement <4 x type> %indx.vect.2, type %scalar.3, i32 3
  V_ASSERT(NULL != foundEntry->multiScalarValues[0] && "expected to find multi-scalar");

  // If we are going to use many insert elemenets to create this vector,
  // then if the vector is a PHI-Node, perhaps it is better
  // to duplicate the PHI-Node instead, so we return true.
  return true;
}

void PacketizeFunction::obtainVectorizedValue(Value **retValue, Value * origValue,
                                              Instruction * origInst)
{
  V_ASSERT((
    origValue->getType()->isIntegerTy() ||
    origValue->getType()->isFloatingPointTy())
    && "Trying to get a packetized value of non-primitive type!");
  *retValue = NULL;
  if (isa<Instruction>(origValue))
  {
    Instruction *origInst = cast<Instruction>(origValue);
    if (m_VCM.count(origValue))
    {
      // Entry is found in VCM
      VCMEntry * foundEntry = m_VCM[origValue];
      if (foundEntry->vectorValue != NULL)
      {
        // Vectored value is found (pre-prepared)
        *retValue = foundEntry->vectorValue;
      }
      else
      {
        // Vectored value does not exist - create it
        if (foundEntry->isScalarRemoved == false)
        {
          V_ASSERT(!origValue->getType()->isVectorTy() && "Original value is expected to be scalar");
          // Original value is kept, so just need to broadcast it
          //  %temp   = insertelement <4 x Type> undef  , Type %value, i32 0
          //  %vector = shufflevector <4 x Type> %temp, <4 x Type> %undef, <4 x i32> <0,0,0,0>
          Instruction *shuffleInst = VectorizerUtils::createBroadcast(origValue, m_packetWidth, findInsertPoint(origValue), true);

          // Add to return structure and update VCM
          *retValue = shuffleInst;
          foundEntry->vectorValue = shuffleInst;
        }
        else
        {
          // Cannot use original value. Must assemble the multi-scalars
          //   %temp.vect.0 = insertelement <4 x type> undef       , type %scalar.0, i32 0
          //   %temp.vect.1 = insertelement <4 x type> %indx.vect.0, type %scalar.1, i32 1
          //   %temp.vect.2 = insertelement <4 x type> %indx.vect.1, type %scalar.2, i32 2
          //   %temp.vect.3 = insertelement <4 x type> %indx.vect.2, type %scalar.3, i32 3
          V_ASSERT(NULL != foundEntry->multiScalarValues[0] && "expected to find multi-scalar");
          UndefValue *undefVect =
            UndefValue::get(VectorType::get(origValue->getType(), m_packetWidth));

          Instruction * insertPoint = foundEntry->multiScalarValues[m_packetWidth - 1];
          Value * prevResult = undefVect;
          for (unsigned index = 0; index < m_packetWidth; ++index)
          {
            Value *constIndex = ConstantInt::get(Type::getInt32Ty(context()), index);
            Instruction *newInst = InsertElementInst::Create(prevResult,
              foundEntry->multiScalarValues[index] , constIndex, "temp.vect");

            V_ASSERT(newInst && "inst creation failure");
            VectorizerUtils::SetDebugLocBy(newInst, origInst);
            newInst->insertAfter(findInsertPoint(insertPoint));
            prevResult = newInst;
            insertPoint = newInst;
          }
          foundEntry->vectorValue = insertPoint;
          *retValue = insertPoint;
        }
      }
    }
    else
    {
      V_ASSERT(origInst && "received a NULL origInst");
      // Entry was not found in VCM. Means it will be defined in a following basic block
      createDummyVectorVal(origValue, retValue);
    }
  }
  else if (isa<Argument>(origValue))
  {
    if (m_VCM.count(origValue))
    {
      // Entry is found in VCM
      VCMEntry * foundEntry = m_VCM[origValue];
      V_ASSERT(foundEntry->vectorValue != NULL && "expected vector value for argument");
      *retValue = foundEntry->vectorValue;
    }
    else
    {
      // Create a new entry in VCM
      Instruction* vectorValue;
      switch (m_depAnalysis->whichDepend(origValue)) {
        case WIAnalysis::CONSECUTIVE:
	  vectorValue = VectorizerUtils::createConsecutiveVector(origValue, m_packetWidth,
								 findInsertPoint(origValue),
								 false);
	  break;
        case WIAnalysis::RANDOM:
	  vectorValue = VectorizerUtils::createBroadcast(origValue, m_packetWidth,
							 findInsertPoint(origValue),
							 false);
	  break;
        default:
	  vectorValue = NULL;
	  break;
      }
      if (vectorValue) {
	VCMEntry* newEntry = allocateNewVCMEntry();
	newEntry->vectorValue = vectorValue;
	newEntry->isScalarRemoved = false;
	m_VCM.insert(std::pair<Value *, VCMEntry *>(origValue, newEntry));
	*retValue = vectorValue;
      }
    }
  }

  if (*retValue == NULL)
  {
    Value * broadcastedVal;
    // Check if this value is a "proper" constant value, or an Undef
    if (isa<Constant>(origValue))
    {
      // Create a broadcasted constant (no need to make an instruction for this)
      broadcastedVal = ConstantVector::getSplat(m_packetWidth, cast<Constant>(origValue));
    }
    else
    {
      // Check in BAG (broadcast arguments and globals) whether we have this value already
      if (m_BAG.count(origValue))
      {
	// Value was already broadcasted. use it
	broadcastedVal = m_BAG[origValue];
	V_ASSERT(broadcastedVal && "BAG held null value");
      }
      else
      {
	// Need to broadcast the value
	//    %temp   = insertelement <4 x Type> undef  , Type %value, i32 0
	//    %vector = shufflevector <4 x Type> %temp, <4 x Type> %undef, <4 x i32> <0,0,0,0>
	broadcastedVal = VectorizerUtils::createBroadcast(origValue, m_packetWidth, findInsertPoint(origValue), true);
	// Add broadcast to BAG
	m_BAG.insert(std::pair<Value*,Value*>(origValue, broadcastedVal));
      }
    }

    // Put broadcasted constant in returned structure
    *retValue = broadcastedVal;
  }

  V_PRINT(packetizer,
      "\t\tObtained vectorized value(s) of type: " << *((*retValue)->getType()) << "\n");
}

void PacketizeFunction::obtainMultiScalarValues(Value *retValues[],
                                                Value *origValue,
                                                Instruction *origInst)
{
  if (!isa<Instruction>(origValue))
  {
    // Original value is not an instruction. So simply broadcast it
    for (unsigned i = 0; i < m_packetWidth; i++)
      retValues[i] = origValue;
  }
  else
  {
    Instruction *origInst = cast<Instruction>(origValue);
    if (m_VCM.count(origValue))
    {
      // Entry is found in VCM
      VCMEntry * foundEntry = m_VCM[origValue];
      if (foundEntry->isScalarRemoved == false)
      {
        // Use the scalar value multiple times
        for (unsigned i = 0; i < m_packetWidth; i++)
          retValues[i] = origValue;
      }
      else
      {
        // Must either find multi-scalar values, or break down the vector value
        if (foundEntry->multiScalarValues[0])
        {
          // found pre-prepared multi-scalar values
          for (unsigned i = 0; i < m_packetWidth; i++)
            retValues[i] = foundEntry->multiScalarValues[i];
        }
        else if(isa<SExtInst>(origInst) || isa<ZExtInst>(origInst))
        {
          // For performance reasons it is better to use the original non-extended vector
          // because in general "vector extract -> scalar extend" X86 sequence is better
          // than the "vector extend -> vector extract" for following multiscalar operation.

          // TODO: To avoid negative performance impact do this only if all users
          //       of this SExt/ZExt are non-packetiziable

          SmallVector<Value *, 16> nonExtValues(m_packetWidth);
          obtainMultiScalarValues(nonExtValues.data(), origInst->getOperand(0), NULL);

          Instruction * insertPoint = findInsertPoint(foundEntry->vectorValue);
          for (unsigned index = 0; index < m_packetWidth; ++index) {
            CastInst * pCast = CastInst::Create((Instruction::CastOps)origInst->getOpcode(),
                                                 nonExtValues[index], origInst->getType());
            VectorizerUtils::SetDebugLocBy(pCast, origInst);

            pCast->insertAfter(insertPoint);
            retValues[index] = pCast;
            foundEntry->multiScalarValues[index] = pCast;
            insertPoint = pCast;
          }
        }
        else
        {
          // Failed to find multi-scalar values. Break down the vectorized value instead
          //     %extract0 = extractelement <4 x Type> %vector, i32 0
          //     %extract1 = extractelement <4 x Type> %vector, i32 1
          //     %extract2 = extractelement <4 x Type> %vector, i32 2
          //     %extract3 = extractelement <4 x Type> %vector, i32 3
          V_ASSERT(foundEntry->vectorValue && "expected a vector value");
          Instruction * insertPoint = findInsertPoint(foundEntry->vectorValue);
          for (unsigned index = 0; index < m_packetWidth; ++index)
          {
            Value * constIndex = ConstantInt::get(Type::getInt32Ty(context()), index);
            Instruction *newEE =
              ExtractElementInst::Create(foundEntry->vectorValue, constIndex, "extract");

            VectorizerUtils::SetDebugLocBy(newEE, origInst);
            retValues[index] = newEE;
            foundEntry->multiScalarValues[index] = newEE;
            newEE->insertAfter(insertPoint);
            insertPoint = newEE;
          }
        }
      }
    }
    else
    {
      V_ASSERT(origInst && "expected to have origInst");
      // Entry was not found in VCM. Means it will be defined in a following basic block
      createDummyMultiScalarVals(origValue, retValues);
    }
  }
}

void PacketizeFunction::useOriginalConstantInstruction(Instruction *origInst)
{
  V_PRINT(packetizer, "\t\tNot Duplicated Instruction\n");
  V_ASSERT(0 == m_VCM.count(origInst) && "should not be appearing in VCM already");
  VCMEntry * newEntry = allocateNewVCMEntry();
  newEntry->isScalarRemoved = false;
  m_VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));
}


void PacketizeFunction::createVCMEntryWithVectorValue(Instruction *origInst,
                                                      Instruction *vectoredValue)
{
  V_ASSERT(0 == m_VCM.count(origInst) && "should not be appearing in VCM already");
  VCMEntry * newEntry = allocateNewVCMEntry();
  newEntry->isScalarRemoved = true;
  newEntry->vectorValue = vectoredValue;
  VectorizerUtils::SetDebugLocBy(vectoredValue, origInst);
  m_VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));
}

void PacketizeFunction::createVCMEntryWithMultiScalarValues(Instruction * origInst,
                                                            Instruction * multiScalarValues[])
{
  V_ASSERT(0 == m_VCM.count(origInst) && "should not be appearing in VCM already");
  VCMEntry * newEntry = allocateNewVCMEntry();
  newEntry->isScalarRemoved = true;
  for (unsigned i = 0; i < m_packetWidth; i++) {
    newEntry->multiScalarValues[i] = multiScalarValues[i];
    VectorizerUtils::SetDebugLocBy(multiScalarValues[i], origInst);
  }
  m_VCM.insert(std::pair<Value *, VCMEntry *>(origInst, newEntry));
}


PacketizeFunction::VCMEntry* PacketizeFunction::allocateNewVCMEntry()
{
  // If index of next free VCMEntry is over the array size, create a new array
  if (m_VCMArrayLocation == ESTIMATED_INST_NUM)
  {
    // Create new VCMAllocationArray, push it to the vector of arrays, and set free index to 0
    m_VCMAllocationArray = new VCMEntry[ESTIMATED_INST_NUM];
    m_VCMArrays.push_back(m_VCMAllocationArray);
    m_VCMArrayLocation = 0;
  }
  VCMEntry * newEntry = &(m_VCMAllocationArray[m_VCMArrayLocation++]);
  // Make sure the entry is clean
  newEntry->vectorValue = NULL;
  newEntry->multiScalarValues[0] = NULL;
  return newEntry;
}

void PacketizeFunction::releaseAllVCMEntries()
{
  V_ASSERT(m_VCMArrays.size() > 0 && "At least one buffer should be allocated");
  while (m_VCMArrays.size() > 1)
  {
    // If there are additional allocated entry Arrays, release them (leave only the first)
    VCMEntry * popEntry = m_VCMArrays.pop_back_val();
    delete[] popEntry;
  }
  // set the "current" pointer to the only remaining array
  m_VCMAllocationArray = m_VCMArrays[0];
  m_VCMArrayLocation = 0;
}


void PacketizeFunction::createDummyVectorVal(Value *origValue, Value **vectorVal)
{
  V_ASSERT(!isa<VectorType>(origValue->getType()) && "cannot packetize vectors");

  VCMEntry * dummyEntry;

  // First, try to find if the needed dummy values already exist
  if (m_deferredResMap.count(origValue))
  {
    dummyEntry = m_deferredResMap[origValue];
    if (NULL != dummyEntry->vectorValue)
    {
      // Dummy values already exist. use them
      V_PRINT(packetizer, "\t\tFound existing Dummy Vector value/s \n");
      *vectorVal = dummyEntry->vectorValue;
      return;
    }
  }
  else
  {
    dummyEntry = allocateNewVCMEntry();
    m_deferredResOrder.push_back(origValue);
  }


  // Create the dummy values and place them in VCMEntry
  Type* origType = origValue->getType();
  Type *dummyType = m_soaAllocaAnalysis->isSoaAllocaRelatedPointer(origValue) ?
    VectorizerUtils::convertSoaAllocaType(origType, m_packetWidth) :
    VectorType::get(origType, m_packetWidth);
  V_PRINT(packetizer, "\t\tCreate Dummy Vector value/s (of type " << *dummyType << ")\n");
  Constant *dummyPtr = ConstantPointerNull::get(dummyType->getPointerTo());

  dummyEntry->vectorValue = new LoadInst(dummyPtr);
  *vectorVal = dummyEntry->vectorValue;

  // Insert into deferred resolution map/list
  m_deferredResMap.insert(std::pair<Value *, VCMEntry *>(origValue, dummyEntry));
}


void PacketizeFunction::createDummyMultiScalarVals(Value *origValue, Value *multiScalarVals[])
{
  VCMEntry * dummyEntry;

  // First, try to find if the needed dummy values already exist
  if (m_deferredResMap.count(origValue))
  {
    dummyEntry = m_deferredResMap[origValue];
    if (dummyEntry->multiScalarValues[0] != NULL)
    {
      // Dummy values already exist. use them..
      V_PRINT(packetizer, "\t\tFound existing Dummy Multi-scalar value/s \n");
      for (unsigned i = 0; i < m_packetWidth; i++)
        multiScalarVals[i] = dummyEntry->multiScalarValues[i];
      return;
    }
  }
  else
  {
    dummyEntry = allocateNewVCMEntry();
    m_deferredResOrder.push_back(origValue);
  }

  // Create the dummy values and place them in VCMEntry
  Type* origType = origValue->getType();
  Type *dummyType = m_soaAllocaAnalysis->isSoaAllocaRelatedPointer(origValue) ?
    VectorizerUtils::convertSoaAllocaType(origType, m_packetWidth) : origType;
  V_PRINT(packetizer, "\t\tCreate Dummy value/s (of type " << *dummyType << ")\n");
  Constant *dummyPtr = ConstantPointerNull::get(dummyType->getPointerTo());

  for (unsigned i = 0; i < m_packetWidth; i++)
  {
    dummyEntry->multiScalarValues[i] = new LoadInst(dummyPtr);
    multiScalarVals[i] = dummyEntry->multiScalarValues[i];
  }

  // Insert into deferred resolution map/list
  m_deferredResMap.insert(std::pair<Value *, VCMEntry *>(origValue, dummyEntry));
}


bool PacketizeFunction::resolveDeferredInstructions()
{
  // Iterate over the deferred insts list, and resolve them
  std::vector<Value *>::iterator currIter = m_deferredResOrder.begin();
  std::vector<Value *>::iterator endIter = m_deferredResOrder.end();
  for (;currIter != endIter; ++currIter)
  {
    Value * origVal = *currIter;
    VCMEntry * dummyEntry = m_deferredResMap[origVal];
    V_PRINT(packetizer, "\tDRL Going to fix value of orig inst: " << *origVal << "\n");

    if (!m_VCM.count(origVal)) {
      V_ASSERT(0 && "The target instruction is not in VCM");
    }

    // check (in the dummy values) if vectorized values are required
    if (NULL != dummyEntry->vectorValue)
    {
      Value *resolvedVal;
      // Plcing "NULL" as the "origInst" value in obtainVectorizedValues
      // should be safe, as long as we know there is a VCM entry...
      obtainVectorizedValue(&resolvedVal, origVal, NULL);

      // Replace dummy value with proper value
      dummyEntry->vectorValue->replaceAllUsesWith(resolvedVal);
      // Deleted and not "erased from parent" - because dummy was never insert to function!
      delete dummyEntry->vectorValue;
    }

    // check (in the dummy values) if multi scalar values are required
    if (NULL != dummyEntry->multiScalarValues[0])
    {
      Value * resolvedVals[MAX_PACKET_WIDTH];
      // Placing "NULL" as the "origInst" value in obtainMultiScalarValues should be safe,
      // as long as we know there is a VCM entry...
      obtainMultiScalarValues(resolvedVals, origVal, NULL);

      // Replace dummy values with proper values
      for (unsigned i = 0; i < m_packetWidth; ++i)
      {
        dummyEntry->multiScalarValues[i]->replaceAllUsesWith(resolvedVals[i]);
        // Deleted and not "erased from parent" - because dummy was never insert to function!
        delete dummyEntry->multiScalarValues[i];
      }
    }
  }
  return true;
}


} // namespace

