#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===--- VPOParoptTargetSPIRV.cpp - SPIRV-specific offloading support -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// VPOParoptTargetSPIRV.cpp implements SPIRV-specific parts of the omp target
/// feature.
///
//===----------------------------------------------------------------------===//

#include "VPOParoptMasterThreadRegion.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"

#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/PostDominators.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Scalar/InferAddressSpaces.h"
#include "llvm/Transforms/Scalar/SROA.h"

#include "llvm/Analysis/VPO/VPOParoptConstants.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-target-spirv"

static cl::opt<uint64_t> KernelArgsSizeLimit(
    "vpo-paropt-kernel-args-size-limit", cl::Hidden, cl::init(1024),
    cl::desc("Maximum total size in bytes of the arguments for a kernel"));

static cl::opt<bool> SimplifyBarrierFences(
    "vpo-paropt-simplify-workgroup-barrier-fences", cl::Hidden, cl::init(true),
    cl::ZeroOrMore,
    cl::desc(
        "Try to eliminate global fences when adding workgroup barriers after "
        "parallel regions"));

static cl::opt<bool> DisableParallelBarriers(
    "vpo-paropt-disable-parallel-workgroup-barriers", cl::Hidden,
    cl::init(false), cl::ZeroOrMore,
    cl::desc("Disable adding workgroup barriers after parallel regions"));

static cl::opt<uint32_t> FixedSIMDWidth(
    "vpo-paropt-fixed-simd-width", cl::Hidden, cl::init(0),
    cl::desc("Fixed SIMD width for all target regions in the module."));

extern cl::opt<bool> AtomicFreeReduction;
extern cl::opt<bool> AtomicFreeReductionUseSLM;
extern cl::opt<uint32_t> AtomicFreeRedLocalBufSize;
extern cl::opt<uint32_t> AtomicFreeRedGlobalBufSize;

// Return true if the device triple contains spir64 or spir.
bool VPOParoptTransform::deviceTriplesHasSPIRV() {
  for (const auto &T : MT->getDeviceTriples()) {
    if (T.getArch() == Triple::ArchType::spir ||
        T.getArch() == Triple::ArchType::spir64)
      return true;
  }
  return false;
}

