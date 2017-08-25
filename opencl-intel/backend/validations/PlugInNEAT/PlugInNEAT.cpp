/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  PlugInNEAT.cpp


\*****************************************************************************/

#define DEBUG_TYPE "NEATPlugin"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "InterpreterPluggable.h"
#include "llvm/IR/DataLayout.h"

#include "PlugInNEAT.h"
#include "NEAT_WRAP.h"
#include "IMemoryObject.h"
#include "Buffer.h"
#include "Image.h"
#include "OCLBuiltinParser.h"
#include "Helpers.h"
#include "BLTImages.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

// !!!! HACK
// Do not move #include "CL/cl.h" before including <math.h> since on VS2008 it generates
// Removing annoying ‘ceil’ : attributes not present on previous declaration warning C4985
#include "cl_types.h"
#include "CL/cl.h"

using namespace llvm;
using namespace Validation;
using namespace Validation::Exception;

//===----------------------------------------------------------------------===//
//                     Various Helper Functions
//===----------------------------------------------------------------------===//

const NEATStructLayout * NEATDataLayout ::getStructLayout( const StructType *Ty ) const
{
  NEATStructLayout *SL = 0;
  // Create the struct layout.  Because it is variable length, we
  // malloc it, then use placement new.
  int NumElts = Ty->getNumElements();
  NEATStructLayout *L =
    (NEATStructLayout *)malloc(sizeof(NEATStructLayout)+(NumElts-1) * sizeof(uint64_t));

  // Set SL before calling StructLayout's ctor.  The ctor could cause other
  // entries to be added to TheMap, invalidating our reference.
  SL = L;

  new (L) NEATStructLayout(Ty, *this);

  return L;
}


// [LLVM 3.6 UPGRADE] The implementation of the following function template is
//                    taken from LLVM 3.4 as is.
/// RoundUpAlignment - Round the specified value up to the next alignment
/// boundary specified by Alignment.  For example, 7 rounded up to an
/// alignment boundary of 4 is 8.  8 rounded up to the alignment boundary of 4
/// is 8 because it is already aligned.
template <typename UIntTy>
static UIntTy RoundUpAlignment(UIntTy Val, unsigned Alignment) {
  assert((Alignment & (Alignment-1)) == 0 && "Alignment must be power of 2!");
  return (Val + (Alignment-1)) & ~UIntTy(Alignment-1);
}

NEATStructLayout::NEATStructLayout( const StructType *ST, const NEATDataLayout  &DL )
{
  StructAlignment = 0;
  StructSize = 0;
  NumElements = ST->getNumElements();

  // Loop over each of the elements, placing them in memory.
  for (unsigned i = 0, e = NumElements; i != e; ++i) {
    const Type *Ty = ST->getElementType(i);
    unsigned TyAlign = ST->isPacked() ? 1 : DL.getABITypeAlignment(Ty);

    // Add padding if necessary to align the data element properly.
    if ((StructSize & (TyAlign-1)) != 0)
      StructSize = RoundUpAlignment(StructSize, TyAlign);

    // Keep track of maximum alignment constraint.
    StructAlignment = std::max(TyAlign, StructAlignment);

    MemberOffsets[i] = StructSize;
    StructSize += DL.getTypeAllocSize(Ty); // Consume space for this data item
  }

  // Empty structures have alignment of 1 byte.
  if (StructAlignment == 0) StructAlignment = 1;

  // Add padding to the end of the struct so that it could be put in an array
  // and all array elements would be aligned correctly.
  if ((StructSize & (StructAlignment-1)) != 0)
    StructSize = RoundUpAlignment(StructSize, StructAlignment);
}

unsigned NEATDataLayout ::getABITypeAlignment(const Type *Ty) const {
  // TODO: implement!
  return 1; // getAlignment(Ty, true);
}

#define HANDLE_EVENT(__EVENT) if(GetCurEvent() != __EVENT) return;

static void SetValue(Value *V, NEATGenericValue Val, NEATExecutionContext &SF) {
    SF.Values[V] = Val;
}

void NEATPlugIn::handlePreInstExecution(Instruction& I)
{
    SetCurEvent(PRE_INST);
    // Interpret a single instruction & increment the "PC".
    //ExecutionContext &SF = m_pECStack->back();  // Current stack frame

    visit(I);   // Dispatch to one of the visit* methods...
}

void NEATPlugIn::handlePostInstExecution(Instruction& I)
{
    SetCurEvent(POST_INST);
    // Interpret a single instruction & increment the "PC".
    DEBUG(dbgs() << "NEATPlugin is after running : " << I << "\n");
    visit(I);
}

void NEATPlugIn::visitLoadInst(LoadInst &I)
{
    // handle only post instruction execution
    HANDLE_EVENT(POST_INST);
    // if NEAT does not support this type - exit
    if(!m_NTD.IsNEATSupported(I.getType())) return;

    DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

    NEATExecutionContext &SF = m_NECStack.back();

    // load pointer NEATGenericValue
    NEATGenericValue SRC = getOperandValue(I.getPointerOperand(), SF);
    // convert NEATGenericValue to pointer to NEAT
    NEATGenericValue *NEATPtr = (NEATGenericValue*)NGVTOP(SRC);

    NEATGenericValue Result;
    // load NEAT from memory
    LoadValueFromMemory(Result, NEATPtr, I.getType());
    // set result value
    SetValue(&I, Result, SF);

}


NEATGenericValue NEATPlugIn::getConstantExprValue (ConstantExpr *CE,NEATExecutionContext &SF)
{
    switch (CE->getOpcode()) {
    case Instruction::GetElementPtr:
        return executeGEPOperation(CE->getOperand(0), gep_type_begin(CE),
                          gep_type_end(CE), SF);
    default :
           throw Exception::IllegalFunctionCall("[NEATPlug-in::getConstantExprValue] : Not supported NEAT instruction");
      break;
    }

    dbgs() << "[NEATPlug-in] Unhandled ConstantExpr: " << *CE << "\n";
    return NEATGenericValue();
}

NEATGenericValue NEATPlugIn::getConstantValueFromValue (Value *V)
{
    NEATGenericValue Ret;
    if ( isa<UndefValue>(V) ) {
        // if operand value is 'undef' then no need to initialize NEATValue. It's already UNWRITTEN by default.
        // We need just set correct vector length for vector values.
        if (Type::VectorTyID == V->getType()->getTypeID()) {
            Ret.NEATVec.SetWidth(VectorWidthWrapper::ValueOf(cast<VectorType>(V->getType())->getNumElements()));
        }
    } else {

        GenericValue GV = m_pInterp->getOperandValueAdapter(V, m_pECStack->back());

        switch(V->getType()->getTypeID())
        {
        case Type::FloatTyID:
            Ret.NEATVal.SetAccurateVal<float>(GV.FloatVal);
            break;
        case Type::DoubleTyID:
            Ret.NEATVal.SetAccurateVal<double>(GV.DoubleVal);
            break;
        case Type::PointerTyID:
            {
            const PointerType* PT = cast<PointerType>(V->getType());
            if (NEATDataLayout ::IsNEATSupported(PT->getElementType())) {
                if (GlobalValue *GV2 = dyn_cast<GlobalValue>(V)) {
                    Ret = PTONGV(getPointerToGlobal(GV2));
                } else {
                    Exception::IllegalFunctionCall("[NEATPlug-in::getConstantValue] : Value is not GlobalValue");
                }
            } else {
                Ret.PointerVal = GV.PointerVal;
            }
            break;
            }
        case Type::VectorTyID: {
            const VectorType* VT = cast<VectorType>(V->getType());
            Ret.NEATVec.SetWidth(VectorWidthWrapper::ValueOf(VT->getNumElements()));
            for (unsigned int i = 0; i < VT->getNumElements(); ++i)
            {
                if (VT->getElementType()->getTypeID() == Type::FloatTyID) {
                    Ret.NEATVec[i].SetAccurateVal<float>(GV.AggregateVal[i].FloatVal);
                }
                if (VT->getElementType()->getTypeID() == Type::DoubleTyID) {
                    Ret.NEATVec[i].SetAccurateVal<double>(GV.AggregateVal[i].DoubleVal);
                }
            }
            break;
         }                               // TODO: Add Array and Structure data types support!
        default:
            // Unsupported constant data type - do nothing.
            throw Exception::IllegalFunctionCall("[NEATPlug-in::getConstantValue] : Not supported NEAT data type");
            break;
        }
    }
    return Ret;
}

NEATGenericValue NEATPlugIn::getOperandValue(Value *V, NEATExecutionContext &NSF)
{
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(V)) {
        return getConstantExprValue(CE,NSF);
    } else if (dyn_cast<Constant>(V)) {
        return getConstantValueFromValue(V);
    } else if (GlobalValue *GV = dyn_cast<GlobalValue>(V)) {
        return PTONGV(getPointerToGlobal(GV));
    } else {
        return NSF.Values[V];
    }
}

void NEATPlugIn::LoadValueFromMemory(NEATGenericValue &Result,
                                     NEATGenericValue *NEATPtr,
                                     Type *Ty)
{
  switch (Ty->getTypeID()) {
  case Type::FloatTyID:
  case Type::DoubleTyID:
    Result.NEATVal = *((NEATValue*) NEATPtr);
    if(Result.NEATVal.IsUnwritten())
    {
        DEBUG(dbgs() << "[NEATPlugin] warning : loaded UNWRITTEN value.\n");
    }
    break;
  case Type::PointerTyID:
    Result.PointerVal = *((PointerTy*)NEATPtr);
    break;
  case Type::VectorTyID: {
    NEATValue* NEATValuePtr = (NEATValue*) NEATPtr;
    Result.NEATVec.SetWidth(VectorWidthWrapper::ValueOf(cast<VectorType>(Ty)->getNumElements()));
    for (unsigned i = 0; i < Result.NEATVec.GetSize(); ++i)
    {
      Result.NEATVec[i] = NEATValuePtr[i];
    }
    break;
                         }
  case Type::StructTyID: {
    const StructType* STy = dyn_cast<StructType>(Ty);
    const NEATStructLayout *SLO = m_NTD.getStructLayout(STy);

    Result.AggregateVal.resize(STy->getNumElements());
    for (unsigned i = 0; i < Result.AggregateVal.size(); ++i)
    {
      unsigned int offset = SLO->getElementOffset(i); // calculate offset for the current element
      LoadValueFromMemory(Result.AggregateVal[i], (NEATGenericValue*)((uint8_t*)NEATPtr+offset), STy->getElementType(i));
    }
    break;
                         }
  case Type::ArrayTyID: {
    NEATValue* NEATArrayPtr = (NEATValue*) NEATPtr;
    const ArrayType* ATy = cast<ArrayType>(Ty);

    Result.AggregateVal.resize(ATy->getNumElements());
    uint64_t elementSize = m_NTD.getTypeStoreSize(ATy->getElementType());
    for (unsigned i = 0; i < Result.AggregateVal.size(); ++i)
    {
      uint64_t offset = elementSize*i; // calculate offset for the current element
      LoadValueFromMemory(Result.AggregateVal[i], (NEATGenericValue*)((uint8_t*)NEATArrayPtr+offset), ATy->getElementType());
    }
    break;
                        }
  default:
      std::string msg;
      raw_string_ostream Msg(msg);
      Msg << "Cannot load value of type " << *Ty << "!";
      report_fatal_error(Msg.str());
    }
}
void NEATDataLayout ::InitMemory( void* Memory, const Type* Ty ) const
{
    assert(Ty->isSized() && "Cannot InitMemory() on a type that is unsized!");
    switch (Ty->getTypeID()) {
  case Type::LabelTyID:
  case Type::PointerTyID:
      return;
  case Type::ArrayTyID: {
          const ArrayType *ATy = cast<ArrayType>(Ty);
          char* ptr = (char*)Memory;
          for(size_t i = 0; i<ATy->getNumElements(); i++)
          {
            InitMemory(ptr, ATy->getElementType());
            ptr += getTypeStoreSize(ATy->getElementType());
          }
  }
  break;
  case Type::StructTyID: {
      const StructType *STy = cast<StructType>(Ty);
      char* ptr = (char*)Memory;
      for (unsigned int i = 0; i < STy->getNumContainedTypes(); ++i)
      {
          InitMemory(ptr, STy->getContainedType(i));
          ptr += getTypeStoreSize(STy->getContainedType(i));

      }
    break;
  }
  case Type::FloatTyID:
  case Type::DoubleTyID:
      new (Memory) NEATValue();
      break;
  case Type::IntegerTyID:
      return;
  case Type::VectorTyID:
      {
          const VectorType *VTy = cast<VectorType>(Ty);
          size_t toInitCount = VTy->getNumElements();
          if (VTy->getNumElements() == 3) toInitCount = 4;
          char* ptr = (char*)Memory;
          for(size_t i = 0; i<toInitCount; i++)
          {
              InitMemory(ptr, VTy->getElementType());
              ptr += getTypeStoreSize(VTy->getElementType());
          }
      }
      break;
  default:
      throw Validation::Exception::InvalidArgument("NEATDataLayout ::InitMemory(): Unsupported type");
      break;
    }
}


uint64_t NEATDataLayout ::getTypeStoreSize( const Type* Ty ) const
{
  assert(Ty->isSized() && "Cannot getTypeStoreSize() on a type that is unsized!");
  switch (Ty->getTypeID()) {
  case Type::LabelTyID:
  case Type::PointerTyID:
    return sizeof(void*);
  case Type::ArrayTyID: {
    const ArrayType *ATy = cast<ArrayType>(Ty);
    return getTypeStoreSize(ATy->getElementType())*ATy->getNumElements();
                        }
  case Type::StructTyID: {
    const StructType *STy = cast<StructType>(Ty);
    uint64_t size = 0;
    for (unsigned int i = 0; i < STy->getNumContainedTypes(); ++i)
    {
      size += getTypeStoreSize(STy->getContainedType(i));
    }
    return size;
                         }
  case Type::FloatTyID:
    return sizeof(NEATValue);
  case Type::DoubleTyID:
    return sizeof(NEATValue);
  case Type::IntegerTyID:
    return 0;
  case Type::VectorTyID:
      {
      const VectorType *VTy = cast<VectorType>(Ty);
      if (VTy->getNumElements() == 3) return getTypeStoreSize(VTy->getElementType())*4;
      return getTypeStoreSize(VTy->getElementType())*VTy->getNumElements();
      }
  default:
    throw Validation::Exception::InvalidArgument("NEATDataLayout ::getTypeSizeInBits(): Unsupported type");
    break;
  }
  return 0;
}

void NEATPlugIn::visitStoreInst( StoreInst &I )
{
    HANDLE_EVENT(POST_INST);
    // if NEAT does not support this type - exit
    if(!m_NTD.IsNEATSupported(I.getPointerOperand()->getType())) return;

    DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

    NEATExecutionContext &NSF = m_NECStack.back();
    NEATGenericValue Val = getOperandValue(I.getOperand(0), NSF);
    NEATGenericValue Dest = getOperandValue(I.getPointerOperand(), NSF);

    StoreValueToMemory(Val, (NEATGenericValue *)NGVTOP(Dest),
        I.getOperand(0)->getType());

}

/// StoreValueToMemory - Stores the data in Val of type Ty at address Ptr.  Ptr
/// is the address of the memory at which to store Val, cast to GenericValue *.
/// It is not a pointer to a GenericValue containing the address at which to
/// store Val.
void NEATPlugIn::StoreValueToMemory(const NEATGenericValue &Val,
                                         NEATGenericValue *Ptr, Type *Ty)
{

  const unsigned StoreBytes = m_NTD.getTypeStoreSize(Ty);

  switch (Ty->getTypeID())
  {
  case Type::FloatTyID:
  case Type::DoubleTyID:
    *((NEATValue*)Ptr) = Val.NEATVal;
    break;
  case Type::PointerTyID:
    // Ensure 64 bit target pointers are fully initialized on 32 bit hosts.
    if (StoreBytes != sizeof(PointerTy))
      memset(Ptr, 0, StoreBytes);

    *((PointerTy*)Ptr) = Val.PointerVal;
    break;
  case Type::VectorTyID: {
    NEATValue* NEATValuePtr = (NEATValue*) Ptr;
    for (unsigned i = 0; i < Val.NEATVec.GetSize(); ++i)
    {
      NEATValuePtr[i] = Val.NEATVec[i];
    }
    break;
                         }
  case Type::StructTyID: {
      const StructType* STy = dyn_cast<StructType>(Ty);
      const NEATStructLayout *SLO = m_NTD.getStructLayout(STy);

      for (unsigned i = 0; i < Val.AggregateVal.size(); ++i)
      {
          unsigned int offset = SLO->getElementOffset(i); // calculate offset for the current element
          StoreValueToMemory(Val.AggregateVal[i], (NEATGenericValue*)((uint8_t*)Ptr+offset), STy->getElementType(i));
      }
      break;
                         }
  case Type::ArrayTyID: {
      const ArrayType* ATy = dyn_cast<ArrayType>(Ty);

      uint64_t elementSize = m_NTD.getTypeStoreSize(ATy->getElementType());
      for (unsigned i = 0; i < Val.AggregateVal.size(); ++i)
      {
          uint64_t offset = elementSize*i; // calculate offset for the current element
          StoreValueToMemory(Val.AggregateVal[i], (NEATGenericValue*)((uint8_t*)Ptr+offset), ATy->getElementType());
      }
      break;
                        }
  default:
      DEBUG(dbgs() << "Cannot store value of type " << *Ty << "!\n");
  }
}

void NEATPlugIn::visitAllocaInst( AllocaInst &I )
{
    HANDLE_EVENT(POST_INST);
    // if NEAT does not support this type - exit
    if(!m_NTD.IsNEATSupported(I.getType())) return;

    DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

    NEATExecutionContext &NSF = m_NECStack.back();
    ExecutionContext &SF = m_pECStack->back();
    InterpreterPluggable &IP = *m_pInterp;
    Type *Ty = I.getType()->getElementType();  // Type to be allocated

    // Get the number of elements being allocated by the array...
    unsigned NumElements =
        IP.getOperandValueAdapter(I.getOperand(0), SF).IntVal.getZExtValue();

    unsigned TypeSize = (size_t)m_NTD.getTypeAllocSize(Ty);

    // Avoid malloc-ing zero bytes, use max()...
    unsigned MemToAlloc = std::max(1U, NumElements * TypeSize);

    void *Memory = malloc(MemToAlloc);

    DEBUG(dbgs() << "NEATPlugin: Allocated Type: " << *Ty << " (" << TypeSize << " bytes) x "
        << NumElements << " (Total: " << MemToAlloc << ") at "
        << uintptr_t(Memory) << '\n');

    /// To initialize allocated memory NEATValue constructor should be called
    m_NTD.InitMemory(Memory, I.getType()->getElementType());

    NEATGenericValue Result = PTONGV(Memory);
    assert(Result.PointerVal != 0 && "Null pointer returned by malloc!");
    SetValue(&I, Result, NSF);

    if (I.getOpcode() == Instruction::Alloca)
        NSF.Allocas.add(Memory);
    else
        throw Exception::NotImplemented("visitAllocaInst::Why am I here?");

}

uint64_t NEATDataLayout ::getTypeAllocSize( const Type* Ty ) const
{
    return getTypeStoreSize(Ty);
}

void NEATPlugIn::visitGetElementPtrInst( GetElementPtrInst &I )
{
    HANDLE_EVENT(POST_INST);
    NEATExecutionContext &NSF = m_NECStack.back();

    DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

    if ( m_NTD.IsNEATSupported(I.getPointerOperand()->getType()))
    SetValue(&I, executeGEPOperation(I.getPointerOperand(),
        gep_type_begin(I), gep_type_end(I), NSF), NSF);
}

NEATGenericValue NEATPlugIn::executeGEPOperation( Value *Ptr,
                                                   gep_type_iterator I,
                                                   gep_type_iterator E,
                                                   NEATExecutionContext &NSF )
{
    assert(Ptr->getType()->isPointerTy() &&
        "Cannot getElementOffset of a non-pointer type!");

    InterpreterPluggable &IP = *m_pInterp;
    ExecutionContext &SF = m_pECStack->back();

    uint64_t Total = 0;

    for (; I != E; ++I) {
        if (const StructType *STy = I.getStructTypeOrNull()) {
            const NEATStructLayout *SLO = m_NTD.getStructLayout(STy);

            GenericValue IdxGV = IP.getOperandValueAdapter(I.getOperand(), SF);
            unsigned Index = unsigned(IdxGV.IntVal.getZExtValue());

            Total += SLO->getElementOffset(Index);
        } else {
            // Get the index number for the array... which must be long type...
            GenericValue IdxGV = IP.getOperandValueAdapter(I.getOperand(), SF);

            int64_t Idx;
            unsigned BitWidth =
                cast<IntegerType>(I.getOperand()->getType())->getBitWidth();
            if (BitWidth == 32)
                Idx = (int64_t)(int32_t)IdxGV.IntVal.getZExtValue();
            else {
                assert(BitWidth == 64 && "Invalid index type for getelementptr");
                Idx = (int64_t)IdxGV.IntVal.getZExtValue();
            }
            Total += m_NTD.getTypeAllocSize(I.getIndexedType())*Idx;
        }
    }

    NEATGenericValue Result;
    Result.PointerVal = ((char*)(getOperandValue(Ptr, NSF).PointerVal)) + Total;
    DEBUG(dbgs() << "[NEATPlugin]:GEP Index " << Total << " bytes.\n");
    return Result;

}

