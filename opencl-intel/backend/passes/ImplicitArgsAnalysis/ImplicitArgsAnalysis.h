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

#ifndef IMPLICIT_ARGS_ANALYSIS_H
#define IMPLICIT_ARGS_ANALYSIS_H

#include "ImplicitArgsUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/IRBuilder.h"
#include "common_dev_limits.h"
#include <cassert>

using namespace llvm;

using namespace Intel::OpenCL::DeviceBackend;
namespace intel {

static void AppendWithDimension(std::string &S, const Value *Dimension) {
  if (const ConstantInt *C = dyn_cast<ConstantInt>(Dimension))
    S += '0' + C->getZExtValue();
  else
    S += "var";
}

void initializeImplicitArgsAnalysisPass(PassRegistry &);
/// ImplicitArgsAnalysis - TODO: document
class ImplicitArgsAnalysis : public ImmutablePass {
private:
  // Each entry matches an IMPLICIT_ARGS enum
  SmallVector<Type*, 16> ArgTypes;
  // Breakdown of WORK_GROUP_INFO which is a structure
  SmallVector<Type*, 16> WGInfoMembersTypes;
  LLVMContext &C;
  unsigned InitializedTo;
public:
  ImplicitArgsAnalysis(LLVMContext *LC = 0)
      : ImmutablePass(ID), ArgTypes(ImplicitArgsUtils::IA_NUMBER, 0),
        WGInfoMembersTypes(NDInfo::LAST, 0), C(*LC), InitializedTo(0) {
    assert(LC && "Got a NULL context");
    initializeImplicitArgsAnalysisPass(*PassRegistry::getPassRegistry());
  }
  // initDuringRun - must be called by each using pass once before calling other
  // methods
  void initDuringRun(unsigned PointerSizeInBits) {
    assert(PointerSizeInBits == 32 || PointerSizeInBits == 64);
    if (InitializedTo == PointerSizeInBits)
      return;
    InitializedTo = PointerSizeInBits;
    IntegerType *SizetTy = IntegerType::get(C, PointerSizeInBits);
    PointerType *SizetPtrTy = PointerType::get(SizetTy, 0);
    /*
      struct sLocalId
      {
        size_t  Id[MAX_WORK_DIM];
      };
    */
    // Create Work Group/Work Item info structures
    assert(MAX_WI_DIM_POW_OF_2 == 4 && "MAX_WI_DIM_POW_OF_2 is not equal to 4!");
    //use 4 instead of MAX_WORK_DIM for alignment & for better calculation of offset in Local ID buffer
    Type *PaddedDimIdTy = ArrayType::get(SizetTy, MAX_WI_DIM_POW_OF_2);

    /*
      struct sWorkInfo
      {
        size_t        uiWorkDim;
        size_t        GlobalOffset[MAX_WORK_DIM];
        size_t        GlobalSize[MAX_WORK_DIM];
        size_t        LocalSize[WG_SIZE_NUM][MAX_WORK_DIM];
        size_t        WGNumber[MAX_WORK_DIM];
        void*         RuntimeCallBacks;
      };
    */

    Type* AbstractPtr = PointerType::get(StructType::get(C), 0);
    Type* Sizet3Ty = ArrayType::get(SizetTy, MAX_WORK_DIM);
    Type* Sizet2x3Ty = ArrayType::get(ArrayType::get(SizetTy, MAX_WORK_DIM), WG_SIZE_NUM);
    WGInfoMembersTypes[NDInfo::WORK_DIM] = SizetTy;
    WGInfoMembersTypes[NDInfo::GLOBAL_OFFSET] = Sizet3Ty;
    WGInfoMembersTypes[NDInfo::GLOBAL_SIZE] = Sizet3Ty;
    WGInfoMembersTypes[NDInfo::LOCAL_SIZE] = Sizet2x3Ty;
    WGInfoMembersTypes[NDInfo::WG_NUMBER] = Sizet3Ty;
    WGInfoMembersTypes[NDInfo::RUNTIME_INTERFACE] = AbstractPtr;
    WGInfoMembersTypes[NDInfo::BLOCK2KERNEL_MAPPER] = AbstractPtr;
    assert(NDInfo::BLOCK2KERNEL_MAPPER + 1 == NDInfo::LAST);
    // Initialize the implicit argument types
    ArgTypes[ImplicitArgsUtils::IA_SLM_BUFFER] =
        PointerType::get(IntegerType::get(C, 8), 3);
    ArgTypes[ImplicitArgsUtils::IA_WORK_GROUP_INFO] =
        PointerType::get(StructType::get(C, WGInfoMembersTypes, false), 0);
    ArgTypes[ImplicitArgsUtils::IA_WORK_GROUP_ID] = SizetPtrTy;
    ArgTypes[ImplicitArgsUtils::IA_GLOBAL_BASE_ID] = PaddedDimIdTy;
    ArgTypes[ImplicitArgsUtils::IA_BARRIER_BUFFER] =
        PointerType::get(IntegerType::get(C, 8), 0);
    ArgTypes[ImplicitArgsUtils::IA_RUNTIME_HANDLE] =
        PointerType::get(StructType::get(C), 0);
    assert(ImplicitArgsUtils::IA_RUNTIME_HANDLE + 1 ==
           ImplicitArgsUtils::IA_NUMBER);
  }
  // getArgType - Returns the type of the ID implicit argument
  Type *getArgType(unsigned ID) { return ArgTypes[ID]; }
  const Type *getArgType(unsigned ID) const { return ArgTypes[ID]; }
  // getWorkGroupInfoMemberType - Returns the type of the ID member of
  // WORK_GROUP_INFO implicit argument (which is a struct)
  Type *getWorkGroupInfoMemberType(unsigned ID) {
    return WGInfoMembersTypes[ID];
  }
  const Type *getWorkGroupInfoMemberType(unsigned ID) const {
    return WGInfoMembersTypes[ID];
  }

