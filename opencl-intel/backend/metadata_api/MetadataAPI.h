// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

// ocl_recommended_vector_length metadata is used by VPO passes e.g.(VecClone) only.

#ifndef METADATAAPI_H
#define METADATAAPI_H

#include "MetadataAPIImpl.h"

namespace Intel {
namespace MetadataAPI {

struct GlobalVariableMetadataAPI {
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> DepthIsIgnoredTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipePacketSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipePacketAlignTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> PipeDepthTy;
  typedef NamedMDValue<llvm::StringRef, MDValueGlobalObjectStrategy> PipeIOTy;

  GlobalVariableMetadataAPI(llvm::GlobalVariable *Global) :
        DepthIsIgnored(Global, "depth_is_ignored"),
        PipePacketSize(Global, "packet_size"),
        PipePacketAlign(Global, "packet_align"),
        PipeDepth(Global, "depth"),
        PipeIO(Global, "io") {}

  NamedMDValueAccessor<DepthIsIgnoredTy> DepthIsIgnored;
  NamedMDValueAccessor<PipePacketSizeTy> PipePacketSize;
  NamedMDValueAccessor<PipePacketAlignTy> PipePacketAlign;
  NamedMDValueAccessor<PipeDepthTy> PipeDepth;
  NamedMDValueAccessor<PipeIOTy> PipeIO;
};

struct FunctionMetadataAPI {
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> BoolTy;

  FunctionMetadataAPI(llvm::Function *Func)
      : RecursiveCall(Func, "recursive_call"),
        FpgaPipeDynamicAccess(Func, "fpga_pipe_dynamic_access") {}

  NamedMDValueAccessor<BoolTy> RecursiveCall;
  NamedMDValueAccessor<BoolTy> FpgaPipeDynamicAccess;
};

struct KernelMetadataAPI {

  // required attributes
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy>
      ArgAddressSpaceListTy;
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy>
      ArgAccessQualifierListTy;
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy> ArgTypeListTy;
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy>
      ArgBaseTypeListTy;
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy>
      ArgTypeQualifierListTy;
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy> ArgNameListTy;
  typedef NamedMDList<llvm::StringRef, MDValueGlobalObjectStrategy>
      ArgIOAttributeListTy;

  // optional attributes
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> WorkGroupSizeHintTy;
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> ReqdWorkGroupSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> ReqdNumSubGroupsTy;
  typedef NamedHeteroTupleMDList<llvm::Type, int32_t> VecTypeHintTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> VecLenHintTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> MaxGlobalWorkDimTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> CanUseGlobalWorkOffsetTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> AutorunTy;
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> NumComputeUnitsTy;

  KernelMetadataAPI(llvm::Function *Func)
      : ArgAddressSpaceList(Func, "kernel_arg_addr_space"),
        ArgAccessQualifierList(Func, "kernel_arg_access_qual"),
        ArgTypeList(Func, "kernel_arg_type"),
        ArgBaseTypeList(Func, "kernel_arg_base_type"),
        ArgTypeQualifierList(Func, "kernel_arg_type_qual"),
        ArgNameList(Func, "kernel_arg_name"),
        ArgIOAttributeList(Func, "kernel_arg_pipe_io"),

        WorkGroupSizeHint(Func, "work_group_size_hint"),
        ReqdWorkGroupSize(Func, "reqd_work_group_size"),
        ReqdNumSubGroups(Func, "required_num_sub_groups"),
        VecTypeHint(Func, "vec_type_hint"),
        VecLenHint(Func, "intel_vec_len_hint"),
        ReqdIntelSGSize(Func, "intel_reqd_sub_group_size"),
        MaxGlobalWorkDim(Func, "max_global_work_dim"),
        // Attribute tells if kernel can be enqueued
        // with GlobalWorkOffset parameter, hence the naming
        CanUseGlobalWorkOffset(Func, "uses_global_work_offset"),
        //
        Autorun(Func, "autorun"),
        NumComputeUnits(Func, "num_compute_units")
     {
       MDNames.push_back(ArgAddressSpaceList.getID());
       MDNames.push_back(ArgAccessQualifierList.getID());
       MDNames.push_back(ArgTypeList.getID());
       MDNames.push_back(ArgBaseTypeList.getID());
       MDNames.push_back(ArgTypeQualifierList.getID());
       MDNames.push_back(ArgNameList.getID());
       MDNames.push_back(ArgIOAttributeList.getID());

       MDNames.push_back(WorkGroupSizeHint.getID());
       MDNames.push_back(ReqdWorkGroupSize.getID());
       MDNames.push_back(ReqdNumSubGroups.getID());
       MDNames.push_back(VecTypeHint.getID());
       MDNames.push_back(VecLenHint.getID());
       MDNames.push_back(ReqdIntelSGSize.getID());
       MDNames.push_back(MaxGlobalWorkDim.getID());
       MDNames.push_back(CanUseGlobalWorkOffset.getID());
       MDNames.push_back(Autorun.getID());
       MDNames.push_back(NumComputeUnits.getID());
     }