Function *VPOParoptTransform::finalizeKernelFunction(
    WRNTargetNode *WT, Function *Fn, CallInst *&Call,
    const SmallVectorImpl<uint64_t> &MapTypes,
#if INTEL_CUSTOMIZATION
    const SmallVectorImpl<bool> &IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
    const SmallVectorImpl<bool> &IsFunctionPtr,
    const SmallVectorImpl<Constant *> &ConstSizes) {

  assert(isTargetSPIRV() &&
         "finalizeKernelFunction called for non-SPIRV target.");

  FunctionType *FnTy = Fn->getFunctionType();
  auto &C = Fn->getContext();
  Module *M = Fn->getParent();
  const DataLayout &DL = M->getDataLayout();
  SmallVector<Type *, 8> ParamsTy;

  unsigned AddrSpaceGlobal = vpo::ADDRESS_SPACE_GLOBAL;

  // Initialize a descriptor for each kernel argument.
  struct KernelArgInfoDesc {
    // If true, then the argument is passed "by value" rather than
    // as a pointer.
    bool IsByVal = false;
    // Size of the argument in bytes, which is a size of the argument data
    // for "by value" arguments, and a size of a pointer for pointer arguments.
    uint32_t Size = 0;
    // If the argument is passed "by value", then it specifies the new data type
    // for the argument, otherwise it is nullptr.
    // For example, if the original argument is 'struct { int x; }* %arg', then
    // it is passed "by value" as 'struct { char xx[4]; }* byval(...) %arg',
    // and the ByValTy is 'struct { char xx[4]; }'.
    Type *ByValTy = nullptr;
    KernelArgInfoDesc(bool IsByVal, uint32_t Size, Type *ByValTy = nullptr)
        : IsByVal(IsByVal), Size(Size), ByValTy(ByValTy) {
      assert((!IsByVal || ByValTy) &&
             "Must specify ByValTy for IsByVal arguments.");
    }
  };
  std::vector<KernelArgInfoDesc> KernelArgInfo;

  // Maximum total size in bytes of the arguments passed by value.
  uint64_t ByValLimit = KernelArgsSizeLimit + 1;

  // For the given argument of the kernel return the corresponding index
  // in the mapping structures.
  auto GetMapIdxForArgNum = [&MapTypes](uint32_t ArgNum) {
    uint32_t Idx = 0;
    for (auto MT : MapTypes) {
      if ((MT & TGT_MAP_TARGET_PARAM) != 0) {
        if (ArgNum == 0)
          return Idx;
        --ArgNum;
      }
      ++Idx;
    }
    llvm_unreachable("kernel argument without a map descriptor.");
  };

  // Generate '<kernel-name>_kernel_info' global variable that specifies
  // whether a kernel argument is passed by value or not.
  // Versions 1-5 of the data structure use the following format:
  //   struct KernelInfoTy {
  //     uint32_t Version = [1 .. 5];
  //     uint32_t ArgsNum = <number of the kernel arguments>;
  //     struct ArgDescTy {
  //       uint32_t IsByVal = 0/1;
  //       uint32_t Size = <argument size>;
  //     } ArgsDesc[ArgsNum];
  //     uint64_t Attributes1; // Since version 2.
  //     uint64_t WGNum;       // Since version 3.
  //     uint64_t WINum;       // Since version 4.
  //   };
  auto GenerateKernelArgInfoVar =
      [this, &WT](const std::vector<KernelArgInfoDesc> &KernelArgInfo,
                  Function *Fn, bool HasTeamsReduction,
                  bool HasLocalAFReduction, bool HasGlobalAFReduction,
                  bool HasKnownNDRange) {
        auto &C = Fn->getContext();
        size_t ArgsNum = KernelArgInfo.size();
        assert(ArgsNum == Fn->getFunctionType()->params().size() &&
               "Missing param description.");
        assert(ArgsNum <= std::numeric_limits<uint32_t>::max() &&
               "Too many kernel arguments for version 1 kernel info.");
        SmallVector<Type *, 3> KernelInfoInitMemberTypes;
        SmallVector<Constant *, 10> KernelInfoInitBuffer;

        // The current version is 5.
        KernelInfoInitMemberTypes.push_back(Type::getInt32Ty(C));
        KernelInfoInitBuffer.push_back(
            ConstantInt::get(Type::getInt32Ty(C), 5));
        // Specify the number of kernel argument.
        KernelInfoInitMemberTypes.push_back(Type::getInt32Ty(C));
        KernelInfoInitBuffer.push_back(
            ConstantInt::get(Type::getInt32Ty(C), ArgsNum));

        StructType *KernelInfoInitTy;

        if (ArgsNum > 0) {
          StructType *KernelArgInfoTy =
              StructType::create({Type::getInt32Ty(C), Type::getInt32Ty(C)});

          SmallVector<Constant *, 10> KernelArgInfoInitBuffer;
          for (auto &ArgInfo : KernelArgInfo) {
            // For each argument specify whether it is passed by value
            // and the argument size.
            Constant *ArgInit = ConstantStruct::get(
                KernelArgInfoTy,
                {ConstantInt::get(KernelArgInfoTy->getElementType(0),
                                  ArgInfo.IsByVal),
                 ConstantInt::get(KernelArgInfoTy->getElementType(1),
                                  ArgInfo.Size)});
            KernelArgInfoInitBuffer.push_back(ArgInit);
          }
          ArrayType *KernelArgInfoInitArrayTy =
              ArrayType::get(KernelArgInfoTy, ArgsNum);
          Constant *KernelArgInfoInitArray = ConstantArray::get(
              KernelArgInfoInitArrayTy, KernelArgInfoInitBuffer);
          KernelInfoInitMemberTypes.push_back(KernelArgInfoInitArrayTy);
          KernelInfoInitBuffer.push_back(KernelArgInfoInitArray);
        }

        // Init Attributes1.
        uint64_t Attributes1 = 0;
        Attributes1 |= (HasTeamsReduction ? 1 : 0);
        Attributes1 |= (HasKnownNDRange ? (1 << 1) : 0);

        KernelInfoInitMemberTypes.push_back(Type::getInt64Ty(C));
        KernelInfoInitBuffer.push_back(
            ConstantInt::get(Type::getInt64Ty(C), Attributes1));

        uint64_t UseGPURedWGLimit =
            ((HasTeamsReduction && HasGlobalAFReduction) ||
             (!AtomicFreeReductionUseSLM && HasLocalAFReduction))
                ? AtomicFreeRedGlobalBufSize
                : 0;
        KernelInfoInitMemberTypes.push_back(Type::getInt64Ty(C));
        KernelInfoInitBuffer.push_back(
            ConstantInt::get(Type::getInt64Ty(C), UseGPURedWGLimit));

        uint64_t UseGPURedWILimit =
            (AtomicFreeReduction && HasLocalAFReduction &&
             UsedLocalTreeReduction.count(WT))
                ? AtomicFreeRedLocalBufSize
                : 0;
        KernelInfoInitMemberTypes.push_back(Type::getInt64Ty(C));
        KernelInfoInitBuffer.push_back(
            ConstantInt::get(Type::getInt64Ty(C), UseGPURedWILimit));

        KernelInfoInitTy = StructType::create(KernelInfoInitMemberTypes);
        Constant *KernelInfoInit =
            ConstantStruct::get(KernelInfoInitTy, KernelInfoInitBuffer);

        GlobalVariable *KernelInfoVar =
            new GlobalVariable(*Fn->getParent(), KernelInfoInitTy,
                               /*isConstant=*/true, GlobalValue::WeakAnyLinkage,
                               KernelInfoInit, Fn->getName() + "_kernel_info",
                               /*InsertBefore=*/nullptr,
                               GlobalValue::ThreadLocalMode::NotThreadLocal,
                               SpirvOffloadEntryAddSpace);
        KernelInfoVar->setTargetDeclare(true);
      };

  // Modify the kernel function declaration so that function pointer arguments
  // are received by the kernel as cl_ulong values, and all other pointer
  // arguments are in addrspace(1). This also requires aligning the new argument
  // types with the original uses of the arguments inside the function body.
  // For example, cl_ulong function pointer arguments must be casted
  // to their original pointer types, and all other pointer arguments
  // must be addrspacecasted to their original address spaces.
  for (auto ArgTyI = FnTy->param_begin(), ArgTyE = FnTy->param_end();
       ArgTyI != ArgTyE; ++ArgTyI) {
    auto ArgNum = std::distance(FnTy->param_begin(), ArgTyI);
    auto MapIdx = GetMapIdxForArgNum(ArgNum);
    bool IsFunctionPointer = IsFunctionPtr[MapIdx];
    uint64_t ArgSize = 0;
    if (auto *PtrTy = dyn_cast<PointerType>(*ArgTyI)) {
      if (IsFunctionPointer ||
          (!PtrTy->isOpaque() &&
           isa<FunctionType>(PtrTy->getNonOpaquePointerElementType()))) {
        // Kernel arguments representing function pointers
        // must be declared as 64-bit integers.
        Type *ArgTy = Type::getInt64Ty(C);
        ParamsTy.push_back(ArgTy);
        ArgSize = DL.getTypeStoreSize(ArgTy).getFixedValue();
        KernelArgInfo.emplace_back(false, ArgSize);
      } else {
#if INTEL_CUSTOMIZATION
        // Check if we can pass this argument "by value". We have to be sure
        // that libomptarget will be actually able to pass it by value,
        // so we have to do the same checks as in libomptarget (e.g. see
        // the member-of check below).
        ConstantInt *MapSize = cast<ConstantInt>(ConstSizes[MapIdx]);
        uint64_t MapType = MapTypes[MapIdx];
        // We can only apply the optimization to WILOCAL variables.
        // If the whole variable needs to be "visible" by all threads,
        // then making it a kernel argument will not work correctly.
        bool IsWILocal = IsWILocalFirstprivate[MapIdx];
        uint64_t MapSizeVal = MapSize->getLimitedValue(ByValLimit);
        const auto MapMask =
            TGT_MAP_TO | TGT_MAP_TARGET_PARAM | TGT_MAP_PRIVATE;
        if (MapSizeVal != 0 && MapSizeVal < ByValLimit && IsWILocal &&
            (MapType & MapMask) == MapMask &&
            (MapIdx + 1 >= MapTypes.size() ||
             (MapTypes[MapIdx + 1] & TGT_MAP_MEMBER_OF) == 0)) {
          // We can pass it by value, but we have to adjust the data type.
          // It turns out some device compilers do not pass the by value
          // argument objects to the kernels in contiguous manner, e.g.
          // for types like { i8, i8 } they may use 2 4-byte registers
          // for passing the members. To make the register usage more
          // efficient, we represent any user type as a structure
          // with the following members:
          //   { [K x i64], [L x i32], [M x i16], [N x i8] }, where
          // K >= 0, 0 <= L <= 1, 0 <= M <= 1, 0 <= N <= 1
          unsigned ChunkWidth = 8;
          uint64_t RemainingSize = MapSizeVal;
          SmallVector<Type *, 4> ChunkTys;
          while (ChunkWidth >= 1 && RemainingSize > 0) {
            uint64_t ChunkSize = RemainingSize / ChunkWidth;
            if (ChunkSize > 0)
              ChunkTys.push_back(ArrayType::get(
                  Type::getIntNTy(C, ChunkWidth * 8), ChunkSize));

            RemainingSize = (RemainingSize % ChunkWidth);
            ChunkWidth >>= 1;
          }
          Type *AggrTy = StructType::get(C, ChunkTys, /*isPacked=*/true);

          // ByVal pointer arguments must be in addrspace(0).
          ParamsTy.push_back(
              PointerType::get(AggrTy, vpo::ADDRESS_SPACE_PRIVATE));
          KernelArgInfo.emplace_back(true, MapSizeVal, AggrTy);
          ArgSize = MapSizeVal;
        } else {
#endif // INTEL_CUSTOMIZATION

          // This is a pointer argument.
          ArgSize = DL.getPointerSize(AddrSpaceGlobal);
          KernelArgInfo.emplace_back(false, ArgSize);
          ParamsTy.push_back(
              PointerType::getWithSamePointeeType(PtrTy, AddrSpaceGlobal));
#if INTEL_CUSTOMIZATION
        }
#endif // INTEL_CUSTOMIZATION
      }
    } else {
      // A non-pointer argument may appear as a result of scalar
      // FIRSTPRIVATE.
      ParamsTy.push_back(*ArgTyI);
      ArgSize = DL.getTypeStoreSize(*ArgTyI).getFixedValue();
      KernelArgInfo.emplace_back(false, ArgSize);
    }
    ByValLimit -= std::min(ByValLimit, ArgSize);
  }

  GenerateKernelArgInfoVar(KernelArgInfo, Fn, WT->getHasTeamsReduction(),
                           WT->getHasLocalAtomicFreeReduction(),
                           WT->getHasGlobalAtomicFreeReduction(),
                           WT->getHasKnownNDRange());

  Type *RetTy = FnTy->getReturnType();
  FunctionType *NFnTy = FunctionType::get(RetTy, ParamsTy, false);
  Function *NFn = Function::Create(NFnTy, GlobalValue::WeakAnyLinkage);
  NFn->copyAttributesFrom(Fn);
  NFn->setCallingConv(CallingConv::SPIR_KERNEL);
  NFn->addFnAttr("target.declare", "true");

  // Also add the "kernel" attribute, which is used by the OpenMPOpt pass to
  // identify OpenMP Offload kernels that can be optimized.
  NFn->addFnAttr("kernel");

  Fn->getParent()->getFunctionList().insert(Fn->getIterator(), NFn);
  NFn->takeName(Fn);
  NFn->splice(NFn->begin(), Fn);

  // Everything including the routine name has been moved to the new routine.
  // Do the same with the debug information.
  NFn->setSubprogram(Fn->getSubprogram());
  Fn->setSubprogram(nullptr);

  IRBuilder<> Builder(NFn->getEntryBlock().getFirstNonPHI());
  Function::arg_iterator NewArgI = NFn->arg_begin();
  for (Function::arg_iterator I = Fn->arg_begin(), E = Fn->arg_end(); I != E;
       ++I) {
    auto ArgV = &*NewArgI;
    Value *NewArgV = ArgV;
    auto ArgNum = std::distance(Fn->arg_begin(), I);
    auto MapIdx = GetMapIdxForArgNum(ArgNum);
    bool IsFunctionPointer = IsFunctionPtr[MapIdx];
    if (PointerType *OldArgPtrTy = dyn_cast<PointerType>(I->getType())) {
      if (IsFunctionPointer ||
          (!OldArgPtrTy->isOpaque() &&
           isa<FunctionType>(OldArgPtrTy->getNonOpaquePointerElementType()))) {
        // The new argument has 64-bit integer type.
        // We need to cast it to a function pointer type of the original
        // argument.
        NewArgV = Builder.CreateIntToPtr(ArgV, OldArgPtrTy,
                                         ArgV->getName() + Twine(".cast"));
      } else {
        KernelArgInfoDesc &ArgDesc =
            KernelArgInfo[std::distance(Fn->arg_begin(), I)];
        if (ArgDesc.IsByVal) {
          ArgV->addAttr(Attribute::getWithByValType(Builder.getContext(),
                                                    ArgDesc.ByValTy));
          NewArgV = Builder.CreateInBoundsGEP(
              ArgDesc.ByValTy, ArgV, {Builder.getInt32(0), Builder.getInt32(0)},
              ArgV->getName() + Twine(".byval.base"));
        }
        // Create an address space cast for pointer argument.
        unsigned NewAddressSpace =
            cast<PointerType>(ArgV->getType())->getAddressSpace();
        unsigned OldAddressSpace = OldArgPtrTy->getAddressSpace();

        if (NewAddressSpace != OldAddressSpace) {
          // Assert the correct addrspacecast here instead of failing
          // during SPIRV emission.
          assert(OldAddressSpace == vpo::ADDRESS_SPACE_GENERIC &&
                 "finalizeKernelFunction: OpenCL global addrspaces can only be "
                 "casted to generic.");
          NewArgV = Builder.CreatePointerBitCastOrAddrSpaceCast(
              ArgV, I->getType(), ArgV->getName() + Twine(".ascast"));
        }
      }
    }
    I->replaceAllUsesWith(NewArgV);
    NewArgI->takeName(&*I);
    ++NewArgI;
  }

  int SimdWidth = WT->getSPIRVSIMDWidth();
#if INTEL_CUSTOMIZATION
  const VPOParoptConfig *VPC = WI->getVPOParoptConfig();
  if (VPC) {
    uint8_t KernelSimdWidth = VPC->getKernelSPMDSIMDWidth(NFn->getName());
    if (KernelSimdWidth > 0)
      SimdWidth = KernelSimdWidth;
    vpo::RegisterAllocationMode RegisterAllocMode =
        VPC->getRegisterAllocMode(NFn->getName());
    if (RegisterAllocMode != RegisterAllocationMode::DEFAULT) {
      Metadata *AttrMDArgs[] = {ConstantAsMetadata::get(
          Builder.getInt32(static_cast<int>(RegisterAllocMode)))};
      NFn->setMetadata("RegisterAllocMode",
                       MDNode::get(NFn->getContext(), AttrMDArgs));
    }
  }

#endif // INTEL_CUSTOMIZATION
  if (VPOParoptUtils::enableDeviceSimdCodeGen()) {
    SimdWidth = 1;
    NFn->setMetadata("omp_simd_kernel", MDNode::get(NFn->getContext(), {}));
    NFn->setMetadata("sycl_explicit_simd", MDNode::get(NFn->getContext(), {}));
  }

  if (SimdWidth > 0) {
    Metadata *AttrMDArgs[] = {
        ConstantAsMetadata::get(Builder.getInt32(SimdWidth))};
    NFn->setMetadata("intel_reqd_sub_group_size",
                     MDNode::get(NFn->getContext(), AttrMDArgs));
  }

  // Under this option, adding metadate needed by GPU Back-End compiler to
  // generate block loads
  if (VPOParoptUtils::enableDeviceBlockLoad()) {
    Metadata *AttrMDArgs[] = {ConstantAsMetadata::get(Builder.getInt32(0)),
                              ConstantAsMetadata::get(Builder.getInt32(1)),
                              ConstantAsMetadata::get(Builder.getInt32(2))};
    NFn->setMetadata("intel_reqd_workgroup_walk_order",
                     MDNode::get(NFn->getContext(), AttrMDArgs));
  }

  return NFn;
}