  ~ImplicitArgsAnalysis() {
  }

  Value *GenerateGetFromWorkInfo(unsigned RecordID, Value *WorkInfo, unsigned Dimension,
                                    IRBuilder<> &Builder) {
    return GenerateGetFromWorkInfo(
        RecordID, WorkInfo,
        ConstantInt::get(IntegerType::get(C, 32), Dimension), Builder);
  }
  Value *GenerateGetFromWorkInfo(unsigned RecordID, Value *WorkInfo, Value *Dimension,
                                    IRBuilder<> &Builder) {
    assert(RecordID < NDInfo::LAST && "Invalid value for RecordID");
    SmallVector<Value*, 4> params;
    params.push_back(ConstantInt::get(Type::getInt32Ty(C), 0));
    params.push_back(ConstantInt::get(Type::getInt32Ty(C), RecordID));
    params.push_back(Dimension);
    Value *pAddr = Builder.CreateGEP(WorkInfo, ArrayRef<Value *>(params));
    std::string Name(NDInfo::getRecordName(RecordID));
    AppendWithDimension(Name, Dimension);
    //return Builder.CreateAlignedLoad(pAddr, 1, Name);
    return Builder.CreateLoad(pAddr, Name);
  }
  Value *GenerateGetFromWorkInfo(unsigned RecordID, Value *WorkInfo,
                                    IRBuilder<> &Builder) {
    assert(RecordID < NDInfo::LAST && "Invalid value for RecordID");
    SmallVector<Value*, 4> params;
    Type *Int32Ty = Type::getInt32Ty(C);
    params.push_back(ConstantInt::get(Int32Ty, 0));
    params.push_back(ConstantInt::get(Int32Ty, RecordID));
    Value *pAddr = Builder.CreateGEP(WorkInfo, ArrayRef<Value *>(params));
    std::string Name(NDInfo::getRecordName(RecordID));
    //Value *V = Builder.CreateAlignedLoad(pAddr);
    Value *V = Builder.CreateLoad(pAddr);
    if (V->getType() != Int32Ty && RecordID == NDInfo::WORK_DIM)
      V = Builder.CreateTrunc(V, Int32Ty);
    V->setName(Name);
    return V;
  }
  Value *GenerateGetGlobalOffset(Value *WorkInfo, unsigned Dimension,
                                       IRBuilder<> &Builder) {
    return GenerateGetGlobalOffset(
        WorkInfo, ConstantInt::get(IntegerType::get(C, 32), Dimension),
        Builder);
  }
  Value *GenerateGetGlobalOffset(Value *WorkInfo, Value *Dimension,
                                       IRBuilder<> &Builder) {
    return GenerateGetFromWorkInfo(NDInfo::GLOBAL_OFFSET, WorkInfo, Dimension,
                                   Builder);
  }