void NEATPlugIn::callFunction( Function *F,
                              const std::map<Value *, NEATGenericValue> &ArgVals )
{
    // Make a new stack frame... and fill it in.
    m_NECStack.push_back(NEATExecutionContext());

    // Special handling for external functions and OpenCL built-ins.
    if (F->isDeclaration()) {
        // process OpenCL built-in function call ...
        NEATGenericValue Result;
        // it is declaration pop newly created NEAT context back
        // NEAT will not go into this function
        m_NECStack.pop_back();
        if(DetectAndExecuteOCLBuiltins(F, ArgVals, Result))
        {
            // fill in the return value...
            // note since interpreter has not called function yet
            // its execution context is from calling function

            ExecutionContext &CallingSF = m_pECStack->back();
            NEATExecutionContext &SF = m_NECStack.back();
            if (Instruction *I = &*CallingSF.CurInst) {
                // if not void return type
                if (!I->getType()->isVoidTy())
                    SetValue(I, Result, SF);
            }
            return;
        }

        if (NEATDataLayout ::IsNEATSupported(F->getReturnType()))
        {
            throw Exception::NotImplemented("[NEATPlugin]: Not implemented "
              " external call in callFunction with floating point return type"
              + F->getName().str() );
        }

        DEBUG(dbgs() << "[NEATPlugin]: Skipped function " << F->getName() << "\n");
        // TODO: decide what to do with unsupported functions
        // throw Exception::NotImplemented("[NEATPlugin]: Not implemented external call in callFunction " + F->getNameStr() );
        return;
    }


    NEATExecutionContext &NSF = m_NECStack.back();

    // Handle non-varargs arguments...
    for (Function::arg_iterator AI = F->arg_begin(), E = F->arg_end();
        AI != E; ++AI)
    {
        Argument *A = &*AI;
        // if exist NEATValue which matches argument of function
        // then add it to NEAT context. we are going to work with it
        if(ArgVals.find(A) != ArgVals.end())
        {
            SetValue(A, (*ArgVals.find(A)).second, NSF);
        }
    }
}


void NEATPlugIn::handlePreFunctionRun( std::vector<ExecutionContext>& ECStack, InterpreterPluggable& Interp)
{
    SetCurEvent(PRE_FUNC);
    m_pECStack = &ECStack;
    m_pInterp = &Interp;

    ExecutionContext &SF = m_pECStack->back();
    Function *F = SF.CurFunction;
    callFunction(F, m_ArgValues);
}

/// copies buffer with values to NEAT buffer
template<typename T>
inline void _CopyAccurateBufferToNEATBuffer(const IMemoryObject& src, IMemoryObject& dst)
{
  BufferDesc descFP = GetBufferDescription(src.GetMemoryObjectDesc());
  std::size_t vecWidth = descFP.SizeOfVector();
  std::size_t bufLen = descFP.NumOfElements();
  BufferAccessor<T> baf(src);
  BufferAccessor<NEATValue> ba(dst);

  for(std::size_t vec = 0; vec < bufLen; ++vec)
  {
    for(std::size_t el = 0; el < vecWidth; ++el)
    {
      T val = baf.GetElem(vec, el);
      ba.GetElem(vec, el).SetAccurateVal<T>(val);
    }
    // For 3-element vectors size is 4 but only 3 are assigned with values
    // So we should set fourth element status to unwritten
    if(vecWidth == 3)
        ba.GetElem(vec, 3).SetStatus(NEATValue::UNWRITTEN);
  }
}

template <typename T>
void SetNEATValue(const char* src, char*dst)
{
    const T* srcPtr = reinterpret_cast<const T*>(src);
    NEATValue* dstPtr = reinterpret_cast<NEATValue*>(dst);
    *dstPtr = NEATValue(*srcPtr);
}

void CopyBufferElementToNEAT(const char* src, char* dst, const TypeDesc& srcElemDesc, const TypeDesc& dstElemDesc)
{
    switch(srcElemDesc.GetType())
    {
        case Validation::TVOID:
        case Validation::TBOOL:
        case Validation::TCHAR:
        case Validation::TUCHAR:
        case Validation::TSHORT:
        case Validation::TUSHORT:
        case Validation::TINT:
        case Validation::TUINT:
        case Validation::TLONG:
        case Validation::TULONG:
            {
                // Integer type. There is nothing to copy to NEAT buffer.
                break;
            }
        case Validation::THALF:
            {
                SetNEATValue<CFloat16>(src, dst);
                break;
            }
        case Validation::TFLOAT:
            {
                SetNEATValue<float>(src, dst);
                break;
            }
        case Validation::TDOUBLE:
            {
                SetNEATValue<double>(src, dst);
                break;
            }
        case Validation::TPOINTER:
            {
                throw Exception::InvalidArgument("Can't copy pointer to NEAT buffer!");
                break;
            }
        case Validation::TARRAY:
            {
                for (uint64_t i = 0; i < srcElemDesc.GetNumberOfElements(); ++i)
                {
                    CopyBufferElementToNEAT(src, dst, srcElemDesc.GetSubTypeDesc(0), dstElemDesc.GetSubTypeDesc(0));
                    src += srcElemDesc.GetSubTypeDesc(0).GetSizeInBytes();
                    dst += dstElemDesc.GetSubTypeDesc(0).GetSizeInBytes();
                }
                break;
            }
        case Validation::TVECTOR:
            {
                for (uint64_t i = 0; i < srcElemDesc.GetNumberOfElements(); ++i)
                {
                    CopyBufferElementToNEAT(src, dst, srcElemDesc.GetSubTypeDesc(0), dstElemDesc.GetSubTypeDesc(0));
                    src += srcElemDesc.GetSubTypeDesc(0).GetSizeInBytes();
                    dst += dstElemDesc.GetSubTypeDesc(0).GetSizeInBytes();
                }
                break;
            }
        case Validation::TSTRUCT:
            {
                for (uint64_t i = 0; i < srcElemDesc.GetNumOfSubTypes(); ++i)
                {
                    CopyBufferElementToNEAT(src, dst, srcElemDesc.GetSubTypeDesc(i), dstElemDesc.GetSubTypeDesc(i));
                    src += srcElemDesc.GetSubTypeDesc(i).GetSizeInBytes();
                    dst += dstElemDesc.GetSubTypeDesc(i).GetSizeInBytes();
                }
                break;
            }
        case Validation::UNSPECIFIED_TYPE:
            {
                throw Exception::InvalidArgument("Can't copy value of unspecified type to NEAT buffer!");
                break;
            }
        case Validation::INVALID_TYPE:
            {
                throw Exception::InvalidArgument("Can't copy value of invalid type to NEAT buffer!");
                break;
            }
    }
}

void CopyBufferToNEAT (const IMemoryObject& src, IMemoryObject& dst)
{
    BufferDesc srcDesc = GetBufferDescription(src.GetMemoryObjectDesc());
    BufferDesc dstDesc = GetBufferDescription(dst.GetMemoryObjectDesc());
    std::size_t bufferLen = srcDesc.NumOfElements();
    // Number of elements in ordinary buffer and buffer with NEAT must be the same.
    assert(bufferLen == dstDesc.NumOfElements());
    const char *srcPtr = static_cast<const char*>(src.GetDataPtr());
    char *dstPtr = static_cast<char*>(dst.GetDataPtr());
    std::size_t srcBufferSize = srcDesc.GetSizeInBytes();
    std::size_t dstBufferSize = dstDesc.GetSizeInBytes();
    for (std::size_t i = 0; i < bufferLen; ++i)
    {
        CopyBufferElementToNEAT(srcPtr, dstPtr, srcDesc.GetElementDescription(), dstDesc.GetElementDescription());
        srcPtr += srcBufferSize/bufferLen;
        dstPtr += dstBufferSize/bufferLen;
    }
}

// Function creates NEAT "mirror" of OCL program inputs
// NEAT Buffers are created with the same size as input Buffers
// Input buffers are copied to corresponding NEAT Buffers
// NEAT Values are initialized with ACCURATE flag and value from input buffer
// general idea is output BufferContainerList(BCL) should have
// the same number of buffers as input BCL
// it is done since Comparator interface
// accepts inputs BCLs with the equal number of buffers
Validation::IBufferContainer * llvm::CreateNEATBufferContainer(
                                     const Validation::IBufferContainer& BC,
                                     Validation::IBufferContainerList *out_neatBCL)
{
    IBufferContainer* outputContainer = out_neatBCL->CreateBufferContainer();

    // loop over buffers
    for (std::size_t i = 0; i < BC.GetMemoryObjectCount(); ++i)
    {
        IMemoryObject* currMemObj = BC.GetMemoryObject(i);

        if(Image::GetImageName() == currMemObj->GetName())
        {   // image
            // TODO: enable creating NEAT images
            // this is stub
            ImageDesc imageDesc = GetImageDescription(currMemObj->GetMemoryObjectDesc());
            outputContainer->CreateImage(imageDesc);
        }
        else if(Buffer::GetBufferName() == currMemObj->GetName())
        {
            BufferDesc buffDsc = GetBufferDescription(currMemObj->GetMemoryObjectDesc());
            // check data type
            // NEAT supports floating point data types
            if(buffDsc.IsFloatingPoint())
            {
                // if floating point data type
                // then copy to NEAT buffers
                // initializing NEATValues as ACCURATE and value from source buffer
                BufferDesc neatBuffDesc = buffDsc;
                neatBuffDesc.SetNeat(true);
                IMemoryObject* neatBuffer = outputContainer->CreateBuffer(neatBuffDesc);
                if (buffDsc.GetElementDescription().IsStruct())
                {
                    CopyBufferToNEAT(*currMemObj, *neatBuffer);
                }
                else
                {
                    TypeVal dType = UNSPECIFIED_TYPE;
                    if (neatBuffDesc.GetElementDescription().GetType() == TVECTOR)
                        dType = neatBuffDesc.GetElementDescription().GetSubTypeDesc(0).GetType();
                    else
                        dType = neatBuffDesc.GetElementDescription().GetType();
                    switch(dType)
                    {
                    case THALF:
                        _CopyAccurateBufferToNEATBuffer<CFloat16>(*currMemObj, *neatBuffer);
                        break;
                    case TFLOAT:
                        _CopyAccurateBufferToNEATBuffer<float>(*currMemObj, *neatBuffer);
                        break;
                    case TDOUBLE:
                        _CopyAccurateBufferToNEATBuffer<double>(*currMemObj, *neatBuffer);
                        break;
                    default:
                        throw Exception::InvalidArgument("Not supported floating point data type");
                        break;
                    }
                }
            }
            else
            {
                // if integer data type then
                // intentionally set Buffer number of vectors to one
                // and do not copy values
                BufferDesc descInt = buffDsc;
                buffDsc.SetNumOfElements(1);
                outputContainer->CreateBuffer(descInt);
            }
        } // else if("Buffer" == currMemObj->GetName())
        else
            throw Exception::InvalidArgument("Unsupported memory object");
    }
    return outputContainer;
}

void NEATPlugIn::handlePostFunctionRun()
{
    SetCurEvent(POST_FUNC);
}

#define IMPLEMENT_PTR_FLOAT_VAL(_TYPE_DM_){                                 \
    if (elemDesc.GetType() != _TYPE_DM_)                                  \
            throw Exception::InvalidArgument("Pointer type in data doesn't match kernel input type");       \
    neatVal.PointerVal = buffer->GetDataPtr();\
    out_NEATArgValues[&*arg_it] = neatVal;\
    break;}\

static void _CreateNEATBufferContainerByPtr(
                const IMemoryObject* buffer,
                const llvm::PointerType* ptrType,
                Function::arg_iterator& arg_it,
                std::map<Value *, NEATGenericValue>& out_NEATArgValues )
{
    BufferDesc buffDsc = GetBufferDescription(buffer->GetMemoryObjectDesc());
    NEATGenericValue neatVal;
    const Type *Ty = ptrType->getElementType();
    TypeDesc elemDesc = buffDsc.GetElementDescription();
    switch(Ty->getTypeID())
    {
    case Type::FloatTyID:
        IMPLEMENT_PTR_FLOAT_VAL(TFLOAT);
    case Type::DoubleTyID:
        IMPLEMENT_PTR_FLOAT_VAL(TDOUBLE);
    case Type::IntegerTyID:
        return;
    case Type::VectorTyID:
        {
            elemDesc = elemDesc.GetSubTypeDesc(0);
            // obtain type of vector element
            const Type *TyElem = dyn_cast<VectorType>(Ty)->getElementType();
            // if float add ptr to NEAT context
            if(TyElem->isFloatTy())
                IMPLEMENT_PTR_FLOAT_VAL(TFLOAT)
            else if ( TyElem->isDoubleTy() ) // if double also add to context
                IMPLEMENT_PTR_FLOAT_VAL(TDOUBLE)
            else                       // otherwise skip this buffer
                return;
        }
    case Type::StructTyID:
        neatVal.PointerVal = buffer->GetDataPtr();
        out_NEATArgValues[&*arg_it] = neatVal;
        break;
    case Type::PointerTyID:
       throw Exception::InvalidArgument("Do not support buffers of pointers");
    default:
       throw Exception::InvalidArgument("Unexpected data type");
    }
}



#define IMPLEMENT_FLOAT_VAL(_TYPE_DM_) {                                \
    NEATGenericValue neatVal;                                           \
    if (elemDesc.GetType() != _TYPE_DM_)                \
        throw Exception::InvalidArgument("Floating point type in input data doesn't match kernel argument type");       \
    if (buffDsc.NumOfElements() != 1)                                   \
        throw Exception::InvalidArgument("Number of elements in input buffer doesn't match kernel argument length");   \
    BufferAccessor<NEATValue> ba(*currBuffer);                          \
    neatVal.NEATVal = ba.GetElem(0,0);                                  \
    out_NEATArgValues[&*arg_it] = neatVal;                                \
    break;}

void llvm::CreateNEATBufferContainerMap( Function *in_F,
                                  Validation::IBufferContainer& in_BC,
                                  std::map<Value *, NEATGenericValue>& out_NEATArgValues )
{
    // check number of arguments and number of buffers in Buffer Container
    if(in_F->arg_size() != in_BC.GetMemoryObjectCount())
    {
        throw Exception::InvalidArgument("Number of buffers in BufferContainer and \
            number of arguments in function should be the same");
    }

    const std::size_t numOfArguments = in_BC.GetMemoryObjectCount();
    Function::arg_iterator arg_it = in_F->arg_begin();
    for (   std::size_t buf = 0;
        arg_it != in_F->arg_end() && buf < numOfArguments;
        ++arg_it, ++buf)
    {
        Argument* arg = &*arg_it;
        IMemoryObject* currMemObj = in_BC.GetMemoryObject(buf);

        if(Image::GetImageName() == currMemObj->GetName() )
        {
            // TODO: enable NEAT for images with write-only access mode
            continue;
        }
        else if(Buffer::GetBufferName() == currMemObj->GetName())
        {   // buffer
            Buffer * currBuffer = static_cast<Buffer*>(currMemObj);
            BufferDesc buffDsc = GetBufferDescription(currMemObj->GetMemoryObjectDesc());
            const Type *Ty = arg->getType();
            TypeDesc elemDesc = buffDsc.GetElementDescription();

            switch(Ty->getTypeID())
            {
            case Type::FloatTyID:
                IMPLEMENT_FLOAT_VAL(TFLOAT);
            case Type::DoubleTyID:
                IMPLEMENT_FLOAT_VAL(TDOUBLE);
            case Type::PointerTyID:
                _CreateNEATBufferContainerByPtr( currBuffer,
                    cast<PointerType>(Ty), arg_it, out_NEATArgValues );
                break;
            case Type::VectorTyID: {
                const VectorType* VTy = cast<VectorType>(Ty);
                if (VTy->getElementType()->isFloatingPointTy())
                {
                    NEATGenericValue neatVal;
                    BufferAccessor<NEATValue> ba(*currMemObj);
                    neatVal.NEATVec.SetWidth(VectorWidthWrapper::ValueOf(VTy->getNumElements()));
                    for(unsigned int i = 0; i < VTy->getNumElements(); ++i)
                    {
                        neatVal.NEATVec[i] = ba.GetElem(0,i);
                    }
                    out_NEATArgValues[arg] = neatVal;
                    break;
                }
                // If vector contains integer types - skip it.
                continue;
                                   }
            case Type::StructTyID:
                // TODO: implement
                throw Exception::NotImplemented("Implement struct argument by value");
                break;
            case Type::IntegerTyID:
                // ignore integer types
                continue;
            default:
                throw Exception::NotImplemented("Not supported argument");
            }
        } // else if("Buffer" == currMemObj->GetName())
        else
            throw Exception::InvalidArgument("Unsupported memory object");
    } // for (   std::size_t buf = 0;
}
bool NEATDataLayout ::IsNEATSupported( const Type *Ty )
{
// TODO: implement
    switch (Ty->getTypeID()) {
  case Type::PointerTyID:
    {
      const PointerType *PTy = dyn_cast<PointerType>(Ty);
      return IsNEATSupported(PTy->getElementType());
    }
  case Type::ArrayTyID:
    {
      const ArrayType *ATy = dyn_cast<ArrayType>(Ty);
      return IsNEATSupported(ATy->getElementType());
    }
  case Type::StructTyID:
    {
      bool ret = false;
      const StructType *STy = dyn_cast<StructType>(Ty);
      for (size_t ind = 0; ind < STy->getNumContainedTypes(); ++ind)
      {
        ret = ret || IsNEATSupported(STy->getContainedType(ind));
      }
      return ret;
    }
  case Type::FloatTyID:
    return true;
  case Type::VoidTyID:
  case Type::IntegerTyID:
    return false;
  case Type::DoubleTyID:
    return true;
  case Type::VectorTyID:
    {
      const VectorType *VTy = dyn_cast<VectorType>(Ty);
      return IsNEATSupported(VTy->getElementType());
    }
  default:
      assert(Ty->isSized() && "Cannot getTypeInfo() on a type that is unsized!");
      throw Validation::Exception::InvalidArgument("NEATDataLayout ::IsNEATSupported(): Unsupported type");
      break;
    }

    return false;
}


void NEATPlugIn::SwitchToNewBasicBlock(BasicBlock *Dest, ExecutionContext &inSF, NEATExecutionContext &NSF){

  if (!isa<PHINode>(inSF.CurBB->begin())) return;  // Nothing fancy to do

  // [LLVM 3.6 UPGRADE] FIXME: Is the copy of the execution context really needed?
  //ExecutionContext SF = inSF;
  ExecutionContext &SF = inSF;
  BasicBlock *PrevBB = SF.CurBB;      // Remember where we came from...
  SF.CurBB   = Dest;                  // Update CurBB to branch destination
  SF.CurInst = SF.CurBB->begin();     // Update new instruction ptr...

  // Loop over all of the PHI nodes in the current block, reading their inputs.
  std::vector<GenericValue> ResultValues;

  InterpreterPluggable &IP = *m_pInterp;
  for (; PHINode *PN = dyn_cast<PHINode>(SF.CurInst); ++SF.CurInst) {
    // Search for the value corresponding to this previous bb...
    int i = PN->getBasicBlockIndex(PrevBB);
    assert(i != -1 && "PHINode doesn't contain entry for predecessor?");

    // Save the incoming value for this PHI node...
    Value *IncomingValue = PN->getIncomingValue(i);
    GenericValue val = IP.getOperandValueAdapter(IncomingValue, SF);
    ResultValues.push_back(val);
  }

  // Now loop over all of the PHI nodes setting their values...
  SF.CurInst = SF.CurBB->begin();
  for (unsigned i = 0; isa<PHINode>(SF.CurInst); ++SF.CurInst, ++i) {
    PHINode *PN = cast<PHINode>(SF.CurInst);
    const Type *PNType = PN->getType();

    NEATGenericValue val;
    if(PNType->getTypeID() == Type::FloatTyID) {
        val.NEATVal = NEATValue(ResultValues[i].FloatVal);
        SetValue(PN, val, NSF);
    }
    else if(PNType->getTypeID() == Type::DoubleTyID) {
        val.NEATVal = NEATValue(ResultValues[i].DoubleVal);
        SetValue(PN, val, NSF);
    }

  }

}


void NEATPlugIn::visitBranchInst(BranchInst &I) {
  HANDLE_EVENT(PRE_INST);

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");
  NEATExecutionContext &NSF = m_NECStack.back();
  ExecutionContext &SF = m_pECStack->back();
  InterpreterPluggable &IP = *m_pInterp;
  BasicBlock *Dest;

  Dest = I.getSuccessor(0);          // Uncond branches have a fixed dest...
  if (!I.isUnconditional()) {
    Value *Cond = I.getCondition();
    if (IP.getOperandValueAdapter(Cond, SF).IntVal == 0)  // If false cond...
      Dest = I.getSuccessor(1);
  }
  SwitchToNewBasicBlock(Dest, SF, NSF);
}


void NEATPlugIn::visitReturnInst( ReturnInst &I )
{
  // handle only post instruction execution
  HANDLE_EVENT(PRE_INST);
  NEATExecutionContext &SF = m_NECStack.back();
  const Type *RetTy = Type::getVoidTy(I.getContext());
  NEATGenericValue Result;

  // Save away the return value... (if we are not 'ret void')
  if (I.getNumOperands()) {
    RetTy  = I.getReturnValue()->getType();
    // we can get I.getReturnValue()->getType() only if I.getNumOperands() is true,
    // if I.getNumOperands() is false, I.getReturnValue()->getType() is empty
    if (m_NTD.IsNEATSupported(RetTy)) {
        Result = getOperandValue(I.getReturnValue(), SF);
    }
  }

  // if return value is not NEAT supported, make popStackAndReturnValueToCaller to pop
  // the current stack frame, but ignore result
  popStackAndReturnValueToCaller(RetTy, Result);
}