bool VPOParoptTransform::removeClausesFromNestedRegionsExceptSIMD(
    WRegionNode *W, bool &FoundSIMD) const {
  bool Changed = false;
  FoundSIMD = false;
  SmallVector<WRegionNode *, 8> Worklist{W};
  do {
    WRegionNode *W = Worklist.pop_back_val();
    if (W->getDirID() == DIR_OMP_SIMD) {
      W->setEntryDirective(nullptr);
      W->setExitDirective(nullptr);
      FoundSIMD = true;
    }

    if (CallInst *OldEntry = cast_or_null<CallInst>(W->getEntryDirective())) {
      OperandBundleDef Bundles(
          VPOAnalysisUtils::getDirectiveString(OldEntry).str(), std::nullopt);
      CallInst *NewEntry = CallInst::Create(OldEntry, Bundles, OldEntry);
      NewEntry->copyMetadata(*OldEntry);
      OldEntry->replaceAllUsesWith(NewEntry);
      OldEntry->eraseFromParent();
      W->setEntryDirective(NewEntry);
      Changed = true;
    }

    copy(W->getChildren(), std::back_inserter(Worklist));
  } while (!Worklist.empty());
  return Changed;
}

namespace {

// Address space-aware AlisSetTracker which takes into account aliasing rules
// for different address spaces on SPIR-V target.
class AliasSetTrackerSPIRV {
  std::array<std::unique_ptr<AliasSetTracker>, vpo::ADDRESS_SPACE_GENERIC> ASTs;

