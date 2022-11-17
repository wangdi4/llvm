//===- ImplicitArgsUtils.cpp - Implicit argument utilities ----------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelArgType.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/TypeAlignment.h"
#include <algorithm>

using namespace llvm;

StringRef NDInfo::getRecordName(unsigned RecordID) {
  static StringRef Names[NDInfo::LAST] = {"WorkDim",
                                          "GlobalOffset_",
                                          "GlobalSize_",
                                          "LocalSize_",
                                          "NumGroups_",
                                          "RuntimeInterface",
                                          "Block2KernelMapper",
                                          "InternalGlobalSize_",
                                          "InternalLocalSize_",
                                          "InternalNumGroups_"};
  assert(RecordID < NDInfo::LAST && "index is out of range");
  return Names[RecordID];
}

unsigned NDInfo::internalCall2NDInfo(unsigned InternalCall,
                                     bool IsUserWIFunction) {
  assert(InternalCall < ICT_NUMBER);
  switch (InternalCall) {
  case ICT_GET_GLOBAL_OFFSET:
    return NDInfo::GLOBAL_OFFSET;
  case ICT_GET_GLOBAL_SIZE:
    return IsUserWIFunction ? NDInfo::GLOBAL_SIZE
                            : NDInfo::INTERNAL_GLOBAL_SIZE;
  case ICT_GET_LOCAL_SIZE:
  case ICT_GET_ENQUEUED_LOCAL_SIZE:
    return IsUserWIFunction ? NDInfo::LOCAL_SIZE : NDInfo::INTERNAL_LOCAL_SIZE;
  case ICT_GET_NUM_GROUPS:
    return IsUserWIFunction ? NDInfo::WG_NUMBER : NDInfo::INTERNAL_WG_NUMBER;
  default:
    llvm_unreachable("no NDInfo for the internal call");
  }
  return NDInfo::LAST;
}

FunctionArgument::FunctionArgument(const char *Val, size_t Size,
                                   size_t Alignment) {
  this->Size = Size;
  this->Alignment = Alignment;
  // Get aligned destination.
  this->Val = TypeAlignment::align(Alignment, Val);

  AlignedSize = (this->Val - Val); // Get actual alignment.
  AlignedSize += Size;             // Add size.
}

void FunctionArgument::setValue(const char *Val) {

  // TODO : assert pointers not null?
  // Copy value from given src to dest
  std::copy(Val, Val + Size, this->Val);
}

struct ArgData {
  const char *Name;
  bool InitByWrapper;
};

static const ArgData ImpArgs[ImplicitArgsUtils::IA_NUMBER] = {
    {"pLocalMemBase", true},  // IA_SLM_BUFFER,
    {"pWorkDim", false},      // IA_WORK_GROUP_INFO
    {"pWGId", true},          // IA_WORK_GROUP_ID
    {"BaseGlbId", true},      // IA_GLOBAL_BASE_ID
    {"pSpecialBuf", true},    // IA_BARRIER_BUFFER
    {"RuntimeHandle", true}}; // IA_RUNTIME_HANDLE

const char *ImplicitArgsUtils::getArgName(unsigned Idx) {
  // TODO: maybe we don't need impargs?
  assert(Idx < NUM_IMPLICIT_ARGS);
  return ImpArgs[Idx].Name;
}

// Initialize the implicit arguments properties.
ImplicitArgProperties ImplicitArgsUtils::ImplicitArgProps[NUM_IMPLICIT_ARGS];
bool ImplicitArgsUtils::Initialized = false;

const ImplicitArgProperties &
ImplicitArgsUtils::getImplicitArgProps(unsigned int Arg) {
  assert(Arg < NUM_IMPLICIT_ARGS && "Arg is bigger than implicit Args number");
  assert(!ImplicitArgProps[Arg].InitializedByWrapper &&
         "Arg is initialized by wrapper no need for Props!");
  assert(Initialized);
  return ImplicitArgProps[Arg];
}

void ImplicitArgsUtils::initImplicitArgProps(unsigned int SizeOfPtr) {
  for (unsigned int i = 0; i < NUM_IMPLICIT_ARGS; ++i) {
    switch (i) {
    case IA_WORK_GROUP_INFO:
      ImplicitArgProps[i].Size = sizeof(UniformKernelArgs);
      break;
    default:
      ImplicitArgProps[i].Size = SizeOfPtr;
      break;
    }
    ImplicitArgProps[i].Name = ImpArgs[i].Name;
    ImplicitArgProps[i].Alignment = SizeOfPtr;
    ImplicitArgProps[i].InitializedByWrapper = ImpArgs[i].InitByWrapper;
  }
  Initialized = true;
}

void ImplicitArgsUtils::createImplicitArgs(char *Dest) {
  // Start from the beginning of the given dest buffer.
  char *ArgValueDest = Dest;

  // Go over all implicit arguments' properties.
  for (unsigned int i = 0; i < NUM_IMPLICIT_ARGS; ++i) {
    // Only implicit arguments that are not initialized by the wrapper hould be
    // loaded from the parameter structutre.
    if (!ImplicitArgProps[i].InitializedByWrapper) {
      // Create implicit argument pointing at the right place in the dest
      // buffer.
      ImplicitArgument Arg(ArgValueDest, ImplicitArgProps[i]);
      ImplicitArgs[i] = Arg;
      // Advance the dest buffer according to argument's size and alignment.
      ArgValueDest += Arg.getAlignedSize();
    }
  }
}

size_t ImplicitArgsUtils::getAdjustedAlignment(size_t Offset, size_t ST) {
  // Implicit args will be aligned to size_t to allow KNC VBROADCAST's on size_t
  // values.
  return ((Offset + ST - 1) / ST) * ST;
}
