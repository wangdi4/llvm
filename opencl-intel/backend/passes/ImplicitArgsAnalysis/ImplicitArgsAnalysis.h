//===-- llvm/Target/ImplicitArgsAnalysis.h - Device BE <--> PCG ---*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//

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
  LLVMContext &C;
  bool Initialized;
public:
  ImplicitArgsAnalysis(LLVMContext *LC = 0)
      : ImmutablePass(ID), ArgTypes(ImplicitArgsUtils::IA_NUMBER), C(*LC),
        Initialized(false) {
    assert(LC && "Got a NULL context");
    initializeImplicitArgsAnalysisPass(*PassRegistry::getPassRegistry());
  }
  // initDuringRun - must be called by each using pass once before calling other
  // methods
  void initDuringRun(unsigned PointerSizeInBits) {
    if (Initialized)
      return;
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
        size_t        LocalSize[MAX_WORK_DIM];
        size_t        WGNumber[MAX_WORK_DIM];
        size_t        LoopIterCount;
        void*         RuntimeCallBacks;
        sLocalId*     NewLocalId;
      };
    */
    std::vector<Type*> members;
    members.push_back(SizetTy);
    members.push_back(ArrayType::get(SizetTy, MAX_WORK_DIM));   // Global offset
    members.push_back(ArrayType::get(SizetTy, MAX_WORK_DIM));   // Global size
    members.push_back(ArrayType::get(SizetTy, MAX_WORK_DIM));   // WG size/Local size
    members.push_back(ArrayType::get(SizetTy, MAX_WORK_DIM));   // Number of groups
    members.push_back(SizetTy);                                 // Loop iter count
    members.push_back(PointerType::get(StructType::get(C), 0)); // Runtime Callbacks
    members.push_back(PointerType::get(PaddedDimIdTy, 0));      // NewLocalID
    StructType *pWorkDimType = StructType::get(C, members, false);
    // Initialize the implicit argument types
    for (unsigned ID = 0; ID < ArgTypes.size(); ++ID)
    switch (ID) {
    default:
      assert(false && "Unknown implicit arg ID");
      llvm_unreachable("Unknown implicit arg ID");
    case ImplicitArgsUtils::IA_SLM_BUFFER:
      ArgTypes[ID] = PointerType::get(IntegerType::get(C, 8), 3);
      break;
    case ImplicitArgsUtils::IA_WORK_GROUP_INFO:
      ArgTypes[ID] = PointerType::get(pWorkDimType, 0);
      break;
    case ImplicitArgsUtils::IA_WORK_GROUP_ID:
      ArgTypes[ID] = SizetPtrTy;
      break;
    case ImplicitArgsUtils::IA_GLOBAL_BASE_ID:
      ArgTypes[ID] = PaddedDimIdTy;
      break;
    case ImplicitArgsUtils::IA_BARRIER_BUFFER:
      ArgTypes[ID] = PointerType::get(IntegerType::get(C, 8), 0);
      break;
    case ImplicitArgsUtils::IA_CURRENT_WORK_ITEM:
      ArgTypes[ID] = SizetPtrTy;
      break;
    case ImplicitArgsUtils::IA_RUNTIME_CONTEXT:
      ArgTypes[ID] = PointerType::get(StructType::get(C), 0);
      break;
    }
    Initialized = true;
  }
  // GetArgType - Returns the type of the ID implicit argument
  Type *getArgType(unsigned ID) { return ArgTypes[ID]; }

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
  Value *GenerateGetNewLocalID(Value *WorkInfo, Value *Idx,
                                     Value *Dimension, IRBuilder<> &Builder) {
    unsigned RecordID = NDInfo::NEW_LOCAL_ID;
    SmallVector<Value*, 4> params;
    params.push_back(ConstantInt::get(IntegerType::get(C, 32), 0));
    params.push_back(ConstantInt::get(IntegerType::get(C, 32), RecordID));
    Value *pAddr = Builder.CreateGEP(WorkInfo, params);
    Value *Base = Builder.CreateLoad(pAddr);
    params.clear();
    params.push_back(Idx);
    params.push_back(Dimension);
    pAddr = Builder.CreateGEP(Base, params);
    std::string Name(NDInfo::getRecordName(RecordID));
    AppendWithDimension(Name, Dimension);
    return Builder.CreateLoad(pAddr, Name);
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

  Value *GenerateGetLocalSize(Value *WorkInfo, unsigned Dimension,
                                    IRBuilder<> &Builder) {
    return GenerateGetLocalSize(
        WorkInfo, ConstantInt::get(IntegerType::get(C, 32), Dimension),
        Builder);
  }
  Value *GenerateGetLocalSize(Value *WorkInfo, Value *Dimension,
                                    IRBuilder<> &Builder) {
    return GenerateGetFromWorkInfo(NDInfo::LOCAL_SIZE, WorkInfo, Dimension,
                                   Builder);
  }

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
  Value *GenerateGetNewGlobalID(Value *WorkInfo, Value *BaseGlobalID,
                                Value *Idx, Value *Dimension,
                                IRBuilder<> &Builder) {
    std::string Name("NewGlobalID_");
    AppendWithDimension(Name, Dimension);
    Value *LID = GenerateGetNewLocalID(WorkInfo, Idx, Dimension, Builder);
    Value *BGID = GenerateGetBaseGlobalID(BaseGlobalID, Dimension, Builder);
    return Builder.CreateAdd(LID, BGID, Name);
  }
  static char ID; // Pass identification, replacement for typeid
};

} // End namespace
extern "C" llvm::ImmutablePass * createImplicitArgsAnalysisPass(LLVMContext *C);
#endif