  Value *GenerateGetLocalSize(bool uniformWGSize, Value *WorkInfo, Argument *pWGId, Value *Dimension,
                                    IRBuilder<> &Builder) {

    // The OpenCL 2.0 program can be compiled with the option
    // -cl-uniform-work-group-size.
    if(uniformWGSize) {
      return GenerateGetEnqueuedLocalSize(WorkInfo, Dimension, Builder);
    }

    // The non-uniform local size is stored as the second array so
    // get_local_size must return value from it in case it is the last WG
    // in the specified dimension.
    // %WGIdPlusOne = add nsw i64 %WGId, 1
    // %LastWG = icmp eq i64 %WGIdPlusOne, %WGNum
    // %LocalSizeIdx = zext i1 %2 to i32
    Value *WGNum = GenerateGetFromWorkInfo(NDInfo::WG_NUMBER, WorkInfo,
                                           Dimension, Builder);
    Value *WGId = GenerateGetGroupID(pWGId, Dimension, Builder);
    Value *WGIdPlusOne = Builder.CreateNSWAdd(WGId, ConstantInt::get(WGId->getType(), 1));
    Value *LastWG = Builder.CreateICmpEQ(WGNum, WGIdPlusOne);
    Value *LocalSizeIdx = Builder.CreateZExt(LastWG, IntegerType::get(C, 32));

    return GenerateGetLocalSizeGeneric(WorkInfo, LocalSizeIdx,
                                         Dimension, Builder);
  }

  Value *GenerateGetEnqueuedLocalSize(Value *WorkInfo, unsigned Dimension,
                                    IRBuilder<> &Builder) {
    // the uniform local size is stored as the first array
    return GenerateGetEnqueuedLocalSize(
        WorkInfo,
        ConstantInt::get(IntegerType::get(C, 32), Dimension),
        Builder);
  }


  Value *GenerateGetEnqueuedLocalSize(Value *WorkInfo, Value *Dimension,
                                    IRBuilder<> &Builder) {
    // the uniform local size is stored as the first array
    return GenerateGetLocalSizeGeneric(
        WorkInfo,
        ConstantInt::get(IntegerType::get(C, 32), 0),
        Dimension,
        Builder);
  }

private:
  // the following implementation is generic for get_local_size and get_enqueued_local_size
  Value *GenerateGetLocalSizeGeneric(Value *WorkInfo, Value * LocalSizeIdx, Value *Dimension,
                                    IRBuilder<> &Builder) {
    SmallVector<Value*, 4> params;
    params.push_back(ConstantInt::get(Type::getInt32Ty(C), 0));
    params.push_back(ConstantInt::get(Type::getInt32Ty(C), NDInfo::LOCAL_SIZE));
    params.push_back(LocalSizeIdx);
    params.push_back(Dimension);
    Value *pAddr = Builder.CreateGEP(WorkInfo, ArrayRef<Value *>(params));
    std::string Name(NDInfo::getRecordName(NDInfo::LOCAL_SIZE));
    AppendWithDimension(Name, Dimension);
    return Builder.CreateLoad(pAddr, Name);
  }

public:
  Value *GenerateGetGroupID(Value *GroupID, unsigned Dimension,
                                  IRBuilder<> &Builder) {
    return GenerateGetGroupID(
        GroupID, ConstantInt::get(IntegerType::get(C, 32), Dimension),
        Builder);
  }
  Value *GenerateGetGroupID(Value *GroupID, Value *Dimension,
                                  IRBuilder<> &Builder) {
    Value *pIdAddr = Builder.CreateGEP(GroupID, Dimension);
    std::string Name("GroupID_");
    AppendWithDimension(Name, Dimension);
    return Builder.CreateLoad(pIdAddr, Name);
  }

  Value *GenerateGetBaseGlobalID(Value *BaseGlobalID, Value *Dimension,
                                       IRBuilder<> &Builder) {
    std::string Name("BaseGlobalID_");
    AppendWithDimension(Name, Dimension);
    if (ConstantInt *Dim = dyn_cast<ConstantInt>(Dimension)) {
      unsigned D = Dim->getZExtValue();
      return Builder.CreateExtractValue(BaseGlobalID, ArrayRef<unsigned>(D),
                                        Name);
    } else {
      // Cannot create an 'extractvalue' with non-const index, so need to pass
      // thru memory
      IRBuilder<> AllocaBuilder(
          &*Builder.GetInsertBlock()->getParent()->begin()->begin());
      AllocaInst *A = AllocaBuilder.CreateAlloca(BaseGlobalID->getType(), 0,
                                                 "alloc_BaseGlobalID");
      Builder.CreateStore(BaseGlobalID, A);
      std::vector<Value *> Indices;
      Indices.push_back(ConstantInt::get(IntegerType::get(C, 32), 0));
      Indices.push_back(Dimension);
      Value *GEP = Builder.CreateGEP(A, Indices);
      return Builder.CreateLoad(GEP, Name);
    }
  }
  static char ID; // Pass identification, replacement for typeid
};

} // End namespace
extern "C" llvm::ImmutablePass * createImplicitArgsAnalysisPass(LLVMContext *C);
#endif