void NEATPlugIn::visitBinaryOperator( BinaryOperator &I )
{
  HANDLE_EVENT(POST_INST);
  // if NEAT does not support this type - exit
  if(!m_NTD.IsNEATSupported(I.getType())) return;

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();

  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  NEATGenericValue Result;

  // Macros to execute binary operation 'OP' over floating point type TY
  // (float or double) vectors
  // frm is fast_relaxed_math extension
#define FLOAT_VECTOR_FUNCTION_SUFX(OP, TY, SUFX)                                 \
  switch (TY->getTypeID())                                            \
  {                                                                   \
  case Type::FloatTyID:                                               \
        Result.NEATVec = OP##SUFX##f(Src1.NEATVec, Src2.NEATVec);          \
        break;                                                        \
  case Type::DoubleTyID:                                              \
        Result.NEATVec = OP##_d(Src1.NEATVec, Src2.NEATVec);          \
    break;                                                            \
  default:                                                            \
    throw InvalidArgument("visitBinaryOperator: unsupported type");   \
  }

#define FLOAT_VECTOR_FUNCTION(OP, TY) FLOAT_VECTOR_FUNCTION_SUFX(OP, TY, _ )

  const Type *Ty    = I.getOperand(0)->getType();
  if (Ty->isVectorTy())
  {
          switch(I.getOpcode()){
          case Instruction::FAdd:  FLOAT_VECTOR_FUNCTION(NEAT_WRAP::add, dyn_cast<VectorType>(Ty)->getElementType()) break;
          case Instruction::FSub:  FLOAT_VECTOR_FUNCTION(NEAT_WRAP::sub, dyn_cast<VectorType>(Ty)->getElementType()) break;
              // if command line parameter of SATest --fma-neat set on, function mul_fma used to perform multiplication,
              // if --fma-neat set off, commom mul is used. common mul has correctly rounded result, fma_mul makes resulting
              // adds 1 ULP to min and max values of resulting interval.
          case Instruction::FMul:  if(m_bUseFmaNEAT) { FLOAT_VECTOR_FUNCTION(NEAT_WRAP::mul_fma, dyn_cast<VectorType>(Ty)->getElementType()) }
                                   else { FLOAT_VECTOR_FUNCTION(NEAT_WRAP::mul, dyn_cast<VectorType>(Ty)->getElementType()) } break;
          case Instruction::FDiv:  if(isFRMPrecisionOn()){ FLOAT_VECTOR_FUNCTION_SUFX(NEAT_WRAP::div, dyn_cast<VectorType>(Ty)->getElementType(), _frm_)
                                   }else { FLOAT_VECTOR_FUNCTION(NEAT_WRAP::div, dyn_cast<VectorType>(Ty)->getElementType()) } break;
          case Instruction::FRem:  FLOAT_VECTOR_FUNCTION(NEAT_WRAP::fmod, dyn_cast<VectorType>(Ty)->getElementType()) break;
          default:
              throw InvalidArgument("visitBinaryOperator: unsupported vector operator");
          }
  }
  else
  {
#define FLOAT_OPERATOR_SUFX(OP, TY, SUFX)                         \
  switch (TY->getTypeID())                             \
    {                                                  \
  case Type::FloatTyID:                                \
        Result.NEATVal = OP##SUFX##f(Src1.NEATVal, Src2.NEATVal); \
    break;                                               \
  case Type::DoubleTyID:                               \
        Result.NEATVal = OP##_d(Src1.NEATVal, Src2.NEATVal); \
    break;\
  default:                                           \
    throw InvalidArgument("visitBinaryOperator: unsupported type"); \
  }
#define FLOAT_OPERATOR(OP, TY) FLOAT_OPERATOR_SUFX(OP, TY, _ )

    switch(I.getOpcode()){
    case Instruction::FAdd:  FLOAT_OPERATOR(NEAT_WRAP::add, Ty) break;
    case Instruction::FSub:  FLOAT_OPERATOR(NEAT_WRAP::sub, Ty) break;
    case Instruction::FMul:  if(m_bUseFmaNEAT) { FLOAT_OPERATOR(NEAT_WRAP::mul_fma, Ty) } else {FLOAT_OPERATOR(NEAT_WRAP::mul, Ty)} break;
    case Instruction::FDiv:  if(isFRMPrecisionOn()) { FLOAT_OPERATOR_SUFX(NEAT_WRAP::div, Ty, _frm_) } else { FLOAT_OPERATOR(NEAT_WRAP::div, Ty) } break;
    case Instruction::FRem:  FLOAT_OPERATOR(NEAT_WRAP::fmod, Ty) break;
    default:
      throw InvalidArgument("visitBinaryOperator: unsupported scalar operator");
    }
  }


    // set result value
  SetValue(&I, Result, SF);

}

void NEATPlugIn::visitExtractElementInst( ExtractElementInst &I )
{
  HANDLE_EVENT(PRE_INST);
  // if NEAT does not support this type - exit
  if(!m_NTD.IsNEATSupported(I.getType())) return;

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();

  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue Result;

  GenericValue IdxGV = m_pInterp->getOperandValueAdapter(I.getOperand(1), m_pECStack->back());
  const unsigned indx = unsigned(IdxGV.IntVal.getZExtValue());

  // TODO: template parameter is meaningless here. Remove it once it deleted from interface.
  Result.NEATVal = NEAT_WRAP::extractelement_fd(Src1.NEATVec, indx);
  SetValue(&I, Result, SF);
}

void NEATPlugIn::visitInsertElementInst( InsertElementInst &I )
{
  HANDLE_EVENT(PRE_INST);
  // if NEAT does not support this type - exit
  if(!m_NTD.IsNEATSupported(I.getType())) return;

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();

  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  NEATGenericValue Result;

  GenericValue IdxGV = m_pInterp->getOperandValueAdapter(I.getOperand(2), m_pECStack->back());
  const unsigned indx = unsigned(IdxGV.IntVal.getZExtValue());

  // TODO: template parameter is meaningless here. Remove it once it deleted from interface.
  Result.NEATVec = NEAT_WRAP::insertelement_fd(Src1.NEATVec, Src2.NEATVal, indx);
  SetValue(&I, Result, SF);
}

void NEATPlugIn::visitShuffleVectorInst( ShuffleVectorInst &I )
{
  HANDLE_EVENT(PRE_INST);
  // if NEAT does not support this type - exit
  if(!m_NTD.IsNEATSupported(I.getType())) return;

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();

  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  NEATGenericValue Dest;

  GenericValue MaskGV = m_pInterp->getOperandValueAdapter(I.getOperand(2), m_pECStack->back());
  std::vector<unsigned> mask_vec;
  for (unsigned i = 0; i < MaskGV.AggregateVal.size(); ++i)
  {
    mask_vec.push_back((unsigned)(MaskGV.AggregateVal[i].IntVal.getZExtValue()));
  }

  Dest.NEATVec = NEAT_WRAP::shufflevector_fd(Src1.NEATVec, Src2.NEATVec, mask_vec);

  // mask vector is an integer vector got from interpreter
  // The shuffle mask operand is required to be a constant vector
  // with either constant integer or undef values
  Constant *CPV = dyn_cast<Constant>(I.getOperand(2));
  assert(CPV != NULL);
  assert(CPV->getType()->getTypeID() == Type::VectorTyID);
  if(ConstantVector *CV = dyn_cast<ConstantVector>(CPV)) {
      VectorType* VTy = dyn_cast<VectorType>(CPV->getType());
      assert(VTy->getElementType()->isIntegerTy());
      unsigned elemNum = VTy->getNumElements();
      std::vector<unsigned> undef_vec(elemNum,0);

      bool isAnyUndef = false;
      for (unsigned i = 0; i < elemNum; ++i){
          if (isa<UndefValue>(CV->getOperand(i))) {
              isAnyUndef = true;
              undef_vec[i] = 1; // mark undef values
          }
      }

      if(isAnyUndef) {
          if( elemNum != (unsigned)(Dest.NEATVec.GetSize()) ) {
              throw Exception::NEATTrackFailure("NEATPlugin::shufflevector. Wrong mask vector size");
          }
          for (unsigned i = 0; i < (unsigned)(Dest.NEATVec.GetSize()); ++i) {
              if(undef_vec[i]) {
                  Dest.NEATVec[i] = NEATValue(NEATValue::ANY);
              }
          }
      }
  }

  SetValue(&I, Dest, SF);
}

void NEATPlugIn::visitFCmpInst( FCmpInst &I )
{
  HANDLE_EVENT(PRE_INST);
  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();
  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  NEATGenericValue R;
  const Type *Ty    = I.getOperand(0)->getType();

#define COMPARE(TYPE, VALUE, Y)                                                         \
  if (TYPE->getTypeID() == Type::FloatTyID)                                             \
  {                                                                                     \
  R.NEAT##VALUE = NEAT_WRAP::fcmp_f(Src1.NEAT##VALUE, Src2.NEAT##VALUE, CMP_##Y);       \
  }                                                                                     \
  if (TYPE->getTypeID() == Type::DoubleTyID)                                            \
  {                                                                                     \
  R.NEAT##VALUE = NEAT_WRAP::fcmp_d(Src1.NEAT##VALUE, Src2.NEAT##VALUE, CMP_##Y);       \
  }

#define CASE(X)                                                         \
  case FCmpInst::FCMP_##X:                                              \
  if (Ty->isVectorTy())                                                 \
  {                                                                     \
    COMPARE(dyn_cast<VectorType>(Ty)->getElementType(), Vec, X)         \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    COMPARE(Ty, Val, X)                                                 \
  }                                                                     \
  break;

  switch (I.getPredicate()) {
  CASE(FALSE)
  CASE(TRUE)
  CASE(ORD)
  CASE(UNO)
  CASE(UEQ)
  CASE(OEQ)
  CASE(UNE)
  CASE(ONE)
  CASE(ULT)
  CASE(OLT)
  CASE(UGT)
  CASE(OGT)
  CASE(ULE)
  CASE(OLE)
  CASE(UGE)
  CASE(OGE)
  default:
    throw InvalidArgument("Don't know how to handle this FCmp predicate!");
  }

  SetValue(&I, R, SF);
}

void NEATPlugIn::visitSelectInst( SelectInst &I )
{
  HANDLE_EVENT(PRE_INST);
  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  if (!m_NTD.IsNEATSupported(I.getOperand(1)->getType()))
      return;

  NEATExecutionContext &SF = m_NECStack.back();
  const Type *Ty    = I.getOperand(0)->getType();
  GenericValue MaskGV = m_pInterp->getOperandValueAdapter(I.getOperand(0), m_pECStack->back());
  NEATGenericValue Src2 = getOperandValue(I.getOperand(1), SF);
  NEATGenericValue Src3 = getOperandValue(I.getOperand(2), SF);
  NEATGenericValue R;
  if (Ty->isVectorTy())
  {
    if (cast<VectorType>(Ty)->getElementType()->isIntegerTy())
    {
      return;
    }
    std::vector<bool> select_mask;
    for (unsigned i = 0; i < MaskGV.AggregateVal.size(); ++i)
    {
      bool m = MaskGV.AggregateVal[i].IntVal != 0;
      select_mask.push_back(m);
    }
    R.NEATVec = NEAT_WRAP::select_fd(select_mask, Src2.NEATVec, Src3.NEATVec);

  }
  else
  {
    if (I.getOperand(1)->getType()->isVectorTy())
    {
      R.NEATVec = NEAT_WRAP::select_fd(MaskGV.IntVal != 0, Src2.NEATVec, Src3.NEATVec);
    }
    else
    {
      R.NEATVal = NEAT_WRAP::select_fd(MaskGV.IntVal != 0, Src2.NEATVal, Src3.NEATVal);
    }
  }

  SetValue(&I, R, SF);
}

void NEATPlugIn::visitFPTruncInst( FPTruncInst &I )
{
  HANDLE_EVENT(PRE_INST);
  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();
  const Type *Ty    = I.getOperand(0)->getType();
  if (Ty->getTypeID() != Type::DoubleTyID && I.getType()->getTypeID() != Type::FloatTyID)
  {
    throw InvalidArgument("NEATPlugin:FPTrunc could truncate only double to float");
  }
  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue R;
  R.NEATVal = NEAT_WRAP::fptrunc_d2f(Src1.NEATVal);
  SetValue(&I, R, SF);
}

void NEATPlugIn::visitFPExtInst( FPExtInst &I )
{
  HANDLE_EVENT(PRE_INST);
  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();
  const Type *Ty    = I.getOperand(0)->getType();
  if (Ty->getTypeID() != Type::FloatTyID && I.getType()->getTypeID() != Type::DoubleTyID)
  {
    throw InvalidArgument("NEATPlugin:FPExt could extend only float to double");
  }
  NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
  NEATGenericValue R;
  R.NEATVal = NEAT_WRAP::fpext_f2d(Src1.NEATVal);
  SetValue(&I, R, SF);
}

// Bitcasts vector of integers (vec) to one integer value.
// vec - vector of integer to bitcast.
// resBitLen - length of resulting integer in bits.
// vecLen - number of elements in vec.
// isLittleEndian - true if target machine is little-endian.
APInt BitcastIntVectorToIntScalar(GenericValue& vec, unsigned int resBitLen, unsigned vecLen, bool isLittleEndian)
{
    APInt result;
    // Initialization of the result.
    result = result.zext(resBitLen);
    result = 0;
    unsigned int vecElementBitWidth = vec.AggregateVal[0].IntVal.getBitWidth();
    unsigned int ShiftAmt = isLittleEndian ? 0 : vecElementBitWidth*(vecLen-1);
    for (unsigned int i = 0; i < vecLen; ++i) {
        APInt Tmp;
        Tmp.zext(vecElementBitWidth);
        Tmp = vec.AggregateVal[i].IntVal;
        Tmp.zext(resBitLen);
        Tmp = Tmp.shl(ShiftAmt);
        ShiftAmt += isLittleEndian ? vecElementBitWidth : -vecElementBitWidth;
        result |= Tmp;
    }
    return result;
}

