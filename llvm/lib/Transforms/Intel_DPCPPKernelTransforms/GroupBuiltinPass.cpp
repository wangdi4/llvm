//===--GroupBuiltinPass.cpp - Process WorkGroup Builtins ---------*- C++ -*-==//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/GroupBuiltinPass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/TypeSize.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ParameterType.h"

#define DEBUG_TYPE "dpcpp-kernel-group-builtin"

#if (defined(_WIN32) && defined(_MSC_VER))
typedef signed __int64 cl_long;
typedef unsigned __int64 cl_ulong;
#else
typedef int64_t cl_long __attribute__((aligned(8)));
typedef uint64_t cl_ulong __attribute__((aligned(8)));
#endif

#define CL_SCHAR_MAX 127
#define CL_SCHAR_MIN (-127 - 1)
#define CL_CHAR_MAX CL_SCHAR_MAX
#define CL_CHAR_MIN CL_SCHAR_MIN
#define CL_UCHAR_MAX 255
#define CL_SHRT_MAX 32767
#define CL_SHRT_MIN (-32767 - 1)
#define CL_USHRT_MAX 65535
#define CL_INT_MAX 2147483647
#define CL_INT_MIN (-2147483647 - 1)
#define CL_UINT_MAX 0xffffffffU
#define CL_LONG_MAX ((cl_long)0x7FFFFFFFFFFFFFFFLL)
#define CL_LONG_MIN ((cl_long)-0x7FFFFFFFFFFFFFFFLL - 1LL)
#define CL_ULONG_MAX ((cl_ulong)0xFFFFFFFFFFFFFFFFULL)

using namespace llvm;
using namespace llvm::NameMangleAPI;
using namespace CompilationUtils;

char GroupBuiltinLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(GroupBuiltinLegacy, DEBUG_TYPE,
                      "Handle WorkGroup BI calls", false, false)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_END(GroupBuiltinLegacy, DEBUG_TYPE, "Handle WorkGroup BI calls",
                    false, false)

GroupBuiltinLegacy::GroupBuiltinLegacy() : ModulePass(ID) {
  initializeGroupBuiltinLegacyPass(*PassRegistry::getPassRegistry());
}

bool GroupBuiltinLegacy::runOnModule(Module &M) {
  auto &RTS = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                  .getResult()
                  .getRuntimeService();
  return Impl.runImpl(M, RTS);
}

void GroupBuiltinLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
}

