//===-- MetadataAPI.h -----------------------------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_METADATA_API_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_UTILS_METADATA_API_H

#include "MetadataAPIImpl.h"

namespace llvm {
namespace SYCLKernelMetadataAPI {

struct GlobalVariableMetadataAPI {
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipePacketSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipePacketAlignTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipeDepthTy;
  typedef NamedMDValue<std::string, MDValueGlobalObjectStrategy> PipeIOTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipeProtocolTy;

  GlobalVariableMetadataAPI(llvm::GlobalVariable *Global)
      : PipePacketSize(Global, "packet_size"),
        PipePacketAlign(Global, "packet_align"), PipeDepth(Global, "depth"),
        PipeIO(Global, "io"), PipeProtocol(Global, "protocol") {}

  NamedMDValueAccessor<PipePacketSizeTy> PipePacketSize;
  NamedMDValueAccessor<PipePacketAlignTy> PipePacketAlign;
  NamedMDValueAccessor<PipeDepthTy> PipeDepth;
  NamedMDValueAccessor<PipeIOTy> PipeIO;
  NamedMDValueAccessor<PipeProtocolTy> PipeProtocol;
};

struct FunctionMetadataAPI {
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> BoolTy;

  FunctionMetadataAPI(llvm::Function *Func)
      : RecursiveCall(Func, "recursive_call") {}

  NamedMDValueAccessor<BoolTy> RecursiveCall;
};

struct KernelMetadataAPI {

  // required attributes
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy>
      ArgBaseTypeListTy;
  typedef NamedMDList<std::string, MDValueGlobalObjectStrategy>
      ArgIOAttributeListTy;
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> ArgAddrSpaceTy;

  // optional attributes
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> WorkGroupSizeHintTy;
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> ReqdWorkGroupSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> ReqdNumSubGroupsTy;
  typedef NamedHeteroTupleMDList<llvm::Type, int32_t> VecTypeHintTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> VecLenHintTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> MaxGlobalWorkDimTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy>
      CanUseGlobalWorkOffsetTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> AutorunTy;
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> NumComputeUnitsTy;

  KernelMetadataAPI(llvm::Function *Func)
      : ArgBaseTypeList(Func, "kernel_arg_base_type"),
        ArgIOAttributeList(Func, "kernel_arg_pipe_io"),
        ArgAddrSpaceList(Func, "kernel_arg_addr_space"),

        WorkGroupSizeHint(Func, "work_group_size_hint"),
        ReqdWorkGroupSize(Func, "reqd_work_group_size"),
        ReqdNumSubGroups(Func, "required_num_sub_groups"),
        VecTypeHint(Func, "vec_type_hint"),
        VecLenHint(Func, "intel_vec_len_hint"),
        ReqdIntelSGSize(Func, "intel_reqd_sub_group_size"),
        MaxGlobalWorkDim(Func, "max_global_work_dim"),
        CanUseGlobalWorkOffset(Func, "uses_global_work_offset"),
        Autorun(Func, "autorun"), NumComputeUnits(Func, "num_compute_units") {}

  // required attributes
  ArgBaseTypeListTy ArgBaseTypeList;
  ArgIOAttributeListTy ArgIOAttributeList;
  ArgAddrSpaceTy ArgAddrSpaceList;

  // optional attributes
  WorkgroupSizeMDAccessor<WorkGroupSizeHintTy> WorkGroupSizeHint;
  WorkgroupSizeMDAccessor<ReqdWorkGroupSizeTy> ReqdWorkGroupSize;
  NamedMDValueAccessor<ReqdNumSubGroupsTy> ReqdNumSubGroups;
  VecTypeHintTupleMDListAccessor<VecTypeHintTy> VecTypeHint;
  NamedMDValueAccessor<VecLenHintTy> VecLenHint;
  NamedMDValueAccessor<VecLenHintTy> ReqdIntelSGSize; // Alias to VecLenHint
  NamedMDValueAccessor<MaxGlobalWorkDimTy> MaxGlobalWorkDim;
  NamedMDValueAccessor<CanUseGlobalWorkOffsetTy> CanUseGlobalWorkOffset;
  NamedMDValueAccessor<AutorunTy> Autorun;
  WorkgroupSizeMDAccessor<NumComputeUnitsTy> NumComputeUnits;

public:
  const llvm::SmallVectorImpl<llvm::StringRef> &getMDNames() const {
    // Lazily initialize MDNames.
    if (MDNames.empty()) {
      MDNames = {
          ArgBaseTypeList.getID(),   ArgIOAttributeList.getID(),

          WorkGroupSizeHint.getID(), ReqdWorkGroupSize.getID(),
          ReqdNumSubGroups.getID(),  VecTypeHint.getID(),
          VecLenHint.getID(),        ReqdIntelSGSize.getID(),
          MaxGlobalWorkDim.getID(),  CanUseGlobalWorkOffset.getID(),
          Autorun.getID(),           NumComputeUnits.getID(),
      };
    }
    return MDNames;
  }