void NEATPlugIn::visitBitCastInst( BitCastInst &I )
{
    HANDLE_EVENT(PRE_INST);
    DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

    NEATExecutionContext &SF = m_NECStack.back();
    const Type *SrcTy    = I.getOperand(0)->getType();
    const Type *DstTy    = I.getType();
    NEATGenericValue Src1 = getOperandValue(I.getOperand(0), SF);
    NEATGenericValue R;

    // To know byte order on target machine.
    bool isLittleEndian = m_pInterp->getDataLayout().isLittleEndian();
    // Ignore operation that doesn't involve NEAT supported data types
    if (!m_NTD.IsNEATSupported(DstTy) && !m_NTD.IsNEATSupported(SrcTy))
        return;

    // handle the same type bitcast
    if (SrcTy->getTypeID() == DstTy->getTypeID())
    {
        if (SrcTy->isFloatingPointTy()) // example: bitcast float value to float value
        {
            R.NEATVal = Src1.NEATVal;
            SetValue(&I, R, SF);
            return;
        }
        // handle pointer to pointer conversion
        else if(SrcTy->getTypeID() == Type::PointerTyID)
        {
            // allowed cases: float* to float*, [n x float]* to float*, <n x float>* to float*
            // float* to [n x float]*, float* to <n x float>* and the same for doubles
            const Type *localSrcTy = dyn_cast<PointerType>(SrcTy)->getElementType();
            const Type *localDstTy = dyn_cast<PointerType>(DstTy)->getElementType();

            // float* to float*, double* to double*
            if((localSrcTy->getTypeID() == Type::FloatTyID && localDstTy->getTypeID() == Type::FloatTyID) ||
                (localSrcTy->getTypeID() == Type::DoubleTyID && localDstTy->getTypeID() == Type::DoubleTyID))
            {
                DEBUG(dbgs() << "[NEATPlugin] warning : bitcast pointer to pointer\n");
                R.PointerVal = Src1.PointerVal;
                SetValue(&I, R, SF);
                return;
            }

            // <n x float>* to float*, <n x double>* to double*
            if(localSrcTy->getTypeID() == Type::VectorTyID) {
                Type::TypeID VecTypeID = dyn_cast<VectorType>(localSrcTy)->getElementType()->getTypeID();
                if((VecTypeID == Type::FloatTyID && localDstTy->getTypeID() == Type::FloatTyID) ||
                    (VecTypeID == Type::DoubleTyID && localDstTy->getTypeID() == Type::DoubleTyID)){
                        DEBUG(dbgs() << "[NEATPlugin] warning : bitcast pointer to pointer\n");
                        R.PointerVal = Src1.PointerVal;
                        SetValue(&I, R, SF);
                        return;
                }
            }

            // [n x float]* to float*, [n x double]* to double*
            if(localSrcTy->getTypeID() == Type::ArrayTyID) {
                Type::TypeID ArrTypeID = dyn_cast<ArrayType>(localSrcTy)->getElementType()->getTypeID();
                if((ArrTypeID == Type::FloatTyID && localDstTy->getTypeID() == Type::FloatTyID) ||
                    (ArrTypeID == Type::DoubleTyID && localDstTy->getTypeID() == Type::DoubleTyID)) {
                        DEBUG(dbgs() << "[NEATPlugin] warning : bitcast pointer to pointer\n");
                        R.PointerVal = Src1.PointerVal;
                        SetValue(&I, R, SF);
                        return;
                }
            }

            // float* to <n x float>*, double* to <n x double>*
            if(localDstTy->getTypeID() == Type::VectorTyID) {
                Type::TypeID VecTypeID = dyn_cast<VectorType>(localDstTy)->getElementType()->getTypeID();
                if((VecTypeID == Type::FloatTyID && localSrcTy->getTypeID() == Type::FloatTyID) ||
                    (VecTypeID == Type::DoubleTyID && localSrcTy->getTypeID() == Type::DoubleTyID)){
                        DEBUG(dbgs() << "[NEATPlugin] warning : bitcast pointer to pointer\n");
                        R.PointerVal = Src1.PointerVal;
                        SetValue(&I, R, SF);
                        return;
                }
            }

            // float* to [n x float]*, double* to [n x double]*
            if(localDstTy->getTypeID() == Type::ArrayTyID) {
                Type::TypeID ArrTypeID = dyn_cast<ArrayType>(localDstTy)->getElementType()->getTypeID();
                if((ArrTypeID == Type::FloatTyID && localSrcTy->getTypeID() == Type::FloatTyID) ||
                    (ArrTypeID == Type::DoubleTyID && localSrcTy->getTypeID() == Type::DoubleTyID)){
                        DEBUG(dbgs() << "[NEATPlugin] warning : bitcast pointer to pointer\n");
                        R.PointerVal = Src1.PointerVal;
                        SetValue(&I, R, SF);
                        return;
                }
            }

            // <n x float>* to <m x float>*, <n x double>* to <m x double>*
            if(localSrcTy->getTypeID() == Type::VectorTyID && localDstTy->getTypeID() == Type::VectorTyID) {
                Type::TypeID VecSrcTypeID = dyn_cast<VectorType>(localSrcTy)->getElementType()->getTypeID();
                Type::TypeID VecDstTypeID = dyn_cast<VectorType>(localDstTy)->getElementType()->getTypeID();
                if((VecSrcTypeID == Type::FloatTyID && VecDstTypeID == Type::FloatTyID) ||
                   (VecSrcTypeID == Type::DoubleTyID && VecDstTypeID == Type::DoubleTyID)) {
                        DEBUG(dbgs() << "[NEATPlugin] warning : bitcast pointer to pointer\n");
                        R.PointerVal = Src1.PointerVal;
                        SetValue(&I, R, SF);
                        return;
                }
            }

            // if we are here, we don't know how to bitcast
            throw Exception::NEATTrackFailure("NEATPlugin:Invalid arguments for bitcast instruction. Can't bitcast pointer.");
        }
        // vector to vector bitcast
        else if (SrcTy->getTypeID() == Type::VectorTyID)
        {
            Type::TypeID DstTypeID = dyn_cast<VectorType>(DstTy)->getElementType()->getTypeID();
            Type::TypeID SrcTypeID = dyn_cast<VectorType>(SrcTy)->getElementType()->getTypeID();
            if (SrcTypeID == Type::FloatTyID) {
                // <2n x float> to <n x double>
                if (DstTypeID == Type::DoubleTyID) {
                    R.NEATVec = NEAT_WRAP::bitcast_f2d_vec(Src1.NEATVec);
                } else if (DstTypeID == Type::FloatTyID) {
                    // <n x float> to <n x float> is stupid, but maybe it is possible
                    R.NEATVec = Src1.NEATVec;
                } else if (DstTypeID == Type::IntegerTyID) {
                    for (size_t i = 0; i < Src1.NEATVec.GetSize(); ++i)
                    {
                        if (!Src1.NEATVec[i].IsAcc())
                            throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast that floating point value to the integer without ambiguity.");
                    }
                } else {
                    throw Exception::NEATTrackFailure("NEATPlugin: Unsupported type of arguments for bitcast instruction.");
                }
            } else if (SrcTypeID == Type::DoubleTyID) {
                // <n x double> to <2n x float>
                if (DstTypeID == Type::FloatTyID) {
                    R.NEATVec = NEAT_WRAP::bitcast_d2f(Src1.NEATVec);
                } else if (DstTypeID == Type::DoubleTyID) {
                    // <n x double> to <n x double>
                    R.NEATVec = Src1.NEATVec;
                } else if (DstTypeID == Type::IntegerTyID) {
                    for (size_t i = 0; i < Src1.NEATVec.GetSize(); ++i)
                    {
                        if (!Src1.NEATVec[i].IsAcc())
                            throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast that floating point value to the integer without ambiguity.");
                    }
                } else {
                    throw Exception::NEATTrackFailure("NEATPlugin: Unsupported type of arguments for bitcast instruction.");
                }
            } else if (SrcTypeID == Type::IntegerTyID)  {

                DEBUG(dbgs() << "[NEATPlugin] warning : bitcast vector of integers to NEAT vector\n");
                // <n x int32> to <n x float> or <n x int64> to <n x double>
                const unsigned int srcNum = cast<VectorType>(SrcTy)->getNumElements();
                const unsigned int srcBitWidth = cast<VectorType>(SrcTy)->getElementType()->getPrimitiveSizeInBits();
                const unsigned int dstNum = cast<VectorType>(DstTy)->getNumElements();

                R.NEATVec.SetWidth(VectorWidthWrapper::ValueOf(dstNum));
                GenericValue GV = m_pInterp->getOperandValueAdapter(I.getOperand(0), m_pECStack->back());

                if (DstTypeID == Type::FloatTyID)
                {
                    if (dstNum == srcNum) { // bitcast i32 to float
                        for( unsigned int i = 0; i< dstNum; ++i ) {
                            float fVal = float(GV.AggregateVal[i].IntVal.bitsToFloat());
                            R.NEATVec[i] = NEATValue(fVal);
                        }
                    } else if (dstNum > srcNum) { // bitcast i64 to float
                        const unsigned int ratio = dstNum/srcNum;
                        assert (ratio == 2);    // i64 only is supported at the moment.
                        for (unsigned int i = 0; i < srcNum; ++i) {
                            APInt intToBitcast = GV.AggregateVal[i].IntVal;
                            for (unsigned int j = 0; j < ratio; ++j) {
                                float fVal = float(intToBitcast.lshr(j*32).bitsToFloat());
                                R.NEATVec[i*ratio + j] = NEATValue(fVal);
                            }
                        }
                    } else { //(dstNum > srcNum)  // bitcast i8 or i16 to float
                        const unsigned int ratio = srcNum/dstNum;
                        for (unsigned int i = 0; i < dstNum; ++i) {
                            APInt intToBitcast;
                            intToBitcast = intToBitcast.zext(32); // extend to a size of float
                            intToBitcast = 0;
                            unsigned int ShiftAmt = isLittleEndian ? 0 : srcBitWidth*(ratio-1);
                            for (unsigned int j = 0; j < ratio; ++j) {
                                APInt Tmp;
                                Tmp.zext(srcBitWidth);
                                Tmp = GV.AggregateVal[i*ratio + j].IntVal;
                                Tmp.zext(32);
                                Tmp = Tmp.shl(ShiftAmt);
                                ShiftAmt += isLittleEndian ? srcBitWidth : -srcBitWidth;
                                intToBitcast |= Tmp;
                            }
                            float fVal = float(intToBitcast.bitsToFloat());
                            R.NEATVec[i] = NEATValue(fVal);
                        }
                    }
                } else if (DstTypeID == Type::DoubleTyID)
                {
                    if (dstNum == srcNum) {  // bitcast i64 to double
                        for( unsigned int i = 0; i< dstNum; i++ ) {
                            double dVal = double(GV.AggregateVal[i].IntVal.bitsToDouble());
                            R.NEATVec[i] = NEATValue(dVal);
                        }
                    } else { //(dstNum > srcNum)  // bitcast i8, i16 or i32 to double
                        const unsigned int ratio = srcNum/dstNum;
                        for (unsigned int i = 0; i < dstNum; ++i) {
                            APInt intToBitcast;
                            intToBitcast = intToBitcast.zext(64); // extend to a size of float
                            intToBitcast = 0;
                            unsigned int ShiftAmt = isLittleEndian ? 0 : srcBitWidth*(ratio-1);
                            for (unsigned int j = 0; j < ratio; ++j) {
                                APInt Tmp;
                                Tmp.zext(srcBitWidth);
                                Tmp = GV.AggregateVal[i*ratio + j].IntVal;
                                Tmp.zext(64);
                                Tmp = Tmp.shl(ShiftAmt);
                                ShiftAmt += isLittleEndian ? srcBitWidth : -srcBitWidth;
                                intToBitcast |= Tmp;
                            }
                            double dVal = double(intToBitcast.bitsToDouble());
                            R.NEATVec[i] = NEATValue(dVal);
                        }
                    }
                } else
                    throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast to different bit width.");

            } else
                throw Exception::NEATTrackFailure("NEATPlugin: Unsupported type of arguments for bitcast instruction.");

            SetValue(&I, R, SF);
            return;
        } // if (SrcTy->getTypeID() == Type::VectorTyID)
    } // if (SrcTy->getTypeID() == DstTy->getTypeID())

    // double value to <2 x float>
    if (SrcTy->getTypeID() == Type::DoubleTyID && DstTy->getTypeID() == Type::VectorTyID)
    {
        if (dyn_cast<VectorType>(DstTy)->getElementType()->getTypeID() == Type::FloatTyID)
        {
            R.NEATVec = NEAT_WRAP::bitcast_d2f(Src1.NEATVal);
            SetValue(&I, R, SF);
        } else if (dyn_cast<VectorType>(DstTy)->getElementType()->getTypeID() == Type::IntegerTyID) {
            if (!Src1.NEATVal.IsAcc()) {
                throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast that floating point value to the integer without ambiguity.");
            }
        }
        return;
    }

    if (SrcTy->getTypeID() == Type::VectorTyID)
    {
        Type::TypeID SrcTypeID = dyn_cast<VectorType>(SrcTy)->getElementType()->getTypeID();
        if (SrcTypeID == Type::FloatTyID)
        {
            // <2 x float> to double value
            if (DstTy->getTypeID() == Type::DoubleTyID) {
                R.NEATVal = NEAT_WRAP::bitcast_f2d_val(Src1.NEATVec);
            } else if (DstTy->getTypeID() == Type::IntegerTyID) { // <2 x float> to i64
                for (size_t i = 0; i < Src1.NEATVec.GetSize(); ++i) {
                    if (!Src1.NEATVec[i].IsAcc()) {
                        throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast that floating point value to the integer without ambiguity.");
                    }
                }
            }
        } else if (SrcTypeID == Type::IntegerTyID) {
            GenericValue GV = m_pInterp->getOperandValueAdapter(I.getOperand(0), m_pECStack->back());
            const unsigned int srcNum = cast<VectorType>(SrcTy)->getNumElements();

            // <4 x i8> or <2 x i16> to float value
            if (DstTy->getTypeID() == Type::FloatTyID) {
                float fVal = float(BitcastIntVectorToIntScalar(GV, 32, srcNum, isLittleEndian).bitsToFloat());
                R.NEATVal = NEATValue(fVal);
            } else if (DstTy->getTypeID() == Type::DoubleTyID) {
                double dVal = double(BitcastIntVectorToIntScalar(GV, 64, srcNum, isLittleEndian).bitsToDouble());
                R.NEATVal = NEATValue(dVal);
            }
        } else {
            throw Exception::NEATTrackFailure("NEATPlugin:Invalid arguments for bitcast instruction. Can't bitcast to non-floating point data types.");
        }
        SetValue(&I, R, SF);
        return;
    }

    // int32 to float, int64 to double
    if (SrcTy->getTypeID() == Type::IntegerTyID)
    {
        unsigned int srcBitWidth = cast<IntegerType>(SrcTy)->getBitWidth();
        GenericValue GV = m_pInterp->getOperandValueAdapter(I.getOperand(0), m_pECStack->back());

        if (DstTy->getTypeID() == Type::FloatTyID && srcBitWidth == 32) // bitcast i32 to float
        {
            float fVal = float(GV.IntVal.bitsToFloat());
            R.NEATVal = NEATValue(fVal);
        } else if (DstTy->getTypeID() == Type::DoubleTyID && srcBitWidth == 64)  // bitcast i64 to double
        {
            double dVal = double(GV.IntVal.bitsToDouble());
            R.NEATVal = NEATValue(dVal);
        } else if (DstTy->getTypeID() == Type::VectorTyID) {
            assert (srcBitWidth == 64);    // i64 only is supported at the moment.
            assert(cast<VectorType>(DstTy)->getElementType()->isFloatTy());
            APInt intToBitcast = GV.IntVal;
            R.NEATVec.SetWidth(VectorWidthWrapper::ValueOf(cast<VectorType>(DstTy)->getNumElements()));
            for (unsigned int j = 0; j < srcBitWidth/32; ++j) {
                float fVal = float(intToBitcast.lshr(j*32).bitsToFloat());
                R.NEATVec[j] = NEATValue(fVal);
            }
        } else
            throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast to different bit width.");

        SetValue(&I, R, SF);
        return;
    }

  // float to int32, double to int64
  if (SrcTy->isFloatingPointTy())
  {
      if (DstTy->isIntegerTy() || (DstTy->isVectorTy() && dyn_cast<VectorType>(DstTy)->getElementType()->isIntegerTy()))
      {
          if (!Src1.NEATVal.IsAcc())
              throw Exception::NEATTrackFailure("NEATPlugin: Can't bitcast that floating point value to the integer without ambiguity.");
      }
      return;
  }

  // if we are here, we don't know how to bitcast
    throw Exception::NEATTrackFailure("NEATPlugin:Invalid arguments for bitcast instruction.");
}

void NEATPlugIn::visitExtractValueInst( ExtractValueInst &I )
{
  HANDLE_EVENT(PRE_INST);

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  // if NEAT does not support this type - exit
  if(!m_NTD.IsNEATSupported(I.getType())) return;

  NEATExecutionContext &SF = m_NECStack.back();
  Value *Agg = I.getAggregateOperand();
  NEATGenericValue Dest, TmpVal;
  NEATGenericValue Src = getOperandValue(Agg, SF);

  ExtractValueInst::idx_iterator IdxBegin = I.idx_begin();
  unsigned Num = I.getNumIndices();
  ArrayRef<unsigned> neatIndex;

  for (unsigned i = 0 ; i < Num; ++i) {
    unsigned idx = *IdxBegin;
    ++IdxBegin;
    Type *IndexedType = ExtractValueInst::getIndexedType(Agg->getType(), I.getIndices().slice(i));
    if (IndexedType->isVectorTy() && (dyn_cast<VectorType>(IndexedType)->getElementType()->isFloatTy() || dyn_cast<VectorType>(IndexedType)->getElementType()->isDoubleTy())) {
      TmpVal.NEATVal = Src.NEATVec[idx];
    } else {
      TmpVal = Src.AggregateVal[idx];
    }
    Src = TmpVal;
  }

  Type *IndexedType = ExtractValueInst::getIndexedType(Agg->getType(), I.getIndices());
  switch (IndexedType->getTypeID()) {
        case Type::FloatTyID:
        case Type::DoubleTyID:
          Dest.NEATVal = TmpVal.NEATVal;
          break;
        case Type::ArrayTyID:
        case Type::StructTyID:
          Dest.AggregateVal = TmpVal.AggregateVal;
          break;
        case Type::VectorTyID:
          Dest.NEATVec = TmpVal.NEATVec;
          break;
        case Type::PointerTyID:
          Dest.PointerVal = TmpVal.PointerVal;
          break;
        case Type::IntegerTyID:
          break;
        default:
          DEBUG(dbgs() << "Unhandled destination type for extractelement instruction: " << *IndexedType << "\n");
          llvm_unreachable(0);
  }

  SetValue(&I, Dest, SF);

}

void NEATPlugIn::visitInsertValueInst( InsertValueInst &I )
{
  HANDLE_EVENT(PRE_INST);

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  // if NEAT does not support this type - exit
  if(!m_NTD.IsNEATSupported(I.getType())) return;

  NEATExecutionContext &SF = m_NECStack.back();
  Value *Agg = I.getAggregateOperand();
  NEATGenericValue Dest,*pTmpVal, *pDest;
  NEATGenericValue Src1 = getOperandValue(Agg, SF);
  NEATGenericValue Src2 = getOperandValue(I.getOperand(1), SF);

  Dest = Src1; // Dest is a slightly changed Src1

  ExtractValueInst::idx_iterator IdxBegin = I.idx_begin();
  unsigned Num = I.getNumIndices();

  pDest = &Dest;
  for (unsigned i = 0 ; i < Num; ++i) {
    unsigned idx = *IdxBegin;
    ++IdxBegin;
    const Type *IndexedType = ExtractValueInst::getIndexedType(Agg->getType(), I.getIndices().slice(i));
    if (IndexedType->isVectorTy() && (dyn_cast<VectorType>(IndexedType)->getElementType()->isFloatTy() || dyn_cast<VectorType>(IndexedType)->getElementType()->isDoubleTy())) {
      pTmpVal= (NEATGenericValue*)&pDest->NEATVec[idx];
    } else {
      pTmpVal = &pDest->AggregateVal[idx];
    }
    pDest = pTmpVal;
  }

  for (unsigned i = 0 ; i < Num; ++i) {
    unsigned idx = *IdxBegin;
    ++IdxBegin;
    pTmpVal = &pDest->AggregateVal[idx];
    pDest = pTmpVal;
  }
  // pDest points to the target value in the Dest now

  Type *IndexedType = ExtractValueInst::getIndexedType(Agg->getType(), I.getIndices());

  switch (IndexedType->getTypeID()) {
    case Type::IntegerTyID:
      break;
    case Type::FloatTyID:
    case Type::DoubleTyID:
      *((NEATValue*)pDest) = Src2.NEATVal;
      break;
    case Type::ArrayTyID:
    case Type::StructTyID:
      pDest->AggregateVal = Src2.AggregateVal;
      break;
    case Type::VectorTyID:
      pDest->NEATVec = Src2.NEATVec;
      break;
    case Type::PointerTyID:
      pDest->PointerVal = Src2.PointerVal;
      break;
    default:
      DEBUG(dbgs() << "Unhandled dest type for extractelement instruction: " << *IndexedType << "\n");
      llvm_unreachable(0);
  }

  SetValue(&I, Dest, SF);
}


void NEATPlugIn::visitCallSite( CallSite CS )
{
    // handle call instruction before execution
    HANDLE_EVENT(PRE_INST);

    DEBUG(dbgs() << "[NEATPlugin] running : CallSite: " << *CS.getCalledFunction() << "\n");

    NEATExecutionContext &SF = m_NECStack.back();
    ExecutionContext &EC = m_pECStack->back();
    Function *F = CS.getCalledFunction();

    // Check to see if this is an intrinsic function call...
    if (F && F->isDeclaration())
    {
        switch (F->getIntrinsicID()) {
    case Intrinsic::not_intrinsic:
        break;
    default:
        DEBUG(dbgs() << "[NEATPlugin] Warning:: running unknown intrinsic: " << *CS.getCalledFunction() << "\n");
        return;
        }
        // Ignore execution of "printf" function in NEATPlugIn.
        if (F->getName() == "printf")
            return;
    }

    // obtain Function reference from Interpreter
    GenericValue SRC = m_pInterp->getOperandValueAdapter(CS.getCalledValue(), EC);
    Function *CalledF = (Function*)GVTOP(SRC);

    std::map<Value*, NEATGenericValue> ArgVals;
    Function::arg_iterator AI = CalledF->arg_begin(), AE = CalledF->arg_end();
    for (CallSite::arg_iterator i = CS.arg_begin(),
        e = CS.arg_end(); (i != e) || (AI != AE); ++AI, ++i) {
            Argument* A = &*AI;
            // use the NEAT supported arguments only
            if(m_NTD.IsNEATSupported(A->getType())) {
                ArgVals[A] = getOperandValue(*i, SF);
            }
    }
    callFunction(CalledF, ArgVals);
}
void NEATPlugIn::popStackAndReturnValueToCaller( const Type *RetTy, NEATGenericValue Result )
{
  HANDLE_EVENT(PRE_INST); //
  // Pop the current stack frame.
  m_NECStack.pop_back();

  if (m_NECStack.empty()) {  // Finished OpenCL kernel. Return type could be *only* void.
    if (RetTy && !RetTy->isVoidTy()) {
      DEBUG(dbgs() << "OpenCL kernel returns non-void result!\n");
    }
  } else if(m_NTD.IsNEATSupported(RetTy)) {
    // If we have a previous stack frame, and we have a previous call,
    // fill in the return value...
    ExecutionContext &CallingSF = (*m_pECStack)[m_pECStack->size()-2];
    NEATExecutionContext &SF = m_NECStack.back();
    if (Instruction *I = CallingSF.Caller.getInstruction()) {
      // Save result...
      if (!CallingSF.Caller.getType()->isVoidTy())
        SetValue(I, Result, SF);
    }
  }
}

// Method to get argument value.
const NEATGenericValue& GetArg(Value* arg,
                         const std::map<Value*, NEATGenericValue> &ArgVals){
    std::map<Value *, NEATGenericValue>::const_iterator it0 = ArgVals.find(arg);
    assert ( it0 != ArgVals.end());
    return it0->second;
}


///////////////// shuffle built-in function ///////////////////////
void NEATPlugIn::execute_shuffle(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const Type *Ty0 = arg0->getType();
    if(!m_NTD.IsNEATSupported(Ty0)) return;

    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);

    GenericValue ValArg1 = GetGenericArg(1);

    std::vector<uint32_t> mask_vec;
    for (unsigned i = 0; i < ValArg1.AggregateVal.size(); ++i)
    {
        mask_vec.push_back((uint32_t)(ValArg1.AggregateVal[i].IntVal.getLimitedValue()));
    }

    Result.NEATVec = NEAT_WRAP::shuffle_fd(ValArg0.NEATVec, mask_vec);
}

///////////////// shuffle2 built-in function ///////////////////////
void NEATPlugIn::execute_shuffle2(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const Type *Ty0 = arg0->getType();
    Value *arg1 = &*Fit++;
    const Type *Ty1 = arg1->getType();
    if((!m_NTD.IsNEATSupported(Ty0)) || (!m_NTD.IsNEATSupported(Ty1))) return;

    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);

    if (ValArg0.NEATVec.GetSize() != ValArg1.NEATVec.GetSize())
        throw Exception::NEATTrackFailure("[NEATPlugIn::execute_shuffle2]: wrong vector size.");

    GenericValue ValArg2 = GetGenericArg(2);

   std::vector<uint32_t> mask_vec;
   for (unsigned i = 0; i < ValArg2.AggregateVal.size(); ++i)
   {
       mask_vec.push_back((uint32_t)(ValArg2.AggregateVal[i].IntVal.getLimitedValue()));
   }

    Result.NEATVec = NEAT_WRAP::shuffle2_fd(ValArg0.NEATVec, ValArg1.NEATVec, mask_vec);
}

///////////////// atomic_xchg built-in function ///////////////////////
void NEATPlugIn::execute_atomic_xchg(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const Type *Ty0 = arg0->getType();
    Value *arg1 = &*Fit++;
    const Type *Ty1 = arg1->getType();
    if((!m_NTD.IsNEATSupported(Ty0)) || (!m_NTD.IsNEATSupported(Ty1))) return;

    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);

    NEATValue * pNeatVal = static_cast<NEATValue *>(ValArg0.PointerVal);

    Result.NEATVal = NEAT_WRAP::atomic_xchg_fd(pNeatVal, ValArg1.NEATVal);
}

///////////////// cross built-in function ///////////////////////
void NEATPlugIn::execute_cross(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();
    if (Ty0->isVectorTy() || Ty1->isVectorTy()) {
        const Type *TyElem = dyn_cast<VectorType>(Ty0)->getElementType();

    if (ValArg0.NEATVec.GetSize() != ValArg1.NEATVec.GetSize())
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_cross]: wrong vector size.");

    if (ValArg0.NEATVec.GetSize() != 3 && ValArg0.NEATVec.GetSize() != 4)
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_cross]: wrong vector size.");

        if (TyElem->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::cross_f(ValArg0.NEATVec, ValArg1.NEATVec);
        } else if (TyElem->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::cross_d(ValArg0.NEATVec, ValArg1.NEATVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_cross]: Not valid vector data type for built-in");}
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_cross]: Not valid data type for built-in");
    }
}

///////////////// Step built-in function ///////////////////////
void NEATPlugIn::execute_step(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();
    if (Ty0->isFloatTy()) {
        if (Ty1->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::step_f(ValArg0.NEATVal, ValArg1.NEATVec);
        } else {
            Result.NEATVal = NEAT_WRAP::step_f(ValArg0.NEATVal, ValArg1.NEATVal);
        }
    } else if (Ty0->isDoubleTy()) {
        if (Ty1->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::step_d(ValArg0.NEATVal, ValArg1.NEATVec);
        } else {
            Result.NEATVal = NEAT_WRAP::step_d(ValArg0.NEATVal, ValArg1.NEATVal);
        }
    } else if (Ty0->isVectorTy()) {
        const Type *TyElem = dyn_cast<VectorType>(Ty0)->getElementType();
        if (TyElem->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::step_f(ValArg0.NEATVec, ValArg1.NEATVec);
        } else if (TyElem->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::step_d(ValArg0.NEATVec, ValArg1.NEATVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::step]: Not valid vector data type for built-in");}
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::step]: Not valid data type for built-in");
    }
}

///////////////// smoothstep built-in function ///////////////////////
void NEATPlugIn::execute_smoothstep(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);
    Value *arg2 = &*Fit++;
    const NEATGenericValue& ValArg2 = GetArg(arg2, ArgVals);

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();
    const Type *Ty2 = arg2->getType();
    if (Ty0->isFloatTy()) {
        if (Ty1->isFloatTy() && Ty2->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::smoothstep_f(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVec);
        } else if (Ty1->isFloatTy() && Ty2->isFloatTy()) {
            Result.NEATVal = NEAT_WRAP::smoothstep_f(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::smoothstep]: Not valid data type for built-in");
        }
    } else if (Ty0->isDoubleTy()) {
        if (Ty1->isDoubleTy() && Ty2->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::smoothstep_d(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVec);
        } else if (Ty1->isDoubleTy() && Ty2->isDoubleTy()) {
            Result.NEATVal = NEAT_WRAP::smoothstep_d(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::smoothstep]: Not valid data type for built-in");
        }
    } else if (Ty0->isVectorTy()) {
        const Type *TyElem = dyn_cast<VectorType>(Ty0)->getElementType();
        if (TyElem->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::smoothstep_f(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        } else if (TyElem->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::smoothstep_d(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::smoothstep]: Not valid vector data type for built-in");}
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::smoothstep]: Not valid data type for built-in");
    }
}