  // required attributes
  ArgAddressSpaceListTy ArgAddressSpaceList;
  ArgAccessQualifierListTy ArgAccessQualifierList;
  ArgTypeListTy ArgTypeList;
  ArgBaseTypeListTy ArgBaseTypeList;
  ArgTypeQualifierListTy ArgTypeQualifierList;
  ArgNameListTy ArgNameList;
  ArgIOAttributeListTy ArgIOAttributeList;

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
  const llvm::SmallVectorImpl<llvm::StringRef>& getMDNames() const
    { return MDNames; }
  bool hasVecLength()
    { return VecLenHint.hasValue() || ReqdIntelSGSize.hasValue(); }
  int getVecLength()
    {
      if (VecLenHint.hasValue()) return VecLenHint.get();
      else return ReqdIntelSGSize.get();
    }
  void setReqdIntelSGSize(int Size)
    {
      ReqdIntelSGSize.set(Size);
    }
private:
  llvm::SmallVector<llvm::StringRef, 16> MDNames;
};

// internal attributes
struct KernelInternalMetadataAPI {
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> LocalBufferSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      BarrierBufferSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy>
      KernelExecutionLengthTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> MaxWGDimensionsTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> KernelHasBarrierTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> KernelHasGlobalSyncTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> KernelHasSubgroupsTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> NoBarrierPathTy;
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

  KernelInternalMetadataAPI(llvm::Function *Func)
      : LocalBufferSize(Func, "local_buffer_size"),
        BarrierBufferSize(Func, "barrier_buffer_size"),
        KernelExecutionLength(Func, "kernel_execution_length"),
        MaxWGDimensions(Func, "max_wg_dimensions"),
        KernelHasBarrier(Func, "kernel_has_barrier"),
        KernelHasGlobalSync(Func, "kernel_has_global_sync"),
        KernelHasSubgroups(Func, "kernel_has_sub_groups"),
        NoBarrierPath(Func, "no_barrier_path"),
        VectorizedWidth(Func, "vectorized_width"),
        OclRecommendedVectorLength(Func, "ocl_recommended_vector_length"),
        BlockLiteralSize(Func, "block_literal_size"),
        PrivateMemorySize(Func, "private_memory_size"),
        VectorizationDimension(Func, "vectorization_dimension"),
        CanUniteWorkgroups(Func, "can_unite_workgroups"),
        VectorizedKernel(Func, "vectorized_kernel"),
        KernelWrapper(Func, "kernel_wrapper"),
        ScalarizedKernel(Func, "scalarized_kernel"),
        UseFPGAPipes(Func, "use_fpga_pipes")
    {
      MDNames.push_back(LocalBufferSize.getID());
      MDNames.push_back(BarrierBufferSize.getID());
      MDNames.push_back(KernelExecutionLength.getID());
      MDNames.push_back(MaxWGDimensions.getID());
      MDNames.push_back(KernelHasBarrier.getID());
      MDNames.push_back(KernelHasGlobalSync.getID());
      MDNames.push_back(KernelHasSubgroups.getID());
      MDNames.push_back(NoBarrierPath.getID());
      MDNames.push_back(VectorizedWidth.getID());
      MDNames.push_back(OclRecommendedVectorLength.getID());
      MDNames.push_back(BlockLiteralSize.getID());
      MDNames.push_back(PrivateMemorySize.getID());
      MDNames.push_back(VectorizationDimension.getID());
      MDNames.push_back(CanUniteWorkgroups.getID());
      MDNames.push_back(VectorizedKernel.getID());
      MDNames.push_back(KernelWrapper.getID());
      MDNames.push_back(ScalarizedKernel.getID());
      MDNames.push_back(UseFPGAPipes.getID());
    }