  bool hasVecLength() const {
    return VecLenHint.hasValue() || ReqdIntelSGSize.hasValue();
  }

  int getVecLength() const {
    if (VecLenHint.hasValue())
      return VecLenHint.get();
    else
      return ReqdIntelSGSize.get();
  }

  void setReqdIntelSGSize(int Size) { ReqdIntelSGSize.set(Size); }

private:
  mutable llvm::SmallVector<llvm::StringRef, 0> MDNames;
};

// internal attributes
struct KernelInternalMetadataAPI {
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> LocalBufferSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      BarrierBufferSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      KernelExecutionLengthTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> MaxWGDimensionsTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> KernelHasGlobalSyncTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> KernelHasSubgroupsTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> NoBarrierPathTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> HasMatrixCallTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> SubgroupEmuSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> VectorizedWidthTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> BlockLiteralSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      PrivateMemorySizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      VectorizationDimensionTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> CanUniteWorkgroupsTy;
  typedef NamedMDValue<llvm::Function, MDValueGlobalObjectStrategy>
      VectorizedKernelTy;
  typedef NamedMDValue<llvm::Function, MDValueGlobalObjectStrategy>
      KernelWrapperTy;
  typedef NamedMDValue<llvm::Function, MDValueGlobalObjectStrategy>
      ScalarizedKernelTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> UseFPGAPipesTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      SubGroupConstructionModeTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy>
      UseAddrSpaceQualifierFuncTy;
  typedef NamedMDList<llvm::Constant, MDValueGlobalObjectStrategy>
      ArgTypeNullValListTy;

  KernelInternalMetadataAPI(llvm::Function *Func)
      : LocalBufferSize(Func, "local_buffer_size"),
        BarrierBufferSize(Func, "barrier_buffer_size"),
        KernelExecutionLength(Func, "kernel_execution_length"),
        MaxWGDimensions(Func, "max_wg_dimensions"),
        KernelHasGlobalSync(Func, "kernel_has_global_sync"),
        KernelHasSubgroups(Func, "kernel_has_sub_groups"),
        NoBarrierPath(Func, "no_barrier_path"),
        HasMatrixCall(Func, "has_matrix_call"),
        VectorizedWidth(Func, "vectorized_width"),
        SubgroupEmuSize(Func, "sg_emu_size"),
        RecommendedVL(Func, "recommended_vector_length"),
        BlockLiteralSize(Func, "block_literal_size"),
        PrivateMemorySize(Func, "private_memory_size"),
        VectorizationDimension(Func, "vectorization_dimension"),
        CanUniteWorkgroups(Func, "can_unite_workgroups"),
        VectorizedKernel(Func, "vectorized_kernel"),
        VectorizedMaskedKernel(Func, "vectorized_masked_kernel"),
        KernelWrapper(Func, "kernel_wrapper"),
        ScalarKernel(Func, "scalar_kernel"),
        UseFPGAPipes(Func, "use_fpga_pipes"),
        SubGroupConstructionMode(Func, "sg_construction_mode"),
        UseAddrSpaceQualifierFunc(Func, "use_addrspace_qualifier_func"),
        ArgTypeNullValList(Func, "arg_type_null_val") {}

