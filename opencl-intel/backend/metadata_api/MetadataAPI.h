//==--- MetadataAPI.h - API for accessing LLVM Metadata -- C++ -------------=
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===

#ifndef METADATAAPI_H
#define METADATAAPI_H

#include "MetadataAPIImpl.h"

namespace Intel {
namespace MetadataAPI {

// TBD for FPGA specific Metadata
// look for channels.cl in CodeGenOpenCL tests in clang.
struct GlobalVariableMetadataAPI {
  GlobalVariableMetadataAPI(llvm::GlobalVariable *Global) {}
};

struct FunctionMetadataAPI {
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> FuncPtrCallTy;
  typedef NamedMDValue<bool, MDValueGlobalObjectStrategy> RecursiveCallTy;

  FunctionMetadataAPI(llvm::Function *Func)
      : FuncPtrCall(Func, "func_ptr_call"),
        RecursiveCall(Func, "recursive_call") {}

  NamedMDValueAccessor<FuncPtrCallTy> FuncPtrCall;
  NamedMDValueAccessor<RecursiveCallTy> RecursiveCall;
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

  // optional attributes
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> WorkGroupSizeHintTy;
  typedef NamedMDList<int32_t, MDValueGlobalObjectStrategy> ReqdWorkGroupSizeTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> ReqdNumSubGroupsTy;
  typedef NamedHeteroTupleMDList<llvm::Type, int32_t> VecTypeHintTy;
  typedef NamedMDValue<int32_t, MDValueGlobalObjectStrategy> VecLenHintTy;

  KernelMetadataAPI(llvm::Function *Func)
      : ArgAddressSpaceList(Func, "kernel_arg_addr_space"),
        ArgAccessQualifierList(Func, "kernel_arg_access_qual"),
        ArgTypeList(Func, "kernel_arg_type"),
        ArgBaseTypeList(Func, "kernel_arg_base_type"),
        ArgTypeQualifierList(Func, "kernel_arg_type_qual"),
        ArgNameList(Func, "kernel_arg_name"),

        WorkGroupSizeHint(Func, "work_group_size_hint"),
        ReqdWorkGroupSize(Func, "reqd_work_group_size"),
        ReqdNumSubGroups(Func, "required_num_sub_groups"),
        VecTypeHint(Func, "vec_type_hint"),
        VecLenHint(Func, "intel_vec_len_hint") {}

  // required attributes
  ArgAddressSpaceListTy ArgAddressSpaceList;
  ArgAccessQualifierListTy ArgAccessQualifierList;
  ArgTypeListTy ArgTypeList;
  ArgBaseTypeListTy ArgBaseTypeList;
  ArgTypeQualifierListTy ArgTypeQualifierList;
  ArgNameListTy ArgNameList;

  // optional attributes
  WorkgroupSizeMDAccessor<WorkGroupSizeHintTy> WorkGroupSizeHint;
  WorkgroupSizeMDAccessor<ReqdWorkGroupSizeTy> ReqdWorkGroupSize;
  NamedMDValueAccessor<ReqdNumSubGroupsTy> ReqdNumSubGroups;
  VecTypeHintTupleMDListAccessor<VecTypeHintTy> VecTypeHint;
  NamedMDValueAccessor<VecLenHintTy> VecLenHint;
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

  KernelInternalMetadataAPI(llvm::Function *Func)
      : LocalBufferSize(Func, "local_buffer_size"),
        BarrierBufferSize(Func, "barrier_buffer_size"),
        KernelExecutionLength(Func, "kernel_execution_length"),
        MaxWGDimensions(Func, "max_wg_dimensions"),
        KernelHasBarrier(Func, "kernel_has_barrier"),
        KernelHasGlobalSync(Func, "kernel_has_global_sync"),
        NoBarrierPath(Func, "no_barrier_path"),
        VectorizedWidth(Func, "vectorized_width"),
        BlockLiteralSize(Func, "block_literal_size"),
        PrivateMemorySize(Func, "private_memory_size"),
        VectorizationDimension(Func, "vectorization_dimension"),
        CanUniteWorkgroups(Func, "can_unite_workgroups"),
        VectorizedKernel(Func, "vectorized_kernel"),
        KernelWrapper(Func, "kernel_wrapper"),
        ScalarizedKernel(Func, "scalarized_kernel") {}

  // internal attributes
  NamedMDValueAccessor<LocalBufferSizeTy> LocalBufferSize;
  NamedMDValueAccessor<BarrierBufferSizeTy> BarrierBufferSize;
  NamedMDValueAccessor<KernelExecutionLengthTy> KernelExecutionLength;
  NamedMDValueAccessor<MaxWGDimensionsTy> MaxWGDimensions;
  NamedMDValueAccessor<KernelHasBarrierTy> KernelHasBarrier;
  NamedMDValueAccessor<KernelHasGlobalSyncTy> KernelHasGlobalSync;
  NamedMDValueAccessor<NoBarrierPathTy> NoBarrierPath;
  NamedMDValueAccessor<VectorizedWidthTy> VectorizedWidth;
  NamedMDValueAccessor<BlockLiteralSizeTy> BlockLiteralSize;
  NamedMDValueAccessor<PrivateMemorySizeTy> PrivateMemorySize;
  NamedMDValueAccessor<VectorizationDimensionTy> VectorizationDimension;
  NamedMDValueAccessor<CanUniteWorkgroupsTy> CanUniteWorkgroups;
  NamedMDValueAccessor<VectorizedKernelTy> VectorizedKernel;
  NamedMDValueAccessor<KernelWrapperTy> KernelWrapper;
  NamedMDValueAccessor<ScalarizedKernelTy> ScalarizedKernel;
};

// required attributes
struct ModuleMetadataAPI {
  typedef NamedMDList<int32_t, MDValueModuleStrategy> SpirVersionListTy;
  typedef NamedMDList<int32_t, MDValueModuleStrategy> OpenCLVersionListTy;
  typedef NamedMDList<int32_t, MDValueModuleStrategy> UsedExtentionsListTy;
  typedef NamedMDList<llvm::StringRef, MDValueModuleStrategy>
      OptionalCoreFeaturesListTy;
  typedef NamedMDList<llvm::StringRef, MDValueModuleStrategy> CompilerOptionsListTy;

  ModuleMetadataAPI(llvm::Module *pModule) :
        SpirVersionList(pModule, "opencl.spir.version"),
        OpenCLVersionList(pModule, "opencl.ocl.version"),
        UsedExtentionsList(pModule, "opencl.used.extensions"),
        OptionalCoreFeaturesList(pModule, "opencl.used.optional.core.features"),
        CompilerOptionsList(pModule, "opencl.compiler.options") {}

  SpirVersionListTy SpirVersionList;
  OpenCLVersionListTy OpenCLVersionList;
  UsedExtentionsListTy UsedExtentionsList;
  OptionalCoreFeaturesListTy OptionalCoreFeaturesList;
  CompilerOptionsListTy CompilerOptionsList;
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
        GASWarningsList(pModule, "opencl.gas_warning_line_numbers") {}

  NamedMDValueAccessor<GlobalVariableTotalSizeTy> GlobalVariableTotalSize;
  NamedMDValueAccessor<GASCounterTy> GASCounter;
  GASWarningsListTy GASWarningsList;
};

} // end namespace MetadataAPI
} // end namespace Intel

#endif