///////////////// Clamp built-in function ///////////////////////
void NEATPlugIn::execute_clamp(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);
    Value *arg2 = &*Fit++;
    const NEATGenericValue& ValArg2 = GetArg(arg2, ArgVals);

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();

    if (Ty1->isFloatTy()) {
        if (Ty0->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::clamp_f(ValArg0.NEATVec, ValArg1.NEATVal, ValArg2.NEATVal);
        } else {
            Result.NEATVal = NEAT_WRAP::clamp_f(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
        }
    } else if (Ty1->isDoubleTy()) {
        if (Ty0->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::clamp_d(ValArg0.NEATVec, ValArg1.NEATVal, ValArg2.NEATVal);
        } else {
            Result.NEATVal = NEAT_WRAP::clamp_d(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
        }
    } else if (Ty1->isVectorTy()) {
        const Type *TyElem = dyn_cast<VectorType>(Ty1)->getElementType();
        if (TyElem->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::clamp_f(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        } else if (TyElem->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::clamp_d(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        } else if (TyElem->isIntegerTy()) {
            return;
        }else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::clamp]: Not valid vector data type for built-in");
        }
    } else if (Ty1->isIntegerTy()){
        return;
    }
    else {
        throw Exception::IllegalFunctionCall("[NEATPlug-in::clamp]: Not valid data type for built-in");
    }
}


// Method to obtain integer value by argument index.
GenericValue NEATPlugIn::GetGenericArg(size_t ArgIdx) {
    ExecutionContext &CallingSF = m_pECStack->back();
    CallSite CS(&(static_cast<CallInst&>(*CallingSF.CurInst)));
    Value* arg = 0;
    CallSite::arg_iterator i = CS.arg_begin();
    for (size_t j = 0; j < ArgIdx; ++j, ++i) {}
    arg = *i;
    return m_pInterp->getOperandValueAdapter(arg, CallingSF);
}

void NEATPlugIn::execute_vload_half(Function *F,
                                    const std::map<Value *, NEATGenericValue> &ArgVals,
                                    NEATGenericValue& Result,
                                    const OCLBuiltinParser::ArgVector& ArgList)
{
    GenericValue ValArg0 = GetGenericArg(0);
    size_t offset = ValArg0.IntVal.getZExtValue();
    GenericValue ValArg1 = GetGenericArg(1);
    uint16_t* p = (uint16_t*) GVTOP(ValArg1);
    Result.NEATVal = NEAT_WRAP::vload_half(offset, p);
    return;
}

// Macro to define the vload_half functions for different vector length sizes.
#define EXECUTE_VLOAD_HALF(n)                               \
    void NEATPlugIn::execute_vload_half ## n(Function *F,   \
    const std::map<Value *, NEATGenericValue> &ArgVals,     \
    NEATGenericValue& Result,                               \
    const OCLBuiltinParser::ArgVector& ArgList) {           \
    GenericValue ValArg0 = GetGenericArg(0);                \
    size_t offset = ValArg0.IntVal.getZExtValue();          \
    GenericValue ValArg1 = GetGenericArg(1);                \
    uint16_t* p = (uint16_t*) GVTOP(ValArg1);               \
    Result.NEATVec = NEAT_WRAP::vload_half_ ## n ## _false(offset, p);  \
    return;                                                 \
}

EXECUTE_VLOAD_HALF(2)
EXECUTE_VLOAD_HALF(3)
EXECUTE_VLOAD_HALF(4)
EXECUTE_VLOAD_HALF(8)
EXECUTE_VLOAD_HALF(16)

// Macro to define the vloada_half functions for different vector length sizes.
#define EXECUTE_VLOADA_HALF(n)                               \
    void NEATPlugIn::execute_vloada_half ## n(Function *F,   \
    const std::map<Value *, NEATGenericValue> &ArgVals,     \
    NEATGenericValue& Result,                               \
    const OCLBuiltinParser::ArgVector& ArgList) {           \
    GenericValue ValArg0 = GetGenericArg(0);                \
    size_t offset = ValArg0.IntVal.getZExtValue();          \
    GenericValue ValArg1 = GetGenericArg(1);                \
    uint16_t* p = (uint16_t*) GVTOP(ValArg1);               \
    Result.NEATVec = NEAT_WRAP::vload_half_ ## n ## _true(offset, p);  \
    return;                                                 \
}

EXECUTE_VLOADA_HALF(2)
EXECUTE_VLOADA_HALF(3)
EXECUTE_VLOADA_HALF(4)
EXECUTE_VLOADA_HALF(8)
EXECUTE_VLOADA_HALF(16)

void NEATPlugIn::execute_vloada_half(Function *F,
                                    const std::map<Value *, NEATGenericValue> &ArgVals,
                                    NEATGenericValue& Result,
                                    const OCLBuiltinParser::ArgVector& ArgList)
{
    execute_vload_half(F, ArgVals, Result, ArgList);
}

typedef NEATVector (*vloadOp)(size_t, const NEATValue*);

// Macro to define the vload functions for different vector length sizes.
#define EXECUTE_VLOAD(n)                                                                \
void NEATPlugIn::execute_vload ## n(Function *F,                                        \
                                const std::map<Value *, NEATGenericValue> &ArgVals,     \
                                NEATGenericValue& Result,                               \
                                const OCLBuiltinParser::ArgVector& ArgList) {           \
    Function::arg_iterator Fit = F->arg_begin();                                        \
    GenericValue ValArg0 = GetGenericArg(0);                                            \
    size_t offset = ValArg0.IntVal.getZExtValue();                                      \
    Fit++;                                                                              \
    Value *arg1 = &*Fit++;                                                                \
    const Type *Ty1 = arg1->getType();                                                  \
    if(!m_NTD.IsNEATSupported(Ty1)) return;                                             \
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);                            \
    NEATValue* p = (NEATValue*) NGVTOP(ValArg1);                                        \
    const PointerType *PTy = dyn_cast<PointerType>(Ty1);                                \
    const Type *ETy = PTy->getElementType();                                            \
    if (ETy->isFloatTy()) {                                                             \
        Result.NEATVec = NEAT_WRAP::vload ## n ## _f(offset, p);                        \
    } else if (ETy->isDoubleTy()) {                                                     \
        Result.NEATVec = NEAT_WRAP::vload ## n ## _d(offset, p);                        \
    } else if (ETy->isIntegerTy()) {                                                    \
        return;                                                                         \
    } else {                                                                            \
        throw Exception::IllegalFunctionCall("[NEATPlug-in::vload]: Invalid data type for vload built-in"); \
    }                                                                                   \
}

EXECUTE_VLOAD(2)
EXECUTE_VLOAD(3)
EXECUTE_VLOAD(4)
EXECUTE_VLOAD(8)
EXECUTE_VLOAD(16)

#define EXECUTE_VSTORE_HALF_RT(mode, name)                                                  \
void NEATPlugIn::execute_v##name##mode(Function *F,                                         \
                                     const std::map<Value *, NEATGenericValue> &ArgVals,    \
                                     NEATGenericValue& Result,                              \
                                     const OCLBuiltinParser::ArgVector& ArgList)            \
{                                                                                           \
    Function::arg_iterator Fit = F->arg_begin();                                            \
    Value *arg0 = &*Fit++;                                                                    \
    const Type *Ty0 = arg0->getType();                                                      \
    if(!m_NTD.IsNEATSupported(Ty0)) return;                                                  \
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);                                \
    GenericValue ValArg1 = GetGenericArg(1);                                                \
    size_t offset = ValArg1.IntVal.getZExtValue();                                          \
    const GenericValue& ValArg2 = GetGenericArg(2);                                         \
    uint16_t* p = (uint16_t*) GVTOP(ValArg2);                                               \
    if (Ty0->isFloatTy()) {                                                                 \
        NEAT_WRAP::v##name##mode##_f(ValArg0.NEATVal, offset, p);                           \
    } else if (Ty0->isDoubleTy()) {                                                         \
        NEAT_WRAP::v##name##mode##_d(ValArg0.NEATVal, offset, p);                           \
    } else {                                                                                \
        throw Exception::IllegalFunctionCall("[NEATPlug-in::vstore_half]: Invalid data type"\
                                             " for vstore_half built-in");                  \
    }                                                                                       \
}

#define EXECUTE_VSTORE_HALF(mode)      \
    EXECUTE_VSTORE_HALF_RT(mode,store) \
    EXECUTE_VSTORE_HALF_RT(mode,storea)

EXECUTE_VSTORE_HALF(_half)
EXECUTE_VSTORE_HALF(_half_rte)
EXECUTE_VSTORE_HALF(_half_rtz)
EXECUTE_VSTORE_HALF(_half_rtp)
EXECUTE_VSTORE_HALF(_half_rtn)

// Macro to define the vload_half functions for different vector length sizes.
#define EXECUTE_VSTORE_HALF_N_RT(n, name, plugin_mode, alu_mode)                        \
    void NEATPlugIn::execute_v##name ## plugin_mode (Function *F,                       \
    const std::map<Value *, NEATGenericValue> &ArgVals,                                 \
    NEATGenericValue& Result,                                                           \
    const OCLBuiltinParser::ArgVector& ArgList) {                                       \
    Function::arg_iterator Fit = F->arg_begin();                                        \
    Value *arg0 = &*Fit++;                                                                \
    const Type *Ty2 = arg0->getType();                                                  \
    if(!m_NTD.IsNEATSupported(Ty2)) return;                                             \
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);                            \
    const NEATVector data = ValArg0.NEATVec;                                            \
    GenericValue ValArg1 = GetGenericArg(1);                                            \
    size_t offset = ValArg1.IntVal.getZExtValue();                                      \
    const GenericValue& ValArg2 = GetGenericArg(2);                                     \
    uint16_t* p = (uint16_t*) GVTOP(ValArg2);                                           \
    const VectorType *PTy = dyn_cast<VectorType>(Ty2);                                  \
    const Type *ETy = PTy->getElementType();                                            \
    if (ETy->isFloatTy()) {                                                             \
        NEAT_WRAP::v##name ## alu_mode ## _ ## n ## _f(data, offset, p);                \
    } else if (ETy->isDoubleTy()) {                                                     \
        NEAT_WRAP::v##name ## alu_mode ## _ ## n ## _d(data, offset, p);                \
    } else {                                                                            \
        throw Exception::IllegalFunctionCall("[NEATPlug-in::vstore_half]: Invalid data" \
                                             " type for vstore_half built-in");         \
    }                                                                                   \
}

#define EXECUTE_VSTORE_HALF_RT_ALL_N(n, name)                        \
    EXECUTE_VSTORE_HALF_N_RT(n, name, _half##n, _half)               \
    EXECUTE_VSTORE_HALF_N_RT(n, name, _half##n##_rte, _half##_rte)   \
    EXECUTE_VSTORE_HALF_N_RT(n, name, _half##n##_rtz, _half##_rtz)   \
    EXECUTE_VSTORE_HALF_N_RT(n, name, _half##n##_rtp, _half##_rtp)   \
    EXECUTE_VSTORE_HALF_N_RT(n, name, _half##n##_rtn, _half##_rtn)   \

#define EXECUTE_VSTORE_HALF_ALL_N(n)\
    EXECUTE_VSTORE_HALF_RT_ALL_N(n, store)\
    EXECUTE_VSTORE_HALF_RT_ALL_N(n, storea)

EXECUTE_VSTORE_HALF_ALL_N(2)
EXECUTE_VSTORE_HALF_ALL_N(3)
EXECUTE_VSTORE_HALF_ALL_N(4)
EXECUTE_VSTORE_HALF_ALL_N(8)
EXECUTE_VSTORE_HALF_ALL_N(16)

typedef void (*vstoreOp)(NEATVector, size_t, const NEATValue*);

// Macro to define the vstore functions for different vector length sizes.

#define EXECUTE_VSTORE(n)                                                                \
void NEATPlugIn::execute_vstore ## n(Function *F,                                        \
                                const std::map<Value *, NEATGenericValue> &ArgVals,     \
                                NEATGenericValue& Result,                               \
                                const OCLBuiltinParser::ArgVector& ArgList) {           \
    Function::arg_iterator Fit = F->arg_begin();                                        \
    Value *arg0 = &*Fit++;                                                                \
    const Type *Ty0 = arg0->getType();                                                  \
    if(!m_NTD.IsNEATSupported(Ty0)) return;                                             \
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);                            \
    const NEATVector data = ValArg0.NEATVec;                                            \
    GenericValue ValArg1 = GetGenericArg(1);                                            \
    size_t offset = ValArg1.IntVal.getZExtValue();                                      \
    Fit++;                                                                              \
    Value *arg2 = &*Fit++;                                                                \
    const NEATGenericValue& ValArg2 = GetArg(arg2, ArgVals);                            \
    NEATValue* p = (NEATValue*) NGVTOP(ValArg2);                                        \
    const Type *Ty2 = arg2->getType();                                                  \
    const PointerType *PTy = dyn_cast<PointerType>(Ty2);                                \
    const Type *ETy = PTy->getElementType();                                            \
    if (ETy->isFloatTy()) {                                                             \
        NEAT_WRAP::vstore ## n ## _f(data, offset, p);                                  \
    } else if (ETy->isDoubleTy()) {                                                     \
        NEAT_WRAP::vstore ## n ## _d(data, offset, p);                                  \
    } else if (ETy->isIntegerTy()) {                                                    \
        return;                                                                         \
    } else {                                                                            \
        throw Exception::IllegalFunctionCall("[NEATPlug-in::vstore]: Invalid data type for vstore built-in"); \
    }                                                                                   \
}

EXECUTE_VSTORE(2)
EXECUTE_VSTORE(3)
EXECUTE_VSTORE(4)
EXECUTE_VSTORE(8)
EXECUTE_VSTORE(16)

// Macro to define the convert_float functions for different vector length sizes.
#define EXECUTE_CONVERT_F(n)                                                  \
template <typename T>                                                       \
NEATVector convert_f ## n(GenericValue arg)                                 \
{                                                                           \
    T src[n];                                                               \
    for (unsigned int i = 0; i < n; ++i)                                    \
    {                                                                       \
        src[i] = T(arg.AggregateVal[i].IntVal.getLimitedValue());           \
    }                                                                       \
    return NEAT_WRAP::convert_float_ ## n(src);                             \
}                                                                           \
void NEATPlugIn::execute_convert_float ## n(Function *F,                    \
            const std::map<Value *, NEATGenericValue> &ArgVals,             \
            NEATGenericValue& Result,                                       \
            const OCLBuiltinParser::ArgVector& ArgList)                     \
{                                                                           \
    Function::arg_iterator Fit = F->arg_begin();                            \
    Value* arg = &*Fit++;                                                     \
    const Type *Ty = arg->getType();                                        \
    const VectorType *VTy = cast<VectorType>(Ty);                           \
    const Type *ElemTy = VTy->getElementType();                             \
    if (ElemTy->isDoubleTy())                                               \
    {                                                                       \
        NEATValue src[n];                                                   \
        NEATGenericValue argVal = GetArg(arg, ArgVals);                     \
        for (uint32_t i = 0; i < n; ++i)                                    \
        {                                                                   \
            src[i] = argVal.NEATVec[i];                                     \
        }                                                                   \
        Result.NEATVec = NEAT_WRAP::convert_float_d_ ## n(src);             \
    } else if (ElemTy->isIntegerTy()) {                                     \
        GenericValue argVal = GetGenericArg(0);                             \
        const IntegerType *ITy = cast<IntegerType>(VTy->getElementType());  \
        switch (ITy->getBitWidth())                                         \
        {                                                                   \
        case 8:                                                             \
            if (ArgList[0].vecType.elType == OCLBuiltinParser::CHAR)        \
                Result.NEATVec = convert_f ## n<int8_t>(argVal);            \
            else                                                            \
                Result.NEATVec = convert_f ## n<uint8_t>(argVal);           \
            break;                                                          \
        case 16:                                                            \
            if (ArgList[0].vecType.elType == OCLBuiltinParser::SHORT)       \
                Result.NEATVec = convert_f ## n<int16_t>(argVal);           \
            else                                                            \
                Result.NEATVec = convert_f ## n<uint16_t>(argVal);          \
            break;                                                          \
        case 32:                                                            \
            if (ArgList[0].vecType.elType == OCLBuiltinParser::INT)         \
                Result.NEATVec = convert_f ## n<int32_t>(argVal);           \
            else                                                            \
                Result.NEATVec = convert_f ## n<uint32_t>(argVal);          \
            break;                                                          \
        case 64:                                                            \
            if (ArgList[0].vecType.elType == OCLBuiltinParser::LONG)        \
                Result.NEATVec = convert_f ## n<int64_t>(argVal);           \
            else                                                            \
                Result.NEATVec = convert_f ## n<uint64_t>(argVal);          \
            break;                                                          \
        }                                                                   \
    } else {                                                                \
        throw Exception::InvalidArgument("[NEATPlug-in::convert_float]: "   \
                    "Not valid data type for convert_float built-in");       \
    }                                                                       \
}

EXECUTE_CONVERT_F(2)
EXECUTE_CONVERT_F(3)
EXECUTE_CONVERT_F(4)
EXECUTE_CONVERT_F(8)
EXECUTE_CONVERT_F(16)

// Convert_float function for integer scalar arguments
template <typename T>
NEATValue convert_f(GenericValue argVal)
{
    T val = argVal.IntVal.getSExtValue();
    return NEAT_WRAP::convert_float(&val);
}

// Convert_float built-in executor.
void NEATPlugIn::execute_convert_float(Function *F,
    const std::map<Value *, NEATGenericValue> &ArgVals,
    NEATGenericValue& Result,
    const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value* arg = &*Fit++;
    GenericValue argVal = GetGenericArg(0);
    const Type *Ty = arg->getType();
    if (Ty->isDoubleTy())
    {
        NEATGenericValue argVal = GetArg(arg, ArgVals);
        Result.NEATVal = NEAT_WRAP::convert_float_d(&(argVal.NEATVal));
    } else if (Ty->isIntegerTy()) {
        const IntegerType *ITy = cast<IntegerType>(Ty);
        switch (ITy->getBitWidth())
        {
        case 8:
            if (ArgList[0].basicType == OCLBuiltinParser::CHAR)
                Result.NEATVal = convert_f<int8_t>(argVal);
            else
                Result.NEATVal = convert_f<uint8_t>(argVal);
            break;
        case 16:
            if (ArgList[0].basicType == OCLBuiltinParser::SHORT)
                Result.NEATVal = convert_f<int16_t>(argVal);
            else
                Result.NEATVal = convert_f<uint16_t>(argVal);
            break;
        case 32:
            if (ArgList[0].basicType == OCLBuiltinParser::INT)
                Result.NEATVal = convert_f<int32_t>(argVal);
            else
                Result.NEATVal = convert_f<uint32_t>(argVal);
            break;
        case 64:
            if (ArgList[0].basicType == OCLBuiltinParser::LONG)
                Result.NEATVal = convert_f<int64_t>(argVal);
            else
                Result.NEATVal = convert_f<uint64_t>(argVal);
            break;
        }
    } else {
        throw Exception::InvalidArgument("[NEATPlug-in::convert_float]: Not valid data type.");
    }
}

// Macro to define the convert_float functions for different vector length sizes.
#define EXECUTE_CONVERT_D(n)                                                \
    template <typename T>                                                   \
    NEATVector convert_d ## n(GenericValue arg)                             \
    {                                                                       \
        T src[n];                                                           \
        for (unsigned int i = 0; i < n; ++i)                                \
        {                                                                   \
            src[i] = T(arg.AggregateVal[i].IntVal.getLimitedValue());       \
        }                                                                   \
        return NEAT_WRAP::convert_double_ ## n(src);                        \
    }                                                                       \
                                                                            \
    void NEATPlugIn::execute_convert_double ## n(Function *F,               \
    const std::map<Value *, NEATGenericValue> &ArgVals,                     \
    NEATGenericValue& Result,                                               \
    const OCLBuiltinParser::ArgVector& ArgList)                             \
    {                                                                       \
        Function::arg_iterator Fit = F->arg_begin();                        \
        Value* arg = &*Fit++;                                                 \
        const Type *Ty = arg->getType();                                    \
        const VectorType *VTy = cast<VectorType>(Ty);                       \
        const Type *ElemTy = VTy->getElementType();                         \
        if (ElemTy->isFloatTy())                                            \
        {                                                                   \
            NEATValue src[n];                                               \
            NEATGenericValue argVal = GetArg(arg, ArgVals);                 \
            for (uint32_t i = 0; i < n; ++i)                                \
            {                                                               \
                src[i] = argVal.NEATVec[i];                                 \
            }                                                               \
            Result.NEATVec = NEAT_WRAP::convert_double_f_ ## n(src);        \
        } else if (ElemTy->isIntegerTy()) {                                 \
            GenericValue argVal = GetGenericArg(0);                         \
            const IntegerType *ITy = cast<IntegerType>(VTy->getElementType());\
            switch (ITy->getBitWidth())                                     \
            {                                                               \
                case 8:                                                     \
                if (ArgList[0].vecType.elType == OCLBuiltinParser::CHAR)    \
                    Result.NEATVec = convert_d ## n<int8_t>(argVal);        \
                else                                                        \
                    Result.NEATVec = convert_d ## n<uint8_t>(argVal);       \
                break;                                                      \
                case 16:                                                    \
                if (ArgList[0].vecType.elType == OCLBuiltinParser::SHORT)   \
                    Result.NEATVec = convert_d ## n<int16_t>(argVal);       \
                else                                                        \
                    Result.NEATVec = convert_d ## n<uint16_t>(argVal);      \
                break;                                                      \
                case 32:                                                    \
                if (ArgList[0].vecType.elType == OCLBuiltinParser::INT)     \
                    Result.NEATVec = convert_d ## n<int32_t>(argVal);       \
                else                                                        \
                    Result.NEATVec = convert_d ## n<uint32_t>(argVal);      \
                break;                                                      \
                case 64:                                                    \
                if (ArgList[0].vecType.elType == OCLBuiltinParser::LONG)    \
                    Result.NEATVec = convert_d ## n<int64_t>(argVal);       \
                else                                                        \
                    Result.NEATVec = convert_d ## n<uint64_t>(argVal);      \
                break;                                                      \
            }                                                               \
        }                                                                   \
    }

EXECUTE_CONVERT_D(2)
EXECUTE_CONVERT_D(3)
EXECUTE_CONVERT_D(4)
EXECUTE_CONVERT_D(8)
EXECUTE_CONVERT_D(16)

// Convert_double function for integer scalar arguments
template <typename T>
NEATValue convert_d(GenericValue argVal)
{
    T val = argVal.IntVal.getSExtValue();
    return NEAT_WRAP::convert_double((T*)&val);
}

// Convert_double built-in executor.
void NEATPlugIn::execute_convert_double(Function *F,
                                       const std::map<Value *, NEATGenericValue> &ArgVals,
                                       NEATGenericValue& Result,
                                       const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value* arg = &*Fit++;
    const Type *Ty = arg->getType();
    if (Ty->isFloatTy())
    {
        NEATGenericValue argVal = GetArg(arg, ArgVals);
        Result.NEATVal = NEAT_WRAP::convert_double_f(&(argVal.NEATVal));
    } else if (Ty->isIntegerTy()) {
        const IntegerType *ITy = cast<IntegerType>(Ty);
        GenericValue argVal = GetGenericArg(0);
        switch (ITy->getBitWidth())
        {
        case 8:
            if (ArgList[0].basicType == OCLBuiltinParser::CHAR)
                Result.NEATVal = convert_d<int8_t>(argVal);
            else
                Result.NEATVal = convert_d<uint8_t>(argVal);
            break;
        case 16:
            if (ArgList[0].basicType == OCLBuiltinParser::SHORT)
                Result.NEATVal = convert_d<int16_t>(argVal);
            else
                Result.NEATVal = convert_d<uint16_t>(argVal);
            break;
        case 32:
            if (ArgList[0].basicType == OCLBuiltinParser::INT)
                Result.NEATVal = convert_d<int32_t>(argVal);
            else
                Result.NEATVal = convert_d<uint32_t>(argVal);
            break;
        case 64:
            if (ArgList[0].basicType == OCLBuiltinParser::LONG)
                Result.NEATVal = convert_d<int64_t>(argVal);
            else
                Result.NEATVal = convert_d<uint64_t>(argVal);
            break;
        }
    }
}

void NEATPlugIn::execute_async_work_group_copy(Function *F,
                                       const std::map<Value *, NEATGenericValue> &ArgVals,
                                       NEATGenericValue& Result,
                                       const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value* arg0 = &*Fit++;

    const Type *Ty = arg0->getType();
    const PointerType *PTy = dyn_cast<PointerType>(Ty);
    const Type *ETy = PTy->getElementType();

    if(!m_NTD.IsNEATSupported(ETy)) return;

    const NEATGenericValue dst = GetArg(arg0, ArgVals);
    Value* arg1 = &*Fit++;
    const NEATGenericValue src = GetArg(arg1, ArgVals);
    GenericValue num_gentypes = GetGenericArg(2);

    if (ETy->isFloatingPointTy())
    {
        NEATValue *dstP = (NEATValue *)dst.PointerVal;
        NEATValue *srcP = (NEATValue *)src.PointerVal;
        for (uint64_t i = 0; i < num_gentypes.IntVal.getZExtValue(); ++i, ++dstP, ++srcP)
        {
            *dstP = *srcP;
        }
    } else if (ETy->isVectorTy() && dyn_cast<VectorType>(ETy)->getElementType()->isFloatingPointTy())
    {
        NEATValue *dstP = (NEATValue *)dst.PointerVal;
        NEATValue *srcP = (NEATValue *)src.PointerVal;
        for (uint64_t i = 0; i < num_gentypes.IntVal.getZExtValue() * dyn_cast<VectorType>(ETy)->getNumElements(); ++i, ++dstP, ++srcP)
        {
            *dstP = *srcP;
        }
    }
}

void NEATPlugIn::execute_async_work_group_strided_copy(Function *F,
                                               const std::map<Value *, NEATGenericValue> &ArgVals,
                                               NEATGenericValue& Result,
                                               const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value* arg0 = &*Fit++;

    const Type *Ty = arg0->getType();
    const PointerType *PTy = dyn_cast<PointerType>(Ty);
    const Type *ETy = PTy->getElementType();

    if(!m_NTD.IsNEATSupported(ETy)) return;

    const NEATGenericValue dst = GetArg(arg0, ArgVals);
    Value* arg1 = &*Fit++;
    const NEATGenericValue src = GetArg(arg1, ArgVals);
    GenericValue num_gentypes = GetGenericArg(2);
    uint64_t stride = GetGenericArg(3).IntVal.getZExtValue();

    if (ETy->isFloatingPointTy())
    {
        NEATValue *dstP = (NEATValue *)dst.PointerVal;
        NEATValue *srcP = (NEATValue *)src.PointerVal;
        if (ArgList[0].ptrType.AddrSpace == OCLBuiltinParser::GLOBAL)
        {
            for (uint64_t i = 0; i < num_gentypes.IntVal.getZExtValue(); ++i, dstP += stride, ++srcP)
            {
                *dstP = *srcP;
            }
        }
        else
        {
            for (uint64_t i = 0; i < num_gentypes.IntVal.getZExtValue(); ++i, ++dstP, srcP += stride)
            {
                *dstP = *srcP;
            }
        }
    } else if (ETy->isVectorTy() && dyn_cast<VectorType>(ETy)->getElementType()->isFloatingPointTy())
    {
        NEATValue *dstP = (NEATValue *)dst.PointerVal;
        NEATValue *srcP = (NEATValue *)src.PointerVal;
        if (ArgList[0].ptrType.AddrSpace == OCLBuiltinParser::GLOBAL)
        {
            for (uint64_t i = 0; i < num_gentypes.IntVal.getZExtValue(); ++i, dstP+=dyn_cast<VectorType>(ETy)->getNumElements()*(stride-1))
                for (uint64_t j = 0; j < dyn_cast<VectorType>(ETy)->getNumElements(); ++j, ++dstP, ++srcP)
                    *dstP = *srcP;
        }
        else
        {
            for (uint64_t i = 0; i < num_gentypes.IntVal.getZExtValue(); ++i, srcP+=dyn_cast<VectorType>(ETy)->getNumElements()*(stride-1))
                for (uint64_t j = 0; j < dyn_cast<VectorType>(ETy)->getNumElements(); ++j, ++dstP, ++srcP)
                    *dstP = *srcP;
        }
    }
}

#define RELATIONAL_TWO_ARGS(FuncName) \
    Function::arg_iterator Fit = F->arg_begin();\
    Value* arg0 = &*Fit++;\
    const NEATGenericValue x = GetArg(arg0, ArgVals);\
    Value* arg1 = &*Fit++;\
    const NEATGenericValue y = GetArg(arg1, ArgVals);\
    const Type *Ty = arg0->getType();\
    if (Ty->isFloatTy()) {\
        Result.NEATVal = NEAT_WRAP::FuncName##_f(x.NEATVal, y.NEATVal);\
        if (Result.NEATVal.IsUnknown())\
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: the result of comparison is UNKNOWN.");\
    } else if (Ty->isDoubleTy()) {\
        Result.NEATVal = NEAT_WRAP::FuncName##_d(x.NEATVal, y.NEATVal);\
        if (Result.NEATVal.IsUnknown())\
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: the result of comparison is UNKNOWN.");\
    } else if (Ty->isVectorTy()) {\
        const Type *ETy = cast<VectorType>(Ty)->getElementType();\
        if (x.NEATVec.GetSize() != y.NEATVec.GetSize()) \
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: different vector sizes."); \
        if (x.NEATVec.GetSize() != cast<VectorType>(Ty)->getNumElements()) \
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: wrong vector size."); \
        if (ETy->isFloatTy()) {\
            Result.NEATVec = NEAT_WRAP::FuncName##_f(x.NEATVec, y.NEATVec);\
        } else if (ETy->isDoubleTy()) {\
            Result.NEATVec =  NEAT_WRAP::FuncName##_d(x.NEATVec, y.NEATVec);\
        }\
        for (size_t i = 0; i < Result.NEATVec.GetSize(); ++i)\
            if (Result.NEATVec[i].IsUnknown())\
                throw Exception::NEATTrackFailure("[NEATPlugIn::##FuncName##]: the result of comparison is UNKNOWN.");\
    }

