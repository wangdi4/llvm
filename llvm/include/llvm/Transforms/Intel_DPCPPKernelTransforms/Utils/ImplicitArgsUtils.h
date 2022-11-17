//===- ImplicitArgsUtils.h - Implicit argument utilities --------*- C++ -*-===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_IMPLICIT_ARGS_UTILS_H
#define INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_IMPLICIT_ARGS_UTILS_H

#include "llvm/ADT/StringRef.h"

namespace llvm {

class CallbackContext;
class ExtendedExecutionContext;

namespace NDInfo {
// Keep these values in same order as structure so they can be used as indices
// for GEP accesses.
enum _NDInfo {
  WORK_DIM,
  GLOBAL_OFFSET,
  GLOBAL_SIZE,
  LOCAL_SIZE,
  WG_NUMBER,
  RUNTIME_INTERFACE,
  BLOCK2KERNEL_MAPPER,
  INTERNAL_GLOBAL_SIZE,
  INTERNAL_LOCAL_SIZE,
  INTERNAL_WG_NUMBER,
  LAST
};

StringRef getRecordName(unsigned RecordID);

unsigned internalCall2NDInfo(unsigned InternalCall,
                             bool IsUserWIFunction = false);

} // namespace NDInfo

enum TInternalCallType : int {
  ICT_NONE,
  ICT_GET_BASE_GLOBAL_ID,
  ICT_GET_SPECIAL_BUFFER,
  ICT_GET_WORK_DIM,
  ICT_GET_GLOBAL_SIZE,
  ICT_GET_LOCAL_SIZE,
  ICT_GET_ENQUEUED_LOCAL_SIZE,
  ICT_GET_NUM_GROUPS,
  ICT_GET_GROUP_ID,
  ICT_GET_GLOBAL_OFFSET,
  // special functions that need update
  ICT_PRINTF,
  ICT_PREFETCH,
  // int enqueue_kernel_varargs()
  ICT_ENQUEUE_KERNEL_LOCALMEM,
  // int enqueue_kernel_events_varargs()
  ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM,
  ICT_NUMBER
};

class FunctionArgument {

public:
  /// Constructor.
  /// \param Val implict argument's value destination pointer.
  /// \param Size implict argument's size.
  /// \param Alignment implict argument's alignment.
  FunctionArgument(const char *Val, size_t Size, size_t Alignment);

  /// Returns the size of this argument
  virtual size_t getSize() const { return Size; }

  /// Returns the alignment of this argument.
  virtual size_t getAlignment() const { return Alignment; }

  /// Interface implementation.
  ///  Returns the size with alignments needed to be done
  ///         to destination pointer of this argument.
  virtual size_t getAlignedSize() const { return AlignedSize; }

  /// Sets the value of this argument.
  /// \param Val the src from which to copy the value.
  virtual void setValue(const char *Val);

  /// Gets the value of this argument.
  virtual void *getValue() { return *((void **)Val); }

  virtual ~FunctionArgument() {}

protected:
  /// Implict argument's value destination pointer.
  char *Val;

  /// Implict argument's size.
  size_t Size;

  /// Implict argument's alignment.
  size_t Alignment;

  /// Implict argument's size + destination pointer alignment.
  size_t AlignedSize;
};

///  ImplicitArgProperties struct used to describe each implicit argument.
struct ImplicitArgProperties {
  /// Implicit argument's name.
  const char *Name;

  /// Implicit argument's size.
  size_t Size;

  /// Implicit argument's alignment.
  size_t Alignment;

  /// Indicates if implicit argument is initialized by the wrapper.
  bool InitializedByWrapper;
};

class ImplicitArgument : public FunctionArgument {

public:
  /// Constructor
  /// \param Val Implict argument's value destination pointer.
  /// \param ImplicitArgProps Implicit argument properties.
  ImplicitArgument(char *Val, const ImplicitArgProperties &ImplicitArgProps)
      : FunctionArgument(Val, ImplicitArgProps.Size,
                         ImplicitArgProps.Alignment) {}

  ImplicitArgument() : FunctionArgument(nullptr, 0, 0) {}
};

/// ImplicitArgsUtils class used to provide helper utilies for handling
/// implicit arguments.
class ImplicitArgsUtils {

public:
  enum IMPLICIT_ARGS {
    IA_SLM_BUFFER,
    IA_WORK_GROUP_INFO,
    IA_WORK_GROUP_ID,
    IA_GLOBAL_BASE_ID,
    IA_BARRIER_BUFFER,
    IA_RUNTIME_HANDLE,
    IA_NUMBER
  };
  static const unsigned int NUM_IMPLICIT_ARGS = IA_NUMBER;

  ImplicitArgsUtils() {}

  ~ImplicitArgsUtils() {}

  /// Return the implicit argument properties of given argument index.
  /// \param Arg the implicit argument index.
  /// @returns the implicit argument properties.
  static const ImplicitArgProperties &getImplicitArgProps(unsigned int Arg);

  /// Initialize properties on implicit arguments in run time.
  /// \param SizeOfPtr size of pointer, depends on target machine.
  static void initImplicitArgProps(unsigned int SizeOfPtr);

  /// Indicate that the properties were initialized.
  static bool Initialized;

  /// Create implicit arguments based on the implicit arguments properties.
  /// \param Dest a buffer that should hold the values of the implicit
  /// arguments.
  void createImplicitArgs(char *Dest);

  static const char *getArgName(unsigned Idx);

  /// Returns a value which is >= 'offset' for the offset of the implicit args
  /// from the start of the kernel uniform args. Used to ensure the implicit
  /// args
  /// is located at a  correctly aligned address.
  /// \param Offset the offset that possibly needs to be adjusted.
  /// \param ST sizeof(size_t).
  static size_t getAdjustedAlignment(size_t Offset, size_t ST);

private:
  /// static list of implicit argument properties.
  static ImplicitArgProperties ImplicitArgProps[NUM_IMPLICIT_ARGS];

  /// list of implicit arguments.
  ImplicitArgument ImplicitArgs[NUM_IMPLICIT_ARGS];
};

} // namespace llvm

#endif // INTEL_DPCPP_KERNEL_TRANSFORMS_UTILS_IMPLICIT_ARGS_UTILS_H