  void add(unsigned AS, Instruction &I) {
    if (AS == vpo::ADDRESS_SPACE_GENERIC) {
      ASTs[vpo::ADDRESS_SPACE_PRIVATE]->add(&I);
      ASTs[vpo::ADDRESS_SPACE_LOCAL]->add(&I);
      ASTs[vpo::ADDRESS_SPACE_GLOBAL]->add(&I);
      return;
    }
    ASTs[AS]->add(&I);
  }

public:
  AliasSetTrackerSPIRV(BatchAAResults &BAAR) {
    ASTs[ADDRESS_SPACE_PRIVATE] = std::make_unique<AliasSetTracker>(BAAR);
    ASTs[ADDRESS_SPACE_GLOBAL] = std::make_unique<AliasSetTracker>(BAAR);
    ASTs[ADDRESS_SPACE_CONSTANT] = std::make_unique<AliasSetTracker>(BAAR);
    ASTs[ADDRESS_SPACE_LOCAL] = std::make_unique<AliasSetTracker>(BAAR);
  }

  void add(Instruction &I) {
    if (!I.mayReadOrWriteMemory() || VPOAnalysisUtils::isOpenMPDirective(&I))
      return;
    if (auto *LI = dyn_cast<LoadInst>(&I)) {
      add(LI->getPointerAddressSpace(), *LI);
      return;
    }
    if (auto *SI = dyn_cast<StoreInst>(&I)) {
      add(SI->getPointerAddressSpace(), *SI);
      return;
    }
    if (auto *AI = dyn_cast<AtomicCmpXchgInst>(&I)) {
      add(AI->getPointerAddressSpace(), *AI);
      return;
    }
    if (auto *AI = dyn_cast<AtomicRMWInst>(&I)) {
      add(AI->getPointerAddressSpace(), *AI);
      return;
    }
    if (auto *MI = dyn_cast<AnyMemSetInst>(&I)) {
      add(MI->getArgOperand(0)->getType()->getPointerAddressSpace(), *MI);
      return;
    }
    if (auto *MI = dyn_cast<AnyMemTransferInst>(&I)) {
      add(MI->getArgOperand(0)->getType()->getPointerAddressSpace(), *MI);
      add(MI->getArgOperand(1)->getType()->getPointerAddressSpace(), *MI);
      return;
    }
    if (auto *II = dyn_cast<IntrinsicInst>(&I)) {

      // Ignore prefetches when building alias sets; they should not influence
      // barrier insertion.
      if (II->getIntrinsicID() == Intrinsic::prefetch)
        return;
    }
    if (auto *CI = dyn_cast<CallBase>(&I)) {
      static const StringSet<> IgnoreCalls{
          "_Z13get_global_idj", "_Z12get_local_idj", "_Z14get_local_sizej",
          "_Z14get_num_groupsj", "_Z12get_group_idj"};

      if (Function *Callee = CI->getCalledFunction()) {
        if (IgnoreCalls.contains(Callee->getName()))
          return;

        // Also ignore __builtin_spirv_OpenCL_prefetch_* and
        // __builtin_IB_lsc_prefetch_global_* style prefetches.
        if (Callee->getName().starts_with("__builtin_spirv_OpenCL_prefetch_") ||
            Callee->getName().starts_with("__builtin_IB_lsc_prefetch_global_"))
          return;
      }

      // TODO: Perhaps we can do something smarter here taking into account
      // function and argument attributes, but so far just fall though to the
      // code that handles unknown instructions.
    }

    // Conservatively add instruction to all underlying address space specific
    // alias sets.
    for (auto &AST : ASTs)
      AST->add(&I);
  }

  // Return alias sets for the given address space.
  const ilist<AliasSet> &getAliasSets(unsigned AS) const {
    return ASTs[AS]->getAliasSets();
  }
};

} // anonymous namespace

