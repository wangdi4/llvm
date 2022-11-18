//===- KernelArgType.h - DPC++ kernel argument types ----------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_KERNEL_ARG_TYPE_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_KERNEL_ARG_TYPE_H

#include "DevLimits.h"
#include <cstddef>

#ifdef _WIN32
#pragma pack(push, 1)
#define PACKED
#else
#define PACKED __attribute__((packed))
#endif

namespace llvm {
/*! \enum KernelArgumentType
 * Defines possible values for kernel argument types
 */
enum KernelArgumentType {
  KRNL_ARG_INT = 0,       //!< Argument is a signed integer.
  KRNL_ARG_UINT,          //!< Argument is an unsigned integer.
  KRNL_ARG_FLOAT,         //!< Argument is a float.
  KRNL_ARG_DOUBLE,        //!< Argument is a double.
  KRNL_ARG_VECTOR,        //!< Argument is a vector of basic types, like int8,
                          //!< float4, etc.
  KRNL_ARG_VECTOR_BY_REF, //!< Argument is a byval pointer to a vector of
                          //!< basic types, like int8, float4, etc.
  KRNL_ARG_SAMPLER,       //!< Argument is a sampler object
  KRNL_ARG_COMPOSITE,     //!< Argument is a user defined struct
  KRNL_ARG_PTR_LOCAL,     //!< Argument is a pointer to array declared in local
                          //!< memory Memory object types bellow this line
  KRNL_ARG_PTR_GLOBAL,    //!< Argument is a pointer to array in global memory
                          //!< of various types The array type could be char,
                          //!< short, int, float or double User must pass a
                          //!< handle to a memory buffer for this argument type
  KRNL_ARG_PTR_CONST,     //!< Argument is a pointer to buffer declared in
                          //!< constant(global) memory
  KRNL_ARG_PTR_IMG_2D,    //!< Argument is a pointer to 2D image
  KRNL_ARG_PTR_IMG_2D_DEPTH,     //!< Argument is a pointer to 2D image depth
  KRNL_ARG_PTR_IMG_3D,           //!< Argument is a pointer to 3D image
  KRNL_ARG_PTR_IMG_2D_ARR,       //!< Argument is a pointer to a 2D image array
  KRNL_ARG_PTR_IMG_2D_ARR_DEPTH, //!< Argument is a pointer to a 2D image
                                 //!< depth array
  KRNL_ARG_PTR_IMG_1D,           //!< Argument is a pointer to 1D image
  KRNL_ARG_PTR_IMG_1D_ARR,       //!< Argument is a pointer to 1D image array
  KRNL_ARG_PTR_IMG_1D_BUF,       //!< Argument is a pointer to 1D image buffer
  KRNL_ARG_PTR_BLOCK_LITERAL,    //!< Argument is a pointer to Block Literal
                                 //!< structure
  KRNL_ARG_PTR_QUEUE_T,     //!< Argument is a pointer to device execution queue
  KRNL_ARG_PTR_SAMPLER_T,   //!< Argument is a pointer to a sampler object
  KRNL_ARG_PTR_PIPE_T,      //!< Argument is a pointer to a pipe
  KRNL_ARG_PTR_CLK_EVENT_T, //!< Argument is a pointer to a device side event
};

/*! \struct KernelArgument
 *  \brief Defines possible values for kernel argument types.
 */
struct KernelArgument {
  KernelArgumentType Ty;      //!< Type of the argument.
  unsigned int SizeInBytes;   //!< Size of the argument in bytes
  unsigned int OffsetInBytes; //!< Offset of the argument in argument buffer
};

/**
 * This struct hold all the uniform arguments which will be passed to runtime
 * to start execution of kernel.
 */
struct PACKED UniformKernelArgs {
  // ND Range Work Description
  // Kernel explicit arguments in same order as in  kernel declaration
  // Alignment of type must be same as sizeof returns on the type
  // gentype arg1;
  // gentype arg2;
  // .  .  .
  // gentype argN;
  // Kernel implicit arguments continue here
  // Start of WG size info
  size_t WorkDim;                    // Filled by the runtime
  size_t GlobalOffset[MAX_WORK_DIM]; // Filled by the runtime
  size_t GlobalSize[MAX_WORK_DIM];   // Filled by the runtime
  size_t LocalSize[WG_SIZE_NUM]
                  [MAX_WORK_DIM]; // Filled by the runtime, updated by the BE in
                                  // case of (0,0,0) LocalSize[0] contains
                                  // unifrom local sizes LocalSize[1] contains
                                  // non-unifrom local sizes
  size_t WGCount[MAX_WORK_DIM];   // Updated by the BE, based on GLOBAL/LOCAL
  // For Opencl2.0: this is a IDeviceCommandManager: the printf interface thing
  void *RuntimeInterface; // Updated by runtime
  /// reference to BlockToKernelMapper object. Class does not own it
  void *Block2KernelMapper; // Updated by the BE
  // The following three members represent the actual enqueued work sizes chosen
  // by the runtime -- they might differ from GlobalSize, LocalSize and WGCount
  // above when users set different subgroup construction modes.
  size_t InternalGlobalSize[MAX_WORK_DIM];
  size_t InternalLocalSize[WG_SIZE_NUM][MAX_WORK_DIM];
  size_t InternalWGCount[MAX_WORK_DIM];
  // End of WG size info
  size_t MinWorkGroupNum;   // Filled by the runtime, Required by the heuristic
  // Internal for Running the kernel
  const void *UniformJITEntryPoint;    // Filled by the BE
  const void *NonUniformJITEntryPoint; // Filled by the BE
};

#ifdef _WIN32
#pragma pack(pop)
#endif

// Sampler type as defined in sycl kernels.
typedef unsigned int _sampler_t;

} // namespace llvm

#endif