#define RELATIONAL_ONE_ARG(FuncName) \
    Function::arg_iterator Fit = F->arg_begin();\
    Value* arg0 = &*Fit++;\
    const NEATGenericValue x = GetArg(arg0, ArgVals);\
    const Type *Ty = arg0->getType();\
    if (Ty->isFloatTy()) {\
        Result.NEATVal = NEAT_WRAP::FuncName##_f(x.NEATVal);\
        if (Result.NEATVal.IsUnknown())\
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: the result of comparison is UNKNOWN.");\
    } else if (Ty->isDoubleTy()) {\
        Result.NEATVal = NEAT_WRAP::FuncName##_d(x.NEATVal);\
        if (Result.NEATVal.IsUnknown())\
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: the result of comparison is UNKNOWN.");\
    } else if (Ty->isVectorTy()) {\
        const Type *ETy = cast<VectorType>(Ty)->getElementType();\
        if (x.NEATVec.GetSize() != cast<VectorType>(Ty)->getNumElements()) \
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_##FuncName##]: wrong vector size."); \
        if (ETy->isFloatTy()) {\
            Result.NEATVec = NEAT_WRAP::FuncName##_f(x.NEATVec);\
        } else if (ETy->isDoubleTy()) {\
            Result.NEATVec = NEAT_WRAP::FuncName##_d(x.NEATVec);\
        }\
        for (size_t i = 0; i < Result.NEATVec.GetSize(); ++i)\
            if (Result.NEATVec[i].IsUnknown())\
                throw Exception::NEATTrackFailure("[NEATPlugIn::##FuncName##]: the result of comparison is UNKNOWN.");\
    }

void NEATPlugIn::execute_ISequal(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISequal)
}
void NEATPlugIn::execute_ISnotequal(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISnotequal)
}
void NEATPlugIn::execute_ISgreater(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISgreater)
}
void NEATPlugIn::execute_ISgreaterequal(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISgreaterequal)
}
void NEATPlugIn::execute_ISless(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISless)
}
void NEATPlugIn::execute_ISlessequal(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISlessequal)
}
void NEATPlugIn::execute_ISlessgreater(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISlessgreater)
}
void NEATPlugIn::execute_ISfinite(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_ONE_ARG(ISfinite)
}
void NEATPlugIn::execute_ISinf(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_ONE_ARG(ISinf)
}
void NEATPlugIn::execute_ISnan(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_ONE_ARG(ISnan)
}
void NEATPlugIn::execute_ISnormal(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_ONE_ARG(ISnormal)
}
void NEATPlugIn::execute_ISordered(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISordered)
}
void NEATPlugIn::execute_ISunordered(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_TWO_ARGS(ISunordered)
}
void NEATPlugIn::execute_signbit(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    RELATIONAL_ONE_ARG(signbit)
}

void NEATPlugIn::execute_bitselect(Function *F,
                                   const std::map<Value *, NEATGenericValue> &ArgVals,
                                   NEATGenericValue& Result,
                                   const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const Type *Ty = arg0->getType();
    if(!m_NTD.IsNEATSupported(Ty)) return;
    // all three input values or vectors should have the same type (and size for vectors)

    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);
    Value *arg2 = &*Fit++;
    const NEATGenericValue& ValArg2 = GetArg(arg2, ArgVals);

    if(Ty != arg1->getType() || Ty != arg2->getType())
        throw Exception::NEATTrackFailure("[NEATPlugIn::execute_bitselect]: input data types are not the same.");

    // only floating point type values or vectors are supoorted.
    // in case of integer or boolean or other type input data just do nothing

    if(Ty->isFloatTy())  {
        Result.NEATVal = NEAT_WRAP::bitselect_f (ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
    }
    else if(Ty->isDoubleTy()) {
        Result.NEATVal = NEAT_WRAP::bitselect_d (ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
    }
    else if(Ty->isVectorTy()) {
        const Type *TyElem = dyn_cast<VectorType>(Ty)->getElementType();
        if(TyElem->isFloatTy())  {
            Result.NEATVec = NEAT_WRAP::bitselect_f (ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        }
       else if(TyElem->isDoubleTy()) {
           Result.NEATVec = NEAT_WRAP::bitselect_d (ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
       }
    }
}

void NEATPlugIn::execute_select(Function *F,
                               const std::map<Value *, NEATGenericValue> &ArgVals,
                               NEATGenericValue& Result,
                               const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const Type *Ty = arg0->getType();
    Value *arg1 = &*Fit++;

    if(!m_NTD.IsNEATSupported(Ty)) return;

    const NEATGenericValue& a = GetArg(arg0, ArgVals);
    const NEATGenericValue& b = GetArg(arg1, ArgVals);

    if(Ty != arg1->getType())
        throw Exception::NEATTrackFailure("[NEATPlugIn::execute_select]: input data types of first and second arguments are not the same.");

    GenericValue c = GetGenericArg(2);

    // only floating point type values or vectors are supoorted.
    // in case of integer or boolean or other type input data just do nothing
    // both input values or vectors should have the same type (and size for vectors)
    if (Ty->isFloatTy()) {
        int64_t intVal = int64_t(c.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::select_f(a.NEATVal, b.NEATVal, intVal);
    } else if (Ty->isDoubleTy()) {
        int64_t intVal = int64_t(c.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::select_d(a.NEATVal, b.NEATVal, intVal);
    } else if (Ty->isVectorTy()) {
        const Type *ETy = cast<VectorType>(Ty)->getElementType();

        if (ETy->isFloatTy() || ETy->isDoubleTy() ) {
            if (a.NEATVec.GetSize() != b.NEATVec.GetSize() || a.NEATVec.GetSize() != size_t(c.AggregateVal.size()) ||
               (a.NEATVec.GetSize() != cast<VectorType>(Ty)->getNumElements()) )
                throw Exception::NEATTrackFailure("[NEATPlugIn::execute_select]: wrong vector size.");

            std::vector<int64_t> condVec;
            for (uint32_t i = 0; i < uint32_t(c.AggregateVal.size()); ++i) {
                int64_t intVal = int64_t(c.AggregateVal[i].IntVal.getSExtValue());
                condVec.push_back(intVal);
            }

            if (ETy->isFloatTy()) {
                Result.NEATVec = NEAT_WRAP::select_f(a.NEATVec, b.NEATVec, condVec);
            } else if (ETy->isDoubleTy()) {
                Result.NEATVec = NEAT_WRAP::select_d(a.NEATVec, b.NEATVec, condVec);
            }
        }
    }
}

void NEATPlugIn::execute_ilogb(Function *F,
                               const std::map<Value *, NEATGenericValue> &ArgVals,
                               NEATGenericValue& Result,
                               const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value* arg0 = &*Fit++;
    const NEATGenericValue x = GetArg(arg0, ArgVals);
    const Type *Ty = arg0->getType();
    if (Ty->isFloatTy()) {
        Result.NEATVal = NEAT_WRAP::ilogb_f(x.NEATVal);
        if (Result.NEATVal.IsUnknown())
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_ilogb]: the result of operation is UNKNOWN.");
    }  else if (Ty->isDoubleTy()) {
        Result.NEATVal = NEAT_WRAP::ilogb_d(x.NEATVal);
        if (Result.NEATVal.IsUnknown())
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_ilogb]: the result of operation is UNKNOWN.");
    } else if (Ty->isVectorTy()) {
        const Type *ETy = cast<VectorType>(Ty)->getElementType();

        if (x.NEATVec.GetSize() != cast<VectorType>(Ty)->getNumElements())
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_ilogb]: wrong vector size.");

        if (ETy->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::ilogb_f(x.NEATVec);
        } else if (ETy->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::ilogb_d(x.NEATVec);
        }

        for (size_t i = 0; i < Result.NEATVec.GetSize(); ++i)
            if (Result.NEATVec[i].IsUnknown())
                throw Exception::NEATTrackFailure("[NEATPlugIn::execute_ilogb]: the result of operation is UNKNOWN.");
    }
}


///////////////// lgamma_r built-in function ///////////////////////
void execute_lgamma_r(Function *F,
                      const std::map<Value *, NEATGenericValue> &ArgVals,
                      NEATGenericValue& Result,
                      const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);

    const Type *Ty0 = arg0->getType();

    if (Ty0->isVectorTy()) {
        const Type *TyElem0 = dyn_cast<VectorType>(Ty0)->getElementType();

        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // results written to vector intVec
        std::vector<int> intVec;

        if (TyElem0->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::lgamma_r_f(ValArg0.NEATVec, intVec);
        } else if (TyElem0->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::lgamma_r_d(ValArg0.NEATVec, intVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::lgamma_r]: Not valid vector data type for built-in");}
    } else if (Ty0->isDoubleTy()) {
        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // result written to variable intRes
        int32_t intRes;
        Result.NEATVal = NEAT_WRAP::lgamma_r_d(ValArg0.NEATVal, &intRes);
    } else if (Ty0->isFloatTy()) {
        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // result written to variable intRes
        int32_t intRes;
        Result.NEATVal = NEAT_WRAP::lgamma_r_f(ValArg0.NEATVal, &intRes);
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::lgamma_r]: Not valid data type for built-in");
    }
}


///////////////// remquo built-in function ///////////////////////
void execute_remquo(Function *F,
                      const std::map<Value *, NEATGenericValue> &ArgVals,
                      NEATGenericValue& Result,
                      const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();

    if (Ty0->isVectorTy() && Ty1->isVectorTy()) {

        if (ValArg0.NEATVec.GetSize() != ValArg1.NEATVec.GetSize())
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_remquo]: wrong vector size.");

        const Type *TyElem0 = dyn_cast<VectorType>(Ty0)->getElementType();
        const Type *TyElem1 = dyn_cast<VectorType>(Ty1)->getElementType();

        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // results written to vector intVec
        std::vector<int> intVec;

        if (TyElem0->isFloatTy() && TyElem1->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::remquo_f(ValArg0.NEATVec, ValArg1.NEATVec, intVec);
        } else if (TyElem0->isDoubleTy() && TyElem1->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::remquo_d(ValArg0.NEATVec, ValArg1.NEATVec, intVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_remquo]: Not valid vector data type for built-in");}
    } else if (Ty0->isDoubleTy() && Ty1->isDoubleTy()) {
        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // result written to variable intRes
        int32_t intRes;
        Result.NEATVal = NEAT_WRAP::remquo_d(ValArg0.NEATVal, ValArg1.NEATVal, &intRes);
    } else if (Ty0->isFloatTy() && Ty1->isFloatTy()) {
        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // result written to variable intRes
        int32_t intRes;
        Result.NEATVal = NEAT_WRAP::remquo_f(ValArg0.NEATVal, ValArg1.NEATVal, &intRes);
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_remquo]: Not valid data type for built-in");
    }
}

///////////////// nan built-in function ///////////////////////
void NEATPlugIn::execute_nan(Function *F,
                               const std::map<Value *, NEATGenericValue> &ArgVals,
                               NEATGenericValue& Result,
                               const OCLBuiltinParser::ArgVector& ArgList)
{
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    GenericValue ValArg = GetGenericArg(0);
    const Type *Ty = arg0->getType();
    if (Ty->isIntegerTy()) {
        int n = Ty->getPrimitiveSizeInBits();
        int val = ValArg.IntVal.getSExtValue();
        if( n == 32) {
            Result.NEATVal = NEAT_WRAP::nan_f(uint32_t(val));
        }  else if (n  == 64 ) {
            Result.NEATVal = NEAT_WRAP::nan_d(uint64_t(val));
        } else
            throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_nan]: Not valid data type for built-in (scalar)");
    } else if (Ty->isVectorTy()) {
        const VectorType *VTy = cast<VectorType>(Ty);
        int n = VTy->getElementType()->getPrimitiveSizeInBits();
        if (n == 32) {
            std::vector<uint32_t> vec0;
            for (unsigned i = 0; i < ValArg.AggregateVal.size(); ++i)
                vec0.push_back((uint32_t)(ValArg.AggregateVal[i].IntVal.getZExtValue()));
            Result.NEATVec = NEAT_WRAP::nan_f(vec0);
        } else if (n == 64) {
            std::vector<uint64_t> vec0;
            for (unsigned i = 0; i < ValArg.AggregateVal.size(); ++i)
                vec0.push_back((uint64_t)(ValArg.AggregateVal[i].IntVal.getZExtValue()));
            Result.NEATVec = NEAT_WRAP::nan_d(vec0);
        } else
            throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_nan]: Not valid data type for built-in (vector)");

        if (Result.NEATVec.GetSize() != cast<VectorType>(Ty)->getNumElements())
            throw Exception::NEATTrackFailure("[NEATPlugIn::execute_nan]: wrong vector size.");

    } else
        throw Exception::IllegalFunctionCall("[NEATPlug-in::execute_nan]: Not valid data type for built-in");
}