bool VPOParoptTransform::needBarriersAfterParallel(
    WRegionNode *W, Function *KernelF,
    SmallDenseMap<Instruction *, bool> &InsertBarrierAt) {
  if (DisableParallelBarriers)
    return false;

  // With side-effects instructions barriers will be inserted after each
  // parallel region, but if there are no such instructions we need to
  // recompute places that require a barrier for correctness.
  InsertBarrierAt.clear();

  DenseMap<BasicBlock *, WRegionNode *> Exit2ParRegion;

  // Find all parallel regions at any nesting level inside the target region,
  // but skip parallel regions nested inside another parallel. Adding a
  // barrier after such regions may cause a deadlock.
  SmallVector<WRegionNode *, 8> Worklist;
  copy(W->getChildren(), std::back_inserter(Worklist));
  while (!Worklist.empty()) {
    WRegionNode *W = Worklist.pop_back_val();
    if (W->getIsPar()) {
      if (W->getParent()->getIsPar())
        continue;
      Exit2ParRegion[W->getExitBBlock()] = W;
    }
    copy(W->getChildren(), std::back_inserter(Worklist));
  }

  // No need to add barriers if there is only one or no parallel regions.
  if (Exit2ParRegion.size() <= 1)
    return false;

  // Setup AA for the outlined function.
  DominatorTree DT(*KernelF);
  BasicAAResult BAR(KernelF->getParent()->getDataLayout(), *KernelF, *TLI, *AC,
                    &DT, OptLevel);
  AAResults AAR(*TLI);
  AAR.addAAResult(BAR);
  BatchAAResults BAAR(AAR);

  SmallDenseMap<WRegionNode *, std::unique_ptr<AliasSetTrackerSPIRV>> ASTs;
  auto GetAliasSets = [&ASTs, &BAAR](WRegionNode *W,
                                     unsigned AS) -> const ilist<AliasSet> & {
    auto P = ASTs.insert({W, nullptr});
    if (P.second) {
      P.first->second = std::make_unique<AliasSetTrackerSPIRV>(BAAR);

      W->populateBBSet();
      for (BasicBlock *BB : W->blocks()) {
        if (W->getExitDirective() == &BB->front())
          continue;

        Instruction *I1 = (BB == W->getEntryBBlock())
                              ? W->getEntryDirective()->getNextNode()
                              : &BB->front();
        Instruction *I2 = (BB == W->getExitBBlock())
                              ? W->getExitDirective()->getPrevNode()
                              : &BB->back();

        for (auto &I : make_range(I1->getIterator(), ++I2->getIterator()))
          P.first->second->add(I);
      }
    }
    return P.first->second->getAliasSets(AS);
  };

  // Otherwise for each of parallel region walk predecessors to see if we can
  // find any parallel region on all paths to the target region entry. All
  // regions that we find require a barrier.
  for (auto &P : Exit2ParRegion) {
    SmallPtrSet<BasicBlock *, 8> Visited{W->getEntryBBlock()};
    std::queue<BasicBlock *> Worklist;
    auto GrowWorklist = [&Worklist, &Visited](BasicBlock *BB) {
      if (Visited.insert(BB).second)
        for (BasicBlock *PredBB : predecessors(BB))
          if (!Visited.contains(PredBB))
            Worklist.push(PredBB);
    };
    GrowWorklist(P.second->getEntryBBlock());

    while (!Worklist.empty()) {
      BasicBlock *BB = Worklist.front();
      Worklist.pop();

      // If this is an exit block of a parallel region then this region
      // needs a barrier.
      auto It = Exit2ParRegion.find(BB);
      if (It != Exit2ParRegion.end()) {
        bool &IsGlobal = InsertBarrierAt
                             .insert({It->second->getExitDirective(),
                                      OptLevel < 3 || !SimplifyBarrierFences})
                             .first->second;
        if (!IsGlobal) {
          // Global fence is needed if either the parallel region that we are
          // analysing or the region reacheable though predecessors writes to
          // global memory while the other one reads from or writes to it. In
          // that case we add a barrier with global fence after the reacheable
          // parallel region if both regions access the same memory.
          const ilist<AliasSet> &ASL1 =
              GetAliasSets(It->second, vpo::ADDRESS_SPACE_GLOBAL);
          const ilist<AliasSet> &ASL2 =
              GetAliasSets(P.second, vpo::ADDRESS_SPACE_GLOBAL);

          IsGlobal |= any_of(ASL1, [&ASL2, &BAAR](const AliasSet &AS1) {
            if (AS1.isForwardingAliasSet())
              return false;

            for (const AliasSet &AS2 : ASL2) {
              if (AS2.isForwardingAliasSet())
                continue;

              // If any of these alias sets write to memory while the other
              // one reads from or writes to it, then alias sets may
              // potentially overlap. In this case we need to check if these
              // alias sets alias.
              if ((AS1.isMod() && (AS2.isMod() || AS2.isRef())) ||
                  (AS2.isMod() && (AS1.isMod() || AS1.isRef()))) {
                // Check if alias sets alias.
                if (AS1.aliases(AS2, BAAR))
                  return true;
              }
            }
            return false;
          });
        }
      }

      GrowWorklist(BB);
    }
  }

  return !InsertBarrierAt.empty();
}

/// This function checks if the instruction is a SIMD intrinsic instruction,
static bool isSimdDirective(Instruction *Inst) {
  auto IntrinInst = dyn_cast<IntrinsicInst>(Inst);

  if (!IntrinInst || !IntrinInst->hasOperandBundles()) {
    return false;
  }

  int ID = VPOAnalysisUtils::getDirectiveID(Inst);

  if (ID == DIR_OMP_SIMD || ID == DIR_OMP_END_SIMD)
    return true;

  return false;
}

/// This function checks if the instruction is an intrinsic instruction,
/// and depending on the boolean flag, whether it is target, parallel
/// parallel loop or simd loop.
static bool isParOrTargetDirective(Instruction *Inst, bool isTarget = false,
                                   bool isSIMD = false) {
  auto IntrinInst = dyn_cast<IntrinsicInst>(Inst);

  if (!IntrinInst || !IntrinInst->hasOperandBundles()) {
    return false;
  }

  int ID = VPOAnalysisUtils::getDirectiveID(Inst);

  if (isSIMD)
    return ID == DIR_OMP_SIMD;

  if (isTarget)
    return ID == DIR_OMP_TARGET;

  switch (ID) {
  case DIR_OMP_PARALLEL:
  case DIR_OMP_PARALLEL_LOOP:
  case DIR_OMP_PARALLEL_SECTIONS:
  case DIR_OMP_DISTRIBUTE_PARLOOP:
  case DIR_OMP_SIMD:
    // DIR_OMP_TEAMS used to be here, but it prevented guarding
    // instructions with side effects inside OpenMP teams regions.
    // We may want to get it back here, so that we insert barriers
    // around OpenMP teams region. This is currently not required,
    // since there may not be any code between "target" and "teams".
    return true;
  }

  return false;
}

/// Get the exit region directive intrinsic instruction, corresponding
/// to the begin region directive. The user instruction of the
/// begin region is the exit region directive. If \p DirectiveBegin is same
/// as \p KernelEntryDir, then returns \p KernelExitDir.
static Instruction *getExitInstruction(Instruction *DirectiveBegin,
                                       Instruction *KernelEntryDir,
                                       Instruction *KernelExitDir) {
  assert(DirectiveBegin && isa<IntrinsicInst>(DirectiveBegin) &&
         "Unexpected begin directive.");

  if (DirectiveBegin == KernelEntryDir)
    return KernelExitDir;

  // Assumption: Every intrinsic instruction that begins a directive
  // has a single exit directive corresponding to it.
  // So, if we iterate through its users, then the
  // intrinsic instruction that uses it must be the exit directive.
  // For KernelEntryDir, it's possible to not have any use, as the use is
  // replaced with 'token none' before invoking CodeExtractor, but it has
  // already been handled above.
  for (auto U : DirectiveBegin->users()) {
    if (auto DEnd = dyn_cast<IntrinsicInst>(U)) {
      LLVM_DEBUG(dbgs() << "\n Directive End::" << *DEnd << "\n");
      return DEnd;
    }
  }
  return nullptr;
}