  // internal attributes
  NamedMDValueAccessor<LocalBufferSizeTy> LocalBufferSize;
  NamedMDValueAccessor<BarrierBufferSizeTy> BarrierBufferSize;
  NamedMDValueAccessor<KernelExecutionLengthTy> KernelExecutionLength;
  NamedMDValueAccessor<MaxWGDimensionsTy> MaxWGDimensions;
  NamedMDValueAccessor<KernelHasGlobalSyncTy> KernelHasGlobalSync;
  NamedMDValueAccessor<KernelHasSubgroupsTy> KernelHasSubgroups;
  NamedMDValueAccessor<NoBarrierPathTy> NoBarrierPath;
  NamedMDValueAccessor<HasMatrixCallTy> HasMatrixCall;
  NamedMDValueAccessor<VectorizedWidthTy> VectorizedWidth;
  NamedMDValueAccessor<SubgroupEmuSizeTy> SubgroupEmuSize;
  NamedMDValueAccessor<VectorizedWidthTy> RecommendedVL;
  NamedMDValueAccessor<BlockLiteralSizeTy> BlockLiteralSize;
  NamedMDValueAccessor<PrivateMemorySizeTy> PrivateMemorySize;
  NamedMDValueAccessor<VectorizationDimensionTy> VectorizationDimension;
  NamedMDValueAccessor<CanUniteWorkgroupsTy> CanUniteWorkgroups;
  NamedMDValueAccessor<VectorizedKernelTy> VectorizedKernel;
  NamedMDValueAccessor<VectorizedKernelTy> VectorizedMaskedKernel;
  NamedMDValueAccessor<KernelWrapperTy> KernelWrapper;
  NamedMDValueAccessor<ScalarizedKernelTy> ScalarKernel;
  NamedMDValueAccessor<UseFPGAPipesTy> UseFPGAPipes;
  NamedMDValueAccessor<SubGroupConstructionModeTy> SubGroupConstructionMode;
  NamedMDValueAccessor<UseAddrSpaceQualifierFuncTy> UseAddrSpaceQualifierFunc;
  ArgTypeNullValListTy ArgTypeNullValList;

public:
  const llvm::SmallVectorImpl<llvm::StringRef> &getMDNames() const {
    // Lazily initialize MDNames.
    if (MDNames.empty()) {
      MDNames = {LocalBufferSize.getID(),
                 BarrierBufferSize.getID(),
                 KernelExecutionLength.getID(),
                 MaxWGDimensions.getID(),
                 KernelHasGlobalSync.getID(),
                 KernelHasSubgroups.getID(),
                 NoBarrierPath.getID(),
                 VectorizedWidth.getID(),
                 RecommendedVL.getID(),
                 BlockLiteralSize.getID(),
                 PrivateMemorySize.getID(),
                 VectorizationDimension.getID(),
                 CanUniteWorkgroups.getID(),
                 VectorizedKernel.getID(),
                 VectorizedMaskedKernel.getID(),
                 KernelWrapper.getID(),
                 ScalarKernel.getID(),
                 UseFPGAPipes.getID(),
                 HasMatrixCall.getID(),
                 SubGroupConstructionMode.getID(),
                 UseAddrSpaceQualifierFunc.getID(),
                 ArgTypeNullValList.getID()};
    }
    return MDNames;
  }

private:
  mutable llvm::SmallVector<llvm::StringRef, 0> MDNames;
};

// required attributes
struct ModuleMetadataAPI {
  typedef NamedMDList<int32_t, MDValueModuleStrategy> SpirVersionListTy;
  typedef NamedMDList<int32_t, MDValueModuleStrategy> OpenCLVersionListTy;
  typedef NamedMDList<int32_t, MDValueModuleStrategy> SPIRVSourceListTy;
  typedef NamedMDList<llvm::StringRef, MDValueModuleStrategy>
      OptionalCoreFeaturesListTy;

  ModuleMetadataAPI(llvm::Module *pModule)
      : SpirVersionList(pModule, "opencl.spir.version"),
        SPIRVSourceList(pModule, "spirv.Source"),
        OpenCLVersionList(pModule, "opencl.ocl.version") {
    MDNames.push_back(SpirVersionList.getID());
    MDNames.push_back(SPIRVSourceList.getID());
    MDNames.push_back(OpenCLVersionList.getID());
  }

  SpirVersionListTy SpirVersionList;
  SPIRVSourceListTy SPIRVSourceList;
  OpenCLVersionListTy OpenCLVersionList;

public:
  const llvm::SmallVectorImpl<llvm::StringRef> &getMDNames() const {
    return MDNames;
  }

private:
  llvm::SmallVector<llvm::StringRef, 8> MDNames;
};

// kernels
struct KernelList : public NamedMDList<llvm::Function, MDValueModuleStrategy> {
  typedef NamedMDList::item_type KernelTy;
  typedef NamedMDList::vector_type KernelVectorTy;

  KernelList(llvm::Module *pModule) : NamedMDList(pModule, "sycl.kernels") {}

  KernelList(llvm::Module &Module) : NamedMDList(&Module, "sycl.kernels") {}
};

} // namespace SYCLKernelMetadataAPI
} // namespace llvm

#endif
