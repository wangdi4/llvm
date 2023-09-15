//===- TypeAlignment.cpp - Type alignment utilities------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/Utils/TypeAlignment.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

using namespace llvm;

size_t TypeAlignment::getSize(const KernelArgument &Arg) {
  switch (Arg.Ty) {
  case KRNL_ARG_INT:
    return Arg.SizeInBytes;

  case KRNL_ARG_UINT:
    return Arg.SizeInBytes;

  case KRNL_ARG_FLOAT:
    return Arg.SizeInBytes;

  case KRNL_ARG_DOUBLE:
    return Arg.SizeInBytes;

  case KRNL_ARG_VECTOR:
  case KRNL_ARG_VECTOR_BY_REF: {
    // Extract the vector element size and the number of vector elements
    unsigned int elemSize = Arg.SizeInBytes >> 16;
    unsigned int numElements = (Arg.SizeInBytes) & 0xFFFF;
    return elemSize * numElements;
  }

  case KRNL_ARG_SAMPLER:
    return sizeof(int);

  case KRNL_ARG_COMPOSITE:
    return Arg.SizeInBytes;

  case KRNL_ARG_PTR_LOCAL:
  case KRNL_ARG_PTR_GLOBAL:
  case KRNL_ARG_PTR_CONST:
  case KRNL_ARG_PTR_IMG_1D:
  case KRNL_ARG_PTR_IMG_1D_ARR:
  case KRNL_ARG_PTR_IMG_1D_BUF:
  case KRNL_ARG_PTR_IMG_2D:
  case KRNL_ARG_PTR_IMG_2D_DEPTH:
  case KRNL_ARG_PTR_IMG_3D:
  case KRNL_ARG_PTR_IMG_2D_ARR:
  case KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
  case KRNL_ARG_PTR_BLOCK_LITERAL:
  case KRNL_ARG_PTR_QUEUE_T:
  case KRNL_ARG_PTR_SAMPLER_T:
  case KRNL_ARG_PTR_PIPE_T:
  case KRNL_ARG_PTR_CLK_EVENT_T:
    return Arg.SizeInBytes;
  }

  llvm_unreachable("Unknown KernelArgument type");
}

size_t TypeAlignment::getAlignment(const KernelArgument &Arg) {
  size_t Alignment;
  switch (Arg.Ty) {
  case KRNL_ARG_VECTOR:
  case KRNL_ARG_VECTOR_BY_REF: {
    size_t vectorAlignment = getSize(Arg);

    unsigned int numElements = (Arg.SizeInBytes) & 0xFFFF;

    // Vectors of 3 elements need to be aligned to a 4-elements vector.
    if (numElements == 3) {
      // Align num elems to 4 elements by adding elemSize.
      unsigned int elemSize = Arg.SizeInBytes >> 16;
      vectorAlignment += elemSize;
    }

    Alignment = vectorAlignment;
    // Adding assert to check we are following the OpenCL spec:
    // A built-in data type that is not a power of two bytes in size must be
    // aligned to the next larger power of two.
    assert((0 == (Alignment & (Alignment - 1))) &&
           "Alignment is not power of 2!");
  } break;
  case KRNL_ARG_PTR_BLOCK_LITERAL:
  case KRNL_ARG_COMPOSITE:
    // No alignment for structures.
    Alignment = 0;
    break;
  default:
    Alignment = getSize(Arg);
    break;
  }

  return Alignment;
}

size_t TypeAlignment::align(size_t Alignment, size_t Offset) {

  // Check if there is a need to align
  if (Alignment == 0) {
    return Offset;
  }

  return (Offset + Alignment - 1) & ~(Alignment - 1);
}

char *TypeAlignment::align(size_t Alignment, const char *Ptr) {
  return reinterpret_cast<char *>(
      align(Alignment, reinterpret_cast<size_t>(Ptr)));
}