  // internal attributes
  NamedMDValueAccessor<LocalBufferSizeTy> LocalBufferSize;
  NamedMDValueAccessor<BarrierBufferSizeTy> BarrierBufferSize;
  NamedMDValueAccessor<KernelExecutionLengthTy> KernelExecutionLength;
  NamedMDValueAccessor<MaxWGDimensionsTy> MaxWGDimensions;
  NamedMDValueAccessor<KernelHasBarrierTy> KernelHasBarrier;
  NamedMDValueAccessor<KernelHasGlobalSyncTy> KernelHasGlobalSync;
  NamedMDValueAccessor<KernelHasSubgroupsTy> KernelHasSubgroups;
  NamedMDValueAccessor<NoBarrierPathTy> NoBarrierPath;
  NamedMDValueAccessor<VectorizedWidthTy> VectorizedWidth;
  NamedMDValueAccessor<VectorizedWidthTy> OclRecommendedVectorLength;
  NamedMDValueAccessor<BlockLiteralSizeTy> BlockLiteralSize;
  NamedMDValueAccessor<PrivateMemorySizeTy> PrivateMemorySize;
  NamedMDValueAccessor<VectorizationDimensionTy> VectorizationDimension;
  NamedMDValueAccessor<CanUniteWorkgroupsTy> CanUniteWorkgroups;
  NamedMDValueAccessor<VectorizedKernelTy> VectorizedKernel;
  NamedMDValueAccessor<KernelWrapperTy> KernelWrapper;
  NamedMDValueAccessor<ScalarizedKernelTy> ScalarizedKernel;
  NamedMDValueAccessor<UseFPGAPipesTy> UseFPGAPipes;

public:
  const llvm::SmallVectorImpl<llvm::StringRef>& getMDNames() const
    { return MDNames; }
private:
  llvm::SmallVector<llvm::StringRef, 16> MDNames;
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

// OpenCL kernels
struct KernelList : public NamedMDList<llvm::Function, MDValueModuleStrategy> {
  typedef NamedMDList::item_type KernelTy;
  typedef NamedMDList::vector_type KernelVectorTy;

  KernelList(llvm::Module *pModule) : NamedMDList(pModule, "opencl.kernels") {}

  KernelList(llvm::Module &Module) : NamedMDList(&Module, "opencl.kernels") {}
};

// internal attributes
struct ModuleInternalMetadataAPI {
  typedef NamedMDValue<int32_t, MDValueModuleStrategy>
      GlobalVariableTotalSizeTy;
  typedef NamedMDValue<int32_t, MDValueModuleStrategy> GASCounterTy;
  typedef NamedMDList<int32_t, MDValueModuleStrategy> GASWarningsListTy;

  ModuleInternalMetadataAPI(llvm::Module *pModule)
      : GlobalVariableTotalSize(pModule, "opencl.global_variable_total_size"),
        GASCounter(pModule, "opencl.gen_addr_space_pointer_counter"),
        GASWarningsList(pModule, "opencl.gas_warning_line_numbers")
    {
      MDNames.push_back(GlobalVariableTotalSize.getID());
      MDNames.push_back(GASCounter.getID());
      MDNames.push_back(GASWarningsList.getID());
    }

  NamedMDValueAccessor<GlobalVariableTotalSizeTy> GlobalVariableTotalSize;
  NamedMDValueAccessor<GASCounterTy> GASCounter;
  GASWarningsListTy GASWarningsList;

public:
  const llvm::SmallVectorImpl<llvm::StringRef>& getMDNames() const
    { return MDNames; }
private:
  llvm::SmallVector<llvm::StringRef, 16> MDNames;
};

} // end namespace MetadataAPI
} // end namespace Intel

#endif