///////////////// read_imagef built-in function ///////////////////////
void NEATPlugIn::execute_read_imagef(Function *F,
                  const std::map<Value *, NEATGenericValue> &ArgVals,
                  NEATGenericValue& Result,
                  const OCLBuiltinParser::ArgVector& ArgList)
{
    cl_mem_obj_descriptor * memobj = (cl_mem_obj_descriptor *)GetGenericArg(0).PointerVal;

    // samplerless read image functions have 2 parameters,
    // functions with sampler have 3 parameters
    const bool IsSamplerLess = bool(ArgList.size() == 2);
    size_t n = 0;
    uint32_t sampler;

    if(IsSamplerLess) {
        n = 1;
    } else {
        sampler = GetGenericArg(1).IntVal.getZExtValue();
        n = 2;
    }

    // data type of coordinates == float or int
    const OCLBuiltinParser::BasicArgType CoordTy= ArgList[n].vecType.elType;

    // coordinates
    float u = 0.0f, v = 0.0f, w = 0.0f;

    const GenericValue& CoordGV = GetGenericArg(n);

    const cl_mem_object_type objType = memobj->memObjType;

    if( OCLBuiltinParser::FLOAT == CoordTy)
    {
        // TODO: take float coordinates from NEAT context
        // since its NEAT variable natively it should be taken from NEAT context
        // not from Interpreter Context

        // HACK !!!
        // we take coordinates from Interpreter not from NEAT context

        OCLBuiltins::getCoordsByImageType<float,float>(objType,CoordGV,u,v,w);

    } else {
       // int coordinates
       OCLBuiltins::getCoordsByImageType<int32_t,float>(objType,CoordGV,u,v,w);
    }

    DEBUG(dbgs() << "[NEATPlugin] Coordinates u=" << u <<" v=" << v << " w=" << w << "\n");

    cl_image_format im_fmt;
    Conformance::image_descriptor desc = OCLBuiltins::CreateConfImageDesc(*memobj, im_fmt);
    Conformance::image_sampler_data imageSampler;

    if( IsSamplerLess)
    {
        imageSampler.addressing_mode = CL_ADDRESS_NONE;
        imageSampler.filter_mode = CL_FILTER_NEAREST;
        imageSampler.normalized_coords = false;
    }
    else
    {
        imageSampler = OCLBuiltins::CreateSamplerData(sampler);
    }

    NEATVector neatPixelVec = NEAT_WRAP::read_imagef_src_noneat_f(
        memobj->pData, // void *imageData,
        &desc, // image_descriptor *imageInfo,
        u, v, w,
        &imageSampler); // image_sampler_data *imageSampler,

    // return NEAT vector
    Result.NEATVec = neatPixelVec;
}

///////////////// mix built-in function ///////////////////////
void execute_mix(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);
    Value *arg1 = &*Fit++;
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);
    Value *arg2 = &*Fit++;
    const NEATGenericValue& ValArg2 = GetArg(arg2, ArgVals);

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();
    const Type *Ty2 = arg2->getType();

    if (Ty2->isFloatTy()) {
        if (Ty0->isVectorTy() && Ty1->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::mix_f(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVal);
        } else if (Ty0->isFloatTy() && Ty1->isFloatTy()) {
            Result.NEATVal = NEAT_WRAP::mix_f(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::mix]: Not valid data type for built-in");
        }
    } else if (Ty2->isDoubleTy()) {
        if (Ty0->isVectorTy() && Ty1->isVectorTy()) {
            Result.NEATVec = NEAT_WRAP::mix_d(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVal);
        } else if (Ty0->isDoubleTy() && Ty1->isDoubleTy()) {
            Result.NEATVal = NEAT_WRAP::mix_d(ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::mix]: Not valid data type for built-in");
        }
    } else if (Ty2->isVectorTy() && Ty0->isVectorTy() && Ty1->isVectorTy()) {
        const Type *TyElem = dyn_cast<VectorType>(Ty2)->getElementType();
        if (TyElem->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::mix_f(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        } else if (TyElem->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::mix_d(ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::mix]: Not valid vector data type for built-in");}
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::mix]: Not valid data type for built-in");
    }
}

///////////////// frexp built-in function ///////////////////////
void execute_frexp(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){
    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);

    const Type *Ty0 = arg0->getType();

    if (Ty0->isVectorTy()) {
        const Type *TyElem0 = dyn_cast<VectorType>(Ty0)->getElementType();

        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // results written to vector intVec
        std::vector<int> intVec;

        if (TyElem0->isFloatTy()) {
            Result.NEATVec = NEAT_WRAP::frexp_f(ValArg0.NEATVec, intVec);
        } else if (TyElem0->isDoubleTy()) {
            Result.NEATVec = NEAT_WRAP::frexp_d(ValArg0.NEATVec, intVec);
        } else {
            throw Exception::IllegalFunctionCall("[NEATPlug-in::frexp]: Not valid vector data type for built-in");}
    } else if (Ty0->isDoubleTy()) {
        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // result written to variable intRes
        int intRes;
        Result.NEATVal = NEAT_WRAP::frexp_d(ValArg0.NEATVal, &intRes);
    } else if (Ty0->isFloatTy()) {
        // we do not support integer types in NEAT ALU so far, that's why we don't use
        // result written to variable intRes
        int intRes;
        Result.NEATVal = NEAT_WRAP::frexp_f(ValArg0.NEATVal, &intRes);
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::frexp]: Not valid data type for built-in");
    }
}


///////////////// ldexp built-in function ///////////////////////
void NEATPlugIn::execute_ldexp(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){


    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);

    GenericValue ValArg1 = GetGenericArg(1);
    Value *arg1 = &*Fit++;

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();

    if (Ty0->isVectorTy()) {
        const Type *TyElem0 = dyn_cast<VectorType>(Ty0)->getElementType();

        if( Ty1->isVectorTy() ) {

            unsigned sizeIntVec = unsigned(ValArg1.AggregateVal.size());

            if(ValArg0.NEATVec.GetSize() != size_t(sizeIntVec))
                throw Exception::IllegalFunctionCall("[NEATPlug-in::ldexp]: Not valid vector size for built-in");

            std::vector<int> intVec;
            for (unsigned i = 0; i < sizeIntVec; ++i)
            {
                int a = int(ValArg1.AggregateVal[i].IntVal.getSExtValue());
                intVec.push_back(a);
            }
            if (TyElem0->isFloatTy()) {
                Result.NEATVec = NEAT_WRAP::ldexp_f(ValArg0.NEATVec, intVec);
            } else if (TyElem0->isDoubleTy()) {
                Result.NEATVec = NEAT_WRAP::ldexp_d(ValArg0.NEATVec, intVec);
            } else {
                throw Exception::IllegalFunctionCall("[NEATPlug-in::ldexp]: Not valid vector data type for built-in");
            }
        } else if ( Ty1->isIntegerTy() ) {
            int val = int(ValArg1.IntVal.getSExtValue());
            if (TyElem0->isFloatTy()) {
                Result.NEATVec = NEAT_WRAP::ldexp_f(ValArg0.NEATVec, val);
            } else if (TyElem0->isDoubleTy()) {
                Result.NEATVec = NEAT_WRAP::ldexp_d(ValArg0.NEATVec, val);
            } else {
                throw Exception::IllegalFunctionCall("[NEATPlug-in::ldexp]: Not valid vector data type for built-in");
            }
        } else throw Exception::IllegalFunctionCall("[NEATPlug-in::ldexp]: Not valid data type for built-in");

    } else if (Ty0->isDoubleTy()) {
        int val = int(ValArg1.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::ldexp_d(ValArg0.NEATVal, val);
    } else if (Ty0->isFloatTy()) {
        int val = int(ValArg1.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::ldexp_f(ValArg0.NEATVal, val);
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::ldexp]: Not valid data type for built-in");
    }
}

///////////////// pown built-in function ///////////////////////
void NEATPlugIn::execute_pown(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){


    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);

    GenericValue ValArg1 = GetGenericArg(1);
    Value *arg1 = &*Fit++;

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();

    if (Ty0->isVectorTy()) {
        const Type *TyElem0 = dyn_cast<VectorType>(Ty0)->getElementType();

        if( Ty1->isVectorTy() ) {

            size_t sizeIntVec = ValArg1.AggregateVal.size();

            if(ValArg0.NEATVec.GetSize() != sizeIntVec)
                throw Exception::IllegalFunctionCall("[NEATPlug-in::pown]: Not valid vector size for built-in");

            std::vector<int32_t> intVec;
            for (size_t i = 0; i < sizeIntVec; ++i)
            {
                int32_t a = int32_t(ValArg1.AggregateVal[i].IntVal.getSExtValue());
                intVec.push_back(a);
            }
            if (TyElem0->isFloatTy()) {
                Result.NEATVec = NEAT_WRAP::pown_f(ValArg0.NEATVec, intVec);
            } else if (TyElem0->isDoubleTy()) {
                Result.NEATVec = NEAT_WRAP::pown_d(ValArg0.NEATVec, intVec);
            } else {
                throw Exception::IllegalFunctionCall("[NEATPlug-in::pown]: Not valid vector data type for built-in");
            }
        } else throw Exception::IllegalFunctionCall("[NEATPlug-in::pown]: Not valid data type for built-in");

    } else if (Ty0->isDoubleTy()) {
        int32_t val = int32_t(ValArg1.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::pown_d(ValArg0.NEATVal, val);
    } else if (Ty0->isFloatTy()) {
        int32_t val = int32_t(ValArg1.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::pown_f(ValArg0.NEATVal, val);
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::pown]: Not valid data type for built-in");
    }
}
///////////////// rootn built-in function ///////////////////////
void NEATPlugIn::execute_rootn(Function *F,
                 const std::map<Value *, NEATGenericValue> &ArgVals,
                 NEATGenericValue& Result,
                 const OCLBuiltinParser::ArgVector& ArgList){


    Function::arg_iterator Fit = F->arg_begin();
    Value *arg0 = &*Fit++;
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);

    GenericValue ValArg1 = GetGenericArg(1);
    Value *arg1 = &*Fit++;

    const Type *Ty0 = arg0->getType();
    const Type *Ty1 = arg1->getType();

    if (Ty0->isVectorTy()) {
        const Type *TyElem0 = dyn_cast<VectorType>(Ty0)->getElementType();

        if( Ty1->isVectorTy() ) {

            unsigned sizeIntVec = unsigned(ValArg1.AggregateVal.size());

            if(ValArg0.NEATVec.GetSize() != size_t(sizeIntVec))
                throw Exception::IllegalFunctionCall("[NEATPlug-in::rootn]: Not valid vector size for built-in");

            std::vector<int> intVec;
            for (unsigned i = 0; i < sizeIntVec; ++i)
            {
                int a = int(ValArg1.AggregateVal[i].IntVal.getSExtValue());
                intVec.push_back(a);
            }
            if (TyElem0->isFloatTy()) {
                Result.NEATVec = NEAT_WRAP::rootn_f(ValArg0.NEATVec, intVec);
            } else if (TyElem0->isDoubleTy()) {
                Result.NEATVec = NEAT_WRAP::rootn_d(ValArg0.NEATVec, intVec);
            } else {
                throw Exception::IllegalFunctionCall("[NEATPlug-in::rootn]: Not valid vector data type for built-in");
            }
        } else throw Exception::IllegalFunctionCall("[NEATPlug-in::rootn]: Not valid data type for built-in");

    } else if (Ty0->isDoubleTy()) {
        int val = int(ValArg1.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::rootn_d(ValArg0.NEATVal, val);
    } else if (Ty0->isFloatTy()) {
        int val = int(ValArg1.IntVal.getSExtValue());
        Result.NEATVal = NEAT_WRAP::rootn_f(ValArg0.NEATVal, val);
    } else{
        throw Exception::IllegalFunctionCall("[NEATPlug-in::rootn]: Not valid data type for built-in");
    }
}