/// If the given \p CallI is a __kmpc_critical call,
/// then this function will return its pairing __kmpc_end_critical call,
/// otherwise, it will return nullptr.
/// The function assumes that there are no early exits from the critical
/// region, which is a valid assumption for SPIR targets (where we can
/// only support well-formed critical regions).
static CallInst *getCriticalEndCall(CallInst *CallI) {
  auto CalledF = CallI->getCalledOperand()->stripPointerCasts();

  if (!CalledF || !CalledF->hasName() ||
      CalledF->getName() != "__kmpc_critical" ||
      CalledF->getName() != "__kmpc_critical_simd")
    return nullptr;

  SmallVector<BasicBlock *, 32> WorkStack{CallI->getParent()};
  SmallPtrSet<BasicBlock *, 32> Visited;
  while (!WorkStack.empty()) {
    BasicBlock *BB = WorkStack.pop_back_val();

    for (auto &I : *BB)
      if (auto *EndCallI = dyn_cast<CallInst>(&I)) {
        auto CalledF = EndCallI->getCalledOperand()->stripPointerCasts();

        if (CalledF && CalledF->hasName() &&
            (CalledF->getName() == "__kmpc_end_critical" ||
             CalledF->getName() == "__kmpc_critical_simd"))
          return EndCallI;
      }

    Visited.insert(BB);
    assert(BB->getTerminator()->getNumSuccessors() > 0 &&
           "Block inside critical section has zero successors.");
    // Place the successors into the work stack.
    for (auto *SBB : successors(BB))
      if (Visited.count(SBB) == 0)
        WorkStack.push_back(SBB);
  }

  llvm_unreachable("OpenMP critical region is malformed.");
  return nullptr;
}