PreservedAnalyses GroupBuiltinPass::run(Module &M, ModuleAnalysisManager &MAM) {
  auto &RTS = MAM.getResult<BuiltinLibInfoAnalysis>(M).getRuntimeService();
  return runImpl(M, RTS) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

Constant *GroupBuiltinPass::getInitializationValue(Function *Func) {

  // Collect parameters for initialization value selection
  Type *RetType = Func->getReturnType();
  unsigned DataWidth = RetType->isVectorTy()
                           ? cast<FixedVectorType>(RetType)->getNumElements()
                           : 1;
  StringRef FuncName = Func->getName();
  assert(isMangledName(FuncName) &&
         "WG BI function name is expected to be mangled!");
  reflection::FunctionDescriptor FD = demangle(FuncName);
  reflection::RefParamType Param = FD.Parameters[0];
  if (reflection::VectorType *VecParam =
          reflection::dyn_cast<reflection::VectorType>(Param.get()))
    Param = VecParam->getScalarType();
  reflection::PrimitiveType *PrimitiveParam =
      reflection::dyn_cast<reflection::PrimitiveType>(Param.get());
  assert(PrimitiveParam && "WG function parameter should be either primitive "
                           "or vector of primitives");
  reflection::TypePrimitiveEnum DataEnum = PrimitiveParam->getPrimitive();

  Constant *InitVal = nullptr;
  Type *Int8Type = Type::getInt8Ty(*Context);
  Type *Int16Type = Type::getInt16Ty(*Context);
  Type *Int32Type = Type::getInt32Ty(*Context);
  Type *Int64Type = Type::getInt64Ty(*Context);
  // Act according to the function's logic
  if (isWorkGroupAll(FuncName)) {
    // Initial value for work_group_all: 0x1
    switch (DataEnum) {
    case reflection::PRIMITIVE_INT:
      InitVal = ConstantInt::get(Int32Type, 1);
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if (isWorkGroupMax(FuncName)) {
    // Initial value for work_group_..._max: MIN
    switch (DataEnum) {
    case reflection::PRIMITIVE_INT:
      InitVal = ConstantInt::get(Int32Type, CL_INT_MIN);
      break;
    case reflection::PRIMITIVE_UINT:
      InitVal = ConstantInt::get(Int32Type, 0);
      break;
    case reflection::PRIMITIVE_LONG:
      InitVal = ConstantInt::get(Int64Type, CL_LONG_MIN);
      break;
    case reflection::PRIMITIVE_ULONG:
      InitVal = ConstantInt::get(Int64Type, 0);
      break;
    case reflection::PRIMITIVE_HALF:
      InitVal =
          ConstantFP::get(*Context, APFloat::getInf(APFloat::IEEEhalf(), true));
      break;
    case reflection::PRIMITIVE_FLOAT:
      InitVal = ConstantFP::get(*Context,
                                APFloat::getInf(APFloat::IEEEsingle(), true));
      break;
    case reflection::PRIMITIVE_DOUBLE:
      InitVal = ConstantFP::get(*Context,
                                APFloat::getInf(APFloat::IEEEdouble(), true));
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if (isWorkGroupMin(FuncName)) {
    // Initial value for work_group_..._min: MAX
    switch (DataEnum) {
    case reflection::PRIMITIVE_INT:
      InitVal = ConstantInt::get(Int32Type, CL_INT_MAX);
      break;
    case reflection::PRIMITIVE_UINT:
      InitVal = ConstantInt::get(Int32Type, CL_UINT_MAX);
      break;
    case reflection::PRIMITIVE_LONG:
      InitVal = ConstantInt::get(Int64Type, CL_LONG_MAX);
      break;
    case reflection::PRIMITIVE_ULONG:
      InitVal = ConstantInt::get(Int64Type, CL_ULONG_MAX);
      break;
    case reflection::PRIMITIVE_HALF:
      InitVal = ConstantFP::get(*Context,
                                APFloat::getInf(APFloat::IEEEhalf(), false));
      break;
    case reflection::PRIMITIVE_FLOAT:
      InitVal = ConstantFP::get(*Context,
                                APFloat::getInf(APFloat::IEEEsingle(), false));
      break;
    case reflection::PRIMITIVE_DOUBLE:
      InitVal = ConstantFP::get(*Context,
                                APFloat::getInf(APFloat::IEEEdouble(), false));
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if(isWorkGroupMul(FuncName)){
    switch (DataEnum) {
    case reflection::PRIMITIVE_CHAR:
    case reflection::PRIMITIVE_UCHAR:
      InitVal = ConstantInt::get(Int8Type, 1);
      break;
    case reflection::PRIMITIVE_SHORT:
    case reflection::PRIMITIVE_USHORT:
      InitVal = ConstantInt::get(Int16Type, 1);
      break;
    case reflection::PRIMITIVE_INT:
    case reflection::PRIMITIVE_UINT:
      InitVal = ConstantInt::get(Int32Type, 1);
      break;
    case reflection::PRIMITIVE_LONG:
    case reflection::PRIMITIVE_ULONG:
      InitVal = ConstantInt::get(Int64Type, 1);
      break;
    case reflection::PRIMITIVE_HALF:
      InitVal = ConstantFP::get(*Context, APFloat(APFloat::IEEEhalf(), 1));
      break;
    case reflection::PRIMITIVE_FLOAT:
      InitVal = ConstantFP::get(*Context, APFloat(APFloat::IEEEsingle(), 1));
      break;
    case reflection::PRIMITIVE_DOUBLE:
      InitVal = ConstantFP::get(*Context, APFloat(APFloat::IEEEdouble(), 1));
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if(isWorkGroupReduceBitwiseAnd(FuncName)){
    switch (DataEnum) {
    case reflection::PRIMITIVE_CHAR:
      InitVal = ConstantInt::get(Int8Type, -1);
      break;
    case reflection::PRIMITIVE_UCHAR:
      InitVal = ConstantInt::get(Int8Type, CL_UCHAR_MAX);
      break;
    case reflection::PRIMITIVE_SHORT:
      InitVal = ConstantInt::get(Int16Type, -1);
      break;
    case reflection::PRIMITIVE_USHORT:
      InitVal = ConstantInt::get(Int16Type, CL_USHRT_MAX);
      break;
    case reflection::PRIMITIVE_INT:
      InitVal = ConstantInt::get(Int32Type, -1);
      break;
    case reflection::PRIMITIVE_UINT:
      InitVal = ConstantInt::get(Int32Type, CL_UINT_MAX);
      break;
    case reflection::PRIMITIVE_LONG:
      InitVal = ConstantInt::get(Int64Type, -1);
      break;
    case reflection::PRIMITIVE_ULONG:
      InitVal = ConstantInt::get(Int64Type, CL_ULONG_MAX);
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if(isWorkGroupReduceBitwiseOr(FuncName) ||
            isWorkGroupReduceBitwiseXor(FuncName)){
    switch (DataEnum) {
    case reflection::PRIMITIVE_CHAR:
    case reflection::PRIMITIVE_UCHAR:
      InitVal = ConstantInt::get(Int8Type, 0);
      break;
    case reflection::PRIMITIVE_SHORT:
    case reflection::PRIMITIVE_USHORT:
      InitVal = ConstantInt::get(Int16Type, 0);
      break;
    case reflection::PRIMITIVE_INT:
    case reflection::PRIMITIVE_UINT:
      InitVal = ConstantInt::get(Int32Type, 0);
      break;
    case reflection::PRIMITIVE_LONG:
    case reflection::PRIMITIVE_ULONG:
      InitVal = ConstantInt::get(Int64Type, 0);
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if(isWorkGroupReduceLogicalAnd(FuncName)){
    switch (DataEnum) {
    case reflection::PRIMITIVE_INT:
      InitVal = ConstantInt::get(Int32Type, 1);
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  } else if(isWorkGroupReduceLogicalOr(FuncName) ||
            isWorkGroupReduceLogicalXor(FuncName)){
    switch (DataEnum) {
    case reflection::PRIMITIVE_INT:
      InitVal = ConstantInt::get(Int32Type, 0);
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  }else {
    // Initial value for the rest of WG functions: zero
    switch (DataEnum) {
    case reflection::PRIMITIVE_CHAR:
    case reflection::PRIMITIVE_UCHAR:
      InitVal = ConstantInt::get(Int8Type, 0);
      break;
    case reflection::PRIMITIVE_SHORT:
    case reflection::PRIMITIVE_USHORT:
      InitVal = ConstantInt::get(Int16Type, 0);
      break;
    case reflection::PRIMITIVE_INT:
    case reflection::PRIMITIVE_UINT:
      InitVal = ConstantInt::get(Int32Type, 0);
      break;
    case reflection::PRIMITIVE_LONG:
    case reflection::PRIMITIVE_ULONG:
      InitVal = ConstantInt::get(Int64Type, 0);
      break;
    case reflection::PRIMITIVE_HALF:
      InitVal =
          ConstantFP::get(*Context, APFloat::getZero(APFloat::IEEEhalf()));
      break;
    case reflection::PRIMITIVE_FLOAT:
      InitVal =
          ConstantFP::get(*Context, APFloat::getZero(APFloat::IEEEsingle()));
      break;
    case reflection::PRIMITIVE_DOUBLE:
      InitVal =
          ConstantFP::get(*Context, APFloat::getZero(APFloat::IEEEdouble()));
      break;
    default:
      llvm_unreachable("Unsupported WG argument type");
    }
  }

  // Broadcast the constant to vector if relevant
  if (DataWidth > 1) {
    InitVal =
        ConstantVector::getSplat(ElementCount::getFixed(DataWidth), InitVal);
  }

  return InitVal;
}

CallInst *GroupBuiltinPass::getWICall(Instruction *Before, StringRef FuncName,
                                      unsigned DimIdx) {
  // Arguments and parameters
  SmallVector<Type *, 8> ArgTypes;
  SmallVector<Value *, 8> Params;
  Type *Int32Type = Type::getInt32Ty(*Context);
  ArgTypes.push_back(Int32Type);
  Params.push_back(ConstantInt::get(Int32Type, DimIdx));
  // Function object
  FunctionType *FuncType = FunctionType::get(SizeT, ArgTypes, false);
  Function *Func = dyn_cast<Function>(
      M->getOrInsertFunction(FuncName, FuncType).getCallee());
  assert(
      Func &&
      "Non-function object with the same signature identified in the module");
  // Function call
  CallInst *Call =
      CallInst::Create(Func, ArrayRef<Value *>(Params), "WIcall", Before);
  assert(Call && "Couldn't create CALL instruction!");
  return Call;
}

static inline bool isMaskedBroadcast(const Function *F) {
  // For non-masked broadcast, the last argument is a scalar local ID, while
  // the the masked version, it's a vector mask.
  auto *FTy = F->getFunctionType();
  unsigned LastOpIdx = FTy->getNumParams() - 1;
  return FTy->getParamType(LastOpIdx)->isVectorTy();
}

static inline unsigned getNDimForBroadcast(const Function *F) {
  unsigned NDim = F->getFunctionType()->getNumParams() - 1;
  if (isMaskedBroadcast(F))
    NDim--;
  return NDim;
}

// Generates sequence for LinearID calculation for 2D workgroup
static Instruction *calculate2DimLinearID(Instruction *Before, Value *LocalID_0,
                                          Value *LocalSize_0,
                                          Value *LocalID_1) {
  // Calculate 2-dimensional LinearID: local_id(1)*get_local_size(0)+local_id(0)
  BinaryOperator *Mul =
      BinaryOperator::CreateMul(LocalSize_0, LocalID_1, "", Before);
  return BinaryOperator::CreateAdd(Mul, LocalID_0, "getLinearId2D", Before);
}

// Generates sequence for LinearID calculation for 3D workgroup
static Instruction *calculate3DimLinearID(Instruction *Before,
                                          Value *LinearID_2Dim,
                                          Value *LocalSize_0,
                                          Value *LocalSize_1,
                                          Value *LocalID_2) {
  // Calculate 3-dimensional LinearID:
  //    <2-dimensional LinearID> +
  //    local_id(2)*get_local_size(0)*get_local_size(1)
  BinaryOperator *Mul1 =
      BinaryOperator::CreateMul(LocalSize_1, LocalID_2, "", Before);
  BinaryOperator *Mul2 =
      BinaryOperator::CreateMul(LocalSize_0, Mul1, "", Before);
  return BinaryOperator::CreateAdd(Mul2, LinearID_2Dim, "getLinearId3D",
                                   Before);
}

Instruction *GroupBuiltinPass::getLinearIDForBroadcast(CallInst *WGCallInstr) {
  auto *CalledF = WGCallInstr->getCalledFunction();
  unsigned NDim = getNDimForBroadcast(CalledF);

  // For 1-dim workgroup - return get_local_id(0)
  Instruction *RetVal = getWICall(WGCallInstr, mangledGetLID(), 0);
  if (NDim > 1) {
    // For multi-dimensional - start from 2-dim calculation
    CallInst *LocalSize_0 = getWICall(WGCallInstr, mangledGetLocalSize(), 0);
    CallInst *LocalID_1 = getWICall(WGCallInstr, mangledGetLID(), 1);
    RetVal = calculate2DimLinearID(WGCallInstr, RetVal, LocalSize_0, LocalID_1);
    if (NDim > 2) {
      // For 3-dim - account for dimension#2
      CallInst *LocalSize_1 = getWICall(WGCallInstr, mangledGetLocalSize(), 1);
      CallInst *LocalID_2 = getWICall(WGCallInstr, mangledGetLID(), 2);
      RetVal = calculate3DimLinearID(WGCallInstr, RetVal, LocalSize_0,
                                     LocalSize_1, LocalID_2);
    }
  }
  return RetVal;
}

Value *GroupBuiltinPass::calculateLinearIDForBroadcast(CallInst *WGCallInstr) {
  auto *CalledF = WGCallInstr->getCalledFunction();
  unsigned NDim = getNDimForBroadcast(CalledF);

  // For single-dimensional we return local_id parameter as is
  Value *RetVal = WGCallInstr->getArgOperand(1);
  if (NDim > 1) {
    // For multi-dimensional - start from 2-dim calculation
    CallInst *LocalSize_0 = getWICall(WGCallInstr, mangledGetLocalSize(), 0);
    RetVal = calculate2DimLinearID(WGCallInstr, RetVal, LocalSize_0,
                                   WGCallInstr->getArgOperand(2));
    if (NDim > 2) {
      // For 3-dim - account for dimension#2
      CallInst *LocalSize_1 = getWICall(WGCallInstr, mangledGetLocalSize(), 1);
      RetVal =
          calculate3DimLinearID(WGCallInstr, RetVal, LocalSize_0, LocalSize_1,
                                WGCallInstr->getArgOperand(3));
    }
  }
  return RetVal;
}

bool GroupBuiltinPass::runImpl(Module &M, RuntimeService &RTS) {
  this->M = &M;
  Context = &M.getContext();
  auto DL = M.getDataLayout();
  unsigned PointerSizeInBits = M.getDataLayout().getPointerSizeInBits(0);
  assert((32 == PointerSizeInBits || 64 == PointerSizeInBits) &&
         "Unsupported pointer size");
  SizeT = IntegerType::get(*Context, PointerSizeInBits);

  // Initialize barrier utils class with current module.
  Utils.init(&M);

  // Handle async and pipe work-group built-ins.
  InstVec CallWGSimpleFunc =
      Utils.getWGCallInstructions(CALL_BI_TYPE_WG_ASYNC_OR_PIPE);
  for (auto *I : CallWGSimpleFunc) {
    CallInst *WGSimpleCallInst = cast<CallInst>(I);
    // Add Barrier before async function call instruction.
    Utils.createBarrier(WGSimpleCallInst);
    // Add dummyBarrier after async function call instruction.
    Instruction *DummyBarrierCall = Utils.createDummyBarrier();
    DummyBarrierCall->insertAfter(WGSimpleCallInst);
  }

  // Handle WorkGroup built-ins.
  InstVec CallWgFunc = Utils.getWGCallInstructions(CALL_BI_TYPE_WG);
  FuncSet VisitedFunctions;
  for (auto *I : CallWgFunc) {
    CallInst *WGCallInst = cast<CallInst>(I);
    // Replace call to WG-wide function with that to per-WI function
    // (which accumulates the result):

    // Collect info about caller function (where the call resides).
    Function *Func = WGCallInst->getFunction();
    Instruction *FirstInstr = &*Func->getEntryBlock().begin();
    // Mark the function as visited.
    if (VisitedFunctions.insert(Func)) {
      // This is the first time we visit this function.
      //
      // This pass creates alloca instructions and initialize them as part
      // of the solution to resolve a work group builtin.
      // These allocas are uniform for all work items in a group.
      // However, barrier pass handles all allocas as non-uniform.
      // So, in order to prevent that, we add a marker "dummyBarrier" at begin
      // of the function, and make sure all alloca and initialization
      // instructions are added before this marker. Need to add the marker only
      // once.
      Instruction *DummyBarrierCall = Utils.createDummyBarrier(FirstInstr);
      // Update the first instruction to be the marker. All alloca & initialize
      // instructions will be created before this first instruction.
      FirstInstr = DummyBarrierCall;
    }

    // Info about this function call.
    unsigned NumArgs = WGCallInst->arg_size();
    Function *Callee = WGCallInst->getCalledFunction();
    assert(Callee && "Unexpected indirect function invocation");
    StringRef FuncName = Callee->getName();
    Type *RetType = WGCallInst->getType();

    // 1. Add alloca for the accumulated result at the function start...
    AllocaInst *Result = new AllocaInst(RetType, DL.getAllocaAddrSpace(),
                                        "AllocaWGResult", FirstInstr);

    // 2. Initialize the result according to function name and argument
    // type.
    Value *InitValue = getInitializationValue(Callee);
    (void)new StoreInst(InitValue, Result, FirstInstr);

    // 3. Extend function parameter list with pointer to of the accumulated
    // result.

    // a. At first - produce parameter list for the new callee.
    bool IsBroadcast = isWorkGroupBroadCast(FuncName);
    bool IsMaskedBroadcast = IsBroadcast && isMaskedBroadcast(Callee);
    SmallVector<Value *, 2> Params;
    if (!IsBroadcast) {
      // For all WG function, but broadcasts - keep original arguments intact.
      for (unsigned Idx = 0; Idx < NumArgs; Idx++) {
        Value *Arg = WGCallInst->getArgOperand(Idx);
        Params.push_back(Arg);
      }
    } else {
      // For broadcasts - replace "local_id" argument with LINEAR local_id,
      // followed by LINEAR local ID of the WI original 'gentype' argument.
      Value *Arg = WGCallInst->getArgOperand(0);
      Params.push_back(Arg);
      // Linear form of local_id parameter.
      Arg = calculateLinearIDForBroadcast(WGCallInst);
      Params.push_back(Arg);
      // Linear local ID of the WI.
      Arg = getLinearIDForBroadcast(WGCallInst);
      Params.push_back(Arg);

      // Mask
      if (IsMaskedBroadcast) {
        Arg = *(WGCallInst->arg_end() - 1);
        Params.push_back(Arg);
      }
    }
    // Append pointer to return value accumulator.
    Params.push_back(Result);

    // b. Remangle the function name upon appended accumulated result's pointer.
    assert(isMangledName(FuncName) &&
           "WG BI function name is expected to be mangled!");
    reflection::FunctionDescriptor FD = demangle(FuncName);

    if (IsBroadcast) {
      reflection::RefParamType MaskTy;
      if (IsMaskedBroadcast)
        MaskTy = FD.Parameters.back();
      auto IDTy = FD.Parameters[1]; // Get ID type of the local ID

      FD.Parameters.resize(1); // Keep the type of the value to broadcast
      FD.Parameters.push_back(IDTy); // Add type for linear local ID
      FD.Parameters.push_back(IDTy); // Add type for linear ID
      if (IsMaskedBroadcast)
        FD.Parameters.push_back(MaskTy);
    }
    // We're utilizing 'gentype' parameter attribute from the 1st argument,
    // because for a WG function its return type is ALWAYS a pointer to the
    // type of 1st parameter. The better way would be to use a constructor
    // of type argument out of LLVM type, however there is no such in
    // NameMangleAPI. We mangle a __private pointer to that 'gentype'.
    reflection::PointerType *GenT =
        new reflection::PointerType(FD.Parameters[0]);
    GenT->addAttribute(reflection::ATTR_PRIVATE);
    FD.Parameters.push_back(reflection::RefParamType(GenT));
    std::string newFuncName = mangle(FD);

    // c. Create function declaration object (unless the module contains it
    // already)
    // Get the new function declaration out of built-in module list.
    Function *LibFunc = RTS.findFunctionInBuiltinModules(newFuncName);
    assert(LibFunc && "WG builtin is not supported in built-in module");
    Function *NewFunc = importFunctionDecl(this->M, LibFunc);
    assert(NewFunc && "Non-function object with the same signature "
                      "identified in the module");

    // 4. Prepare the call with that to function with extended parameter list.
    CallInst *NewCall = CallInst::Create(NewFunc, ArrayRef<Value *>(Params),
                                         "CallWGForItem", WGCallInst);
    assert(NewCall && "Couldn't create CALL instruction!");
    NewCall->setAttributes(WGCallInst->getAttributes());
    NewCall->setCallingConv(WGCallInst->getCallingConv());
    if (WGCallInst->getDebugLoc()) {
      NewCall->setDebugLoc(WGCallInst->getDebugLoc());
    }

    // 5. Create barrier() call immediately AFTER per-WI call.
    (void)Utils.createBarrier(WGCallInst);

    // 6. For uniform & vectorized WG function - finalize
    // the result
    if (isWorkGroupUniform(FuncName)) {

      // a. Load 'alloca' value of the accumulated result.
      LoadInst *LoadResult = new LoadInst(Result->getAllocatedType(), Result,
                                          "LoadWGFinalResult", WGCallInst);

      // b. Create finalization function object:
      // Remangle with unique name (derived from original WG function name)
      // [Note that the signature is as of original WG function]
      std::string FinalizeFuncName;
      if (!IsBroadcast && RetType->isVectorTy()) {
        FinalizeFuncName = appendWorkGroupFinalizePrefix(FuncName);
      } else {
        FinalizeFuncName = getWorkGroupIdentityFinalize(FuncName);
      }
      // Create function
      // Get the new function declaration out of built-in modules list.
      Function *LibFunc = RTS.findFunctionInBuiltinModules(FinalizeFuncName);
      assert(LibFunc && "WG builtin is not supported in built-in module");
      Function *FinalizeFunc = importFunctionDecl(this->M, LibFunc);
      assert(FinalizeFunc && "Non-function object with the same signature "
                             "identified in the module");

      // c. Create call to finalization function object.
      SmallVector<Value *, 8> Params;
      assert(LoadResult->getType() == FinalizeFunc->arg_begin()->getType() &&
             "First arg type of finalize helper doesn't match!");
      Params.push_back(LoadResult);
      // Create dummy (unused) undef args.
      for (auto I = FinalizeFunc->arg_begin() + 1, E = FinalizeFunc->arg_end();
           I != E; ++I)
        Params.push_back(UndefValue::get(I->getType()));
      CallInst *FinalizeCall =
          CallInst::Create(FinalizeFunc, Params, "CallFinalizeWG", WGCallInst);
      assert(FinalizeCall && "Couldn't create CALL instruction!");

      NewCall = FinalizeCall;
    }

    // 7. Re-initialize the alloca (in case we are in a loop) and create dummy
    // barrier immediately AFTER it to assure we initialize once per work-group.
    (void)new StoreInst(InitValue, Result, WGCallInst);
    (void)Utils.createDummyBarrier(WGCallInst);

    // 8. Discard old function call.
    WGCallInst->replaceAllUsesWith(NewCall);
    WGCallInst->eraseFromParent();
  }

  return !CallWGSimpleFunc.empty() || !CallWgFunc.empty();
}

ModulePass *llvm::createGroupBuiltinLegacyPass() {
  return new llvm::GroupBuiltinLegacy();
}