///////////////// min and max built-in function ///////////////////////
#define EXECUTE_MINMAX(fn)\
    void NEATPlugIn::execute_##fn(Function *F,                                          \
                 const std::map<Value *, NEATGenericValue> &ArgVals,        \
                 NEATGenericValue& Result,                                   \
                 const OCLBuiltinParser::ArgVector& ArgList)                \
{                                                                           \
    Function::arg_iterator Fit = F->arg_begin();                            \
    Value *arg0 = &*Fit++;                                                    \
    Value *arg1 = &*Fit++;                                                    \
    const Type *Ty0 = arg0->getType();                                      \
    const Type *Ty1 = arg1->getType();                                      \
    if(!(m_NTD.IsNEATSupported(Ty0) && m_NTD.IsNEATSupported(Ty1))) return; \
    const NEATGenericValue& ValArg0 = GetArg(arg0, ArgVals);                \
    const NEATGenericValue& ValArg1 = GetArg(arg1, ArgVals);                \
    if (Ty0->isVectorTy() && Ty1->isFloatTy())                              \
    {                                                                       \
     Result.NEATVec = NEAT_WRAP::fn##_f(ValArg0.NEATVec, ValArg1.NEATVal);  \
    }                                                                       \
    else if(Ty0->isVectorTy() && Ty1->isDoubleTy())                         \
    {                                                                       \
     Result.NEATVec = NEAT_WRAP::fn##_d(ValArg0.NEATVec, ValArg1.NEATVal);  \
    }                                                                       \
    else if(Ty0->isFloatTy() && Ty1->isFloatTy())                           \
    {                                                                       \
     Result.NEATVal = NEAT_WRAP::fn##_f(ValArg0.NEATVal, ValArg1.NEATVal);  \
    }                                                                       \
    else if(Ty0->isDoubleTy() && Ty1->isDoubleTy())                         \
    {                                                                       \
     Result.NEATVal = NEAT_WRAP::fn##_d(ValArg0.NEATVal, ValArg1.NEATVal);  \
    }                                                                       \
    else if (Ty0->isIntegerTy() && Ty1->isIntegerTy())                      \
    {                                                                       \
      return;                                                               \
    }                                                                       \
    else if (Ty0->isVectorTy() && Ty1->isIntegerTy())                       \
    {                                                                       \
      return;                                                               \
    }                                                                       \
    else if(Ty0->isVectorTy() && Ty1->isVectorTy())                         \
    {                                                                       \
        const Type *TyElem = dyn_cast<VectorType>(Ty0)->getElementType();   \
        if (TyElem->isFloatTy())                                            \
        {                                                                   \
         Result.NEATVec = NEAT_WRAP::fn##_f(ValArg0.NEATVec, ValArg1.NEATVec);\
        }                                                                   \
        else if(TyElem->isDoubleTy())                                       \
        {                                                                   \
         Result.NEATVec = NEAT_WRAP::fn##_d(ValArg0.NEATVec, ValArg1.NEATVec);\
        }                                                                   \
        else if (TyElem->isIntegerTy())                                     \
        {                                                                   \
          return;                                                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            throw Exception::IllegalFunctionCall(                           \
                "[NEATPlug-in::minmax]: Not valid data type for built-in");   \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        throw Exception::IllegalFunctionCall(                               \
            "[NEATPlug-in::minmax]: Not valid data type for built-in");       \
    }                                                                       \
}

EXECUTE_MINMAX(min)
EXECUTE_MINMAX(max)

/// macro to process built-in function argument type :
/// scalar float or double;
/// or vector float or double;
/// and to call corresponding method in NEATALU
///
/// @param _aluname - NEAT ALU name of function implementing built-in
/// _aluname should be equal to string containing name of built-in
/// extracted from Function *f
/// @param args - scalar arguments to function
/// @param argsvec - vector arguments to function
/// there might be several arguments (1,2,3, more)
#define UBER_BI_CALL(_aluname, args, argsvec)\
    const Type *Ty = Arg0->getType();\
if(Ty->isFloatTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_f args ;}\
else if(Ty->isDoubleTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_d args ;}\
else if(Ty->isVectorTy()){\
    const Type *TyElem = dyn_cast<VectorType>(Ty)->getElementType();\
    if(TyElem->isFloatTy()){\
    Result.NEATVec = NEAT_WRAP::_aluname##_f argsvec ;}\
else if(TyElem->isDoubleTy()){\
    Result.NEATVec = NEAT_WRAP::_aluname##_d argsvec ;}\
else{ \
    throw Exception::IllegalFunctionCall("[NEATPlug-in::"+std::string(#_aluname)+"]: vector data type is not vector of floats or doubles");}\
}\
else{\
    throw Exception::IllegalFunctionCall("[NEATPlug-in::"+std::string(#_aluname)+"]: data type is not float or double");}

#define UBER_BI_CALL_FRM(_aluname, args, argsvec)\
    const Type *Ty = Arg0->getType();\
if(Ty->isFloatTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_frm_f args ;}\
else if(Ty->isVectorTy()){\
    const Type *TyElem = dyn_cast<VectorType>(Ty)->getElementType();\
    if(TyElem->isFloatTy()){\
    Result.NEATVec = NEAT_WRAP::_aluname##_frm_f argsvec ;}\
else{ \
    throw Exception::IllegalFunctionCall("[NEATPlug-in]: Not valid vector data type for built-in");}\
}\
else{\
    throw Exception::IllegalFunctionCall("[NEATPlug-in]: Not valid data type for built-in");}


/// macro to process built-in function argument type :
/// scalar float or double;
/// or vector float or double;
/// and to call corresponding method in NEATALU
/// function returns scalar for both scalar and vector input
/// @param _aluname - NEAT ALU name of function implementing built-in
/// _aluname should be equal to string containing name of built-in
/// extracted from Function *f
/// @param args - scalar arguments to function
/// @param argsvec - vector arguments to function
/// there might be several arguments (1,2,3, more)
#define UBER_BI_CALL_SCALAR_OUT(_aluname, args, argsvec)\
    const Type *Ty = Arg0->getType();\
if(Ty->isFloatTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_f args ;}\
else if(Ty->isDoubleTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_d args ;}\
else if(Ty->isVectorTy()){\
    const Type *TyElem = dyn_cast<VectorType>(Ty)->getElementType();\
    if(TyElem->isFloatTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_f argsvec ;}\
else if(TyElem->isDoubleTy()){\
    Result.NEATVal = NEAT_WRAP::_aluname##_d argsvec ;}\
else{ \
    throw Exception::IllegalFunctionCall("[NEATPlug-in::"+std::string(#_aluname)+"]: vector data type is not vector of floats or doubles");}\
}\
else{\
    throw Exception::IllegalFunctionCall("[NEATPlug-in::"+std::string(#_aluname)+"]: data type is not float or double");}

/// macro Extracts function argument and its NEATGenericValue from NEAT context
/// @param idx - number of function argument
#define GET_ARG(idx)\
    Value * const Arg ## idx = &*Fit++;\
    std::map<Value *, NEATGenericValue>::const_iterator it ## idx =\
    ArgVals.find(Arg ## idx);\
    assert ( it ## idx != ArgVals.end());\
    const NEATGenericValue& ValArg ## idx = it ## idx->second;

// define macro HANDLE_BI_ONEARG for handling built-in like "gentype f(gentype x)"
#define HANDLE_BI_ONEARG(_num, _aluname)  \
    if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0)\
    UBER_BI_CALL(_aluname, (ValArg0.NEATVal), (ValArg0.NEATVec));\
    return true;}

// define macro HANDLE_BI_ONEARG_FRM for handling built-in like "gentype f(gentype x)" in fast relaxed mode
#define HANDLE_BI_ONEARG_FRM(_num, _aluname)  \
    if(#_aluname == BINameStr && isFRMPrecisionOn()){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0)\
    UBER_BI_CALL_FRM(_aluname, (ValArg0.NEATVal), (ValArg0.NEATVec));\
    return true;}

// define macro HANDLE_BI_ONEARG_SCALAR_OUT for handling built-in like "float f(floatn x)"
#define HANDLE_BI_ONEARG_SCALAR_OUT(_num, _aluname)  \
    if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0)\
    UBER_BI_CALL_SCALAR_OUT(_aluname, (ValArg0.NEATVal), (ValArg0.NEATVec));\
    return true;}

// define macro HANDLE_BI_TWOARG for handling built-in like
// "gentype f(gentype x, gentype y)"
#define HANDLE_BI_TWOARG(_num, _aluname) \
    if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) \
    UBER_BI_CALL(_aluname, (ValArg0.NEATVal, ValArg1.NEATVal),\
    (ValArg0.NEATVec, ValArg1.NEATVec)) \
    return true;}

// define macro HANDLE_BI_TWOARG_FRM for handling built-in like
// "gentype f(gentype x, gentype y)" in fast relaxed math mode
#define HANDLE_BI_TWOARG_FRM(_num, _aluname) \
    if(#_aluname == BINameStr && isFRMPrecisionOn()){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) \
    UBER_BI_CALL_FRM(_aluname, (ValArg0.NEATVal, ValArg1.NEATVal),\
    (ValArg0.NEATVec, ValArg1.NEATVec)) \
    return true;}

// define macro HANDLE_BI_TWOARG_SCALAR_OUT for handling built-in like
// "float f(floatn x, floatn y)"
#define HANDLE_BI_TWOARG_SCALAR_OUT(_num, _aluname) \
    if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) \
    UBER_BI_CALL_SCALAR_OUT(_aluname, (ValArg0.NEATVal, ValArg1.NEATVal),\
    (ValArg0.NEATVec, ValArg1.NEATVec)) \
    return true;}

// define macro HANDLE_BI_THREEARG for handling built-in like
// "gentype f(gentype x, gentype y, gentype z)"
#define HANDLE_BI_THREEARG(_num, _aluname) \
if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) GET_ARG(2)\
    UBER_BI_CALL(_aluname, (ValArg0.NEATVal, ValArg1.NEATVal, ValArg2.NEATVal),\
        (ValArg0.NEATVec, ValArg1.NEATVec, ValArg2.NEATVec))\
    return true;}

// define macro HANDLE_BI_ONEIN_TWOOUT_ARGS for handling built-in like
// "gentype f(gentype x, gentype *y)"
#define HANDLE_BI_ONEIN_TWOOUT_ARGS(_num, _aluname) \
    if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) \
    NEATValue * pNeatVal = \
       static_cast<NEATValue *>(ValArg1.PointerVal);\
    VectorWidth vWidth = V1;\
    if(Arg0->getType()->isVectorTy()) vWidth = ValArg0.NEATVec.GetWidth();\
    NEATVector NeatVec(vWidth);\
    UBER_BI_CALL(_aluname, (ValArg0.NEATVal, pNeatVal),\
        (ValArg0.NEATVec, NeatVec))\
    if(Ty->isVectorTy()){\
    for(std::size_t _i=0; _i<ValArg0.NEATVec.GetSize();++_i){\
        pNeatVal[_i] = NeatVec[_i];}\
    }\
    return true;}

// define macro HANDLE_BI_ONEIN_TWOOUT_ARGS for handling built-in like
// "gentype f(gentype x, gentype *y)"
#define HANDLE_BI_ONEIN_TWOOUT_ARGS_FRM(_num, _aluname) \
    if(#_aluname == BINameStr && isFRMPrecisionOn()){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) \
    NEATValue * pNeatVal = \
       static_cast<NEATValue *>(ValArg1.PointerVal);\
    VectorWidth vWidth = V1;\
    if(Arg0->getType()->isVectorTy()) vWidth = ValArg0.NEATVec.GetWidth();\
    NEATVector NeatVec(vWidth);\
    UBER_BI_CALL_FRM(_aluname, (ValArg0.NEATVal, pNeatVal),\
        (ValArg0.NEATVec, NeatVec))\
    if(Ty->isVectorTy()){\
    for(std::size_t _i=0; _i<ValArg0.NEATVec.GetSize();++_i){\
        pNeatVal[_i] = NeatVec[_i];}\
    }\
    return true;}

// define macro handling built-in like
// "gentype f(gentype x, gentype y)"
// "gentype f(gentype x, scalartype y)"
#define HANDLE_BI_SECOND_VECTOR_OR_SCALAR(_num, _aluname) \
    if(#_aluname == BINameStr){\
    Function::arg_iterator Fit = F->arg_begin();\
    GET_ARG(0) GET_ARG(1) \
    if (Arg1->getType()->isVectorTy()){\
    UBER_BI_CALL(_aluname, (ValArg0.NEATVal, ValArg1.NEATVal),\
    (ValArg0.NEATVec, ValArg1.NEATVec)) \
    } else {\
    UBER_BI_CALL(_aluname, (ValArg0.NEATVal, ValArg1.NEATVal),\
    (ValArg0.NEATVec, ValArg1.NEATVal)) \
    }\
    return true;}

// define macro HANDLE_BI_EXECUTE for handling built-in via
// execute method
#define HANDLE_BI_EXECUTE(_num, _aluname) \
    if(#_aluname == BINameStr){\
    execute_ ## _aluname(F, ArgVals, Result, ArgList);\
    return true;}

bool NEATPlugIn::DetectAndExecuteOCLBuiltins( Function *F,
                                             const std::map<Value *, NEATGenericValue> &ArgVals,
                                             NEATGenericValue& Result )
{
    std::string BINameStr;
    OCLBuiltinParser::ArgVector ArgList;

    // try to extract string with OCL built-in
    if(!OCLBuiltinParser::ParseOCLBuiltin(F->getName(),
                    BINameStr, ArgList))
    {
        // TODO: add here detection of non regular OCL built-ins names
        // TODO: detect names which don't have format like "@_Z5trunc...
        // TODO: without number of symbols after "Z"
        // example: __pownd2, __frexpgd2, ...
        return false;
    }

    // detect specific built-ins
    ////////////////////////////////////////////////////////////////////////////////////////////
    // Math part OCL spec 6.11.2 Math functions
    // TODO: uncomment macros when specific built-ins are implemented in NEAT ALU

    //opencl 2.0 fast relaxed math functions Table 7.2
    //placed in top in order to call frm funtions instead of usual if frm extension enabled

    HANDLE_BI_ONEARG_FRM(190, cos);
    HANDLE_BI_ONEARG_FRM(191, exp);
    HANDLE_BI_ONEARG_FRM(192, exp2);
    HANDLE_BI_ONEARG_FRM(193, exp10);
    HANDLE_BI_ONEARG_FRM(194, log);
    HANDLE_BI_ONEARG_FRM(195, log2);
    HANDLE_BI_ONEARG_FRM(196, sin);
    HANDLE_BI_ONEIN_TWOOUT_ARGS_FRM(72, sincos)
    HANDLE_BI_ONEARG_FRM(198, tan);
    HANDLE_BI_TWOARG_FRM(199, pow);

    ////////////////////////////////////////////////////////////////////////////////////////////
    HANDLE_BI_ONEARG(  1,     acos)
    HANDLE_BI_ONEARG(  2,     acosh)
    HANDLE_BI_ONEARG(  3,     acospi)
    HANDLE_BI_ONEARG(  4,     asin)
    HANDLE_BI_ONEARG(  5,     asinh)
    HANDLE_BI_ONEARG(  6,     asinpi)
    HANDLE_BI_ONEARG(  7,     atan)
    HANDLE_BI_TWOARG(  8,     atan2)
    HANDLE_BI_ONEARG(  9,     atanh)
    HANDLE_BI_ONEARG( 10,     atanpi)
    HANDLE_BI_TWOARG( 11,     atan2pi)
    HANDLE_BI_ONEARG( 12,     cbrt)
    HANDLE_BI_ONEARG( 13,     ceil)
    HANDLE_BI_TWOARG( 14,     copysign)
    HANDLE_BI_ONEARG( 15,     cos)
    HANDLE_BI_ONEARG( 16,     cosh)
    HANDLE_BI_ONEARG( 17,     cospi)
//    HANDLE_BI_ONEARG( 18,     erfc)
//    HANDLE_BI_ONEARG( 19,     erf)
    HANDLE_BI_ONEARG( 20,     exp)
    HANDLE_BI_ONEARG( 21,     exp2)
    HANDLE_BI_ONEARG( 22,     exp10)
    HANDLE_BI_ONEARG( 23,     expm1)
    HANDLE_BI_ONEARG( 24,     fabs)
    HANDLE_BI_TWOARG( 25,     fdim)
    HANDLE_BI_ONEARG( 26,     floor)
    HANDLE_BI_THREEARG( 27,     fma)
    HANDLE_BI_SECOND_VECTOR_OR_SCALAR( 28,     fmax)
    HANDLE_BI_SECOND_VECTOR_OR_SCALAR( 30,     fmin)
    HANDLE_BI_TWOARG( 32,     fmod)
    HANDLE_BI_ONEIN_TWOOUT_ARGS(33, fract)

    // frexp(gentype x, __global gentype *iptr)
    // frexp(gentype x, __local gentype *iptr)
    // frexp(gentype x, __private gentype *iptr)
    HANDLE_BI_EXECUTE( 36,     frexp)
    HANDLE_BI_TWOARG( 39,     hypot)
    HANDLE_BI_EXECUTE(40, ilogb)
    // ldexp(gentype x, intn n)
    // ldexp(gentype x, int n)
    HANDLE_BI_EXECUTE( 41,     ldexp)
    HANDLE_BI_ONEARG( 43,     lgamma)
    HANDLE_BI_EXECUTE( 44, lgamma_r)
    HANDLE_BI_ONEARG( 47,     log)
    HANDLE_BI_ONEARG( 48,     log2)
    HANDLE_BI_ONEARG( 49,     log10)
    HANDLE_BI_ONEARG( 50,     log1p)
    HANDLE_BI_ONEARG( 51,     logb)
    HANDLE_BI_THREEARG( 52,     mad)
    HANDLE_BI_TWOARG( 53,     maxmag)
    HANDLE_BI_TWOARG( 54,     minmag)
    HANDLE_BI_ONEIN_TWOOUT_ARGS( 55,     modf)
    HANDLE_BI_EXECUTE(58, nan)
    HANDLE_BI_TWOARG( 59,     nextafter)
    HANDLE_BI_TWOARG( 60,     pow)
    HANDLE_BI_EXECUTE( 61,     pown)
    HANDLE_BI_TWOARG( 62,     powr)
    HANDLE_BI_TWOARG( 63,     remainder)
    // remquo(gentype x, gentype y, __global intn *signp)
    // remquo(gentype x, gentype y,__local intn *signp)
    // remquo(gentype x, gentype z,__private intn *signp)
    HANDLE_BI_EXECUTE( 64, remquo)
    HANDLE_BI_ONEARG( 67,     rint)
    HANDLE_BI_EXECUTE( 68,     rootn)
    HANDLE_BI_ONEARG( 69,     round)
    HANDLE_BI_ONEARG( 70,     rsqrt)
    HANDLE_BI_ONEARG( 71,     sin)
    HANDLE_BI_ONEIN_TWOOUT_ARGS(72, sincos)
    HANDLE_BI_ONEARG( 73,     sinh)
    HANDLE_BI_ONEARG( 74,     sinpi)
    HANDLE_BI_ONEARG( 75,     sqrt)
    HANDLE_BI_ONEARG( 76,     tan)
    HANDLE_BI_ONEARG( 77,     tanh)
    HANDLE_BI_ONEARG( 78,     tanpi)
//    HANDLE_BI_ONEARG( 79,     tgamma)
    HANDLE_BI_ONEARG( 80,     trunc)
    HANDLE_BI_EXECUTE(81, step)
    HANDLE_BI_EXECUTE(82, vload2)
    HANDLE_BI_EXECUTE(83, vload3)
    HANDLE_BI_EXECUTE(84, vload4)
    HANDLE_BI_EXECUTE(85, vload8)
    HANDLE_BI_EXECUTE(86, vload16)
    HANDLE_BI_EXECUTE(87, convert_float)
    HANDLE_BI_EXECUTE(88, convert_float2)
    HANDLE_BI_EXECUTE(89, convert_float3)
    HANDLE_BI_EXECUTE(90, convert_float4)
    HANDLE_BI_EXECUTE(91, convert_float8)
    HANDLE_BI_EXECUTE(92, convert_float16)
    HANDLE_BI_EXECUTE(93, max)
    HANDLE_BI_EXECUTE(94, min)
    HANDLE_BI_EXECUTE(95, mix)

    HANDLE_BI_ONEARG( 96, radians)
    HANDLE_BI_ONEARG( 97, degrees)

    HANDLE_BI_EXECUTE(100, vstore2)
    HANDLE_BI_EXECUTE(101, vstore3)
    HANDLE_BI_EXECUTE(102, vstore4)
    HANDLE_BI_EXECUTE(103, vstore8)
    HANDLE_BI_EXECUTE(104, vstore16)

    HANDLE_BI_EXECUTE( 105,     clamp)
    HANDLE_BI_ONEARG( 106,     sign)
    HANDLE_BI_EXECUTE( 107,     smoothstep)

    HANDLE_BI_EXECUTE( 108, cross)
    HANDLE_BI_TWOARG_SCALAR_OUT( 109, dot)
    HANDLE_BI_TWOARG_SCALAR_OUT( 110, distance)
    HANDLE_BI_ONEARG_SCALAR_OUT( 111, length)
    HANDLE_BI_ONEARG( 112, normalize)
    HANDLE_BI_TWOARG_SCALAR_OUT( 113, fast_distance)
    HANDLE_BI_ONEARG_SCALAR_OUT( 114, fast_length)
    HANDLE_BI_ONEARG( 115, fast_normalize)

    HANDLE_BI_ONEARG( 120,     native_sin)
    HANDLE_BI_ONEARG( 121,     native_cos)
    HANDLE_BI_ONEARG( 122,     native_log2)
    HANDLE_BI_ONEARG( 123,     native_rsqrt)
    HANDLE_BI_ONEARG( 124,     native_log)
    HANDLE_BI_ONEARG( 125,     native_log10)
    HANDLE_BI_ONEARG( 126,     native_exp)
    HANDLE_BI_ONEARG( 127,     native_exp2)
    HANDLE_BI_ONEARG( 128,     native_exp10)

    HANDLE_BI_ONEARG( 129,     half_log)
    HANDLE_BI_ONEARG( 130,     half_log2)
    HANDLE_BI_ONEARG( 131,     half_log10)
    HANDLE_BI_ONEARG( 132,     half_exp)
    HANDLE_BI_ONEARG( 133,     half_exp2)
    HANDLE_BI_ONEARG( 134,     half_exp10)

    HANDLE_BI_EXECUTE(135,  async_work_group_copy)
    HANDLE_BI_EXECUTE(136,  async_work_group_strided_copy)

    HANDLE_BI_EXECUTE(138, read_imagef)
    HANDLE_BI_TWOARG(139, native_divide)
    HANDLE_BI_ONEARG(140, native_recip)
    HANDLE_BI_ONEARG(141, native_tan)
    HANDLE_BI_ONEARG(142, native_sqrt)
    HANDLE_BI_TWOARG(143, native_powr)

    HANDLE_BI_EXECUTE(139, vload_half)
    HANDLE_BI_EXECUTE(140, vload_half2)
    HANDLE_BI_EXECUTE(141, vload_half3)
    HANDLE_BI_EXECUTE(142, vload_half4)
    HANDLE_BI_EXECUTE(143, vload_half8)
    HANDLE_BI_EXECUTE(144, vload_half16)
    HANDLE_BI_EXECUTE(145, vloada_half)
    HANDLE_BI_EXECUTE(146, vloada_half2)
    HANDLE_BI_EXECUTE(147, vloada_half3)
    HANDLE_BI_EXECUTE(148, vloada_half4)
    HANDLE_BI_EXECUTE(149, vloada_half8)
    HANDLE_BI_EXECUTE(150, vloada_half16)
    HANDLE_BI_EXECUTE(151, vstore_half)
    HANDLE_BI_EXECUTE(152, vstore_half2)
    HANDLE_BI_EXECUTE(153, vstore_half3)
    HANDLE_BI_EXECUTE(154, vstore_half4)
    HANDLE_BI_EXECUTE(155, vstore_half8)
    HANDLE_BI_EXECUTE(156, vstore_half16)
    HANDLE_BI_EXECUTE(1000, vstorea_half)
    HANDLE_BI_EXECUTE(1000, vstorea_half2)
    HANDLE_BI_EXECUTE(1000, vstorea_half3)
    HANDLE_BI_EXECUTE(1000, vstorea_half4)
    HANDLE_BI_EXECUTE(1000, vstorea_half8)
    HANDLE_BI_EXECUTE(1000, vstorea_half16)
    HANDLE_BI_EXECUTE(157, convert_double)
    HANDLE_BI_EXECUTE(158, convert_double2)
    HANDLE_BI_EXECUTE(159, convert_double3)
    HANDLE_BI_EXECUTE(160, convert_double4)
    HANDLE_BI_EXECUTE(161, convert_double8)
    HANDLE_BI_EXECUTE(162, convert_double16)
    HANDLE_BI_ONEARG( 163,     half_rsqrt)
    HANDLE_BI_ONEARG( 164,     half_sqrt)
    HANDLE_BI_ONEARG( 165,     half_cos)
    HANDLE_BI_ONEARG( 166,     half_sin)
    HANDLE_BI_ONEARG( 167,     half_tan)
    HANDLE_BI_TWOARG( 168,     half_powr)
    HANDLE_BI_TWOARG( 169,     half_divide)
    HANDLE_BI_ONEARG( 170,     half_recip)
    HANDLE_BI_EXECUTE(171, ISequal)
    HANDLE_BI_EXECUTE(172, ISnotequal)
    HANDLE_BI_EXECUTE(173, ISgreater)
    HANDLE_BI_EXECUTE(174, ISgreaterequal)
    HANDLE_BI_EXECUTE(175, ISless)
    HANDLE_BI_EXECUTE(176, ISlessequal)
    HANDLE_BI_EXECUTE(177, ISlessgreater)
    HANDLE_BI_EXECUTE(178, ISfinite)
    HANDLE_BI_EXECUTE(179, ISinf)
    HANDLE_BI_EXECUTE(180, ISnan)
    HANDLE_BI_EXECUTE(181, ISnormal)
    HANDLE_BI_EXECUTE(182, ISordered)
    HANDLE_BI_EXECUTE(183, ISunordered)
    HANDLE_BI_EXECUTE(184, signbit)
    HANDLE_BI_EXECUTE(185, bitselect)
    HANDLE_BI_EXECUTE(186, select) // built-in, not instruction
    HANDLE_BI_EXECUTE(187, shuffle);
    HANDLE_BI_EXECUTE(188, shuffle2);
    HANDLE_BI_EXECUTE(189, atomic_xchg);
    return false;
}

void NEATPlugIn::visitUIToFPInst( UIToFPInst &I )
{
  HANDLE_EVENT(PRE_INST);

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();
  ExecutionContext &EC = m_pECStack->back();

  const Type* ST = I.getOperand(0)->getType();
  GenericValue Src1 = m_pInterp->getOperandValueAdapter(I.getOperand(0), EC);
  Type* RT = I.getType();
  NEATGenericValue Result;
  if (Type::VectorTyID == ST->getTypeID())
  {
    std::vector<uint64_t> vec;
    vec.resize(cast<VectorType>(RT)->getNumElements());
    for (size_t i = 0; i < Src1.AggregateVal.size(); ++i)
    {
      vec[i] = Src1.AggregateVal[i].IntVal.getZExtValue();
    }
    Type* DstVecType = RT->getScalarType();
    if (Type::FloatTyID == DstVecType->getTypeID())
    {
      Result.NEATVec = NEAT_WRAP::ToFloat_f(vec);
    }
    if (Type::DoubleTyID == DstVecType->getTypeID())
    {
      Result.NEATVec = NEAT_WRAP::ToFloat_d(vec);
    }
  }
  else
  {
    if (RT->getTypeID() == Type::FloatTyID)
    {
      Result.NEATVal = NEAT_WRAP::ToFloat_f(Src1.IntVal.getZExtValue());
    }
    else
    {
      Result.NEATVal = NEAT_WRAP::ToFloat_d(Src1.IntVal.getZExtValue());
    }
  }
  SetValue(&I, Result, SF);
}

void NEATPlugIn::visitSIToFPInst( SIToFPInst &I )
{
  HANDLE_EVENT(PRE_INST);

  DEBUG(dbgs() << "[NEATPlugin] running : " << I << "\n");

  NEATExecutionContext &SF = m_NECStack.back();
  ExecutionContext &EC = m_pECStack->back();

  const Type* ST = I.getOperand(0)->getType();
  GenericValue Src1 = m_pInterp->getOperandValueAdapter(I.getOperand(0), EC);
  Type* RT = I.getType();
  NEATGenericValue Result;
  if (Type::VectorTyID == ST->getTypeID())
  {
    std::vector<int64_t> vec;
    vec.resize(cast<VectorType>(RT)->getNumElements());
    for (size_t i = 0; i < Src1.AggregateVal.size(); ++i)
    {
      vec[i] = Src1.AggregateVal[i].IntVal.getSExtValue();
    }
    Type* DstVecType = RT->getScalarType();
    if (Type::FloatTyID == DstVecType->getTypeID())
    {
      Result.NEATVec = NEAT_WRAP::ToFloat_f(vec);
    }
    if (Type::DoubleTyID == DstVecType->getTypeID())
    {
      Result.NEATVec = NEAT_WRAP::ToFloat_d(vec);
    }
  }
  else
  {
    if (RT->getTypeID() == Type::FloatTyID)
    {
      Result.NEATVal = NEAT_WRAP::ToFloat_f(Src1.IntVal.getSExtValue());
    }
    else
    {
      Result.NEATVal = NEAT_WRAP::ToFloat_d(Src1.IntVal.getSExtValue());
    }
  }
  SetValue(&I, Result, SF);
}

/// getPointerToGlobalIfAvailable - This returns the address of the specified
/// global value if it has already been codegen'd, otherwise it returns null.
///
void *NEATPlugIn::getPointerToGlobalIfAvailable(const GlobalValue *GV) {

    GlobalAddressMapTy::iterator I =
        m_GlobalAddressMap.find(GV);
    return I != m_GlobalAddressMap.end() ? I->second : NULL;
}

void * NEATPlugIn::getOrEmitGlobalVariable( const GlobalVariable *GV )
{
    Type *GlobalType = GV->getType()->getElementType();

    // if NEAT does not support type return NULL
    // and do not allocate global variable
    if(!NEATDataLayout ::IsNEATSupported(GlobalType))
        return 0;

    void *GA = getPointerToGlobalIfAvailable((GlobalValue*) GV);

    if (GA == NULL) {

        // If it's not already specified, allocate memory for the global.
        size_t TypeSize = (size_t)m_NTD.getTypeAllocSize(GlobalType);

        void *Memory = 0;
        // Allocate enough memory to hold the type...
        Memory = new int8_t[TypeSize];

        DEBUG(dbgs() << "NEATPlugin: getOrEmitGlobalVariable "
            "Allocated global variable Type : "
            << *GlobalType << " (" << TypeSize << " bytes) x \n");

        // copy pointer to map
        assert((Memory != 0) && "GlobalMapping already established!");
        m_GlobalAddressMap.insert(std::pair<const GlobalValue*, void * const>(GV, Memory));
    }

    return m_GlobalAddressMap[GV];
}

NEATPlugIn::~NEATPlugIn()
{
}

void * NEATPlugIn::getPointerToGlobal( const GlobalValue *GV )
{
    void *GA = getPointerToGlobalIfAvailable(GV);
    if(GA == NULL)
    {
        throw Exception::ValidationExceptionBase(
            "NEATPlugIn::getPointerToGlobal "
            "Attempt to access  Global variable that has not been created");
    }

    return GA;
}

bool NEATPlugIn::isFRMPrecisionOn()
{
    return ( std::find(m_cFlags.begin(), m_cFlags.end(), CL_FAST_RELAXED_MATH) != m_cFlags.end() &&
        std::find(m_cFlags.begin(), m_cFlags.end(), CL_STD_20) != m_cFlags.end()
        );
}