/// Guard instructions that have side effects, so that only master thread
/// (thread_id == 0) in each team executes it.
void VPOParoptTransform::guardSideEffectStatements(WRegionNode *W,
                                                   Function *KernelF) {

  assert(isTargetSPIRV() &&
         "guardSideEffectStatements() called for non-SPIRV target.");

  Instruction *ParDirectiveBegin = nullptr;
  Instruction *ParDirectiveExit = nullptr;
  Instruction *TargetDirectiveBegin = nullptr;
  Instruction *TargetDirectiveExit = nullptr;
  CallInst *KernelEntryDir = cast<CallInst>(W->getEntryDirective());
  CallInst *KernelExitDir = cast<CallInst>(W->getExitDirective());

  SmallDenseMap<Instruction *, bool> InsertBarrierAt;
  SmallVector<BasicBlock *, 10> ParBBVector, TargetBBSet;
  SmallVector<Instruction *, 10> EntryDirectivesToDelete;
  SmallVector<Instruction *, 10> ExitDirectivesToDelete;
  SmallVector<std::pair<CallInst *, CallInst *>, 10> OmpCriticalCalls;

  // Find the parallel region begin and end directives,
  // and add barriers at the entry and exit of parallel region.
  for (inst_iterator I = inst_begin(KernelF), E = inst_end(KernelF); I != E;
       ++I) {

    // Collect OpenMP directive calls so that we can delete them
    // later. We may want to keep SIMD directives at some point,
    // but right now they are not supported for SPIR-V targets.
    // Moreover, llvm-spirv is not able to translate llvm.directive.region
    // intrinsic calls.
    if (VPOAnalysisUtils::isOpenMPDirective(&*I) &&
        // Target directives require special processing (see below).
        &*I != KernelEntryDir && &*I != KernelExitDir) {
      // Distinguish between entry and other directives, since
      // we want to delete the entry directives after their
      // exit companions.
      if (!VPOParoptUtils::enableDeviceSimdCodeGen() || !isSimdDirective(&*I)) {
        if (VPOAnalysisUtils::isBeginDirective(&*I))
          EntryDirectivesToDelete.push_back(&*I);
        else
          ExitDirectivesToDelete.push_back(&*I);
      }
    }

    if (isParOrTargetDirective(&*I) &&
        // Barriers must not be inserted around SIMD loops.
        !isParOrTargetDirective(&*I, false, true)) {

      ParDirectiveBegin = &*I;
      ParDirectiveExit =
          getExitInstruction(ParDirectiveBegin, KernelEntryDir, KernelExitDir);
      assert(ParDirectiveExit && "Par region exit directive not found.");

      SmallVector<BasicBlock *, 10> TempParBBVec;
      GeneralUtils::collectBBSet(ParDirectiveBegin->getParent(),
                                 ParDirectiveExit->getParent(), TempParBBVec);
      ParBBVector.append(TempParBBVec.begin(), TempParBBVec.end());

      // Insert a barrier after the region exit. This barrier synchronizes
      // all side effects that might have happened inside the region
      // with all the consumers of this side effects that are reachable
      // from the region's exit. Note that we do not need a barrier
      // before the region, since we insert barriers after all side effect
      // instructions that may reach the region's entry.
      InsertBarrierAt.insert({ParDirectiveExit, true});

      ParDirectiveBegin = nullptr;
      ParDirectiveExit = nullptr;
    } else if (TargetDirectiveBegin == nullptr &&
               isParOrTargetDirective(&*I, true)) {
      TargetDirectiveBegin = &*I;
      TargetDirectiveExit = getExitInstruction(TargetDirectiveBegin,
                                               KernelEntryDir, KernelExitDir);
      assert(TargetDirectiveExit && "Target region exit directive not found.");

      GeneralUtils::collectBBSet(TargetDirectiveBegin->getParent(),
                                 TargetDirectiveExit->getParent(), TargetBBSet);
    } else if (auto *CallI = dyn_cast<CallInst>(&*I))
      if (auto *EndCallI = getCriticalEndCall(CallI))
        // Collect critical regions. If a side-effect instruction
        // inside a critical region must be guarded (e.g. the critical
        // region is inside a teams region), then there must be no
        // barrier at the merge point.
        //
        // FIXME: this is not really needed, because we guard the
        //        lock acqure/release calls with the master thread checks
        //        as well. For some reason specACCEL/552 hangs with
        //        the barrier calls.
        OmpCriticalCalls.push_back({CallI, EndCallI});
  }

  SmallPtrSet<BasicBlock *, 10> ParBBSet(ParBBVector.begin(),
                                         ParBBVector.end());
  SmallPtrSet<BasicBlock *, 10> CriticalBBSet;

  if (!VPOParoptUtils::enableDeviceSimdCodeGen()) {
    for (auto &CriticalPair : OmpCriticalCalls) {
      SmallVector<BasicBlock *, 32> BBSet;
      GeneralUtils::collectBBSet(CriticalPair.first->getParent(),
                                 CriticalPair.second->getParent(), BBSet);
      CriticalBBSet.insert(BBSet.begin(), BBSet.end());
    }
  }

  // Iterate over all instructions and form master thread regions to contain any
  // instructions that can't be safely run on all threads.

  DominatorTree DT(*KernelF);
  PostDominatorTree PDT(*KernelF);
  MasterThreadRegionFinder RegionFinder(ParBBSet, DT, PDT);

  for (BasicBlock *const BB : TargetBBSet) {

    // Do not add master thread regions in parallel region basic blocks.
    if (ParBBSet.contains(BB))
      continue;

    // Set up iteration bounds to only iterate instructions that are not already
    // part of multi-basic block regions.
    Instruction *Inst = RegionFinder.multiBBRegionEnd(BB);
    Instruction *EndInst = RegionFinder.multiBBRegionStart(BB);

    // Adjust the bounds to only iterate instructions within the target region
    // if this is a target entry/exit block.
    if (TargetDirectiveBegin->getParent() == BB)
      Inst = TargetDirectiveBegin->getNextNode();
    if (TargetDirectiveExit->getParent() == BB)
      EndInst = TargetDirectiveExit;

    while (Inst != EndInst) {

      // If this instruction needs to be guarded, add a master thread region for
      // it.
      if (needsMasterThreadGuard(Inst)) {
        LLVM_DEBUG(
            dbgs() << "\n"
                   << __FUNCTION__ << ": Instruction has side effect::" << *Inst
                   << "\nBasicBlock: " << Inst->getParent()->getName() << "\n");
        const MasterThreadRegion &Region = RegionFinder.findMasterThreadRegion(
            Inst, CriticalBBSet.contains(BB));

        // Skip any other instructions in this region that are in this basic
        // block.
        if (Region.getEnd()->getParent() != BB)
          break;
        Inst = Region.getEnd();
        continue;
      }

      // Otherwise, continue to the next instruction.
      Inst = Inst->getNextNonDebugInstruction();
    }
  }

  SmallVector<MasterThreadRegion> Regions = RegionFinder.foundRegions();

  if (!Regions.empty()) {
    // Prepare the master check predicate, which looks like
    //   (get_local_id(0) == 0 && get_local_id(1) == 0 &&
    //    get_local_id(2) == 0)
    //
    // TODO: we may optimize this later, based on the number of dimensions
    //       used for the target region.
    IRBuilder<> Builder(KernelEntryDir);
    auto *ZeroConst = Constant::getNullValue(GeneralUtils::getSizeTTy(F));
    Value *MasterCheckPredicate = nullptr;

    for (unsigned Dim = 0; Dim < 3; ++Dim) {
      CallInst *const LocalId =
          VPOParoptUtils::genLocalIdCall(Dim, KernelEntryDir);

      // At this point in device compilation we shouldn't get adverse effects by
      // allowing code motioning of these function calls, so the function can be
      // marked with the relevant attributes for later optimizations.
      Function *const LocalIdCallee = LocalId->getCalledFunction();
      LocalIdCallee->setDoesNotAccessMemory();
      LocalIdCallee->setDoesNotThrow();
      LocalIdCallee->setNoSync();
      LocalIdCallee->setWillReturn();

      Value *Predicate = Builder.CreateICmpEQ(LocalId, ZeroConst);

      if (!MasterCheckPredicate) {
        MasterCheckPredicate = Predicate;
        continue;
      }

      MasterCheckPredicate = Builder.CreateAnd(MasterCheckPredicate, Predicate);
    }

    MasterCheckPredicate->setName("is.master.thread");

    // Codegen the regions that have been found.
    DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);
    for (MasterThreadRegion &Region : Regions) {
      Region.insertBroadcasts(TargetDirectiveBegin);
      Region.insertBarriers();
      Region.insertGuard(MasterCheckPredicate, DTU, *LI, W);
    }
  }

  if (!Regions.empty() ||
      // FIXME: if there are multiple parallel regions,
      //        then we need to synchronize between them, otherwise
      //        some data written in a parallel region
      //        may be out of date for reading in the succeeding
      //        parallel region. The check here is not enough,
      //        because we may read data written in a parallel region
      //        in "omp target" code succeeding the parallel region.
      //        For now to avoid performance regressions, we insert
      //        barriers only when there are multiple regions inside
      //        "omp target" at any nesting level. The complete fix would
      //        be to check if any load operation after a parallel region
      //        may read data that was potentially updated inside
      //        the parallel region. Can we use alias information for that?
      needBarriersAfterParallel(W, KernelF, InsertBarrierAt)) {
    for (auto &InsertPt : InsertBarrierAt) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Insert Barrier with "
                        << (InsertPt.second ? "global" : "local")
                        << " fence at :" << InsertPt.first << "\n");
      insertWorkGroupBarrier(InsertPt.first, InsertPt.second);
    }
  }

  // Delete the directives.
  // The target directives will be removed by paroptTransforms().
  for (auto *I : ExitDirectivesToDelete)
    VPOUtils::stripDirectives(*I->getParent());
  for (auto *I : EntryDirectivesToDelete)
    VPOUtils::stripDirectives(*I->getParent());

  // Remove all clauses from the "omp target" entry directive.
  // The extra references in the clauses may prevent address space
  // inferring. We cannot remove the directive call yet, because
  // the removal in paroptTransforms() will complain.
  OperandBundleDef B(
      std::string(VPOAnalysisUtils::getDirectiveString(KernelEntryDir)),
      std::nullopt);
  // The following call clones the original directive call
  // with just the directive name in the operand bundles.
  auto *NewEntryDir = CallInst::Create(KernelEntryDir, {B}, KernelEntryDir);
  NewEntryDir->copyMetadata(*KernelEntryDir);
  KernelEntryDir->replaceAllUsesWith(NewEntryDir);
  KernelEntryDir->eraseFromParent();
  W->setEntryDirective(NewEntryDir);

  if (VPOParoptUtils::enableDeviceSimdCodeGen())
    VPOUtils::stripDirectives(*KernelExitDir->getParent());
}

// Remove SIMD directives from the function body.
static void removeSimdDirectives(Function *F) {
  for (BasicBlock &BB : *F)
    for (auto I = BB.begin(), E = BB.end(); I != E;)
      if (auto *II = dyn_cast<IntrinsicInst>(I++)) {
        int ID = VPOAnalysisUtils::getDirectiveID(II);
        if (ID == DIR_OMP_SIMD)
          II->replaceAllUsesWith(UndefValue::get(II->getType()));
        if (ID == DIR_OMP_SIMD || ID == DIR_OMP_END_SIMD)
          II->eraseFromParent();
      }
}

// Run SROA pass on the given function.
static void runSROA(Function *F) {
  FunctionAnalysisManager FAM;
  FAM.registerPass([&] { return PassInstrumentationAnalysis(); });
  FAM.registerPass([&] { return DominatorTreeAnalysis(); });
  FAM.registerPass([&] { return AssumptionAnalysis(); });
  FAM.registerPass([&] { return TargetIRAnalysis(); });

  FunctionPassManager FPM;
  FPM.addPass(SROAPass(SROAPass(SROAOptions::ModifyCFG)));
  FPM.run(*F, FAM);
}

void VPOParoptTransform::genTargetSPIRVOffloadingCode(WRNTargetNode *WT,
                                                      Function *NewF,
                                                      CallInst *NewCall) {
  SmallVector<Constant *, 16> ConstSizes;
  SmallVector<uint64_t, 16> MapTypes;
  SmallVector<GlobalVariable *, 16> Names;
  SmallVector<Value *, 16> Mappers;
#if INTEL_CUSTOMIZATION
  SmallVector<bool> IsWILocalFirstprivate;
#endif // INTEL_CUSTOMIZATION
  SmallVector<bool> IsFunctionPtr;
  bool HasRuntimeEvaluationCaptureSize = false;
  (void)getTargetDataInfo(WT, NewCall, ConstSizes, MapTypes, Names, Mappers,
#if INTEL_CUSTOMIZATION
                          IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                          IsFunctionPtr, HasRuntimeEvaluationCaptureSize);
  NewF = finalizeKernelFunction(WT, NewF, NewCall, MapTypes,
#if INTEL_CUSTOMIZATION
                                IsWILocalFirstprivate,
#endif // INTEL_CUSTOMIZATION
                                IsFunctionPtr, ConstSizes);

  LLVM_DEBUG(dbgs() << "\nAfter finalizeKernel Dump the function ::" << *NewF
                    << "\n");

  // CodeExtractor may replace existing SIMD directive entry instructions
  // with new ones, but it does not update directive entries in WRNs.
  // Because of that SIMD WRNs may contain invalid entry directive
  // addresses, so any attempt to dereference SIMD WRNs entry directive
  // leads to a segfault. Remove all SIMD directives from the function
  // unless SIMD device codegen is enabled.
  if (!VPOParoptUtils::enableDeviceSimdCodeGen())
    removeSimdDirectives(NewF);

  // Remove clauses from all directives in the outlined function in
  // preparation for the address space inference optimization.
  // The directives themselves will later be removed in
  // guardSideEffectStatements() after they are no longer needed.
  bool ContainsSIMD = false;
  removeClausesFromNestedRegionsExceptSIMD(WT, ContainsSIMD);

  // Cleanup redundant allocas/loads/stores if barrier fences simplification
  // is enabled. It improves quality of the address space inference
  // optimization which in turn is needed for simplifying fences.
  if (OptLevel > 2 && SimplifyBarrierFences)
    runSROA(NewF);

  // Run address space inference optimization to get rid
  // of addrspace(4) accesses, which result in run-time dispatches.
  if (!VPOParoptUtils::enableDeviceSimdCodeGen() || !ContainsSIMD) {
    // When W contains a SIMD directive (and SIMD device codegen
    // is enabled), we don't run InferAddrSpaces as it can cause
    // mismatches in the Values on the clause list, and those used
    // in the region. e.g.
    //    Before                           |        After
    // ------------------------------------+------------------------------
    //  %x1 = addrspacecast i32* %x        | %x1 = addrspacecast i32* %x
    //            to i32 addrspace(4)*     |          to i32 addrspace(4)*
    //                                     |
    //  ...["SIMD",...,"PRIVATE"(%x1)      | ...["SIMD",...,"PRIVATE"(%x1)
    //                                     |
    //  store i32 0, i32 addrspace(4)* %x1 | store i32 0, i32* %x
    //                                     |
    InferAddrSpaces(AC, DT, TTI, vpo::ADDRESS_SPACE_GENERIC, *NewF);
    LLVM_DEBUG(dbgs() << "\nAfter InferAddrSpaces the function ::" << *NewF);
  }

  // Run guardSideEffectStatements() after finalizeKernelFunction(),
  // because the latter may modify some of the kernel arguments,
  // e.g. change arguments that were originally outlined as addrspace(1)
  // pointer argument into byval arguments from addrspace(0).
  // We must not guard accesses through such pointer arguments with
  // the master-thread checks, otherwise, the code may produce
  // wrong results.
  guardSideEffectStatements(WT, NewF);
  LLVM_DEBUG(dbgs() << "\nAfter guardSideEffectStatements the function ::"
                    << *NewF);
}

// Set SIMD widening width for the target region based
// on simdlen() clauses of the enclosed SIMD loops (if any).
void VPOParoptTransform::propagateSPIRVSIMDWidth() const {
  for (auto *W : WRegionList) {
    auto *WT = dyn_cast<WRNTargetNode>(W);
    if (!WT)
      continue;

    if (FixedSIMDWidth > 0) {
      // Override the SIMD width with an option.
      WT->setSPIRVSIMDWidth(FixedSIMDWidth);
      continue;
    }

    // Choose minimum SIMD width, if there are multiple SIMD loops.
    unsigned MinSIMDLen = 0;
    SmallVector<WRegionNode *, 32> WorkList{WT};
    while (!WorkList.empty()) {
      unsigned CurSIMDLen = 0;
      auto *WChild = WorkList.pop_back_val();

      if (WChild != WT && isa<WRNTargetNode>(WChild))
        // If this is an enclosed target region, then
        // we have already processed it and we can take its
        // SIMD width without processing the children.
        CurSIMDLen = cast<WRNTargetNode>(WChild)->getSPIRVSIMDWidth();
      else if (!isa<WRNVecLoopNode>(WChild)) {
        WorkList.insert(WorkList.end(), WChild->wrn_child_begin(),
                        WChild->wrn_child_end());
        continue;
      } else
        CurSIMDLen = WChild->getSimdlen();

      if (CurSIMDLen == 0 ||
          // Ignore unsupported SIMD widths.
          (CurSIMDLen != 8 && CurSIMDLen != 16 && CurSIMDLen != 32))
        continue;
      if (MinSIMDLen == 0 || MinSIMDLen > CurSIMDLen)
        MinSIMDLen = CurSIMDLen;
    }

    if (MinSIMDLen != 0)
      WT->setSPIRVSIMDWidth(MinSIMDLen);
  }
}
#endif // INTEL_COLLAB
